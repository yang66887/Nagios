#!/usr/bin/python

import os
import json
from pathlib import Path
import time
import re
import tempfile
import gzip
import io
import requests
import datetime

# 错误日志文件路径
ERROR_LOG_PATH = "/data/nagios/var/influxdb_loader_errors.log"

# influx_format 函数
def influx_format(file_content):
    try:
        data = json.loads(file_content)
        Host = data['host']
        Measurement = 'Processes_List'
        PROCESSES_LIST = data['processes']
        Timestamp = data['timestamp']
        
        lines = []
        for i, process in enumerate(PROCESSES_LIST):
            try:
                UserName = process['username']
                Name = process['name']
                PID = process['pid']
                CPU_Percent = process['cpu_percent'][0]
                MEM_Percent = process['mem_percent'][0]
                Exec = process['cmd'].replace('"', "'")
                
                if UserName.isdigit():
                    UserName = 'UID_' + UserName
                if not Exec:
                    Exec = "Unknown"
                
                current_timestamp = Timestamp + i
                
                line = (
                    f"{Measurement},Host={Host} "
                    f'i_UserName="{UserName}",'
                    f'j_ProcessName="{Name}",'
                    f"k_PID={PID},"
                    f"l_CPU_Percent={CPU_Percent},"
                    f"m_MEM_Percent={MEM_Percent},"
                    f'n_Exec="{Exec}" '
                    f"{current_timestamp}"
                )
                lines.append(line)
                
            except Exception:
                continue
                
        return "\n".join(lines)
    
    except Exception:
        return ""

# influx_write 函数（InfluxDB 1.x + GZIP + 错误日志）
def influx_write(temp_file_path):
    # 硬编码配置
    INFLUX_URL = "http://127.0.0.1:8086"
    INFLUX_DB = "nagios_perfdata"
    INFLUX_USER = "nagios"
    INFLUX_PASSWORD = "hgmV4-Z49&8xzUc8AQ"
    
    write_url = f"{INFLUX_URL}/write"
    params = {"db": INFLUX_DB, "precision": "ns"}
    headers = {"Content-Encoding": "gzip", "Content-Type": "text/plain; charset=utf-8"}
    
    # 设置认证
    auth = None
    if INFLUX_USER and INFLUX_PASSWORD:
        auth = (INFLUX_USER, INFLUX_PASSWORD)
    
    try:
        with open(temp_file_path, 'r', encoding='utf-8') as f:
            data = f.read()
        
        if not data.strip():
            return True
        
        # GZIP压缩
        gzip_buffer = io.BytesIO()
        with gzip.GzipFile(fileobj=gzip_buffer, mode='wb') as gz_file:
            gz_file.write(data.encode('utf-8'))
        compressed_data = gzip_buffer.getvalue()
        
        # 发送请求
        response = requests.post(
            write_url,
            params=params,
            headers=headers,
            data=compressed_data,
            auth=auth,
            timeout=30  # 30秒超时
        )
        
        if response.status_code == 204:
            return True
        else:
            # 记录错误到日志文件
            log_error(f"写入失败: HTTP {response.status_code} - {response.text}")
            return False
    
    except Exception as e:
        # 记录异常到日志文件
        log_error(f"写入请求异常: {str(e)}")
        return False

def log_error(message):
    """
    将错误信息写入日志文件
    :param message: 错误消息
    """
    try:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"[{timestamp}] {message}\n"
        
        with open(ERROR_LOG_PATH, "a", encoding="utf-8") as log_file:
            log_file.write(log_entry)
    except Exception as e:
        # 如果日志写入失败，暂时不做处理
        pass

# 主处理函数
def process_files(directory, batch_size=10000):
    nano_pattern = re.compile(r"^\d{19}$")
    
    with tempfile.NamedTemporaryFile(mode='w+', delete=False, suffix='.txt') as temp_file:
        temp_file_path = temp_file.name
    
    line_count = 0
    
    for file_path in Path(directory).rglob('*'):
        if not file_path.is_file():
            continue
            
        if nano_pattern.match(file_path.name):
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                formatted = influx_format(content)
                
                if formatted:
                    with open(temp_file_path, 'a', encoding='utf-8') as temp_file:
                        temp_file.write(formatted + "\n")
                    
                    new_lines = len(formatted.split('\n'))
                    line_count += new_lines
                    
                    if line_count >= batch_size:
                        if influx_write(temp_file_path):
                            open(temp_file_path, 'w').close()
                            line_count = 0
                
                os.unlink(file_path)
                
            except:
                pass
    
    if line_count > 0:
        influx_write(temp_file_path)
    
    try:
        os.unlink(temp_file_path)
    except:
        pass

# 主循环
if __name__ == "__main__":
    DATA_DIR = "/data/nagios/var/cache"
    
    while True:
        process_files(DATA_DIR, batch_size=10000)
        time.sleep(60)  # 休眠1分钟

# Install InfluxDB

### 官网页面
>`下拉找到Are you interested in InfluxDB 1.x Open Source?`
```shell
https://www.influxdata.com/downloads/
```

### 安装依赖包
#### ***For EL7***
```shell
yum install wget
```

#### ***For EL8***
```shell
yum install wget logrotate
```

#### ***For Ubuntu 22***
```shell
apt install wget logrotate
```

### 安装InfluxDB
#### ***For EL7 & EL8***
>`下载`
```shell
wget https://download.influxdata.com/influxdb/releases/influxdb-1.11.7.x86_64.rpm
```
>`安装`
```shell
rpm -ivh influxdb-1.11.7.x86_64.rpm
```

#### ***For Ubuntu 22***
>`下载`
```shell
wget https://download.influxdata.com/influxdb/releases/influxdb-1.11.7-amd64.deb
```
>`安装`
```shell
dpkg -i influxdb-1.11.7-amd64.deb
```
>`删除InfluxDB默认数据存储目录`
```shell
systemctl stop influxdb
rm -rf /var/lib/influxdb/{meta,data,wal}
```

### 配置InfluxDB
>`创建InfluxDB数据存储目录`
```shell
mkdir -p /data/influxdb/{meta,data,wal}
chown -R influxdb:influxdb /data/influxdb
```
>`修改InfluxDB默认数据存储目录`
```shell
sed -ri "s/var\/lib/data/g" /etc/influxdb/influxdb.conf
```
>`启用HTTP接口`
```shell
sed -i 's/^\[http\]/[http]\n  bind-address = ":8086"\n  enabled = true\n  log-enabled = false/' /etc/influxdb/influxdb.conf
```
>`修改日志日志级别为warning`
```shell
sed -i 's/^\[logging\]/[logging]\n  level = "warn"/' /etc/influxdb/influxdb.conf
```
>`启动InfluxDB`
```shell
systemctl enable --now influxdb
```
>`登录InfluxDB`
```shell
influx
```
>`创建InfluxDB管理员账号`
```shell
create user "UserName" with password 'Password@123' with all privileges
```
>`退出InfluxDB`
```shell
exit
```
>`开启InfluxDB登录认证`
```shell
sed -i 's/^\[http\]/[http]\n  auth-enabled = true/' /etc/influxdb/influxdb.conf
systemctl restart influxdb
```
>`使用管理员账号登录InfluxDB`
```shell
influx -precision rfc3339 -username UserName -password Password@123
```
>`创建Nagios监控数据库`
>`数据留存策略：180天`
```shell
create database nagios_perfdata with duration 180d
```
>`退出InfluxDB`
```shell
exit
```

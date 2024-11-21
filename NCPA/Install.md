# Install NCPA

### 官网页面
>`官网安装包下载地址`
```shell
https://www.nagios.org/ncpa/#downloads
```
>`github项目地址`
```shell
https://github.com/NagiosEnterprises/ncpa
```
>`源码编译文档`
```shell
https://support.nagios.com/kb/article/ncpa-how-to-build-ncpa-from-source-521.html
```

## 前期准备
>`生成NCPA认证所需的Token值`
>`记录以下命令生成的字符串`
>`配置NCPA时输入`
>`所有服务器要求使用相同的Token值`
```shell
cat /proc/sys/kernel/random/uuid
```

## For Linux

### 安装依赖包
#### ***For EL7 & EL8***
```shell
yum install wget
```
#### ***For Ubuntu 22***
```shell
apt install wget
```

### 安装
#### ***For EL7 & EL8***
>`下载安装包`
```shell
wget https://github.com/yang66887/Nagios/raw/refs/heads/main/NCPA/ncpa-3.1.1-1.x86_64.rpm
```
>`安装`
```shell
rpm -ivh ncpa-3.1.1-1.x86_64.rpm
```
#### ***For Ubuntu 22***
>`下载安装包`
```shell
wget https://github.com/yang66887/Nagios/raw/refs/heads/main/NCPA/ncpa_3.1.1-1_amd64.deb
```
>`安装`
```shell
dpkg -i ncpa_3.1.1-1_amd64.deb
```

### 配置
>`修改默认Token值`
```shell
sed -i 's/mytoken/生成的Token值/g' /usr/local/ncpa/etc/ncpa.cfg
```
>`重启NCPA服务`
```shell
systemctl restart ncpa
```
>`下载自定义插件`
```shell
curl -o /usr/local/ncpa/plugins/systeminfo 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/NCPA/systeminfo'
curl -o /usr/local/ncpa/plugins/nagios_plugins 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/NCPA/nagios_plugins'
```
>`添加执行权限`
```shell
chmod 755 /usr/local/ncpa/plugins/systeminfo
chmod 755 /usr/local/ncpa/plugins/nagios_plugins
```

## For Windows

### 下载安装包
>`官网下载地址`
```shell
https://assets.nagios.com/downloads/ncpa3/ncpa-latest.exe
```

### 安装
>`双击开始安装`
>`输入生成的Token值`
>`其余选项默认`

### 添加自定义插件
>`下载地址`
```shell
https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/NCPA/systeminfo.bat
```
>`自定义插件路径`
```shell
C:\Program Files\Nagios\NCPA\plugins\systeminfo.bat
```

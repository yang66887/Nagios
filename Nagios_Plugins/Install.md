# Install Nagios Plugins

### 官网页面
```shell
https://nagios-plugins.org/
```

### 安装依赖包
#### ***For EL7***
```shell
yum install wget gcc openssl-devel perl
```
#### ***For EL8***
```shell
yum install wget tar make gcc openssl-devel
```
#### ***For Ubuntu 22***
```shell
apt install wget make gcc libssl-dev
```

### 下载源码包
```shell
wget https://github.com/nagios-plugins/nagios-plugins/releases/download/release-2.4.12/nagios-plugins-2.4.12.tar.gz
```

### 编译安装
>`创建Nagios账号`
```shell
useradd -M -s /sbin/nologin nagios
```
>`解压源码包`
```shell
tar xf nagios-plugins-2.4.12.tar.gz
```
>`进入安装目录`
```shell
cd nagios-plugins-2.4.12
```
>`编译并安装`
```shell
./configure \
--with-nagios-user=nagios \
--with-nagios-group=nagios \
--with-openssl \
&& make all \
&& make install
```

### 安装NCPA检查插件 - For Nagios Server
###### ***官方原版插件为Python版本，当检查指标数量较多时，每次检查都需要加载一次Python解释器，导致CPU消耗急剧增加***
>`下载NCPA插件源码 - C语言版本`
```shell
curl -o check_ncpa.c 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/NCPA/check.c'
```
>`安装编译依赖 - For Ubuntu/Debian`
```shell
apt update
apt install -y libcurl4-openssl-dev libjansson-dev gcc
```
>`安装编译依赖 - For CentOS/RHEL`
```shell
yum install -y libcurl-devel jansson-devel gcc
```
>`编译check_ncpa插件`
```shell
gcc -o check_ncpa check_ncpa.c -lcurl -ljansson
```
>`安装check_ncpa插件`
```shell
cp check_ncpa /usr/local/nagios/libexec/
chown nagios:nagios /usr/local/nagios/libexec/check_ncpa
chmod 755 /usr/local/nagios/libexec/check_ncpa
```

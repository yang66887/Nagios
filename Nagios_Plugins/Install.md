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
>`下载插件压缩包`
```shell
curl -o check_ncpa.tar.gz 'https://assets.nagios.com/downloads/ncpa/check_ncpa.tar.gz'
```
>`解压并拷贝到Nagios插件目录`
```shell
tar xf check_ncpa.tar.gz
mv check_ncpa.py /usr/local/nagios/libexec/check_ncpa
```
>`修改插件权限`
```shell
chown nagios:nagios /usr/local/nagios/libexec/check_ncpa
chmod 755 /usr/local/nagios/libexec/check_ncpa
```

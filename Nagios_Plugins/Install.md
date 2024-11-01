# Install Nagios Plugins

### 官网页面
```shell
https://nagios-plugins.org/
```

### 下载源码包
```shell
curl -o nagios-plugins-2.4.12.tar.gz https://github.com/nagios-plugins/nagios-plugins/releases/download/release-2.4.12/nagios-plugins-2.4.12.tar.gz
```

### 安装依赖包
#### ***For EL7***
```shell
yum install gcc openssl-devel perl
```
#### ***For EL8***
```shell
yum install tar make gcc openssl-devel
```
#### ***For Ubuntu 22***
```shell
apt install make gcc libssl-dev
```

### 编译安装
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

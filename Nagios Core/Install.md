# Install Nagios Core

### 官网页面
>`Nagios Core`
```shell
https://www.nagios.org/downloads/nagios-core/
```

### 下载源码包
```shell
curl -o nagios-4.5.7.tar.gz https://assets.nagios.com/downloads/nagioscore/releases/nagios-4.5.7.tar.gz
```

### 安装依赖包
#### ***For EL7***
```shell
yum install gcc gd-devel perl unzip
```

### 编译安装
>`创建Nagios账号`
```shell
useradd -M -s /sbin/nologin nagios
```
>`解压源码包`
```shell
tar xf nagios-4.5.7.tar.gz
```
>`进入安装目录`
```shell
cd nagios-4.5.7
```
>`编译并安装`
```shell
./configure \
&& make all \
&& make install \
&& make install-init \
&& make install-config \
&& make install-commandmode
```

### 配置
>`创建Nagios Web认证文件`
>`-salt 盐值`
>`-apr1 密码`
```shell
echo "nagios:$(openssl passwd -salt 'xxxxXXXX' -apr1 Password@abc123)" >/usr/local/nagios/etc/auth.users
```
>`修改Nagios管理员账号`
```shell
sed -i 's/nagiosadmin/nagios/g' /usr/local/nagios/etc/cgi.cfg
```

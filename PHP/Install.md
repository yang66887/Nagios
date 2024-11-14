# Install PHP 8

### 官网页面
>`Current Stable Version`
```shell
https://www.php.net/downloads
```

### 下载源码包
```shell
curl -o php-8.3.13.tar.gz https://www.php.net/distributions/php-8.3.13.tar.gz
```

### 安装依赖包
#### ***For EL7***
```shell
yum install gcc libxml2-devel sqlite-devel
```
#### ***For EL8***
```shell
yum install tar make gcc libxml2-devel sqlite-devel
```
#### ***For Ubuntu 22***
```shell
apt install make gcc libxml2-dev pkg-config libsqlite3-dev
```

### 编译安装
>`解压源码包`
```shell
tar xf php-8.3.13.tar.gz
```
>`进入安装目录`
```shell
cd php-8.3.13
```
>`编译并安装`
```shell
./configure \
--prefix=/usr/local/php \
--with-config-file-path=/usr/local/php/etc \
--enable-sockets \
--enable-fpm \
&& make \
&& make install
```

### 配置
>`配置php.ini`
```shell
cp php.ini-production /usr/local/php/etc/php.ini
```
>`配置php-fpm.conf`
```shell
mv /usr/local/php/etc/php-fpm.conf.default /usr/local/php/etc/php-fpm.conf
```
>`创建php-fpm pool for nagios`
```shell
mkdir /usr/local/php/var/session
cat >/usr/local/php/etc/php-fpm.d/nagios.conf <<EOF
[nagios]
listen = /usr/local/php/var/run/nagios.socket
listen.owner = nobody
listen.group = $(groups nobody|awk '{print $3}')
listen.mode=0660
listen.allowed_clients = 127.0.0.1

user = nagios
group = nagios
pm = dynamic
pm.max_children = 50
pm.start_servers = 5
pm.min_spare_servers = 5
pm.max_spare_servers = 35

slowlog = /usr/local/php/var/log/nagios-slow.log

php_flag[display_errors] = on
php_admin_value[error_log] = //usr/local/php/var/log/nagios-error.log
php_admin_flag[log_errors] = on
php_admin_flag[expose_php] = off
php_value[session.save_handler] = files
php_value[session.save_path] = /usr/local/php/var/session
EOF
```
>`创建php-fpm.service`
```shell
cat >/usr/lib/systemd/system/php-fpm.service <<EOF
[Unit]
Description=The PHP FastCGI Process Manager
After=network.target

[Service]
Type=simple
PIDFile=/usr/local/php/var/run/php-fpm.pid
ExecStart=/usr/local/php/sbin/php-fpm --nodaemonize --fpm-config /usr/local/php/etc/php-fpm.conf
ExecReload=/bin/kill -USR2
PrivateTmp=true

[Install]
WantedBy=multi-user.target
EOF
```

### 启动php-fpm
```shell
systemctl enable --now php-fpm
```

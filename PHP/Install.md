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
yum -y install tar make gcc libxml2-devel sqlite-devel
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

###############################以下待删除####################################

## For EL7
#### 配置yum源
```shell
yum install https://rpms.remirepo.net/enterprise/remi-release-7.rpm epel-release yum-utils
```
```shell
yum-config-manager --disable 'remi-php*'
yum-config-manager --enable remi-php83
```

#### 安装PHP8及其依赖包
```shell
yum install php php-cli php-common php-sodium php-fpm apr apr-util httpd httpd-tools libsodium mailcap
```

## For EL8
#### 配置yum源
>`需手动修改remi-release版本号为对应操作系统版本号`
```shell
dnf install https://rpms.remirepo.net/enterprise/remi-release-8.xx.rpm epel-release
```

#### 安装PHP8及其依赖包
>`查询php版本`
```shell
dnf module list php
```
>`安装时指定最新版本号`
```shell
dnf module install php:remi-8.xx
```

## For Ubuntu 22.04
#### 配置第三方PHP仓库
```shell
add-apt-repository ppa:ondrej/php
```

#### 安装PHP8及其依赖包
```shell
apt install php8.3 php8.3-fpm apache2-utils
```
>`在线安装可能因为网络原因安装失败，可使用本项目提供的离线deb包`


## 配置php-fpm for nagios
### For EL7 & EL8
```shell
mv /etc/php-fpm.d/www.conf /etc/php-fpm.d/www.conf.bak
cat >>/etc/php-fpm.d/nagios.conf <<EOF
[nagios]	
listen = /var/run/php-fpm/nagios.socket
listen.owner = nobody
listen.group = nobody
listen.mode=0660
listen.allowed_clients = 127.0.0.1

user = nagios
group = nagios
pm = dynamic
pm.max_children = 50
pm.start_servers = 5
pm.min_spare_servers = 5
pm.max_spare_servers = 35

slowlog = /var/log/php-fpm/nagios-slow.log

php_flag[display_errors] = on
php_admin_value[error_log] = /var/log/php-fpm/nagios-error.log
php_admin_flag[log_errors] = on
php_admin_flag[expose_php] = off
php_value[session.save_handler] = files
php_value[session.save_path] = /var/lib/php/session
EOF
```

### For Ubuntu 22.04
>`修改php8.3-fpm systemd服务配置`
```shell
systemctl stop php8.3-fpm
sed -i 's/www\.conf/nagios.conf/g' /lib/systemd/system/php8.3-fpm.service
systemctl daemon-reload
```
>`删除默认php8.3-fpm pool`
```shell
mv /etc/php/8.3/fpm/pool.d/www.conf /etc/php/8.3/fpm/pool.d/www.conf.bak
```
>`创建php8.3-fpm pool for nagios`
```shell
mkdir /var/run/php-fpm /var/log/php-fpm
chown nagios:nagios /var/run/php-fpm /var/log/php-fpm
cat >>/etc/php/8.3/fpm/pool.d/nagios.conf <<EOF
[nagios]
listen = /var/run/php-fpm/nagios.socket
listen.owner = nagios
listen.group = nagios
listen.mode=0660
listen.allowed_clients = 127.0.0.1

user = nagios
group = nagios
pm = dynamic
pm.max_children = 50
pm.start_servers = 5
pm.min_spare_servers = 5
pm.max_spare_servers = 35

slowlog = /var/log/php-fpm/nagios-slow.log

php_flag[display_errors] = on
php_admin_value[error_log] = /var/log/php-fpm/nagios-error.log
php_admin_flag[log_errors] = on
php_admin_flag[expose_php] = off
php_value[session.save_handler] = files
php_value[session.save_path] = /var/lib/php/session
EOF
```

## 启动php-fpm服务
### For EL7 & EL8
```shell
systemctl enable --now php-fpm
```

### For Ubuntu 22.04
```shell
systemctl start php8.3-fpm
```

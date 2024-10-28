# Install Nginx

### 官网页面
>`Stable Version`
```shell
https://nginx.org/en/download.html
```

### 下载源码包
```shell
curl -s -o nginx-1.26.2.tar.gz https://nginx.org/download/nginx-1.26.2.tar.gz
```

### 安装依赖包
#### ***For EL7***
>`添加epel源`
```shell
yum install epel-release
```
>`安装依赖`
```shell
yum install gcc openssl-devel fcgi fcgiwrap spawn-fcgi
```

### 编译安装
>`解压源码包`
```shell
tar xf nginx-1.26.2.tar.gz
```
>`进入安装目录`
```shell
cd nginx-1.26.2
```
>`编译并安装`
```shell
./configure \
--prefix=/usr/local/nginx \
--with-http_v2_module \
--with-http_realip_module \
--with-http_addition_module \
--with-http_sub_module \
--with-http_dav_module \
--with-http_stub_status_module \
--with-http_flv_module \
--with-http_mp4_module \
--with-http_gunzip_module \
--with-http_gzip_static_module \
--with-http_auth_request_module \
--with-http_random_index_module \
--with-http_secure_link_module \
--with-http_slice_module \
--with-mail \
--with-stream \
--with-http_ssl_module \
--with-mail_ssl_module \
--with-stream_ssl_module \
&& make \
&& make install
```

### 配置
>`编辑nginx.conf`
```shell
vi /usr/local/nginx/conf/nginx.conf
```
>`删除默认server并创建server for nagios`
```shell
  server {
    listen       4000;
    server_name  localhost;

    #charset koi8-r;
    #access_log  logs/host.access.log  main;

    server_tokens off;
    auth_basic "Nagios Restricted Access";
    auth_basic_user_file /usr/local/nagios/etc/auth.users;
    root /usr/local/nagios/share;

    location /nagios {
      rewrite ^/nagios/(.*) /$1 break;
      index index.php;
    }

    location ~ .cgi$ {
      root /usr/local/nagios/sbin/;
      rewrite ^/nagios/cgi-bin/(.*).cgi /$1.cgi break;
      include fastcgi_params;
      fastcgi_param AUTH_USER $remote_user;
      fastcgi_param REMOTE_USER $remote_user;
      fastcgi_pass unix:/var/run/fcgiwrap.socket;
      fastcgi_param SCRIPT_FILENAME /usr/local/nagios/sbin/$fastcgi_script_name;
      fastcgi_param PATH_INFO $fastcgi_script_name;
    }

    location ~ .php$ {
      rewrite ^/nagios/(.*) /$1 break;
      include fastcgi_params;
      fastcgi_param AUTH_USER $remote_user;
      fastcgi_param REMOTE_USER $remote_user;
      fastcgi_pass unix:/usr/local/php/var/run/nagios.socket;
      fastcgi_index index.php;
      fastcgi_param SCRIPT_FILENAME /usr/local/nagios/share$fastcgi_script_name;
      fastcgi_param PATH_INFO $fastcgi_script_name;
    }
  }
```
>`创建spawn-fcgi配置文件`
```shell
cat >>/etc/sysconfig/spawn-fcgi <<EOF
FCGI_SOCKET=/var/run/fcgiwrap.socket
FCGI_PROGRAM=$(which fcgiwrap)
FCGI_USER=nobody
FCGI_GROUP=nobody
FCGI_EXTRA_OPTIONS="-M 0700"
OPTIONS="-u \$FCGI_USER -g \
\$FCGI_GROUP -s \
\$FCGI_SOCKET -S \
\$FCGI_EXTRA_OPTIONS \
-F 1 -P /var/run/spawn-fcgi.pid -- \
\$FCGI_PROGRAM"
EOF
```
>`创建nginx.service`
```shell
cat >>/usr/lib/systemd/system/nginx.service <<EOF
[Unit]
Description=nginx - high performance web server
After=network.target remote-fs.target nss-lookup.target
[Service]
Type=forking
ExecStart=/usr/local/nginx/sbin/nginx -c /usr/local/nginx/conf/nginx.conf
ExecReload=/usr/local/nginx/sbin/nginx -s reload
ExecStop=/usr/local/nginx/sbin/nginx -s stop
[Install]
 WantedBy=multi-user.target
EOF
```


### 启动
>`Spawn-fcgi`
```shell
chkconfig --level 35 spawn-fcgi on
systemctl start spawn-fcgi
```
>`Nginx`
```shell
systemctl enable --now nginx
```

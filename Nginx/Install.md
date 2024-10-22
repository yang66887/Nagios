#### 1、下载地址
```shell
wget https://nginx.org/download/nginx-1.26.2.tar.gz
```

#### 2、安装依赖
```shell
yum -y install gcc glibc glibc-common openssl-devel
```

#### 3、编译安装
```shell
tar xf nginx-1.26.2.tar.gz
cd nginx-1.26.2
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
--with-stream_ssl_module
make && make install
```

#### 4、注册Systemd服务
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

#### 5、修改配置文件
```shell
vi /usr/local/nginx/conf/nginx.conf
```
```shell
# 删除默认 server
# 创建 server for nagios
server {
  listen       4000;
  server_name  localhost;

  #charset koi8-r;
  #access_log  logs/host.access.log  main;

  server_tokens off;
  auth_basic "Nagios Restricted Access";
  auth_basic_user_file /usr/local/nagios/etc/htpasswd.users;
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
    fastcgi_pass unix:/var/run/php-fpm/nagios.socket;
    fastcgi_index index.php;
    fastcgi_param SCRIPT_FILENAME /usr/local/nagios/share$fastcgi_script_name;
    fastcgi_param PATH_INFO $fastcgi_script_name;
  }
}
```

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

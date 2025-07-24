# Install Custom plugins

### 安装
#### MySQL Client
>`官网下载页面`
```shell
https://dev.mysql.com/downloads/mysql/
```
>`For EL7`
>`下载rpm包`
>`安装`
```shell
curl -o mysql-community-client-8.0.40-1.el7.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client-8.0.40-1.el7.x86_64.rpm'
curl -o mysql-community-client-plugins-8.0.40-1.el7.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client-plugins-8.0.40-1.el7.x86_64.rpm'
curl -o mysql-community-common-8.0.40-1.el7.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-common-8.0.40-1.el7.x86_64.rpm'
curl -o mysql-community-libs-8.0.40-1.el7.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-libs-8.0.40-1.el7.x86_64.rpm'
curl -o mysql-community-libs-compat-8.0.40-1.el7.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-libs-compat-8.0.40-1.el7.x86_64.rpm'
```
```shell
rpm -Uvh \
mysql-community-client-8.0.40-1.el7.x86_64.rpm \
mysql-community-client-plugins-8.0.40-1.el7.x86_64.rpm \
mysql-community-libs-8.0.40-1.el7.x86_64.rpm \
mysql-community-libs-compat-8.0.40-1.el7.x86_64.rpm \
mysql-community-common-8.0.40-1.el7.x86_64.rpm
```
>`For EL8`
>`下载rpm包`
>`安装`
```shell
curl -o mysql-community-client-8.0.40-1.el8.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client-8.0.40-1.el8.x86_64.rpm'
curl -o mysql-community-client-plugins-8.0.40-1.el8.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client-plugins-8.0.40-1.el8.x86_64.rpm'
curl -o mysql-community-common-8.0.40-1.el8.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-common-8.0.40-1.el8.x86_64.rpm'
curl -o mysql-community-libs-8.0.40-1.el8.x86_64.rpm 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-libs-8.0.40-1.el8.x86_64.rpm'
```
```shell
rpm -ivh \
mysql-community-client-8.0.40-1.el8.x86_64.rpm \
mysql-community-client-plugins-8.0.40-1.el8.x86_64.rpm \
mysql-community-libs-8.0.40-1.el8.x86_64.rpm \
mysql-community-common-8.0.40-1.el8.x86_64.rpm
```
>`For Ubuntu 22`
>`下载deb包`
>`安装`
```shell
curl -o mysql-community-client_8.0.40-1ubuntu22.04_amd64.deb 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client_8.0.40-1ubuntu22.04_amd64.deb'
curl -o mysql-community-client-core_8.0.40-1ubuntu22.04_amd64.deb 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client-core_8.0.40-1ubuntu22.04_amd64.deb'
curl -o mysql-community-client-plugins_8.0.40-1ubuntu22.04_amd64.deb 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-community-client-plugins_8.0.40-1ubuntu22.04_amd64.deb'
curl -o mysql-common_8.0.40-1ubuntu22.04_amd64.deb 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-common_8.0.40-1ubuntu22.04_amd64.deb'
```
```shell
dpkg -i \
mysql-community-client_8.0.40-1ubuntu22.04_amd64.deb \
mysql-community-client-core_8.0.40-1ubuntu22.04_amd64.deb \
mysql-community-client-plugins_8.0.40-1ubuntu22.04_amd64.deb \
mysql-common_8.0.40-1ubuntu22.04_amd64.deb
```


#### Nagios Plugins、MySQL Client、check_ncpa与InfluxDB集成脚本
>`下载集成脚本`
```shell
curl -o /data/nagios/libexec/check_performance https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/check_performance
```
>`下载SQL语句记录文件`
>`用于监控MySQL定制化数据`
>`语句名必须以 SQL_ 开头`
```shell
curl -o /data/nagios/etc/data_sql.list 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/data_sql.list'
```
>`下载进程列表写入插件`
```shell
mkdir /data/nagios/var/cache
curl -o /data/nagios/var/cache/.influx_write.py 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/influx_write.py'
cat >>/etc/crontab <<EOF
# Nagios进程列表定时写入Influxdb
*/5 * * * * root [ $(ps -ef|grep influx_write.py|grep -v grep|wc -l) -eq 0 ] && /usr/bin/python /data/nagios/var/cache/.influx_write.py &>/dev/null
EOF
```
>`修改权限`
```shell
chown nagios:nagios /data/nagios/libexec/check_performance
chmod 755 /data/nagios/libexec/check_performance
chmod 755 /data/nagios/var/cache/.influx_write.py
```

#### 企业微信告警插件
>`单一成员告警插件`
```shell
curl -o /data/nagios/libexec/send_wechat 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/send_wechat'
```
>`群组告警插件`
```shell
curl -o /data/nagios/libexec/send_wechat_group 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/send_wechat_group'
```
>`企业微信成员通讯录`
```shell
curl -o /data/nagios/etc/wechat_user.list 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/wechat_user.list'
```
>`企业微信群组通讯录`
```shell
curl -o /data/nagios/etc/wechat_group.list 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/wechat_group.list'
```
>`修改权限`
```shell
chown nagios:nagios /data/nagios/libexec/send_wechat*
chmod 755 /data/nagios/libexec/send_wechat*
```
>`告警详情页`
>`非必需`
>`需放置到公网可访问的域名下`
>`需将域名配置到send_wechat插件的Nagios_URL参数中`
```shell
curl -o nagios.html 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/nagios_html/nagios.html'
curl -o logo.png 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/nagios_html/logo.png'
curl -o ok.png 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/nagios_html/ok.png'
curl -o warning.png 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/nagios_html/warning.png'
curl -o critical.png 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Custom_Plugins/nagios_html/critical.png'
```

### 配置
>`编辑send_wechat与send_wechat_group插件，填写企业微信自建应用相关认证参数`

>`企业微信自建应用相关信息请自行查阅企业微信开放平台文档`
```shell
https://open.work.weixin.qq.com/wwopen/manual/detail?t=selfBuildApp
```
>`企业微信告警需访问如下地址`
>`网络请自行打通`
```shell
https://qyapi.weixin.qq.com
```

>`编辑wechat_user.list，写入需接收告警人员的姓名和企业微信号`

>`编辑wechat_group.list，写入需接收告警群组的群名以及群成员企业微信号`

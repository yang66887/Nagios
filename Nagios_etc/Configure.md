# Nagios Configure

### 修改Nagios配置文件
>`注释默认监控配置文件`
```shell
sed -i 's/^\(cfg_file=.*.cfg\)/#\1/g' /usr/local/nagios/etc/nagios.cfg
```
>`添加自定义监控配置目录`
```shell
sed -i 's/^\(#cfg_dir=.*servers\)/cfg_dir=\/usr\/local\/nagios\/etc\/monitor\n\1/' /usr/local/nagios/etc/nagios.cfg
```

### 写入额外认证信息
>`以下账号密码与IP地址等信息仅供参考，请自行修改`

>`MySQL用户名与密码`
```shell
cat >>/usr/local/nagios/etc/resource.cfg <<EOF

# MySQL
\$USER2\$='mysql_user'
\$USER3\$='mysql_password'
EOF
```
>`InfluxDB Web Api地址、端口以及用户名和密码`
```shell
cat >>/usr/local/nagios/etc/resource.cfg <<EOF

# InfluxDB
\$USER4\$='192.168.1.180:8086'
\$USER5\$='influxdb_user:influxdb_password'
EOF
```
>`NCPA Token`
```shell
cat >>/usr/local/nagios/etc/resource.cfg <<EOF

# NCPA
\$USER6\$='ncpa_token'
EOF
```

### 添加自定义监控配置
#### ***模板中定义的参数名不可修改，根据实际情况修改参数值即可***
>`创建监控目录`
```shell
mkdir /usr/local/nagios/etc/monitor
```
>`下载配置文件模板`
```shell
curl -o /usr/local/nagios/etc/monitor/commands.cfg 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Nagios_etc/monitor/commands.cfg'
curl -o /usr/local/nagios/etc/monitor/contacts.cfg 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Nagios_etc/monitor/contacts.cfg'
curl -o /usr/local/nagios/etc/monitor/localhost.cfg 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Nagios_etc/monitor/localhost.cfg'
curl -o /usr/local/nagios/etc/monitor/templates.cfg 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Nagios_etc/monitor/templates.cfg'
curl -o /usr/local/nagios/etc/monitor/timeperiods.cfg 'https://raw.githubusercontent.com/yang66887/Nagios/refs/heads/main/Nagios_etc/monitor/timeperiods.cfg'
```
>`修改权限`
```shell
chown -R nagios:nagios /usr/local/nagios/etc/monitor
```
>`添加自定义联系人`
```shell
vi /usr/local/nagios/etc/monitor/contacts.cfg
```
>`添加自定义时间段`
```shell
vi /usr/local/nagios/etc/monitor/timeperiods.cfg
```
>`添加自定义监控`
```shell
vi /usr/local/nagios/etc/monitor/localhost.cfg
```
>`重启Nagios服务`
```shell
systemctl restart nagios
```

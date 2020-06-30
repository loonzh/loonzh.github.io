---
layout: post
title: HAProxy+keepalived+Mycat
categories: [数据库]
tags: [Mycat,keepalive,HAproxy]
---
#### 1. 安装HAProxy

##### 1.1 编译安装
**[安装环境依赖]**  
`yum install gcc gcc-c++ pcre pcre-devel zlib zlib-devel openssl openssl-devel -y`  
<!-- more -->
**[创建安装目录]**  
`mkdir /home/haproxy`  
下载[HAproxy源码](https://src.fedoraproject.org/repo/pkgs/haproxy/)并上传  
**[解压源码]**  
`tar -zxvf haproxy-1.5.14.tar.gz`  
**[编译安装源码]**  
`cd haproxy-1.5.14`  
```
make TARGET=linux2628 ARCH=x86_64 USE_PCRE=1 USE_OPENSSL=1 USE_ZLIB=1 PREFIX=/home/haproxy
[TARGET指定内核版本(查看uname -r),高于2.6.28的建议设为linux2628,ARCH指定系统架构,使用PRCE、OPENSSL、ZLIB]
make install PREFIX=/home/haproxy
```  
##### 1.2 创建配置文件
**[在/home和/etc创建配置文件目录]**  
`mkdir -p /home/haproxy/conf`  
`mkdir -p /etc/haproxy/`  
**[复制配置文件和错误文件]**  
`cp /home/haproxy-1.5.14/examples/haproxy.cfg /home/haproxy/conf/`  
`cp -r /home/haproxy-1.5.14/examples/errorfiles/ /home/haproxy/`  
**[创建软连接]**  
`ln -s /home/haproxy/conf/haproxy.cfg /etc/haproxy/haproxy.cfg`  
`ln -s /home/haproxy/errorfiles/ /etc/haproxy/`  
##### 1.3 设为服务开机自启
**[复制服务脚本至系统服务目录]**  
`cp /home/haproxy-1.5.14/examples/haproxy.init /etc/rc.d/init.d/haproxy`  
**[赋予服务脚本执行权限]**  
`chmod +x /etc/rc.d/init.d/haproxy`  
**[将服务设为开机自启]**  
`chkconfig haproxy --level 2345 on`  
**[添加命令脚本软连接]**  
`ln -s /home/haproxy/sbin/haproxy /usr/sbin`  
#### 2. 配置Mycat负载均衡集群
##### 2.1 修改haproxy.cfg配置文件
`vi /home/haproxy/conf/haproxy.cfg`  
```
# this config needs haproxy-1.1.28 or haproxy-1.2.1
global
 log 127.0.0.1 local0 info
 [定义全局syslog服务器(最多2个)，local0是日志设备(/etc/rsyslog.conf的配置)，默认回收info的日志级别]
 maxconn 4096
 [单个haproxy进程的最大并发连接数]
 chroot /var/lib/haproxy
 [修改HAProxy的工作目录至指定的目录，且在放弃权限前执行chroot()操作，提升haproxy的安全级别]
 pidfile /var/run/haproxy.pid
 [进程文件(默认路径 /var/run/haproxy.pid)]
 user haproxy 
 [同uid，但这里使用用户名]
 group haproxy 
 [同gid，但这里使用用户组名]
 daemon
 [haproxy以后台守护进程形式运行]
 nbproc 1 
 [启动的haproxy进程数, 只能用于守护进程模式,默认为1]
 stats socket /var/lib/haproxy/stats
 node Node1
 [节点名称，用于多haproxy进程共享同一IP地址时]
 description Node1-haproxy
 [当前实例的描述信息]
defaults
 log global
 mode tcp
 [haproxy运行模式，默认为http]
 option tcplog
 option dontlognull
 option redispatch 
 [serverId对应的服务器挂掉后，强制定向到其他健康的服务器]
 retries 3 
 [三次连接失败，则服务器不用]
 maxconn 2000
 [前端的最大并发连接数，默认为2000]
 [haproxy为每个连接维持两个8KB缓冲，适当优化后，1GB的可用RAM空间可维护40000-50000并发连接]
 timeout http-request 10s
 timeout queue 1m
 timeout connect 10s
 [连接超时(默认单位毫秒，可设置us、ms、s、m、h、d)]
 timeout client 1m
 [客户端超时]
 timeout server 1m
 [服务器超时]
 timeout http-keep-alive 10s
 timeout check 10s
 [心跳检测时间间隔]
listen statistics 
[配置haproxy状态页(可视化)]
 mode http
 bind *:8888
 [状态页端口]
 option httplog 
 [启用日志记录HTTP请求]
 stats enable
 stats auth admin:admin
 [状态页用户名和密码, 如需设置多个，另起一行写入即可]
 stats uri /haproxy
 [自定义状态页URL，默认为/admin?stats]
 stats hide-version
 [隐藏状态页的HAProxy版本信息]
 stats refresh 30s
 [状态页自动刷新时间]
 stats admin if TRUE
 [若认证通过，则可管理后端服务器]
 stats realm Hapadmin
 [密码框提示文本，默认为Haproxy\Statistics]
listen mycat_servers
 bind *:3307
 [用来连接Mycat的端口]
 mode tcp
 option tcplog
 [记录TCP请求日志]
 option tcpka
 [是否允许向server和client发送keepalive]
 option httpchk OPTIONS * HTTP/1.1\r\nHost:\ www 
 [向后端服务器发送OPTIONS请求，判断后端服务是否可用(2xx和3xx表示健康，其他响应码或无响应表示服务器故障)]
 balance roundrobin
 [基于权重进行轮叫，定义负载均衡算法，可用于"defaults"、"listen"和"backend"中，默认为轮询]
 server <name> <address>:<port> check port 48700 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 server mycat_02 节点IP:8066 check port 48700 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 [serser在后端声明一个server，只能用于listen和backend区段]
 [<name>为此服务器指定的内部名称，其将会出现在日志及警告信息中]
 [<address>此服务器的IPv4地址，也支持可解析的主机名，但需要在启动时解析主机名至响应的IPV4地址]
 [<port>指定将客户端连接请求发往此服务器时的目标端口，为可选项]
 [weight权重，默认为1，最大值为256，0表示不参与负载均衡]
 [backup设定为备用服务器，仅在负载均衡场景中的其他server均不可用时启用此server]
 [check启动对此server执行监控状态检查，可以借助于额外的其他参数完成更精细的设定]
 [inter设定监控状态检查的时间间隔，单位为毫秒，默认为2000，也可用fastinter和downinter来优化延迟]
 [rise设置server从离线状态转换至正常状态需要检查的次数，默认为2]
 [fall设置server从正常状态转换至离线状态需要检查的次数，默认为3]
 [cookie为指定server设定cookie值，将会在请求入站时被检查，并被后续的请求所选中，以实现持久连接的功能]
 [maxconn服务器接受的最大并发连接数，若发往此服务器的连接数目高于此处指定的值，新的连接将进入请求队列]
```
*注意：多节点部署时node、description的值要做相应调整*   

##### 2.2 根据以上HAProxy配置文件要求做以下配置

1.添加haproxy用户组和用户  
`groupadd haproxy`  
`useradd -g haproxy -s /bin/false haproxy`  
2.创建chroot运行的路径  
`mkdir /var/lib/haproxy`  
`chown -R haproxy.haproxy /var/lib/haproxy/`  
`chown -Rf haproxy.haproxy /usr/local/haproxy/`  
**[防火墙开放3306端口、3307端口和48800端口]**  
`firewall-cmd --permanent --add-port=3306-3307/tcp`  
`firewall-cmd --permanent --add-port=48800/tcp`  
**[重启防火墙]**  
`systemctl reload firewalld`  

##### 2.3 开启rsyslog的haproxy日志记录功能  
HAproxy默认不记录日志，若需记录日志，则需配置系统的syslog。
rsyslog是一个开源工具，被广泛用于Linux系统中通过TCP/UDP协议转发或接收日志消息，可以用作一个网络中的日志监控中心。  
**[安装配置rsyslog服务]**  
`yum -y install rsyslog`  
`vi /etc/rsyslog.conf`*有则取消#开启配置项功能，无则增加配置项*  
```
#### MODULES ####
# Provides UDP syslog reception
$ModLoad imudp
$UDPServerRun 514
[启动UDP端口514，启动后作为服务器工作]
# Provides TCP syslog reception
$ModLoad imtcp
$InputTCPServerRun 514
[启动TCP端口514，启动后作为服务器工作]
```
```
#### GLOBAL DIRECTIVES ####
# Include all config files in /etc/rsyslog.d/
$IncludeConfig /etc/rsyslog.d/*.conf
```  
`vi /etc/rsyslog.d/haproxy.conf`*增加HAProxy日志功能*  

```
local0.* /var/log/haproxy.log
&~
[若不加[&~],则在/var/log/haproxy.log中写入日志外，也会在/var/log/message写入日志]
```
**[重启rsyslog]**  
`systemctl restart rsyslog`  
*HAProxy服务启动后，就能在/var/log/haproxy.log中看到日志了*  
##### 2.4 配置系统内核的IP包转发功能
`vi /etc/sysctl.conf`  
`net.ipv4.ip_forward=1`  
**[使配置生效]**  
`sysctl -p`  
##### 2.5 启动HAProxy
`systemctl start haproxy`  
##### 2.6 通过HAProxy连接Mycat
`mysql -uroot -proot -h节点IP -P[HAProxy连接Mycat的端口]`  
##### 2.7 登录HAProxy的状态信息统计页面
浏览器访问`http://节点IP:8888/[自定义状态页URL]`，用户名和密码都是[admin]。  
#### 3. 配置示例
##### 3.1 MySQL代理示例
```
# this config needs haproxy-1.1.28 or haproxy-1.2.1
global
 log 127.0.0.1 local0 info
 [定义全局syslog服务器(最多2个)，local0是日志设备(/etc/rsyslog.conf的配置)，默认回收info的日志级别]
 maxconn 4096
 [单个haproxy进程的最大并发连接数]
 chroot /var/lib/haproxy
 [修改HAProxy的工作目录至指定的目录，且在放弃权限前执行chroot()操作，提升haproxy的安全级别]
 pidfile /var/run/haproxy.pid
 [进程文件(默认路径 /var/run/haproxy.pid)]
 user haproxy 
 [同uid，但这里使用用户名]
 group haproxy 
 [同gid，但这里使用用户组名]
 daemon
 [haproxy以后台守护进程形式运行]
 nbproc 1 
 [启动的haproxy进程数, 只能用于守护进程模式,默认为1]
 stats socket /var/lib/haproxy/stats
 node Node1
 [节点名称，用于多haproxy进程共享同一IP地址时]
 description Node1-haproxy
 [当前实例的描述信息]
defaults
 log global
 mode tcp
 [haproxy运行模式，默认为http]
 option tcplog
 option dontlognull
 option redispatch 
 [serverId对应的服务器挂掉后，强制定向到其他健康的服务器]
 retries 3 
 [三次连接失败，则服务器不用]
 maxconn 2000
 [前端的最大并发连接数，默认为2000]
 [haproxy为每个连接维持两个8KB缓冲，适当优化后，1GB的可用RAM空间可维护40000-50000并发连接]
 timeout http-request 10s
 timeout queue 1m
 timeout connect 10s
 [连接超时(默认单位毫秒，可设置us、ms、s、m、h、d)]
 timeout client 1m
 [客户端超时]
 timeout server 1m
 [服务器超时]
 timeout http-keep-alive 10s
 timeout check 10s
 [心跳检测时间间隔]
listen statistics 
[配置haproxy状态页(可视化)]
 mode http
 bind *:8888
 [状态页端口]
 option httplog 
 [启用日志记录HTTP请求]
 stats enable
 stats auth admin:admin
 [状态页用户名和密码, 如需设置多个，另起一行写入即可]
 stats uri /haproxy
 [自定义状态页URL，默认为/admin?stats]
 stats hide-version
 [隐藏状态页的HAProxy版本信息]
 stats refresh 30s
 [状态页自动刷新时间]
 stats admin if TRUE
 [若认证通过，则可管理后端服务器]
 stats realm Hapadmin
 [密码框提示文本，默认为Haproxy\Statistics]
frontend mysql
 bind *:3306
 [前端3306端口,任何ip访问3306端口都会将数据转发到mysql服务器群组中]
 mode tcp
 log global
 default_backend mysqlservers
 backend mysqlservers
 balance roundrobin
 [负载均衡算法，基于权重进行轮叫,可用于"defaults"、"listen"和"backend"中，默认为轮询]
 server <name> <address>:<port> check port <port> inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 server 名字 节点IP:3306 check port 3306 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 server mysql3 192.168.106.253:3306 check port 3306 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
```
##### 3.2 Http代理示例
*前端调度器IP：192.168.1.10*  
*后端应用服务器IP: 192.168.1.1 和 192.168.1.2*  
```
global
 log 127.0.0.1 local2
 chroot /var/lib/haproxy
 pidfile /var/run/haproxy.pid
 maxconn 4000
 user haproxy
 group haproxy
 daemon
defaults
 mode http
 log global
 option httplog
 option dontlognull
 option http-server-close
 option forwardfor except 127.0.0.0/8
 option redispatch
 retries 3
 timeout http-request 10s
 timeout queue 1m
 timeout connect 10s
 timeout client 1m
 timeout server 1m
 timeout http-keep-alive 10s
 timeout check 10s
 maxconn 30000
listen stats
 mode http
 bind 0.0.0.0:1080
 stats enable
 stats hide-version
 stats uri /haproxy
 stats realm Haproxy\Statistics
 stats auth admin:admin
 stats admin if TRUE
frontend http-in
 bind *:80
 mode http
 log global
 option httpclose
 option logasap
 [不等待响应结束就记录日志，一般日志会记录响应时长，此不记录响应时长]
 option dontlognull
 [不记录空信息]
 capture request header Host len 20
 [记录请求首部的前20个字符]
 capture request header Referer len 60
 [referer跳转引用，就是上一级]
 default_backend servers
frontend healthcheck
 bind :1099
 [定义外部检测机制]
 mode http
 option httpclose
 option forwardfor
 default_backend servers
 backend servers
 balance roundrobin
 server 名字 节点IP:http端口 check maxconn 2000
 server websrv2 192.168.1.12:80 check maxconn 2000
```
##### 3.3 HAProxy统计页面配置示例
```
listen statistics
 bind *:8009
 [自定义监听端口]
 stats enable
 [启用基于程序编译时默认设置的统计报告]
 stats auth admin:admin
 [设置统计页面用户名和密码]
 stats uri /haproxy
 [自定义统计页面的URL，默认为/haproxy?stats]
 stats hide-version
 [隐藏统计页面上HAProxy的版本信息]
 stats refresh 30s
 [统计页面自动刷新时间]
 stats admin if TRUE
 [如果认证通过就启用管理功能，可以管理后端的服务器]
 stats realm password
 [统计页面密码框上提示文本，默认为Haproxy\Statistics]
```
##### 3.4 动静分离示例
```
frontend webservs
 bind *:80
 acl url_static path_beg -i /static /images /javascript /stylesheets
 acl url_static path_end -i .jpg .gif .png .css .js .html
 acl url_php path_end -i .php
 acl host_static hdr_beg(host) -i img. imgs. video. videos. ftp. image. download.
 use_backend static if url_static or host_static
 use_backend dynamic if url_php
 default_backend dynamic
 backend static
 balance roundrobin
 server 名字 节点IP:Http端口 check maxconn 3000
 backend dynamic
 balance roundrobin
 server node2 192.168.1.112:80 check maxconn 1000
```
#### 4. Keepalived安装
Keepalived主要提供loadbalancing（负载均衡）和 high-availability（高可用）功能。  
负载均衡的实现需要依赖Linux的虚拟服务内核模块(ipvs)。  
高可用则是通过VRRP协议实现多台机器之间的故障转移服务。  
##### 4.1 安装编译所需的依赖包
`yum install gcc gcc-c++ openssl openssl-devel -y`  
##### 4.2 编译安装
下载[Keepalive源码](https://www.keepalived.org/download.html)并上传  
**[解压源码]**  
`tar -zxvf keepalived-1.2.19.tar.gz`  
**[生成Makefile]**  
`cd keepalived-1.2.19`  
`./configure --prefix=/home/keepalived --with-kerneldir=/usr/src/kernels/linux版本`  
*--with-kernel-dir根据自己的linux版本填写(使用命令uname –r查询)*  
**[编译安装]**  
`make`  
`make install`  
##### 4.3 创建配置文件
keepalived启动时会查找`/etc/keepalived/keepalived.conf`(需手动创建)，若无则使用默认配置。  
`cd /home/keepalived/`  
`mkdir /etc/keepalived`  
`cp /home/keepalived/etc/keepalived/keepalived.conf /etc/keepalived/keepalived.conf`  
`cp /home/keepalived/etc/rc.d/init.d/keepalived /etc/rc.d/init.d/keepalived`  
`cp /home/keepalived/etc/sysconfig/keepalived /etc/sysconfig/keepalived`  
`ln -s /home/keepalived/sbin/keepalived /usr/sbin/`  
`ln -s /home/keepalived/sbin/keepalived /sbin/`  
##### 4.4 设为服务开机自启
`chkconfig keepalived on`  
**[查看开机启动服务列表]**  
`chkconfig --list`  
#### 5. 配置HAProxy高可用
##### 5.1 Master配置
`vi /etc/keepalived/keepalived.conf`  
```
! Configuration File for keepalived
global_defs {
[keepalived自带的邮件提醒需要开启sendmail服务，建议用独立的监控或第三方SMTP]
 notification_email {
 [指定keepalived在发生切换时需要发送email的对象，一行一个]
  acassen@firewall.loc
  failover@firewall.loc
  sysadmin@firewall.loc 
 }
 notification_email_from Alexandre.Cassen@firewall.loc
 [指定发件人]
 smtp_server SMTP服务器IP
 [SMTP服务器地址]
 smtp_connect_timeout 30
 [SMTP服务器连接超时时间]
 router_id node1
 [标识本节点的字符串，故障发生时，邮件通知会用到，通常为hostname]
 notificationd LVS_DEVEL
}
vrrp_script chk_haproxy {
[keepalived会定时执行脚本并对执行结果进行分析，动态调整vrrp_instance的优先级]
[如果脚本执行结果为0，并且weight配置的值大于0，则优先级相应的增加]
[如果脚本执行结果非0，并且weight配置的值小于0，则优先级相应的减少]
[其他情况，维持原本配置的优先级，即配置文件中priority对应的值]
 script "/etc/keepalived/haproxy_check.sh"
 [检测haproxy状态的脚本路径]
 interval 2
 [检测时间间隔]
 weight 2
 [如果条件成立，权重+2]
}
vrrp_instance <实例名称>{
[定义虚拟路由冗余协议设置]
 state BACKUP
 [全部设置为备用，开启不抢占后，启动时会自动将priority值大的节点选举为MASTER]
 interface eth0
 [节点固有IP(非VIP)的网卡，用来发VRRP包做心跳检测]
 mcast_src_ip 192.168.1.1
 [本机的ip，根据实际情况修改]
 virtual_router_id 249
 [虚拟路由ID，取值在0-255之间，对应的主备ID必须相同，此外同一网段内ID不能重复]
 priority 150
 [取值范围是1-255(范围外会被识别为100)，master的值最好高于其他机器50点]
 nopreempt
 [master必须配置nopreempt，否则配置的非抢占将不会启用]
 advert_int 1
 [检查间隔默认为1秒(即1秒进行一次master选举)，主备设置必须一致]
 authentication {
 [设置验证信息，主备设置必须一样]
  auth_type PASS
  [认证类型，有PASS和HA（IPSEC），推荐使用默认的PASS(密码只识别前8位)]
  auth_pass 1111
  [PASS认证密码]
 }
 virtual_ipaddress {
 [虚拟IP池，主备设置必须一样]
  192.168.1.10/24 dev eth0 label eth0:0
  [虚拟IP(VIP)地址，允许多个，eth0是本机IP网卡，eth0:0是绑定的虚拟IP的网卡]
 }
 track_script {
  chk_haproxy
  [检查HAProxy服务是否存活]
 }
 notify_backup "/etc/init.d/haproxy restart"
 [切换到backup状态时执行的脚本]
 notify_fault "/etc/init.d/haproxy stop"
 [故障时执行的脚本]
}
```
##### 5.2 Slave配置
**配置文件与Mster一致，仅需改变priority、mcast_src_ip、router_id三者的值**  
`vi /etc/keepalived/keepalived.conf`  
```
! Configuration File for keepalived
global_defs {
[keepalived自带的邮件提醒需要开启sendmail服务，建议用独立的监控或第三方SMTP]
 notification_email {
 [指定keepalived在发生切换时需要发送email的对象，一行一个]
  acassen@firewall.loc
  failover@firewall.loc
  sysadmin@firewall.loc
 }
 notification_email_from Alexandre.Cassen@firewall.loc
 [指定发件人]
 smtp_server SMTP服务器IP
 [SMTP服务器地址]
 smtp_connect_timeout 30
 [SMTP服务器连接超时时间]
 router_id node2
 [标识本节点的字符串，故障发生时，邮件通知会用到，通常为hostname]
 notificationd LVS_DEVEL
}
vrrp_script chk_haproxy {
[keepalived会定时执行脚本并对执行结果进行分析，动态调整vrrp_instance的优先级]
[如果脚本执行结果为0，并且weight配置的值大于0，则优先级相应的增加]
[如果脚本执行结果非0，并且weight配置的值小于0，则优先级相应的减少]
[其他情况，维持原本配置的优先级，即配置文件中priority对应的值]
 script "/etc/keepalived/haproxy_check.sh"
 [检测haproxy状态的脚本路径]
 interval 2
 [检测时间间隔]
 weight 2
 [如果条件成立，权重+2]
}
vrrp_instance <实例名称>{
[定义虚拟路由冗余协议设置]
 state BACKUP
 [全部设置为备用，开启不抢占后，启动时会自动将priority值大的节点选举为MASTER]
 interface eth0
 [节点固有IP(非VIP)的网卡，用来发VRRP包做心跳检测]
 mcast_src_ip 192.168.1.2
 [本机的ip，根据实际情况修改]
 virtual_router_id 249
 [虚拟路由ID，取值在0-255之间，对应的主备ID必须相同，此外同一网段内ID不能重复]
 priority 100
 [取值范围是1-255(范围外会被识别为100)，master的值最好高于其他机器50点]
 nopreempt
 [master必须配置nopreempt，否则配置的非抢占将不会启用]
 advert_int 1
 [检查间隔默认为1秒(即1秒进行一次master选举)，主备设置必须一致]
 authentication {
 [设置验证信息，主备设置必须一样]
  auth_type PASS
  [认证类型，有PASS和HA（IPSEC），推荐使用默认的PASS(密码只识别前8位)]
  auth_pass 1111
  [PASS认证密码]
 }
 virtual_ipaddress {
 [虚拟IP池，主备设置必须一样]
  192.168.1.10/24 dev eth0 label eth0:0
  [虚拟IP(VIP)地址，允许多个，eth0是本机IP网卡，eth0:0是绑定的虚拟IP的网卡]
 }
 track_script {
  chk_haproxy
  [检查HAProxy服务是否存活]
 }
 notify_backup "/etc/init.d/haproxy restart"
 [切换到backup状态时执行的脚本]
 notify_fault "/etc/init.d/haproxy stop"
 [故障时执行的脚本]
}
```
##### 5.3 编写HAProxy状态检测脚本
**若haproxy停止运行，则尝试启动，否则杀死本机keepalived进程，将VIP绑定到Slave上**  
`vi /etc/keepalived/haproxy_check.sh`  
```
#!/bin/bash
START_HAPROXY="/etc/rc.d/init.d/haproxy start"
STOP_HAPROXY="/etc/rc.d/init.d/haproxy stop"
LOG_FILE="/home/keepalived/log/haproxy-check.log"
HAPS=`ps -C haproxy --no-header |wc -l`
date "+%Y-%m-%d %H:%M:%S" >> $LOG_FILE
echo "check haproxy status" >> $LOG_FILE
if [ $HAPS -eq 0 ];then
 echo $START_HAPROXY >> $LOG_FILE
 $START_HAPROXY >> $LOG_FILE 2>&1
 sleep 3
 if [ `ps -C haproxy --no-header |wc -l` -eq 0 ];then
 echo "start haproxy failed, killall keepalived" >> $LOG_FILE
 killall keepalived
 fi
fi
```
**[创建脚本日志目录]**  
`mkdir -p /home/keepalived/log/`  
**[赋予脚本执行权限]**  
`chmod +x /etc/keepalived/haproxy_check.sh`  
**[启动keepalived服务]**  
`systemctl start keepalived`  
##### 5.4 功能测试
**HAproxy监控页面会展示获取VIP的机器的haproxy服务的PID**  
1、依次启动Master和Slave的keepalived(启动keepalived后会自动启动haproxy)  
`systemctl start keepalived`  
2、访问http://虚拟IP:8888/admin?stats  
*可以看到HAproxy统计页面的PID是Master机器上HAproxy进程的PID，此时Master获取了VIP*  
3、关闭Master的HAProxy服务  
`systemctl stop haproxy`  
*会看到HAproxy统计页面先是无法访问，然后可以访问，说明状态检测脚本重新启动了HAproxy*  
4、关闭Master的keepalived服务  
`systemctl stop keepalived`  
**刷新http://虚拟IP:8888/admin?stats**  
*可以看到HAproxy统计页面的PID是Slave机器上HAproxy进程的PID，此时Slave获取了VIP*  
**Keepalive高可用配置成功，一台机器宕机后另一台会接管工作**  
5、重新启动Master的keepalived服务  
`systemctl start keepalived`  
**刷新http://虚拟IP:8888/admin?stats**  
*可以看到此时VIP仍在Slave上，keepalived为了避免浪费资源，未配置notify_backup项，不会抢占VIP*  

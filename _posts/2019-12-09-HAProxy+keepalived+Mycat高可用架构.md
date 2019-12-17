---
layout: post
title: HAProxy+keepalived+Mycat高可用架构
categories: [数据库]
tags: [高可用，Mycat，keepalive，HAproxy]
---
#### 1. 安装依赖包
下载[HAproxy源码](https://src.fedoraproject.org/repo/pkgs/haproxy/)并上传到目标服务器。  
`# yum install gcc gcc-c++ pcre pcre-devel zlib zlib-devel openssl openssl-devel -y`  
<!-- more -->
#### 2. 创建安装目录
`# mkdir /home/haproxy`  
#### 3. 编译安装
`# tar -zxvf haproxy-1.5.14.tar.gz`  
`# cd haproxy-1.5.14`  
`# make TARGET=linux2628 ARCH=x86_64 USE_PCRE=1 USE_OPENSSL=1 USE_ZLIB=1 PREFIX=/home/haproxy`  
*TARGET指定内核版本（查看命令uname -r），高于2.6.28的建议设置为linux2628，ARCH指定系统架构。*  
`# make install PREFIX=/home/haproxy`  
`# cd /home/haproxy/`  
#### 4. 创建配置文件
`# mkdir -p /home/haproxy/conf`  
`# mkdir -p /etc/haproxy/`  
`# cp /home/haproxy/examples/haproxy.cfg /home/haproxy/conf/`  
`# ln -s /home/haproxy/conf/haproxy.cfg /etc/haproxy/haproxy.cfg
`# cp -r /home/haproxy/examples/errorfiles/ /home/haproxy/`  
`# ln -s /home/haproxy/errorfiles /etc/haproxy/errorfiles`  
#### 5. 配置自启动服务
`# cp /home/haproxy/examples/haproxy.init /etc/rc.d/init.d/haproxy`  
`# chmod +x /etc/rc.d/init.d/haproxy`  
设置开机启动  
`# chkconfig --add haproxy`  
`# chkconfig haproxy --level 2345 on`  
添加命令脚本软连接  
`# ln -s /home/haproxy/sbin/haproxy /usr/sbin`  
#### 5. 配置Mycat负载均衡集群
##### 5.1 修改haproxy.cfg 配置文件
`# cd /usr/local/haproxy/`  
`# vim conf/haproxy.cfg`  
```
# this config needs haproxy-1.1.28 or haproxy-1.2.1
# global配置中的参数为进程级别的参数，通常与其运行的操作系统有关
global
 log 127.0.0.1 local0 info 
 # 定义全局的syslog服务器，最多可以定义2个,local0是日志设备，对应于/etc/rsyslog.conf中的配置，默认回收info的日志级别
 #log 127.0.0.1 local1 notice
 #log loghost local0 info
 maxconn 4096 
 # 设定每个haproxy进程所接受的最大并发连接数，其等同于命令行选项"-n"，"ulimitn"自动计算的结果正式参照从参数设定的
 chroot /var/lib/haproxy
 # 修改HAProxy的工作目录至指定的目录并在放弃权限之前执行chroot() 操作，可以提升 haproxy 的安全级别
 pidfile /var/run/haproxy.pid 
 # 进程文件（默认路径 /var/run/haproxy.pid）
 user haproxy 
 # 同uid，但这里使用的为用户名
 group haproxy 
 # 同gid，不过这里为指定的用户组名
 #uid 99
 #gid 99
 daemon 
 # 设置haproxy后台守护进程形式运行
 nbproc 1 
 # 指定启动的haproxy进程个数, 只能用于守护进程模式的haproxy；默认为止启动1个进程，一般只在单进程仅能打开少数文件描述符的场中中才使用多进程模式
 stats socket /var/lib/haproxy/stats
 #debug
 #quiet
 node AcroUC-node1 
 # 定义当前节点的名称，用于HA场景中多haproxy进程共享同一个IP地址时
 description AcroUC-node1-haproxy 
 # 当前实例的描述信息
# defaults：用于为所有其他配置段提供默认参数，这默认配置参数可由下一个"defaults"所重新设定
defaults
 log global
 mode tcp 
 # haproxy运行模式（tcp:四层 , http:七层 , health:状态检查,只会返回OK）
 # tcp：实例运行于纯TCP模式，在客户端和服务器端之间将建立一个全双工的连接，且不会对7层报文做任何类型的检查；通常用于SSL、SSH、SMTP等应用；
 # http：实例运行于HTTP模式，客户端请求在转发至后端服务器之前将被深度分析，所有不与RFC格式兼容的请求都会被拒绝；此为默认模式；
 # health：实例运行于health模式，其对入站请求仅响应“OK”信息并关闭连接，且不会记录任何日志信息 ，此模式将用于相应外部组件的监控状态检测请求
 option tcplog
 option dontlognull
 option redispatch 
 # serverId对应的服务器挂掉后,强制定向到其他健康的服务器
 retries 3 
 # 三次连接失败则服务器不用
 #redispatch
 maxconn 2000
 # 前端的最大并发连接数（默认为2000）
 # 其不能用于backend区段，对于大型站点来说，可以尽可能提高此值以便让haproxy管理连接队列，从而避免无法应答用户请求。当然，此最大值不能超过“global”段中的定义。
 # 此外，haproxy会为每个连接维持两个缓冲，每个缓存的大小为8KB，再加上其他的数据，每个连接将大约占用17KB的RAM空间，这意味着经过适当优化后，1GB的可用RAM空间能够维护40000-50000并发连接。
 # 如果指定了一个过大值，极端场景中，其最终所占据的空间可能会超过当前主机的可用内存，这可能会带来意想不到的结果，因此，将其设定一个可接受值放为明智绝对，其默认为2000。
 #contimeout 5000
 #clitimeout 50000
 #srvtimeout 50000
 timeout http-request 10s
 timeout queue 1m
 timeout connect 10s 
 # 连接超时(默认是毫秒,单位可以设置us,ms,s,m,h,d)
 timeout client 1m 
 # 客户端超时
 timeout server 1m 
 # 服务器超时
 timeout http-keep-alive 10s
 timeout check 10s 
 # 心跳检测
listen statistics 
# 配置haproxy状态页（用来查看的页面）
 mode http
 bind *:8888 
 # 绑定端口
 option httplog 
 # 启用日志记录HTTP请求
 stats enable
 stats auth admin:admin 
 # 统计页面用户名和密码设置, 如果要设置多个，另起一行写入即可
 stats uri /admin?stats 
 # 自定义统计页面的URL，默认为/haproxy?stats
 stats hide-version 
 # 隐藏统计页面上HAProxy的版本信息
 stats refresh 30s 
 # 统计页面自动刷新时间
 stats admin if TRUE 
 #如果认证通过就做管理功能，可以管理后端的服务器
 stats realm Hapadmin 
 # 统计页面密码框上提示文本，默认为Haproxy\ Statistics

listen mycat_servers
 bind *:3307 
 # 绑定端口
 mode tcp
 option tcplog 
 # 记录TCP请求日志
 option tcpka 
 # 是否允许向server和client发送keepalive
 option httpchk OPTIONS * HTTP/1.1\r\nHost:\ www 
 # 后端服务状态检测：向后端服务器的48700端口（端口值在后端服务器上通过xinetd配置）发送 OPTIONS 请求，(原理请参考HTTP协议) ，HAProxy会根据返回内容来判断后端服务是否可用（2xx 和 3xx 的响应码表示健康状态，其他响应码或无响应表示服务器故障）。
 balance roundrobin 
 # 基于权重进行轮叫,定义负载均衡算法，可用于"defaults"、"listen"和"backend"中,默认为轮询方式
 server mycat_01 192.168.106.251:8066 check port 48700 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 server mycat_02 192.168.106.252:8066 check port 48700 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 # 格式：server <name> <address>[:[port]] [param*]
 # serser 在后端声明一个server，只能用于listen和backend区段。
 # <name>为此服务器指定的内部名称，其将会出现在日志及警告信息中。
 # <address>此服务器的IPv4地址，也支持使用可解析的主机名，但要在启动时需要解析主机名至响应的IPV4地址。
 # [:[port]]指定将客户端连接请求发往此服务器时的目标端口，此为可选项。
 # [param*]为此server设定的一系列参数，均为可选项，参数比较多，下面仅说明几个常用的参数：
 # weight:权重，默认为1，最大值为256，0表示不参与负载均衡
 # backup:设定为备用服务器，仅在负载均衡场景中的其他server均不可以启用此server
 # check:启动对此server执行监控状态检查，其可以借助于额外的其他参数完成更精细的设定
 # inter:设定监控状态检查的时间间隔，单位为毫秒，默认为2000，也可以使用fastinter和downinter来根据服务器端专题优化此事件延迟
 # rise:设置server从离线状态转换至正常状态需要检查的次数（不设置的情况下，默认值为2）
 # fall:设置server从正常状态转换至离线状态需要检查的次数（不设置的情况下，默认值为3）
 # cookie:为指定server设定cookie值，此处指定的值将会在请求入站时被检查，第一次为此值挑选的server将会被后续的请求所选中，其目的在于实现持久连接的功能
 # maxconn:指定此服务器接受的最大并发连接数，如果发往此服务器的连接数目高于此处指定的值，其将被放置于请求队列，以等待其他连接被释放
```
*注意：多节点部署时node、description的值要做相应调整。*  
##### 5.2 根据以上HAProxy配置文件要求做以下配置
1.添加haproxy用户组和用户  
`# groupadd haproxy`  
`# useradd -g haproxy -s /bin/false haproxy`  
2.创建chroot运行的路径  
`# mkdir /var/lib/haproxy`  
`# chown -R haproxy.haproxy /var/lib/haproxy/`  
`# chown -Rf haproxy.haproxy /usr/local/haproxy/`  
3.防火墙中打开3306端口、3307端口和48800端口
`# vim /etc/sysconfig/iptables`
增加：  
`-A INPUT -m state --state NEW -m tcp -p tcp --dport 3306 -j ACCEPT`  
`-A INPUT -m state --state NEW -m tcp -p tcp --dport 3307 -j ACCEPT`  
`-A INPUT -m state --state NEW -m tcp -p tcp --dport 48800 -j ACCEPT`  
保存后重启防火墙：  
`# service iptables restart`  
##### 5.3 开启rsyslog的haproxy日志记录功能  
默认情况下 haproxy是不记录日志的，如果需要记录日志，还需要配置系统的syslog，在linux系统中是rsyslog服务。syslog服务器可以用作一个网络中的日志监控中心，rsyslog是一个开源工具，被广泛用于Linux系统以通过TCP/UDP协议转发或接收日志消息。  
安装配置rsyslog服务：  
`# yum install rsyslog -y`  
`# vim /etc/rsyslog.conf`  
*把$ModLoad imudp和$UDPServerRun 514前面的 # 去掉。$ModLoad imudp是模块名，支持UDP协议，$UDPServerRun 514允许514端口接收使用UDP和TCP协议转发过来的日志，而rsyslog在默认情况下，正是在514端口监听UDP。*  
*确认GLOBAL DIRECTIVES段中是否有`$IncludeConfig /etc/rsyslog.d/*.conf`，没有则增加上此配置。*  
`# cd /etc/rsyslog.d/`  
`# vim haproxy.conf`  
增加以下内容：  
```
local0.* /var/log/haproxy.log
&~
```
*如果不加上面的的"&~"配置,则除了在/var/log/haproxy.log中写入日志外，也会写入/var/log/message文件中。*  
`# service rsyslog restart`  
*HAProxy服务启动后，就能在/var/log/haproxy.log中看到日志了。*
##### 5.4 配置系统内核的IP包转发功能
`# vim /etc/sysctl.conf`  
使配置生效:  
`# sysctl -p`  
##### 5.5 启动HAProxy
`# service haproxy start`  
`# ps -ef | grep haproxy`  
`# netstat -tnlp | grep haproxy`  
##### 5.6 通过HAProxy连接Mycat
`# mysql -uroot -proot -h192.168.106.251 -P3307`  
##### 5.7 登录HAProxy的状态信息统计页面
浏览器访问`http://192.168.106.251:8888/admin?stats`，用户名和密码都是admin，haproxy.cfg片段配置。  
##### 5.8 配置案例
3.8.1.MySQL代理示例
# this config needs haproxy-1.1.28 or haproxy-1.2.1
# global配置中的参数为进程级别的参数，通常与其运行的操作系统有关
global
 log 127.0.0.1 local0 info # 定义全局的syslog服务器，最多可以定义2个,local0是日志设备，对应
于/etc/rsyslog.conf中的配置，默认回收info的日志级别
 #log 127.0.0.1 local1 notice
 #log loghost local0 info
 maxconn 4096 # 设定每个haproxy进程所接受的最大并发连接数，其等同于命令行选项"-n"，"ulimitn"自动计算的结果正式参照从参数设定的
 chroot /var/lib/haproxy # 修改HAProxy的工作目录至指定的目录并在放弃权限之前执行chroot() 操作，
可以提升 haproxy 的安全级别
 pidfile /var/run/haproxy.pid # 进程文件（默认路径 /var/run/haproxy.pid）
 user haproxy # 同uid，但这里使用的为用户名
 group haproxy # 同gid，不过这里为指定的用户组名
 #uid 99
 #gid 99
 daemon # 设置haproxy后台守护进程形式运行
 nbproc 1 # 指定启动的haproxy进程个数, 只能用于守护进程模式的haproxy；默认为止启动1个进程，
一般只在单进程仅能打开少数文件描述符的场中中才使用多进程模式
 stats socket /var/lib/haproxy/stats
 #debug
 #quiet
 node AcroUC-node1 # 定义当前节点的名称，用于HA场景中多haproxy进程共享同一个IP地址时
 description AcroUC-node1-haproxy ## 当前实例的描述信息
# defaults：用于为所有其他配置段提供默认参数，这默认配置参数可由下一个"defaults"所重新设定
defaults
 log global
 mode tcp # haproxy运行模式（tcp:四层 , http:七层 , health:状态检查,只会返回OK）
 # tcp：实例运行于纯TCP模式，在客户端和服务器端之间将建立一个全双工的连接，且不会对7层报
文做任何类型的检查；通常用于SSL、SSH、SMTP等应用；
 # http：实例运行于HTTP模式，客户端请求在转发至后端服务器之前将被深度分析，所有不与RFC
格式兼容的请求都会被拒绝；此为默认模式；
 # health：实例运行于health模式，其对入站请求仅响应“OK”信息并关闭连接，且不会记录任何日志
信息 ，此模式将用于相应外部组件的监控状态检测请求
 option tcplog
 option dontlognull
 option redispatch # serverId对应的服务器挂掉后,强制定向到其他健康的服务器
 retries 3 # 三次连接失败则服务器不用
 #redispatch
 maxconn 2000 # 前端的最大并发连接数（默认为2000）
 # 其不能用于backend区段，对于大型站点来说，可以尽可能提高此值以便让haproxy管理连接队列，
从而避免无法应答用户请求。当然，此最大值不能超过“global”段中的定义。
 # 此外，需要留心的是，haproxy会为每个连接维持两个缓冲，每个缓存的大小为8KB，再加上其他
的数据，每个连接将大约占用17KB的RAM空间，这意味着经过适当优化后 ，
 # 有着1GB的可用RAM空间时将维护40000-50000并发连接。
 # 如果指定了一个过大值，极端场景中，其最终所占据的空间可能会超过当前主机的可用内存，
 # 这可能会带来意想不到的结果，因此，将其设定一个可接受值放为明智绝对，其默认为2000
 #contimeout 5000
 #clitimeout 50000
 #srvtimeout 50000
 timeout http-request 10s
 timeout queue 1m
 timeout connect 10s # 连接超时(默认是毫秒,单位可以设置us,ms,s,m,h,d)
 timeout client 1m # 客户端超时
 timeout server 1m # 服务器超时
 timeout http-keep-alive 10s
 timeout check 10s # 心跳检测
listen statistics #配置haproxy状态页（用来查看的页面）
 mode http
 bind *:8888 # 绑定端口
 option httplog # 启用日志记录HTTP请求
 stats enable
 stats auth admin:admin # 统计页面用户名和密码设置, 如果要设置多个，另起一行写入即可
 stats uri /admin?stats # 自定义统计页面的URL，默认为/haproxy?stats
 stats hide-version # 隐藏统计页面上HAProxy的版本信息
 stats refresh 30s # 统计页面自动刷新时间
 stats admin if TRUE #如果认证通过就做管理功能，可以管理后端的服务器
 stats realm Hapadmin # 统计页面密码框上提示文本，默认为Haproxy\ Statistics

frontend mysql
 bind *:3306 #使用3306端口。监听前端端口【表示任何ip访问3306端口都会将数据轮番转发到mysql
服务器群组中】
 mode tcp
 log global
 default_backend mysqlservers
backend mysqlservers
 balance roundrobin # 基于权重进行轮叫,定义负载均衡算法，可用于"defaults"、"listen"和"backend"中,
默认为轮询方式
 server mysql1 192.168.106.251:3306 check port 3306 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 server mysql2 192.168.106.252:3306 check port 3306 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
 server mysql3 192.168.106.253:3306 check port 3306 inter 2000ms rise 2 fall 3 weight 10 maxconn 400
3.8.2.Http代理示例
前端调度器IP：192.168.1.210
后端应用服务器IP: 192.168.1.111 和 192.168.1.112
#---------------------------------------------------------------------
# Global settings
#---------------------------------------------------------------------
global
# to have these messages end up in /var/log/haproxy.log you will
# need to:
#
# 1) configure syslog to accept network log events. This is done
# by adding the '-r' option to the SYSLOGD_OPTIONS in
# /etc/sysconfig/syslog
#
# 2) configure local2 events to go to the /var/log/haproxy.log
# file. A line like the following can be added to
# /etc/sysconfig/syslog
#
# local2.* /var/log/haproxy.log
#
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
stats uri /haproxyadmin?stats
stats realm Haproxy\ Statistics
stats auth admin:admin
stats admin if TRUE
frontend http-in
bind *:80
mode http
log global
option httpclose
option logasap #不等待响应结束就记录日志，表示提前记录日志，一般日志会记录响应时长，此不记录
响应时长
option dontlognull #不记录空信息
capture request header Host len 20 #记录请求首部的前20个字符
capture request header Referer len 60 #referer跳转引用，就是上一级
default_backend servers
frontend healthcheck
bind :1099 #定义外部检测机制
mode http
option httpclose
option forwardfor
default_backend servers
backend servers
balance roundrobin
server websrv1 192.168.1.111:80 check maxconn 2000
server websrv2 192.168.1.112:80 check maxconn 2000
3.8.3.HAProxy统计页面的输出机制
listen statistics
bind *:8009 # 自定义监听端口
stats enable # 启用基于程序编译时默认设置的统计报告
stats auth admin:admin # 统计页面用户名和密码设置
stats uri /admin?stats # 自定义统计页面的URL，默认为/haproxy?stats
stats hide-version # 隐藏统计页面上HAProxy的版本信息
stats refresh 30s # 统计页面自动刷新时间
stats admin if TRUE #如果认证通过就做管理功能，可以管理后端的服务器
stats realm Hapadmin # 统计页面密码框上提示文本，默认为Haproxy\ Statistics
3.8.4.动静分离示例
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
server node1 192.168.1.111:80 check maxconn 3000
backend dynamic
balance roundrobin
server node2 192.168.1.112:80 check maxconn 1000
3.8.5.定义独立日志
[root@AcroUC-node1 haproxy]# vim /etc/rsyslog.conf #为其添加日志功能
[root@AcroUC-node1 haproxy]# vim /etc/rsyslog.d/haproxy.conf #添加HAProxy日志功能
local0.* /var/log/haproxy.log
&~
[root@AcroUC-node1 haproxy]# service rsyslog restart
[root@AcroUC-node1 haproxy]# vim conf/haproxy.cfg #定义HAProxy日志功能
log 127.0.0.1 local0 --------->在global端中添加此行
四、Keepalived介绍
Keepalived是一个免费开源的，用C编写的类似于layer3, 4 & 7交换机制软件，具备我们平时说的第3层、
第4层和第7层交换机的功能。主要提供loadbalancing（负载均衡）和 high-availability（高可用）功能，
负载均衡实现需要依赖Linux的虚拟服务内核模块（ipvs），而高可用是通过VRRP协议实现多台机器之
间的故障转移服务。
上图是Keepalived的功能体系结构，大致分两层：用户空间（user space）和内核空间（kernel space）。
内核空间：主要包括IPVS（IP虚拟服务器，用于实现网络服务的负载均衡）和NETLINK（提供高级路由
及其他相关的网络功能，如果在负载均衡器上启用netfilter/iptable，将会直接影响它的性能。）两个部
份。
用户空间：
WatchDog：负载监控checkers和VRRP进程的状况
VRRP Stack：负载负载均衡器之间的失败切换FailOver，如果只用一个负载均稀器，则VRRP不是必
须的。
Checkers：负责真实服务器的健康检查healthchecking，是keepalived最主要的功能。换言之，可以没
有VRRP Stack，但健康检查healthchecking是一定要有的。
IPVS wrapper：用户发送设定的规则到内核ipvs代码
Netlink Reflector：用来设定vrrp的vip地址等。
keepalived也是模块化设计，不同模块复杂不同的功能，下面是keepalived的组件
core check vrrp libipfwc libipvs-2.4 libipvs-2.6
core：是keepalived的核心，复杂主进程的启动和维护，全局配置文件的加载解析等
check：负责healthchecker(健康检查)，包括了各种健康检查方式，以及对应的配置的解析包括LVS的配置
解析
vrrp：VRRPD子进程，VRRPD子进程就是来实现VRRP协议的
libipfwc：iptables(ipchains)库，配置LVS会用到
libipvs*：配置LVS会用到
注意，keepalived和LVS完全是两码事，只不过他们各负其责相互配合而已
Keepalived正常运行时，会启动3个进程，分别是core、check和vrrp。
vrrp模块是来实现VRRP协议的；
check负责健康检查，包括常见的各种检查方式；
core模块为keepalived的核心，负责主进程的启动、维护以及全局配置文件的加载和解析。
由上图可知，两个子进程都被系统WatchDog看管，两个子进程各自复杂自己的事，healthchecker子进程复
杂检查各自服务器的健康程度，例如HTTP，LVS等等，如果healthchecker子进程检查到MASTER上服务
不可用了，就会通知本机上的兄弟VRRP子进程，让他删除通告，并且去掉虚拟IP，转换为BACKUP状态
Keepalived的所有功能是配置keepalived.conf文件来实现的。
五.Keepalived安装
5.1.安装编译所需的依赖包
[root@AcroUC-node1 mysql]# yum install gcc gcc-c++ openssl openssl-devel -y
5.2.编译安装
解压：
[root@AcroUC-node1 mysql]# tar -zxvf keepalived-1.2.19.tar.gz
配置：
[root@AcroUC-node1 mysql]# cd keepalived-1.2.19
[root@AcroUC-node1 keepalived-1.2.19]# ./configure --prefix=/usr/local/keepalived --with-kerneldir=/usr/src/kernels/2.6.32-573.el6.x86_64/
--with-kernel-dir这个选项根据自己的linux版本进行填写（在linux中使用命令uname –r可以查到）,如果
keepalived的./configure输出Use IPVS Framework 为No 则使用keepalived启用后将无法条用ipvsadm，所以
安装时需要指定kernel
安装：
[root@AcroUC-node1 keepalived-1.2.19]# make
[root@AcroUC-node1 keepalived-1.2.19]# make install
5.3.创建配置文件
安装完成后，需要将keepalived相应的配置文件拷贝到系统相应的目录当中。keepalived启动时会
从/etc/keepalived目录下查找keepalived.conf配置文件，如果没有找到则使用默认的配置。/etc/keepalived目
录安装时默认是没有安装的，需要手动创建。配置文件目录结构如下所示：
[root@AcroUC-node1 keepalived-1.2.19]# cd /usr/local/keepalived/
[root@AcroUC-node1 keepalived]# tree -l etc/
分别对应系统目录（忽略samples目录）：
/etc/keepalived/keepalived.conf
/etc/rc.d/init.d/keepalived
/etc/sysconfig/keepalived
将配置文件拷贝到系统对应的目录下：
[root@AcroUC-node1 keepalived]# mkdir /etc/keepalived
[root@AcroUC-node1 keepalived]# cp /usr/local/keepalived/etc/keepalived/keepalived.conf
/etc/keepalived/keepalived.conf
[root@AcroUC-node1 keepalived]# cp /usr/local/keepalived/etc/rc.d/init.d/keepalived /etc/rc.d/init.d/keepalived
[root@AcroUC-node1 keepalived]# cp /usr/local/keepalived/etc/sysconfig/keepalived /etc/sysconfig/keepalived
[root@AcroUC-node1 keepalived]# ln -s /usr/local/keepalived/sbin/keepalived /usr/sbin/
[root@AcroUC-node1 keepalived]# ln -s /usr/local/keepalived/sbin/keepalived /sbin/
5.4.配置自启动服务
[root@AcroUC-node1 keepalived]# chkconfig --add keepalived
[root@AcroUC-node1 keepalived]# chkconfig keepalived --level 2345 on
六、Keepalived配置HAProxy高可用
6.1.Master配置
注意：去掉注释
[root@AcroUC-node1 keepalived]# vim /etc/keepalived/keepalived.conf
! Configuration File for keepalived
global_defs {
## keepalived 自带的邮件提醒需要开启 sendmail 服务。建议用独立的监控或第三方 SMTP
 notification_email {
acassen@firewall.loc # 指定keepalived在发生切换时需要发送email到的对象，一行一个
failover@firewall.loc
sysadmin@firewall.loc
 }
 notification_email_from Alexandre.Cassen@firewall.loc # 指定发件人
 smtp_server 192.168.200.1 # smtp 服务器地址
 smtp_connect_timeout 30 # smtp 服务器连接超时时间
 router_id AcroUC-node1 # 标识本节点的字符串,通常为hostname,但不一定非得是hostname,故障发生时,
邮件通知会用到
 notificationd LVS_DEVEL
}
## keepalived 会定时执行脚本并对脚本执行的结果进行分析，动态调整 vrrp_instance 的优先级。
## 如果脚本执行结果为 0，并且 weight 配置的值大于 0，则优先级相应的增加。
## 如果脚本执行结果非 0，并且 weight 配置的值小于 0，则优先级相应的减少。
## 其他情况，维持原本配置的优先级，即配置文件中 priority 对应的值。
vrrp_script chk_haproxy {
 script "/etc/keepalived/haproxy_check.sh" ## 检测 haproxy 状态的脚本路径
 interval 2 ## 检测时间间隔
 weight 2 ## 如果条件成立，权重+2
}
## 定义虚拟路由冗余协议设置， VI_haproxy为虚拟路由的标示符，自己定义名称
vrrp_instance VI_haproxy{ # 实例名称
 state BACKUP # 可以是MASTER或BACKUP，不过当其他节点keepalived启动时会将priority比较大的
节点选举为MASTER，默认主设备（priority 值大的）和备用设备（priority 值小的）都设置
为 # BACKUP，设置为开启不抢占，所以都设置为备用
 interface eth0 # 节点固有IP（非VIP）的网卡，用来发VRRP包做心跳检测
 mcast_src_ip 192.168.106.251 #本机的ip，需要修改
 virtual_router_id 249 # 虚拟路由ID,取值在0-255之间,用来区分多个instance的VRRP组播,同一网段内ID
不能重复;主备设置必须一样 ,相同的 VRID 为一个组，他将决定多播的 MAC 地址
 priority 150 # 用来选举master的,要成为master那么这个选项的值最好高于其他机器50个点,该项取值范
围是1-255(在此范围之外会被识别成默认值100)，开启了不抢占，所以此处优先级必须于 #另
一台
 nopreempt # 主设备（priority 值大的）配置一定要加上 nopreempt，否则非抢占也不起作用
 advert_int 1 # 检查间隔默认为1秒,即1秒进行一次master选举(可以认为是健康查检时间间隔);主备设置
必须一样
 ## 设置验证信息，主备设置必须一样
 authentication { # 认证区域,认证类型有PASS和HA（IPSEC）,推荐使用PASS(密码只识别前8位);
 auth_type PASS # 默认是PASS认证，有PASS 和 AH 两种
 auth_pass 1111 # PASS认证密码
 }
 ## 虚拟 IP 池, 主备设置必须一样
 virtual_ipaddress {
 192.168.106.249/24 dev eth0 label eth0:0 # 虚拟VIP地址,允许多个
 }
 ## 将 track_script 块加入 instance 配置块
 track_script {
 chk_haproxy ## 检查 HAProxy 服务是否存活
 }
 notify_backup "/etc/init.d/haproxy restart" #表示当切换到backup状态时,要执行的脚本
 notify_fault "/etc/init.d/haproxy stop" #故障时执行的脚本
}
6.2.Slave配置
注意：去掉注释
配置文件与上面的几乎一样，仅仅改变priority 100【只需要比上面的小即可】、mcast_src_ip
192.168.106.252、router_id AcroUC-node2
6.3.编写 HAProxy 状态检测脚本
根据keepalived.conf中的配置，脚本文件地址为/etc/keepalived/haproxy_check.sh
脚本要求：如果 haproxy 停止运行，尝试启动，如果无法启动则杀死本机的 keepalived 进程， keepalied将
虚拟 ip 绑定到 BACKUP 机器上。内容如下：
[root@AcroUC-node1 keepalived]# vim /etc/keepalived/haproxy_check.sh
#!/bin/bash
START_HAPROXY="/etc/rc.d/init.d/haproxy start"
STOP_HAPROXY="/etc/rc.d/init.d/haproxy stop"
LOG_FILE="/usr/local/keepalived/log/haproxy-check.log"
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
[root@AcroUC-node1 keepalived]# mkdir -p /usr/local/keepalived/log
给脚本赋执行权限
[root@AcroUC-node1 keepalived]# chmod +x /etc/keepalived/haproxy_check.sh
启动keepalived：
[root@AcroUC-node1 keepalived]# service keepalived start
[root@AcroUC-node1 keepalived]# ps aux | grep keepalived
6.4.功能测试
可以通过haproxy监控页面获知谁获取了vip
1、依次启动Master(192.168.106.251)、Slave(192.168.106.252)的keepalived、haproxy（启动keepalived后将
会自动开启haproxy）
Master(192.168.106.251)：
Slave(192.168.106.252)：
2、访问http://192.168.106.249:8888/admin?stats
Master(192.168.106.251)获取了VIP
3、关闭Master(192.168.106.251)的keepalived和HAProxy服务
[root@AcroUC-node1 keepalived]# service haproxy stop
[root@AcroUC-node1 keepalived]# service keepalived stop
刷新http://192.168.106.249:8888/admin?stats
Slave(192.168.106.252)获取了vip，机器正常工作
结果：证明了高可用，挂了一台另一台继续工作
4、重新启动Master(192.168.106.251)的haproxy以及keepalived
[root@AcroUC-node1 keepalived]# service keepalived start
刷新http://192.168.106.249:8888/admin?stats
结果：此时vip回到Master(192.168.106.251)手中，
如果在keepalived.conf中不配置notify_backup项，则可以证明了keepalived配置了不抢占vip，不必浪费资源
去获取vip。
主备切换时执行的脚本
在keepalived.conf配置文件中加入以下内容
1.notify_master“想要执行的脚本路径” #表示当切换到master状态时，要执行的脚本
2.notify_backup “想要执行的脚本路径”#表示当切换到backup状态时，要执行的脚本
3.notify_fault“想要执行的脚本路径”#表示切换出现故障时要执行的脚本



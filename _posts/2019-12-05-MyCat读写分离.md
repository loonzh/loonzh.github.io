---
layout: post
title: MyCat读写分离
categories: [数据库]
tags: [MyCat]
---
**MyCat在MySQL主从复制基础上实现读写分离**  
#### 1. 安装要求
MyCat是基于java开发的，首先确报已经安装了jdk1.7或以上版本的jdk,并设置好环境变量。  
#### 2. 解压安装
`# tar -zxvf Mycat-server-1.5.1-RELEASE-20161130213509-linux.tar.gz`  
`# mv mycat/ /usr/local/mycat/`  
#### 3. 创建MyCat用户和组并设置权限
`# groupadd mycat`  
`# adduser -r -g mycat -s /bin/false mycat`  
`# chown -R mycat.mycat /usr/local/mycat`  
#### 4. 设置MyCat的环境变量
`# vim /etc/profile`  
在文件末尾添加如下内容：  
`export MYCAT_HOME=/usr/local/mycat`  
`export PATH=$PATH:$MYCAT_HOME/bin`  
使生效：  
`# source /etc/profile`  
#### 4. 配置MyCat
**在配置MyCat前，请确认MySQL的主从复制安装配置已完成并正常运行，MyCat不负责数据同步的问题。**  
补充：  
```
(1) MySQL主从复制配置中，如果涉及到函数或存储过程的同步复制，需要在/etc/my.cnf中的[mysqld]段中增加配置log_bin_trust_function_creators=true 或在客户端中设置 set global log_bin_trust_function_creators = 1;
(2) 如果要做读写分离下的主从切换，那么从节点也有可能会变为写节点，因此从节点就不能设置为只读 read_only=1 。
(3) Linux版本的MySQL，需要设置为MySQL大小写不敏感，否则使用MyCAT的时候会提示找不到表的错误。可在/etc/my.cnf的[mysqld]段中增加lower_case_table_names=1 。
```
7.3.1.配置schema.xml
schema.xml是MyCat最重要的配置文件之一，用于设置MyCat的逻辑库、表、数据节点、dataHost等内容，
[root@AcroUC-node1 mysql]# cd /usr/local/mycat/
--bin 启动目录，Linux下运行：./mycat console,首先要chmod +x *注：mycat支持的命令{ console | start | stop | restart | status |
dump }
--conf 配置文件存放配置文件：
 --server.xml：是Mycat服务器参数调整和用户授权的配置文件。
 --schema.xml：是逻辑库定义和表以及分片定义的配置文件。
 --rule.xml： 是分片规则的配置文件，分片规则的具体一些参数信息单独存放为文件，也在这个目录下，配置文件修改需要
重启MyCAT或者通过9066端口reload. 。
 --log4j.xml： 日志存放在logs/log中，每天一个文件，日志的配置是在conf/log4j.xml中，根据自己的需要可以调整输出级别为
debug debug级别下，会输出更多的信息，方便排查问题。
 --autopartition-long.txt,partition-hash-int.txt,sequence_conf.properties， sequence_db_conf.properties 分片相关的id分片规则配置
文件
--lib MyCAT自身的jar包或依赖的jar包的存放目录。
--logs MyCAT日志的存放目录。日志存放在logs/log中，每天一个文件
[root@AcroUC-node1 mycat]# vim conf/schema.xml
<?xml version="1.0"?>
<!DOCTYPE mycat:schema SYSTEM "schema.dtd">
<mycat:schema xmlns:mycat="http://org.opencloudb/">
 <!-- schema定义Mycat的实际逻辑库的位置，多个schema代表多个逻辑库,逻辑库的概念与MySQL中的 database 概念相同 --
>
 <!-- 其中checkSQLschema表明是否检查并过滤SQL中包含schema的情况，如逻辑库为 TESTDB，则可能写为select * from
TESTDB.edu_user，此时会自动过滤TESTDB，SQL变为select * from edu_user，若不会出现上述写法，则可以关闭属性为
false -->
 <!--sqlMaxLimit默认返回的最大记录数限制，例如设置值为100，执行**select * from TESTDB.travelrecord;**的效果为和
执行**select * from TESTDB.travelrecord limit 100;**相同。MyCat1.4版本里面，用户的Limit参数会覆盖掉MyCat的
sqlMaxLimit默认设置-->
 <schema name="ict" checkSQLschema="false" dataNode="ict"></schema>
 <schema name="file_server" checkSQLschema="false" dataNode="file_server"></schema>
 <schema name="im_server" checkSQLschema="false" dataNode="im_server"></schema>

 <!-- dataNode定义Mycat的数据节点,dataNode是逻辑库对应的分片，一个dataNode标签就是一个独立的数据分片。如果配
置多个分片只需要多个dataNode即可 -->
 <!-- dataNode 中的 name 数据表示节点名称， dataHost表示数据主机名称， database表示该节点要路由的数据库的名称 -->
 <dataNode name="ict" dataHost="host" database="ict" />
 <dataNode name="file_server" dataHost="host" database="file_server" />
 <dataNode name="im_server" dataHost="host" database="im_server" />

 <!-- dataHost配置的是实际的后端数据库集群（当然，也可以是非集群） -->
 <!-- 注意：schema中的每一个dataHost中的host属性值必须唯一，否则会出现主从在所有dataHost中全部切换的现象 -->
 <!-- 定义数据主机host，只连接到MySQL读写分离集群中的Master节点，不使用MyCat托管MySQL主从切换 -->
 <!--
 <dataHost name="host" maxCon="500" minCon="20" balance="0"
 writeType="0" dbType="mysql" dbDriver="native" switchType="1" slaveThreshold="100">
 <heartbeat>select user()</heartbeat>
 <writeHost host="hostM1" url="192.168.106.251:3306" user="root" password="root" />
 </dataHost>
 -->
 <!-- 使用MyCat托管MySQL主从切换 -->
 <!-- 定义数据主机host，连接到MySQL读写分离集群，并配置了读写分离和主从切换 -->
 <dataHost name="host" maxCon="500" minCon="20" balance="1"
 writeType="0" dbType="mysql" dbDriver="native" switchType="2" slaveThreshold="100">
 <!-- 通过show slave status检测主从状态，当主宕机以后，发生切换，从变为主，原来的主变为从，这时候show slave
status就会发生错误，因为原来的主没有开启slave，不建议直接使用switch操作，而是在DB中做主从对调。 -->
 <heartbeat>show slave status</heartbeat>
 <!-- can have multi write hosts -->
 <writeHost host="hostM1" url="192.168.106.251:3306" user="root" password="root" />
 <writeHost host="hostM2" url="192.168.106.252:3306" user="root" password="root" />
 <!-- can have multi read hosts -->
 <writeHost host="hostS1" url="192.168.106.253:3306" user="root" password="root" />
 </dataHost>
 <!--
 name属性:唯一标识dataHost标签，供上层的标签使用。
 maxCon属性:指定每个读写实例连接池的最大连接。
 minCon属性:指定每个读写实例连接池的最小连接，初始化连接池的大小。
 balance属性:负载均衡类型，目前的取值有3种：
 1. balance="0", 不开启读写分离机制，所有读操作都发送到当前可用的writeHost上。
 2. balance="1"，全部的readHost与stand by writeHost参与select语句的负载均衡，简单的说，当双主双从模式(M1-
>S1，M2->S2，并且M1与 M2互为主备)，正常情况下，M2,S1,S2都参与select语句的负载均衡。
 3. balance="2"，所有读操作都随机的在writeHost、readhost上分发。
 4. balance="3"，所有读请求随机的分发到wiriteHost对应的readhost执行，writeHost不负担读压力，注意balance=3只在
1.4及其以后版本有，1.3没有。
 writeType属性:负载均衡类型，目前的取值有3种：
 1. writeType="0", 所有写操作发送到配置的第一个writeHost，第一个挂了切到还生存的第二个writeHost，重新启动后
已切换后的为准，切换记录在配置文件中:dnindex.properties .
 2. writeType="1"，所有写操作都随机的发送到配置的writeHost，1.5以后废弃不推荐,因为仅仅对于galera for mysql集
群这种多主多节点都能写入的集群起效，此时Mycat会随机选择一个writeHost并写入数据，对于非galera for mysql集群，请
不要配置writeType=1，会导致数据库不一致的严重问题。
 switchType属性:
 -1 表示不自动切换。
 1 默认值，自动切换。
 2 基于MySQL主从同步的状态决定是否切换。心跳语句为 show slave status
 3 基于MySQL galary cluster的切换机制（适合集群）（1.4.1） 心跳语句为 show status like ‘wsrep%’
 dbType属性:指定后端连接的数据库类型，目前支持二进制的mysql协议，还有其他使用JDBC连接的数据库。例如：
mongodb、oracle、spark等。
 dbDriver属性:指定连接后端数据库使用的Driver，目前可选的值有native和JDBC。使用native的话，因为这个值执行的
是二进制的mysql协议，所以可以使用mysql和maridb。其他类型的数据库则需要使用JDBC驱动来支持。
从1.6版本开始支持postgresql的native原始协议。
如果使用JDBC的话需要将符合JDBC 4标准的驱动JAR包放到MYCAT\lib目录下，并检查驱动JAR包中包括如下目录结构的
文件：META-INF\services\java.sql.Driver。在这个文件内写上具体的Driver类名，例如：com.mysql.jdbc.Driver。
 tempReadHostAvailable属性:如果配置了这个属性writeHost 下面的readHost仍旧可用，默认0 可配置（0、1）。


 -->
 <!--
 heartbeat标签:这个标签内指明用于和后端数据库进行心跳检查的语句。1.4主从切换的语句必须是：show slave status
 -->
 <!--
 writeHost标签、readHost标签:这两个标签都指定后端数据库的相关配置给mycat，用于实例化后端连接池。唯一不同
的是，writeHost指定写实例、readHost指定读实例，组着这些读写实例来满足系统的要求。在一个dataHost内可以定义多个
writeHost和readHost。但是，如果writeHost指定的后端数据库宕机，那么这个writeHost绑定的所有readHost都将不可用。另
一方面，由于这个writeHost宕机系统会自动的检测到，并切换到备用的writeHost上去。
 host属性:用于标识不同实例，一般writeHost我们使用*M1，readHost我们用*S1。
 url属性:后端实例连接地址，如果是使用native的dbDriver，则一般为address:port这种形式。用JDBC或其他的dbDriver，
则需要特殊指定。当使用JDBC时则可以这么写：jdbc:mysql://localhost:3306/。
 user属性:后端存储实例需要的用户名字
 password属性:后端存储实例需要的密码。
 weight 属性:权重 配置在readhost 中作为读节点的权重（1.4以后）。
 usingDecrypt 属性:是否对密码加密默认0 否如需要开启配置1，同时使用加密程序对密码加密，加密命令为：执行mycat
jar 程序（1.4.1以后）： java -cp Mycat-server-1.4.1-dev.jar io.mycat.util.DecryptUtil 1:host:user:password Mycat-server-1.4.1-
dev.jar 为mycat download 下载目录的jar 1:host:user:password 中 1 为db端加密标志，host为dataHost 的host 名称
 -->
</mycat:schema>
根据主从延时切换：
1.4开始支持MySQL主从复制状态绑定的读写分离机制，让读更加安全可靠，配置如下： MyCAT心跳检查语句配置为 show
slave status ，dataHost 上定义两个新属性： switchType="2" 与 slaveThreshold="100"，此时意味着开启MySQL主从复制状态绑
定的读写分离与切换机制，Mycat心跳机制通过检测 show slave status 中的 "Seconds_Behind_Master", "Slave_IO_Running",
"Slave_SQL_Running" 三个字段来确定当前主从同步的状态以及Seconds_Behind_Master主从复制时延， 当
Seconds_Behind_Master>slaveThreshold时，读写分离筛选器会过滤掉此Slave机器，防止读到很久之前的旧数据，而当主节点宕
机后，切换逻辑会检查Slave上的Seconds_Behind_Master是否为0，为0时则表示主从同步，可以安全切换，否则不会切换。
7.3.2.配置 server.xml
server.xml 主要用于设置系统变量、管理用户、设置用户权限等。
[root@AcroUC-node1 mycat]# vim conf/server.xml
<?xml version="1.0" encoding="UTF-8"?>
<!-- - - Licensed under the Apache License, Version 2.0 (the "License");
 - you may not use this file except in compliance with the License. - You
 may obtain a copy of the License at - - http://www.apache.org/licenses/LICENSE-2.0
 - - Unless required by applicable law or agreed to in writing, software -
 distributed under the License is distributed on an "AS IS" BASIS, - WITHOUT
 WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. - See the
 License for the specific language governing permissions and - limitations
 under the License. -->
<!DOCTYPE mycat:server SYSTEM "server.dtd">
<mycat:server xmlns:mycat="http://org.opencloudb/">
 <system>
 <!--由于mycat最初是时候Foundation DB的sql解析器，而后才添加的Druid的解析器。所以这个属性用来指定默认的解析
器。目前的可用的取值有：druidparser和 fdbparser。使用的时候可以选择其中的一种，目前一般都使用druidparser。
 1.3 解析器默认为fdbparser，1.4默认为druidparser，1.4以后fdbparser作废。-->
 <property name="defaultSqlParser">druidparser</property>
 <!--字符集设置,如果需要配置utf8mb2等特殊字符集可以在 index_to_charset.properties 配置中 配置数据库短的字符集ID=
字符集 例如： 224=utf8mb4 配置字符集的时候一定要坚持mycat的字符集与数据库端的字符集是一致的，可以通过变量来查
询： show variables like 'collation_%'; show variables like 'character_set_%';-->
 <property name="charset">utf8mb4</property>
 <!--<property name="useCompression">1</property>--> <!--1为开启mysql压缩协议-->
 <!--<property name="processorBufferChunk">40960</property> -->
 <!--
 <property name="processors">1</property>
 <property name="processorExecutor">32</property>
 -->
 <!--默认是65535 64K 用于sql解析时最大文本长度 -->
 <!--<property name="maxStringLiteralLength">65535</property>-->
 <!--<property name="sequnceHandlerType">0</property>-->
 <!--<property name="backSocketNoDelay">1</property>-->
 <!--<property name="frontSocketNoDelay">1</property>-->
 <!--<property name="processorExecutor">16</property>-->
 <!--
 <property name="mutiNodeLimitType">1</property> 0：开启小数量级（默认） ；1：开启亿级数据排序
 <property name="mutiNodePatchSize">100</property> 亿级数量排序批量
 <property name="processors">32</property> <property name="processorExecutor">32</property>
 <property name="serverPort">8066</property> <property name="managerPort">9066</property>
 <property name="idleTimeout">300000</property> <property name="bindIp">0.0.0.0</property>
 <property name="frontWriteQueueSize">4096</property> <property name="processors">32</property> -->
 </system>
 <!-- 用户root，对应的MyCat逻辑库连接到的数据节点对应的主机为主从复制集群，并通过MyCat实现了读写分离 -->
 <!--name属性来指定用户名-->
 <user name="root">
 <!--password属性内的文本来修改密码-->
 <property name="password">root</property>
 <!--schemas属性内的文本来控制用户可放问的schema，同时访问多个schema的话使用 , 隔开-->
 <property name="schemas">ict,im_server,file_server</property>
 </user>
 <user name="admin">
 <property name="password">admin</property>
 <property name="schemas">ict,im_server,file_server</property>
 <!--readOnly属性为true 或false来限制用户是否只是可读的-->
 <property name="readOnly">true</property>
 <!--
 benchmark属性:mycat连接服务降级处理： benchmark 基准, 当前端的整体connection数达到基准值是, 对来自该账户的
请求开始拒绝连接，0或不设表示不限制
 例如 <property name="benchmark">1000</property>
 -->
 <!--
 usingDecrypt 属性:是否对密码加密默认0 否 如需要开启配置1，同时使用加密程序对密码加密，加密命令为：
 执行mycat jar 程序： java -cp Mycat-server-1.4.1-dev.jar io.mycat.util.DecryptUtil 0:user:password
 Mycat-server-1.4.1-dev.jar 为mycat download 下载目录的jar
 1:host:user:password 中 0 为前端加密标志
 -->
 <!-- 表级权限: Table级的dml(curd)控制，未设置的Table继承schema的dml -->
 <!-- TODO: 非CURD SQL语句, 透明传递至后端 -->
 <!--
 <privileges check="true">
 <schema name="TESTDB" dml="0110" >
 <table name="table01" dml="0111"></table>
 <table name="table02" dml="1111"></table>
 </schema>
 <schema name="TESTDB1" dml="0110">
 <table name="table03" dml="1110"></table>
 <table name="table04" dml="1010"></table>
 </schema>
 </privileges>
 -->
 <!--
 privileges子节点:对用户的schema及 下级的 table 进行精细化的 DML 权限控制，privileges 节点中的 check 属性是用
于标识是否开启 DML 权限检查， 默认false 标识不检查，当然 privileges 节点不配置，等同 check=false,
由于Mycat一个用户的schemas 属性可配置多个schema ，所以privileges 的下级节点schema 节点同样
可配置多个，对多库多表进行细粒度的DML 权限控制
 参数 说明 事例（禁止增删改查）
 dml insert,update,select,delete 0000
 注： 设置了 schema , 但只设置了个别 table 或 未设置table 的 DML，自动继承 schema 的 DML 属性
 -->

 </user>
 <!--
 <quarantine>
 <whitehost>
 <host host="127.0.0.1" user="mycat"/>
 <host host="127.0.0.2" user="mycat"/>
 </whitehost>
 <blacklist check="false"></blacklist>
 </quarantine>
 -->
</mycat:server>
7.3.3.从节点配置
只需调换一下writehost中节点顺序，其他与主节点保持一样。
7.4.防火墙配置（如果开启）
MyCat的默认数据端口为8066，mycat通过这个端口接收数据库客户端的访问请求。
管理端口为9066，用来接收mycat监控命令、查询mycat运行状况、重新加载配置文件等。
[root@AcroUC-node1 mycat]# vim /etc/sysconfig/iptables
增加：
-A INPUT -m state --state NEW -m tcp -p tcp --dport 8066 -j ACCEPT
-A INPUT -m state --state NEW -m tcp -p tcp --dport 9066 -j ACCEPT
保存后重启防火墙：
[root@AcroUC-node1 mycat]# service iptables restart
7.5.修改日志级别
修改log日志级别为debug，以便通过日志确认基于MyCat的MySQL数据库集群读写分离的数据操作状态（可以在正式上生产前
改成info级别）
[root@AcroUC-node1 mycat]# vim conf/log4j.xml
7.6.启动/停止
(1)控制台启动，这种启动方式在控制台关闭后，root服务也将关闭，适合调试使用：
[root@AcroUC-node1 mycat]# mycat console
(2) 可以采用以下后台启动的方式：
[root@AcroUC-node1 mycat]# mycat start
[root@AcroUC-node1 mycat]# mycat --help
(2)添加自启动服务
[root@AcroUC-node1 mysql]# vim /etc/rc.local
在文件末尾添加如下内容：
export JAVA_HOME=/opt/jdk.1.8.0_172
/usr/local/mycat/bin/mycat start
7.7.连接测试
1.使用mysql自带客户端操作Mycat
[root@AcroUC-node1 ~]# mysql -uroot -proot -P8066 -h192.168.106.251（先创建数据库）
1.使用Navicat连接Mycat
7.8.读写分离测试
创建测试表：
CREATE TABLE `tb_test` (
 `name` varchar(255) DEFAULT NULL,
 `password` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
插入测试数据：
INSERT INTO `ict`.`tb_test` (`name`, `password`) VALUES ('aaa', '123456');
INSERT INTO `ict`.`tb_test` (`name`, `password`) VALUES ('bbb', '123456');
(1) 监听MyCat日志
[root@AcroUC-node1 mycat]# tailf logs/mycat.log
(2) 读测试
执行上面的查询语句，此时对应的MyCat日志信息如下：
多次执行select * from tb_test;语句，Mycat打印出来的日志信息显示读操作请求路由到Master节点2（192.168.106.252）和Slave
节点1（192.168.106.253:3306）
(3) 写测试
mysql> INSERT INTO `tb_test` (`name`, `password`) VALUES ('ccc', '123456');
执行上面的新增插入语句后，此时对应的Mycat日志信息如下：
多次执行以上插入语句，发现新增数据都是从 Master1节点（192.168.106.251）插进入的，并且Slave节点通过Binlog同步了
Master节点中的数据。
八、配置Mycat状态检查服务
Mycat服务主机（AcroUC-node1、AcroUC-node2）上需要增加Mycat服务的状态检测脚本，并开放相应的检测端口，以提供给
HAProxy对Mycat的服务状态进行检测判断。可以使用xinetd来实现，通过xinetd，HAProxy可以用httpchk来检测Mycat的存活状
态。（xinetd即extended internet daemon，xinetd是新一代的网络守护进程服务程序，又叫超级Internet服务器。经常用来管理多
种轻量级Internet服务。xinetd提供类似于inetd+tcp_wrapper的功能，但是更加强大和安全。xinetd为linux系统的基础服务）
需要在Mycat节点主机上配置
8.1.安装xinetd
如果xinetd还没有安装，可使用如下命令安装：
[root@AcroUC-node1 yum.repos.d]# yum install xinetd -y
8.2.添加 includedir /etc/xinetd.d
检查/etc/xinetd.conf的末尾是否有 includedir /etc/xinetd.d ，没有就加上
[root@AcroUC-node1 mysql]# vim /etc/xinetd.conf
8.3.创建/etc/xinetd.d 目录
检查 /etc/xinetd.d 目录是否存在，不存在则创建
[root@AcroUC-node1 mysql]# mkdir /etc/xinetd.d/
8.4.增加Mycat存活状态检测服务配置
[root@AcroUC-node1 mysql]# vim /etc/xinetd.d/mycat_status
增加以下内容：
service mycat_status
{
flags = REUSE
## 使用该标记的socket_type为stream，需要设置wait为no
socket_type = stream ## 封包处理方式，Stream为TCP数据包
port = 48700 ## 服务监听端口
wait = no ## 表示不需等待，即服务将以多线程的方式运行
user = root ## 执行此服务进程的用户
server = /usr/local/bin/mycat_status ## 需要启动的服务脚本
log_on_failure += USERID ## 登录失败记录的内容
disable = no ## 要启动服务，将此参数设置为no
}
去掉注释内容
8.5.添加 /usr/local/bin/mycat_status 服务脚本
[root@AcroUC-node1 mysql]# vim /usr/local/bin/mycat_status
增加以下内容：
#!/bin/bash
#/usr/local/bin/mycat_status.sh
# This script checks if a Mycat server is healthy running on localhost.
# It will return:
# "HTTP/1.x 200 OK\r" (if Mycat is running smoothly)
# "HTTP/1.x 503 Internal Server Error\r" (else)
Mycat=`/usr/local/mycat/bin/mycat status | grep 'not running' | wc -l`
if [ "$Mycat" = "0" ]; then
/bin/echo -e "HTTP/1.1 200 OK\r\n"
else
/bin/echo -e "HTTP/1.1 503 Service Unavailable\r\n"
fi
给新增脚本赋予可执行权限
[root@AcroUC-node1 mysql]# chmod +x /usr/local/bin/mycat_status
8.6.在 /etc/services 中加入 mycat_status 服务
[root@AcroUC-node1 mysql]# vim /etc/services
在文件末尾加入如下内容：
mycat_status 48700/tcp # mycat_status
保存后，重启xinetd服务
[root@AcroUC-node1 mysql]# service xinetd restart
8.7.验证mycat_status服务是否成功启动
[root@AcroUC-node1 mysql]# netstat -tnlp | grep xinetd
能看到上图这样的信息，说明服务配置成功。
8.8.Mycat服务主机的防火墙上打开 48700端口（如果开启）
[root@AcroUC-node1 mysql]# vim /etc/sysconfig/iptables
-A INPUT -m state --state NEW -m tcp -p tcp --dport 48700 -j ACCEPT
保存后重启防火墙：
[root@AcroUC-node1 mysql]# service iptables restart
脚本测试：
[root@AcroUC-node1 mysql]# /usr/local/bin/mycat_status

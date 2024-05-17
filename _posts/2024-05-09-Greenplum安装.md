---
layout: post
title: Greenplum安装
categories: [数据库]
tags: [Greenplum]
---
**安装示例的服务器配置如下：**  
|节点类型|主机名|IP地址|是否联网|  
|:---:|:---:|:---:|:---:|  
|编译服务器|cmake|10.10.10.1|联网|  
|主节点|mdw|10.10.10.2|离线|  
|从节点1|sdw1|10.10.10.3|离线|  
|从节点2|sdw2|10.10.10.4|离线|  
<!-- more -->
#### 1. 配置安装环境
**在cmake、mdw、sdw1和sdw2都进行如下操作：**  
1.关闭防火墙  
`systemctl stop firewalld && systemctl disable firewalld`  
2.关闭SELinux  
`setenforce 0 && sed -i 's/enforcing/disabled/' /etc/selinux/config`
3.修改Linux内核  
```
echo "kernel.shmmni = 4096
kernel.shmall = 4000000000
kernel.sem = 250 512000 100 2048
kernel.sysrq = 1
kernel.core_uses_pid = 1
kernel.msgmnb = 65536
kernel.msgmax = 65536
kernel.msgmni = 2048
net.ipv4.tcp_syncookies = 1
net.ipv4.ip_forward = 0
net.ipv4.tcp_tw_recycle = 1
net.ipv4.tcp_max_syn_backlog = 4096
net.ipv4.conf.defalut.arp_filter = 1
net.ipv4.ip_local_port_range = 1025 65535
net.core.netdev_max_backlog = 10000
net.core.rmem_max = 2097152
net.core.wmem_max = 2097152
vm.overcommit_memory = 2" >> /etc/sysctl.conf
```
```
echo "* soft nofile 524288
* hard nofile 524288
* soft nproc 131072
* hard nproc 131072" >> /etc/security/limits.conf
```
4.创建`gpadmin`用户并设置密码  
`useradd gpadmin && passwd gpadmin`  
5.下载[psutil-5.9.8.tar.gz](https://files.pythonhosted.org/packages/90/c7/6dc0a455d111f68ee43f27793971cf03fe29b6ef972042549db29eec39a2/psutil-5.9.8.tar.gz)并进入解压目录安装  
`tar -zxvf psutil-5.9.8.tar.gz && cd psutil-5.9.8 && python setup.py install`  
#### 2. 编译安装
目前Greenplum没有提供ARM发行版，需要编译Greenplum源代码自行构建ARM版本。  
**编译过程在cmake服务器进行**  
1.建立联网yum源(aarch64)  
```
echo "[ks10-adv-os]
name = Kylin Linux Advanced Server
baseurl = https://update.cs2c.com.cn/NS/V10/V10SP3.1/os/adv/lic/base/aarch64/
gpgcheck = 0
enabled = 1" > /etc/yum.repos.d/kylin_aarch64.repo
```
2.安装依赖软件包  
`yum groupinstall  'Development Tools'`  
`yum install curl-devel bzip2-devel python-devel openssl-devel readline-devel perl-ExtUtils-Embed libxml2-devel perl-devel`  
3.下载[gpdb-6.25.3.tar.gz](https://github.com/greenplum-db/gpdb/archive/refs/tags/6.25.3.tar.gz)并解压进入  
`tar -zxvf gpdb-6.25.3.tar.gz && cd gpdb-6.25.3`  
4.配置安装环境,生成Makefile  
```
CFLAGS="-O0 -g3 -ggdb3" ./configure --prefix=/usr/local/greenplum --with-perl --with-python --with-libxml --enable-debug --enable-cassert --disable-orca --disable-gpcloud --disable-gpfdist --disable-gpfdist --without-zstd
```
5.配置成功后，编译软件  
`make`  
6.安装编译好的软件  
`sudo make install`  
#### 4. 集群初始化
**将`make install`生成的`/usr/local/grenplum`复制到mdw、sdw1和sdw2服务器的相同路径**  
1..在sdw1配置主机名和主机名解析  
`hostnamectl set-hostname sdw1`  
`echo "10.10.10.2   mdw" | sudo tee -a /etc/hosts`  
`echo "10.10.10.3   sdw1" | sudo tee -a /etc/hosts`  
`echo "10.10.10.4   sdw2" | sudo tee -a /etc/hosts`  
2.在sdw1创建数据目录  
`mkdir -p /data/gpdata/primary`  
3.在sdw2配置主机名和主机名解析  
`hostnamectl set-hostname sdw2`  
`echo "10.10.10.2   mdw" | sudo tee -a /etc/hosts`  
`echo "10.10.10.3   sdw1" | sudo tee -a /etc/hosts`  
`echo "10.10.10.4   sdw2" | sudo tee -a /etc/hosts`  
4.在sdw2创建数据目录  
`mkdir -p /data/gpdata/primary`  
5.在mdw配置主机名和主机名解析  
`hostnamectl set-hostname mdw`  
`echo "10.10.10.2   mdw" | sudo tee -a /etc/hosts`  
`echo "10.10.10.3   sdw1" | sudo tee -a /etc/hosts`  
`echo "10.10.10.4   sdw2" | sudo tee -a /etc/hosts`  
6.在mdw创建数据目录  
`mkdir -p /data/gpdata/master`  
7.在mdw登录`gpadmin`用户  
`su - gpadmin`  
8.在mdw配置免密登录sdw1和sdw2  
`ssh-keygen -t rsa`  
`ssh-copy-id gpadmin@sdw1`  
`ssh-copy-id gpadmin@sdw2`  
9.在mdw加载环境变量  
`source /usr/local/greenplum/greenplum_path.sh`  
10.在mdw创建初始化文件`gpinitsystem_config`  
`vi /home/gpadmin/gpinitsystem_config`  
```
ARRAY_NAME="gp_cluster"
MACHINE_LIST_FILE=hosts_file
SEG_PREFIX=gpseg
PORT_BASE=40000
declare -a DATA_DIRECTORY=(/data/gpdata/primary /data/gpdata/primary /data/gpdata/primary)
MASTER_HOSTNAME=mdw
MASTER_DIRECTORY=/data/gpdata/master
MASTER_PORT=5432
TRUSTED_SHELL=ssh
CHECK_POINT_SEGMENTS=8
ENCODING=UNICODE
CREATE_DATABASE=0
```
11.在mdw创建Segment主机列表文件`hosts_file`   
**Segment主机需要大容量硬盘，示例中未将mdw作为Segment主机，实际生产环境如有需要，则在创建`/data/gpdata/master`时一并创建`/data/gpdata/primary`**  
`vi /home/gpadmin/hosts_file`  
```
sdw1
sdw2
```
12.在mdw初始化集群  
`/usr/local/greenplum/bin/gpinitsystem -c /home/gpadmin/gpinitsystem_config`  
13.允许外部IP访问Greenplum数据库  
`echo 'host     all         all         0.0.0.0/0    md5' >> /data/gpdata/master/gpseg-1/pg_hba.conf`  
14.重载`pg_hba.conf`和`postgresql.conf`配置文件  
`/usr/local/greenplum/bin/gpstop -u`  
#### 4. 配置系统服务
1.创建`greenplum.service`系统服务文件  
`vi /etc/systemd/system/greenplum.service`  
```
[Unit]
Description=Greenplum Database Server
After=syslog.target network.target

[Service]
Type=forking
User=gpadmin
Group=gpadmin
Environment="MASTER_DATA_DIRECTORY=/data/gpdata/master/gpseg-1"
ExecStart=/usr/local/greenplum/bin/gpstart -a
ExecStop=/usr/local/greenplum/bin/gpstop -M immediate
Environment="GPHOME=/usr/local/greenplum"
Environment="PYTHONPATH=/usr/local/greenplum/lib/python"
Environment="PATH=/usr/local/greenplum/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
Environment="LD_LIBRARY_PATH=/usr/local/greenplum/lib:$LD_LIBRARY_PATH"
Environment="OPENSSL_CONF=/usr/local/greenplum/etc/openssl.cnf"

[Install]
WantedBy=multi-user.target
```
2.重新加载全部系统服务配置文件  
`systemctl daemon-reload`  
3.启动greenplum系统服务  
`systemctl start greenplum`  
4.使greenplum系统服务开机自启  
`systemctl enable greenplum`  
#### 5. 常用命令
1.快速停止Greenplum  
`/usr/local/greenplum/bin/gpstop -M fast`  
2.强制杀死Greenplum进程  
`ps -ef | grep gpadmin | grep postgres | awk '{print "kill -9 "$2}'`  
3.启动Greenplum  
`/usr/local/greenplum/bin/gpstart`  
4.启动Greenplum并带有详细日志  
`/usr/local/greenplum/bin/gpstart -v`  
5.全量备份  
`gpbackup --leaf-partition-data --dbname 库名 --backup-dir 备份目录 --jobs 并发数`  
6.恢复备份  
`gprestore -backup-dir 备份目录 --create-db --timestamp 备份时间戳`  

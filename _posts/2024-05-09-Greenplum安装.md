---
layout: post
title: Greenplum安装
categories: [数据库]
tags: [Greenplum]
---
#### 1. Greenplum常用命令
1.快速停止Greenplum  
`gpstop -M fast`  
2.强制杀死Greenplum进程  
`ps -ef | grep gpadmin | grep postgres | awk '{print "kill -9 "$2}'`  
3.启动Greenplum  
`gpstart`  
<!-- more -->
4.启动Greenplum并带有详细日志  
`gpstart -v`  
5.重启Greenplum  
`gpstop -r`  
6.重载`pg_hba.conf`和`postgresql.conf`配置文件  
`gpstop -u`  
7.全量备份  
`gpbackup --leaf-partition-data --dbname 库名 --backup-dir 备份目录 --jobs 并发数`  
8.恢复备份  
`gprestore -backup-dir 备份目录 --create-db --timestamp 备份时间戳`  
9.初始化数据库（重装）  
`gpinitsystem -I my_initsystem_config`  
#### 2. 编译安装Greenplum
目前Greenplum没有提供ARM发行版，我们需要编译Greenplum源代码自行构建ARM版本。  
1.建立联网yum源  
``  
``  
2.关闭防火墙  
`systemctl stop firewalld && systemctl disable firewalld`  
3.配置Linux内核，添加下列内容  
`vi /etc/sysctl.conf`  
```
kernel.shmmni = 4096
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
vm.overcommit_memory = 2
```
`vi /etc/security/limits.conf`  
```
* soft nofile 524288
* hard nofile 524288
* soft nproc 131072
* hard nproc 131072
```
4.关闭SELinux  
`sed -i 's/enforcing/disabled/' /etc/selinux/config`  
5.创建gpadmin用户  
`useradd gpadmin && passwd gpadmin`  
6.yum安装依赖软件包  
`yum groupinstall  'Development Tools'`  
`yum install curl-devel bzip2-devel python-devel openssl-devel readline-devel perl-ExtUtils-Embed libxml2-devel perl-devel zstd git`  
wget https://bootstrap.pypa.io/get-pip.py  
python get-pip.py   
pip install psutil lockfile paramiko setuptools epydoc conan  
7.重启服务器使配置生效  
`reboot`  
8.使用`gpadmin`登录，配置SSH免密码登录  
`ssh-keygen -t rsa`  
`ssh-copy-id -i ~/.ssh/id_rsa gpadmin@localhost`  
9.下载[gpdb-6.25.3.tar.gz](https://github.com/greenplum-db/gpdb/archive/refs/tags/6.25.3.tar.gz)并解压进入  
`tar -zxvf gpdb-6.25.3.tar.gz && cd gpdb-6.25.3`  
10.配置安装环境,生成Makefile  
```
CFLAGS="-O0 -g3 -ggdb3" ./configure --with-perl --with-python --with-libxml --enable-debug --enable-cassert --disable-orca --disable-gpcloud --disable-gpfdist --disable-gpfdist
```
11.配置成功后，编译软件  
`make`  
12.安装编译好的软件
`sudo make install`  

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
集群最少需要3个节点，10.10.10.2为主节点，10.10.10.3为数据节点1，10.10.10.4为数据节点2。  
主节点CPU、内存要求高，从节点硬盘要求高。  
1.建立联网yum源  
```
echo "[ks10-adv-os]
name = Kylin Linux Advanced Server
baseurl = https://update.cs2c.com.cn/NS/V10/V10SP3.1/os/adv/lic/base/aarch64/
gpgcheck = 0
enabled = 1" > /etc/yum.repos.d/kylin_aarch64.repo
```
2.关闭防火墙  
`systemctl stop firewalld && systemctl disable firewalld`  
3.配置Linux内核，添加下列内容  
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
4.关闭SELinux  
`sed -i 's/enforcing/disabled/' /etc/selinux/config`  
5.创建gpadmin用户并设置密码  
`useradd gpadmin && passwd gpadmin`  
6.yum安装依赖软件包  
`yum groupinstall  'Development Tools'`  
`yum install curl-devel bzip2-devel python-devel openssl-devel readline-devel perl-ExtUtils-Embed libxml2-devel perl-devel`  
wget https://bootstrap.pypa.io/get-pip.py  
python get-pip.py   
pip install psutil lockfile paramiko setuptools epydoc conan  

https://files.pythonhosted.org/packages/90/c7/6dc0a455d111f68ee43f27793971cf03fe29b6ef972042549db29eec39a2/psutil-5.9.8.tar.gz
7.重启服务器使配置生效  
`reboot`  
8.在从节点1配置主机名和主机名解析  
`hostnamectl set-hostname sdw1`  
`echo "10.10.10.2   mdw" | sudo tee -a /etc/hosts`  
`echo "10.10.10.3   sdw1" | sudo tee -a /etc/hosts`  
`echo "10.10.10.4   sdw2" | sudo tee -a /etc/hosts`  
9.在从节点2配置主机名和主机名解析  
`hostnamectl set-hostname sdw2`  
`echo "10.10.10.2   mdw" | sudo tee -a /etc/hosts`  
`echo "10.10.10.3   sdw1" | sudo tee -a /etc/hosts`  
`echo "10.10.10.4   sdw2" | sudo tee -a /etc/hosts`  
10.在主节点配置主机名和主机名解析  
`hostnamectl set-hostname mdw`  
`echo "10.10.10.2   mdw" | sudo tee -a /etc/hosts`  
`echo "10.10.10.3   sdw1" | sudo tee -a /etc/hosts`  
`echo "10.10.10.4   sdw2" | sudo tee -a /etc/hosts`  
11.使用`su - gpadmin`登录后配置mdw免密登录sdw1和sdw2  
`ssh-keygen -t rsa`  
`ssh-copy-id gpadmin@sdw1`  
`ssh-copy-id gpadmin@sdw2`  
12.下载[gpdb-6.25.3.tar.gz](https://github.com/greenplum-db/gpdb/archive/refs/tags/6.25.3.tar.gz)并解压进入  
`tar -zxvf gpdb-6.25.3.tar.gz && cd gpdb-6.25.3`  
10.配置安装环境,生成Makefile  
```
CFLAGS="-O0 -g3 -ggdb3" ./configure --with-perl --with-python --with-libxml --enable-debug --enable-cassert --disable-orca --disable-gpcloud --disable-gpfdist --disable-gpfdist
```
11.配置成功后，编译软件  
`make`  
12.安装编译好的软件
`sudo make install`  

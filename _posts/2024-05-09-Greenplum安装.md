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

---
layout: post
title: Greenplum问题解决
categories: [数据库]
tags: [Greenplum]
---
#### 1. Greenplum 6.14 pg_xlog文件夹过大问题解决
**Greenplum是基于PostgreSQL的MPP(大规模并行处理)数据库。**  
**使用`pg_resetxlog`重置WAL文件起始序列号**  
1.在主节点和从节点分别使用以下命令切换到`gpadmin`用户。  
`su - gpadmin`  
`source /usr/local/greenplum-db/greenplum_path.sh`  
`export MASTER_DATA_DIRECTORY=/home/gpdata/gpmaster/gpseg-1`  
<!-- more -->
2.在主节点和从节点执行`gpstop -M fast`关闭Greenplum。  
3.在主节点使用如下命令，并记录下`NextXID`和`NextOID`的值：  
`pg_controldata /home/gpdata/gpmaster/gpseg-1 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
4.在主节点使用如下命令，重置WAL文件起始序列号：  
`pg_resetxlog -o <NextOID的值> -x <NextXID的值> -f /home/gpdata/gpmaster/gpseg-1`  
5.在各个从节点使用如下命令，并记录下NextXID和NextOID的值(每个从节点只有4条命令有值，记录这些有值的即可)：  
`pg_controldata /home/gpdata/gpdatam1/gpseg0 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap1/gpseg0 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatam2/gpseg1 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap2/gpseg1 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatam1/gpseg2 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap1/gpseg2 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatam2/gpseg3 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap2/gpseg3 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
6.在各个从节点使用如下命令，重置WAL文件起始序列号(参照第4步有值的命令结果执行4次)：  
`pg_resetxlog -o <NextOID的值> -x <NextXID的值> -f <NextXID和NextOID有值的命令，pg_controldata对应的路径>`  
7.在主节点和从节点执行`gpstart`启动Greenplum。  
#### 2. Greenplum常用命令
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

---
layout: post
title: Greenplum问题解决
categories: [数据库]
tags: [Greenplum]
---
#### 1. Greenplum 6.14 pg_xlog文件夹过大问题解决
**Greenplum是基于PostgreSQL的MPP(大规模并行处理)数据库.**  
由于Greenplum 6.14没有`max_wal_size`等限制WAL文件的配置参数，也没有`pg_archivecleanup`等WAL文件删除工具，所以只有以下两种方案：  
##### 1.1 通过`wal_keep_segments`和`checkpoint_segments`参数限制WAL文件数量(未验证成功)
1.以`su - gpadmin`命令切换到`gpadmin`用户。  
2.在主节点使用`gpconfig -c wal_keep_segments -v 3`命令将`wal_keep_segments`的值设为3。  
3.在主节点和从节点分别找到`postgresql.conf`文件(主节点1个，从节点4个)，将`checkpoint_segments`的值设为3。  
4.使用`gpstop -u`命令重启Greenplum数据库以使配置更改生效。
<!-- more -->
##### 1.2 使用`pg_resetxlog`重置WAL文件起始序列号(已验证成功)
1.在主节点和从节点分别使用`su - gpadmin`命令切换到`gpadmin`用户。 
2.在主节点使用如下命令，并记录下`NextXID`和`NextOID`的值：  
`pg_controldata /home/gpdata/gpmaster/gpseg-1 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
3.在主节点使用如下命令，重置WAL文件起始序列号：  
`pg_resetxlog -o <NextOID的值> -x <NextXID的值> -f /home/gpdata/gpmaster/gpseg-1`  
4.在各个从节点使用如下命令，并记录下NextXID和NextOID的值(每个从节点只有4条命令有值，记录这些有值的即可)：  
`pg_controldata /home/gpdata/gpdatam1/gpseg0 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap1/gpseg0 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatam2/gpseg1 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap2/gpseg1 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatam1/gpseg2 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap1/gpseg2 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatam2/gpseg3 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
`pg_controldata /home/gpdata/gpdatap2/gpseg3 | grep -E "Latest checkpoint's NextXID|Latest checkpoint's NextOID"`  
5.在各个从节点使用如下命令，重置WAL文件起始序列号(参照第4步有值的命令结果执行4次)：  
`pg_resetxlog -o <NextOID的值> -x <NextXID的值> -f <NextXID和NextOID有值的命令，pg_controldata对应的路径>`  

---
layout: post
title: fastDFS+nginx负载均衡
categories: [文件服务]
tags: [文件服务]
---
#### 1. 软件版本
数据库版本为解压版MySQL [mysql-5.7.22-el7-x86_64.tar.gz](https://downloads.mysql.com/archives/community/)
#### 2. 查看系统之前是否有安装过mysql相关版本
`# rpm -qa|grep mysql`  
`# rpm -qa|grep mariadb`  
`# rpm -qa|grep mysql|xargs rpm -e --nodeps`  
`# rpm -qa|grep mariadb|xargs rpm -e --nodeps`  
<!-- more -->

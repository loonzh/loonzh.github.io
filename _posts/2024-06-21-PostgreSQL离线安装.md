---
layout: post
title: PostgreSQL离线安装
categories: [数据库]
tags: [PostgreSQL]
---
**安装示例的服务器配置如下：**  
|节点类型|主机名|IP地址|是否联网|  
|:---:|:---:|:---:|:---:|  
|编译服务器|cmake|10.10.10.1|联网|  
|主节点|mdw|10.10.10.2|离线|  
|从节点1|sdw1|10.10.10.3|离线|  
|从节点2|sdw2|10.10.10.4|离线|  
<!-- more -->
#### 1. 在互联网服务器制作安装包
##### 1.1 安装EPEL软件源
`yum install epel-release`  
##### 1.2 下载libzstd安装包
`yum -y install yum-utils && repotrack libzstd`  
##### 1.3 下载PostgreSQL安装包
[postgresql15/postgresql15-contrib/postgresql15-libs/postgresql15-server](https://yum.postgresql.org/15/redhat/rhel-7-x86_64/repoview/postgresqldbserver15.group.html)  
<!-- more -->
##### 1.4 创建postgresql-15.7.1文件夹并把libzstd安装包和PostgreSQL安装包放进去
`mkdir postgresql-15.7.1`  
##### 1.5 把全部安装包打包成一个压缩包
`tar -zcvf postgresql-15.7.1.tar.gz postgresql-15.7.1`  
#### 2. 在离线服务器安装PostgreSQL
##### 2.1 解压并进入解压后的目录
`tar -zxvf postgresql-15.7.1.tar.gz && cd postgresql-15.7.1`  
##### 2.2 安装PostgreSQL
`rpm -Uvh --force --nodeps *.rpm`  
##### 2.3 初始化PostgreSQL
`/usr/pgsql-15/bin/postgresql-15-setup initdb`  
##### 2.4 设置允许任意IP通过密码连接PostgreSQL
`sed -i "s/^#listen_addresses = 'localhost'/listen_addresses = '*'/g" /var/lib/pgsql/15/data/postgresql.conf`  
`echo "host    all             all             0.0.0.0/0               md5" | sudo tee -a /var/lib/pgsql/15/data/pg_hba.conf`  
##### 2.5 设置开机自启并启动PostgreSQL
`systemctl enable postgresql-15 && systemctl start postgresql-15`  
##### 2.6 设置默认超管用户postgres的密码为postgres
`sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres';"`  
##### 2.7 连接测试
`PGPASSWORD=postgres psql -U postgres -d postgres`  

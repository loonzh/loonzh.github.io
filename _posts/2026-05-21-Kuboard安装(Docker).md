---
layout: post
title: Kuboard安装(Docker)
categories: [Docker]
tags: [Docker]
---
#### 1. 安装数据库
Kuboard v4支持MySQL、MariaDB和OpenGauss数据库，需要提前安装。  
<!-- more -->
#### 2. 配置docker-compose文件(MySQL)
`vi docker-compose.yaml`  
```
services:
  kuboard:
    image: swr.cn-east-2.myhuaweicloud.com/kuboard/kuboard:v4
    container_name: kuboard
    restart: unless-stopped
    ports:
      - "8888:80/tcp"
    environment:
      - TZ=Asia/Shanghai
      - DB_DRIVER=com.mysql.cj.jdbc.Driver
      - DB_URL=jdbc:mysql://10.10.10.12:3306/kuboard?serverTimezone=Asia/Shanghai
      - DB_USERNAME=kuboard
      - DB_PASSWORD=Kuboard123
    volumes:
      - ./kuboard-log:/app/logs
```
`docker-compose up -d`  
#### 3. 登录Kuboard
1. 管理员用户为：`admin`，默认密码为 ：`Kuboard123`，首次登录后按提示更改密码。
2. 点击`Kubernetes 集群`，点击`导入集群`。
3. 在`Kubenetes`控制节点执行`cat /etc/kubernetes/admin.conf`或`cat /root/.kube/config`，将得到的配置文件内容复制到`Kuboard`。
4. 将`apiserver.cluster.local`修改为控制节点地址，保存后即完成了`Kuboard`的安装配置。

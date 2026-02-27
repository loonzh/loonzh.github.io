---
layout: post
title: Jenkins安装(Docker)
categories: [Jenkins]
tags: [Jenkins]
---
#### 1. 拉取Jenkins镜像
访问[Jenkins](https://www.jenkins.io/download/)，在`Stable (LTS)`下找到最新版本号替换如下命令：  
`docker pull jenkins/jenkins:2.541.2-lts`  
<!-- more -->  
#### 2. 创建docker-compose文件
`mkdir -p /usr/local/docker/jinkins_docker`  
`cd /usr/local/docker/jinkins_docker/`  
`vi docker-compose.yml`  
```
services:
 jenkins:
   image: jenkins/jenkins:2.541.2-lts
   container_name: jenkins
   restart: always
   ports:
     - "8080:8080"
     - "50000:50000"
   volumes:
     - ./data:/var/jenkins_home
```
#### 4. 启动docker-compose
`docker-compose up -d`  
第一次启动会创建`data`目录，但是没权限，此时停止，修改Jenkins插件更新源后赋予权限：  
`docker-compose down`  
`vim data/hudson.model.UpdateCenter.xml`  
```
<?xml version='1.1' encoding='UTF-8'?>
<sites>
  <site>
    <id>default</id>
    <url>https://mirrors.huaweicloud.com/jenkins/updates/update-center.json</url>
  </site>
</sites>
```
`chmod -R 777 ../jinkins_docker/`  
`docker-compose up -d`  
此时通过以下命令可以看到Jenkins的管理员密码：  
`docker logs -f jenkins`  
#### 5. 下载
在宿主机通过 http://10.10.10.11:8888 访问Jenkins，输入刚获取的管理员密码。  
选择`选择插件来安装`，在新页面保持默认点击`安装`，等待安装完成。  

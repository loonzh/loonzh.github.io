---
layout: post
title: Gitlab安装(docker)
categories: [Gitlab]
tags: [Gitlab]
---
#### 1. 安装JDK
下载[JDK8u202源码](https://repo.huaweicloud.com/java/jdk/8u202-b08/)并上传到目标机器。  
`tar -zxvf jdk-8u202-linux-x64.tar.gz -C /usr/local/`  
`cd /usr/local`  
`mv jdk1.8.0_202/ jdk`  
<!-- more -->  
#### 2. 安装Maven
下载[Maven源码](https://maven.apache.org/download.cgi)并上传到目标机器。  
`tar -zxvf apache-maven-3.9.12-bin.tar.gz -C /usr/local/`  
`cd /usr/local`  
`mv apache-maven-3.9.12/ maven`  
#### 3. 修改Maven配置文件
`vi maven/conf/settings.xml`  

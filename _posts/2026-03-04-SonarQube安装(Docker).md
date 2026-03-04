---
layout: post
title: SonarQube安装(Docker)
categories: [SonarQube]
tags: [SonarQube]
---
#### 1. 拉取SonarQube镜像和PostgreSQL镜像
访问[SonarQube](https://www.sonarsource.com/zh/products/sonarqube/downloads/)，在`社区建设`找到最新的版本号替换如下命令：  
`docker pull library/sonarqube:lts-community`  
`docker pull postgres:15`  
<!-- more -->  
#### 2. 创建docker-compose文件
`mkdir -p /usr/local/docker/sanarqube_docker`  
`cd /usr/local/docker/sanarqube_docker`  
`vi docker-compose.yml`  
```
services:
  db:
    image: postgres:15
    container_name: db
    ports:
      - "5432:5432"
    networks:
      - sonarnet
    environment:
      POSTGRES_USER: sonar
      POSTGRES_PASSWORD: sonar

  sonarqube:
    image: library/sonarqube:lts-community
    container_name: sonarqube
    depends_on:
      - db
    ports:
      - "9000:9000"
    networks:
      - sonarnet
    environment:
      SONAR_JDBC_URL: jdbc:postgresql://db:5432/sonar
      SONAR_JDBC_USERNAME: sonar
      SONAR_JDBC_PASSWORD: sonar

networks:
  sonarnet:
    driver: bridge
```
#### 4. 启动docker-compose
`docker-compose up -d`  
第一次启动会失败，使用`docker logs -f sonarqube`查看发现报错`max virtual memory areas vm.max_map_count [65530] is too low, increase to at least [262144]`，在如下文件追加虚拟内存配置：  
`vi /etc/sysctl.conf`  
```
vm.max_map_count=262144
```
`sysctl -p`  
`docker-compose up -d`  
#### 5. SonarQube初始化
1. SonarQube启动后，访问`http://10.10.10.12:9000`，使用`admin/admin`登录，按提示修改密码。
2. 

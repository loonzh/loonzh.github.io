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
    restart: always
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
    restart: always
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
2. 点击`Administration`，点击`Marketplace`，点击`I understand the risk`，在下边搜索框搜索`chinese`，选择`Chinese PackLocalization`，点击`Install`。
3. 如果安装失败，查看日志有可能是因为服务器时间不对，使用`ntpdate ntp.tencent.com`给服务器授时，使用`hwclock --systohc`将系统时间写入硬件。
4. 插件安装成功后按提示重启SonarQube，即可看到中文版页面。

#### 6. Jenkins安装SonarQube Scanner插件
1. 在Jenkins点击右上角齿轮，点击`插件管理`，在`Available plugins`搜索`SonarQube Scanner`安装。
2. 在SonarQube点击用户头像，选择`我的账号`，输入自定义用户名，选择`用户令牌`和`永不过期`，点击生成，记录token。
3. 在Jenkins点击右上角齿轮，点击`系统管理`，找到`SonarQube servers`，点击`Add SonarQube`，`Name`自定义，`Server URL`输入`http://10.10.10.12:9000`，在`Server authentication token`右边点击`添加`，选择`Secret text`，范围选`全局`，在`Server`填入token，点击`Create`，在左边的选择框选中刚创建的令牌，保存页面。
4. 访问[SonarScanner CLI](https://docs.sonarsource.com/sonarqube-server/analyzing-source-code/scanners/sonarscanner)下载最新的`SonarScanner`放到Jenkins数据目录，使用`unzip sonar-scanner-cli-8.0.1.6346-linux-x64.zip`解压文件，使用`mv sonar-scanner-8.0.1.6346-linux-x64/ sonar-scanner`重命名文件，使用`rm -rf sonar-scanner-cli-8.0.1.6346-linux-x64.zip`删除安装包。
5. 在在Jenkins点击右上角齿轮，点击`全局工具配置`，在`SonarQube Scanner 安装`下点击`新增SonarQube Scanner`，自定义`Name`，在`SONAR_RUNNER_HOME`填写`/var/jenkins_home/sonar-scanner`，点击`保存`。

#### 7. 在Jenkins项目里配置SonarQube检查
在项目的`配置`点击`增加构建步骤`，选择`Execute SonarQube Scanner `，在`Analysis properties`下填写:  
```
sonar.projectname=${JOB_NAME}
sonar.projectKey=${JOB_NAME}
sonar.source=./
sonar.java.binaries=target
```
保存后重新构建项目，完成后即可在SonarQube看到代码审查结果。  

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
#### 5. Jenkins初始化
1. 在宿主机通过 http://10.10.10.11:8888 访问Jenkins，输入刚获取的管理员密码。  
2. 选择`选择插件来安装`，在新页面保持默认点击`安装`，等待安装完成。  
3. 使用设置的新密码重新登陆Jenkins，点击右上角齿轮，点击`全局工具配置`。
4. 在后端执行mv /usr/local/{maven,jdk} /usr/local/docker/jinkins_docker/data/`把Maven和JDK放到Jenkins的数据目录。
5. 返回前端指定Maven和JDK地址，注意使用容器地址而不是宿主机地址。
6. 在前端点击右上角齿轮，点击`插件管理`，在`Available plugins`搜索`Publish Over SSH`安装。
7. 在前端点击右上角齿轮，点击`系统配置`，最后找到`Publish Over SSH`下的`SSH Server`，按情况填写账号密码等信息。
#### 6. 创建Gitlab仓库并提交本地代码
1. 打开Gitlab创建新项目`mytest`，可见性选择`公开`。  
2. 点击`mytest`,点击`项目设置`，点击`仓库`，点击`受保护的分支`，取消保护。  
3. 在本地项目文件夹打开`Git bash`，依次输入以下命令：
*在当前目录初始化一个新的 Git 仓库*  
`git init`  
*设置远程仓库的 URL地址(origin是远程仓库的默认名称)*  
`git remote set-url origin http://10.10.10.11:8888/root/mytest.git`
*指定分支为main*  
`branch -M main`  
*全局配置 Git的用户名*  
`git config --global user.name "loonzh"`  
*全局配置 Git 的用户邮箱*
`git config --global user.email "loonzh@qq.com"`  
*关闭 SSL 证书验证*  
`git config --global http.sslVerify false`  
*允许使用旧版/不安全的 HTTP 协议*  
`git config --global http.unsafeLegacyProtocolCheck true`  
*配置凭证助手为manager，避免每次推送都输入密码*  
`git config --global credential.helper manager`  
*将当前目录（.）的所有文件添加到暂存区*   
`git add .`  
*将暂存区的文件提交到本地仓库(-m 后面是提交说明)*
`git commit -m "Initial commit"`  
将暂存区的文件提交到本地仓库
*将本地代码推送到远程仓库(-u 建立本地分支与远程分支的关联，后续可直接用 git push)*
`git push -uf origin main`  
5. 打开Gitlab，项目文件已经push到仓库中。
#### 7. 创建Jenkins项目
1. 打开Jenkins，点击`新建Item`，输入项目名后选择`Freestyle project`。
2. 在`源码管理`处配置Gitlab地址并指定分支为`main`。
3. 点击`Build Steos`，选择`调用顶层 Maven 目标`，选择配置好的`maven`版本，在目标框输入`clean package -Dskiptests`

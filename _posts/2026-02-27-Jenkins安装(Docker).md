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
6. 在前端点击右上角齿轮，点击`插件管理`，在`Available plugins`搜索`Publish Over SSH`、`Git Parameter`安装。
8. 在前端点击右上角齿轮，点击`系统配置`，最后找到`Publish Over SSH`下的`SSH Server`，按情况填写账号密码等信息。

#### 6. 创建Gitlab仓库并提交本地代码
1. 打开Gitlab创建新项目`mytest`，可见性选择`公开`。
2. 点击`mytest`,点击`项目设置`，点击`仓库`，点击`受保护的分支`，取消保护。
3. 在本地项目文件夹打开`Git bash`，依次输入以下命令：  
```
在当前目录初始化一个新的 Git 仓库
git init
设置远程仓库的 URL地址(origin是远程仓库的默认名称)
git remote set-url origin http://10.10.10.11:8888/root/mytest.git
指定分支为main
branch -M main
全局配置 Git的用户名
git config --global user.name "loonzh"
全局配置 Git 的用户邮箱
git config --global user.email "loonzh@qq.com"
关闭 SSL 证书验证
git config --global http.sslVerify false
允许使用旧版/不安全的 HTTP 协议
git config --global http.unsafeLegacyProtocolCheck true
配置凭证助手为manager，避免每次推送都输入密码
git config --global credential.helper manager
将当前目录（.）的所有文件添加到暂存区
git add .
将暂存区的文件提交到本地仓库(-m 后面是提交说明)
git commit -m "Initial commit"
将暂存区的文件提交到本地仓库
将本地代码推送到远程仓库(-u 建立本地分支与远程分支的关联，后续可直接用 git push)
git push -uf origin main
```
5. 打开Gitlab，项目文件已经push到仓库中。

#### 7. Jenkins实现基础CI操作
1. 打开Jenkins，点击`新建Item`，输入项目名后选择`Freestyle project`。
2. 在`源码管理`处配置Gitlab地址并指定分支为`main`。
3. 点击`Build Steos`，选择`调用顶层 Maven 目标`，选择配置好的`maven`版本，在目标框输入`clean package -DskipTests`。
4. 点击`With Ant`，选择之前配置的JDK。
5. 点击`构建后操作`，选择`Send build artifacts over SSH`，在`Source files`填写`target/*.jar docker/*`。
6. 在`Exec command`填写：
```
cd /usr/local/test/docker
mv ../target/*jar ./
docker-compose down
docker-compose up -d --build
docker image prune -f
```
7. 在`docker`目录创建`Dockerfile`，以下为示例：
```
FROM frekele/java:jdk8u202
COPY *.jar /usr/local/app.jar
WORKDIR /usr/local
ENTRYPOINT ["java", "-jar", "app.jar"]
```
8. 在`docker`目录创建`docker-compose.yml`，以下为示例：

```yaml
services: 
 mytest: 
   build: 
     context: ./ 
     dockerfile: Dockerfile 
   image: mytest:v1.0.1
   container_name: mytest 
   ports:
     - "8081:8080"
```

9. 推送文件后点击`立即构建`，第一次需要下载镜像所以速度较慢，构建完成后访问`http://10.10.10.12:8081`即可看到`mytest`的欢迎页。

#### 8. Jenkins实现基础CD操作
1. 在`配置`里勾选`参数化构建过程`，添加`Git 参数`，名称和描述自定义，`参数类型`选`标签`，默认值填`origin/main`。
2. 点击`Build Steos`，选择`执行 shell`，输入`git checkout $tag`后把框体移动到`调用顶层 Maven 目标`上方。
3. 打开Gitib新增标签`1.0.0`，保存当前版本。
4. 开发更改代码后push，在Gitlib新增标签`1.0.1`，保存新版本。
5. 在Jenkins点击`Build with Parameters`，选择版本，访问`http://10.10.10.12:8081`即可看到`mytest`的欢迎页相应变化。

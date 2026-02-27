---
layout: post
title: Gitlab安装(Docker)
categories: [Gitlab]
tags: [Gitlab]
---
#### 1. 检查仓库里Gitlab版本
`docker search docker.1ms.run/gitlab`  
<!-- more -->  
#### 2. 选择Star最多的版本拉取
`docker pull gitlab/gitlab-ce`  
#### 3. 创建docker-compose文件
`mkdir -p /usr/local/docker/gitlab_docker`  
`cd /usr/local/docker/gitlab_docker`  
`vi docker-compose.yml`  
```
services:
 gitlab:
   image: gitlab/gitlab-ce:latest
   container_name: gitlab
   restart: always
   hostname: '10.10.10.11'
   environment:
     GITLAB_OMNIBUS_CONFIG: |
       external_url 'http://localhost'
       gitlab_rails['gitlab_shell_ssh_port'] = 2222
       gitlab_rails['initial_root_password'] = 'P@ssW0rd'
       gitlab_rails['time_zone'] = 'Asia/Shanghai'
   ports:
     - "8888:80"
     - "443:443"
     - "2222:22"
   volumes:
     - ./config:/etc/gitlab
     - ./logs:/var/log/gitlab
     - ./data:/var/opt/gitlab
   shm_size: '256m'
```
#### 4. 启动docker-compose
`docker-compose up -d`  
**此时在宿主机通过 http://10.10.10.11:8888 就可以访问到Gitlab了。**  

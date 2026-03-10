---
layout: post
title: RustDesk自建服务器(Docker)
categories: [RustDesk]
tags: [RustDesk]
---
#### 1. 安装Docker和docker-compose
访问[Docker安装](https://loonzh.github.io/docker/2026/02/05/Docker%E5%AE%89%E8%A3%85(%E7%A6%BB%E7%BA%BF)/#)查看安装方法。  
<!-- more -->  
#### 2. 开放网络端口
在云服务商的安全组(Security Group)或防火墙中放行以下端口：  
```
21115/TCP    HBBS (ID服务)
21116/TCP    HBBS (ID服务)
21116/UDP    HBBS (心跳/打洞)
21117/TCP    HBBR (中继服务)
```
#### 3. 安装RustDesk
`mkdir -p /usr/local/docker/rustdesk_docker/data`  
`cd /usr/local/docker/rustdesk_docker/`  
`vi docker-compose.yml`  
```
services:
  hbbs:
    container_name: hbbs
    image: rustdesk/rustdesk-server:latest
    command: hbbs -r rustdesk.loonzh.cn:21117
    volumes:
      - ./data:/root
    network_mode: "host"
    depends_on:
      - hbbr
    restart: unless-stopped

  hbbr:
    container_name: hbbr
    image: rustdesk/rustdesk-server:latest
    command: hbbr
    volumes:
      - ./data:/root
    network_mode: "host"
    restart: unless-stopped
```
`docker-compose up -d`  
`cat ./data/id_ed25519.pub`  
**记录自动生成的加密公钥，配置客户端时需要用到。**  
#### 4. 配置客户端
1. 访问[RuseDesk客户端](https://github.com/rustdesk/rustdesk/releases)，下载客户端。  
2. 打开 RustDesk 客户端，点击ID旁边的菜单图标，点击`网络`，点击`解锁网络设置`，点击`ID/中继服务器`，ID 服务器:rustdesk.loonzh.cn，中继服务器:rustdesk.loonzh.cn，API 服务器(不要填)，Key:加密公钥。  
3. 点击`确认`，如果底部状态栏显示`就绪`，即代表部署成功。  

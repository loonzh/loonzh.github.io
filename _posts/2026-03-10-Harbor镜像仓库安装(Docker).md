---
layout: post
title: Harbor镜像仓库安装(Docker)
categories: [Harbor]
tags: [Harbor]
---
#### 1. 下载Harbor安装包
访问[Harbor](https://github.com/goharbor/harbor/releases)，下载Harbor安装包上传到服务器。  
`tar -zxvf harbor-offline-installer-v2.14.2.tgz -C /usr/local/`  
<!-- more -->  
#### 2. 修改harbor.yml文件
`cd /usr/local/harbor`  
`cp harbor.yml.tmpl harbor.yml`  
`vi harbor.yml`  
将`hostname:`后边的地址换成本机IP或域名，将`https`所在段的内容注销(因为没有证书)。  
#### 3. 安装Harbor
安装方式为Docker安装，需要使用到`Docker`和`docker-compose`，需要提前安装。  
`./install.sh`  
`vi /usr/lib/systemd/system/harbor.service`  
```
[Unit]
Description=Harbor
After=docker.service systemd-networkd.service systemd-resolved.service
Requires=docker.service
Documentation=http://github.com/vmware/harbor

[Service]
Type=simple
Restart=on-failure
RestartSec=5
ExecStart=/usr/bin/docker-compose -f /usr/local/harbor/docker-compose.yml up
ExecStop=/usr/bin/docker-compose -f /usr/local/harbor/docker-compose.yml down

[Install]
WantedBy=multi-user.target
```
`systemctl enable harbor`  
安装完成后访问`http://10.10.10.12`，使用默认账号`admin/Harbor12345`即可进入Harbor。  

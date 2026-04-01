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
#### 2. 创建CA证书
`mkdir -p /data/cert/ca && cd /data/cert`  
` openssl genrsa -out ca/ca.key 4096`  
` openssl req -x509 -new -nodes -key ca/ca.key -sha256 -days 3650 -out ca/ca.crt -subj "/C=CN/ST=Beijing/L=Beijing/O=Harbor/CN=Harbor CA"`  
#### 3. 创建服务器证书
`openssl genrsa -out harbor.key 2048`  
`openssl req -new -key harbor.key -out harbor.csr -subj "/C=CN/ST=Beijing/L=Beijing/O=Harbor/CN=harbor.loonzh.com"`  
```
cat > harbor.ext << EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names
[alt_names]
DNS.1 = harbor.loonzh.com
DNS.2 = loonzh.com
DNS.3 = localhost
IP.1 = 10.10.10.12
IP.2 = 127.0.0.1
EOF
```
`openssl x509 -req -in harbor.csr -CA ca/ca.crt -CAkey ca/ca.key -CAcreateserial -out harbor.crt -days 3650 -sha256 -extfile harbor.ext` 
#### 4. 修改harbor.yml文件
`cp /usr/local/harbor/harbor.yml.tmpl /usr/local/harbor/harbor.yml`  
`vi /usr/local/harbor/harbor.yml`  
将`hostname:`后边的地址换成`IP/域名`，注释`http`和`port: 80`，`certificate:`后改成`/data/cert/harbor.crt`，`private_key:`后改成`/data/cert/harbor.key`。  
#### 5. 安装Harbor
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
安装完成后访问`https://10.10.10.12`，使用默认账号`admin/Harbor12345`即可进入Harbor。  
#### 6. 客户端服务器信任CA
`cp ca.crt /etc/pki/ca-trust/source/anchors/harbor-ca.crt`  
`update-ca-trust`

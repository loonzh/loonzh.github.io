---
layout: post
title: Chrony时间同步(Linux)
categories: [时间同步]
tags: [时间同步]
---
#### 1. 服务端配置
`yum install chrony`  
`vi /etc/chrony.conf`(追加)  
```
allow 10.10.10.0/24
local stratum 10
server 0.pool.ntp.org iburst
server 1.pool.ntp.org iburst
```
<!-- more -->
`systemctl restart chronyd`  
`systemctl enable chronyd`  
#### 2. 客户端配置
`vi /etc/chrony.conf`(替换)  
```
server 10.10.10.12 iburst
```
`systemctl restart chronyd`  
`systemctl enable chronyd`  
`chronyc sources -v`  
`chronyc tracking`  

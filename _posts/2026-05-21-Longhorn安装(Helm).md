---
layout: post
title: Longhorn安装(Helm)
categories: [Longhorn]
tags: [Longhorn]
---
#### 1. 在联网服务器下载Longhorn Helm Chart包
`helm repo add longhorn https://charts.longhorn.io`  
`helm pull longhorn/longhorn --version 1.11.2 --untar`，得到一个名为`longhorn`的文件夹，这就是我们需要的Chart安装包。  
<!-- more -->
#### 2. 在联网服务器导出所需镜像
下载(longhorn-images.txt)[https://github.com/longhorn/longhorn/releases]，得到Longhorn的镜像列表。  
使用以下命令拉取并打包所有镜像：  
```
while read image; do
    docker pull $image
done < longhorn-images.txt
```
`docker save $(cat longhorn-images.txt | paste -sd ' ') -o longhorn-images.tar`  
最终得到一个 longhorn文件夹和一个longhorn-images.tar文件。  
#### 3. 在离线服务器导入镜像
`docker load < longhorn-images.tar`  
```
for image in $(cat longhorn-images.txt); do
    harbor_image="10.10.10.12/longhorn/${image#*/}"
    docker tag $image $harbor_image
    docker push $harbor_image
done
```
#### 4. 在离线服务器各节点安装iSCSI
`yum install iscsi-initiator-utils -y`  
`systemctl start iscsid`  
`systemctl enable iscsid`  
#### 5. 在离线服务器安装Longhorn
`helm install longhorn longhorn --namespace longhorn-system --create-namespace --set image.registry=10.10.10.12 --set image.repository=longhorn`  
安装完成默认是ClusterIP模式，可以使用Kuboard在浏览器查看。  

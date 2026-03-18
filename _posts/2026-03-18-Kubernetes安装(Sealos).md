---
layout: post
title: Kubernetes安装(Sealos)
categories: [Kubernetes]
tags: [Kubernetes]
---
#### 1. 在联网服务器拉取镜像并打包
1. 访问[Sealos源码](https://github.com/labring/sealos/releases)下载二进制包，复制到联网服务器。
2. 使用`tar xvf sealos_5.1.1_linux_amd64.tar.gz sealos`解压，使用`mv sealos /usr/local/bin/`放到全局工具目录，使用`sealos version`验证生效。
3. 访问[Registry Explore网站](https://explore.ggcr.dev/)确定`registry.cn-shanghai.aliyuncs.com/labring/kubernetes`、`registry.cn-shanghai.aliyuncs.com/labring/helm`和`registry.cn-shanghai.aliyuncs.com/labring/cilium`的版本。
4. 使用如下命令拉取远程镜像到本地(使用`sealos images`查看本地镜像)：
```
sealos pull registry.cn-shanghai.aliyuncs.com/labring/kubernetes:v1.31.11
sealos pull registry.cn-shanghai.aliyuncs.com/labring/helm:3.19.2
sealos pull registry.cn-shanghai.aliyuncs.com/labring/cilium:v1.14.19
```
<!-- more -->
5. 使用如下命令将本地镜像打包(仅归档不压缩)：
```
sealos save -o kubernetes-v1.31.11.tar registry.cn-shanghai.aliyuncs.com/labring/kubernetes:v1.31.11
sealos save -o helm-v1.19.2.tar registry.cn-shanghai.aliyuncs.com/labring/helm:v3.19.2
sealos save -o cilium-v1.14.19.tar registry.cn-shanghai.aliyuncs.com/labring/cilium:v1.14.19
```
6. 将镜像包复制到离线服务器(`master`节点)，所有离线服务器使用相同密码。

#### 2. 在离线服务器装载镜像并安装
1. 在离线服务器(`master`节点)部署`Sealos`。
2. 使用如下命令将镜像包装载为本地镜像(使用`sealos images`查看本地镜像)：
```
sealos load -i kubernetes-v1.31.11.tar
sealos load -i helm-v1.19.2.tar
sealos load -i cilium-v1.14.19.tar
```
3. 使用`sealos run registry.cn-shanghai.aliyuncs.com/labring/kubernetes:v1.31.11 registry.cn-shanghai.aliyuncs.com/labring/helm:v3.19.2 registry.cn-shanghai.aliyuncs.com/labring/cilium:v1.14.19 --masters 10.10.10.20 --nodes 10.10.10.21,10.10.10.22 -p L+ah204313805`安装`Kubernetes`。
4. 安装完成后使用`kubectl get node -o wide`查看节点状态，使用`kubectl get pod -A -o wide`查看容器组状态(如果`coredns`没启动，有可能是`/etc/resolv.conf`为空导致`coredns`获取不到上游DNS信息)。
5. 使用`cat .kube/config`获取集群信息(如果`server`后边的信息不对需要先修改)，在图形化界面导入即可管理集群。
6. 使用`kubeadm certs check-expiration`命令查看证书过期时间，会发现`super-admin.conf`的有限期只有一年，到期后可以使用如下脚本自动续期：
`vi /usr/local/bin/renew-k8s-certs.sh`
```
#!/bin/bash
LOG_FILE="/var/log/k8s-cert-renew.log"
DATE=$(date '+%Y-%m-%d %H:%M:%S')

echo "[$DATE] 开始检查并续期证书..." >> $LOG_FILE

# 续期所有证书
kubeadm certs renew all >> $LOG_FILE 2>&1

# 重启控制平面组件（关键步骤）
for component in kube-apiserver kube-controller-manager kube-scheduler etcd; do
  if [ -f "/etc/kubernetes/manifests/$component.yaml" ]; then
    # 临时移走 manifest 文件，触发 Pod 重启
    mv /etc/kubernetes/manifests/$component.yaml /tmp/
    sleep 20
    mv /tmp/$component.yaml /etc/kubernetes/manifests/
  fi
done

# 重启 kubelet
systemctl restart kubelet

# 更新当前用户的 kubeconfig
cp /etc/kubernetes/admin.conf /root/.kube/config

echo "[$DATE] 证书续期完成" >> $LOG_FILE
```
`chmod +x /usr/local/bin/renew-k8s-certs.sh`  
`crontab -e`
```
0 3 1 */2 * /usr/local/bin/renew-k8s-certs.sh
```

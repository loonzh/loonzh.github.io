---
layout: post
title: Kubernetes命令
categories: [Kubernetes]
tags: [Kubernetes]
---
#### 1. 状态和日志
**[查看全部节点状态]**`kubectl get node`  
**[查看全部Pod状态]**`kubectl get pod -A`  
**[查看全部命名空间状态]**`kubectl get ns`  
**[查看运行中Pod日志]**`kubectl -n default logs mysql`  
**[查看node日志]**`kubectl describe node kmnode1`  
**[查看pod日志]**`kubectl -n default describe pod mysql`  
**[查看集群事件]**`kubectl get events`  
**[创建deployment]**`kubectl apply -f mysql-deployment.yml`  
**[重启K8s服务]**`systemctl restart kubelet`  
<!-- more -->
#### 2. 创建NFS存储的StorageClass
1.在存储服务器安装NFS服务并共享文件夹`/StorageShare`  
2.创建RBAC(Role-Based Access Control)  
`vi nfs-rbac.yaml`  
```
apiVersion: v1
kind: Namespace
metadata:
  name: liuzh
---
apiVersion: v1
kind: ServiceAccount
metadata:
  name: nfs-client-provisioner
  namespace: liuzh
---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: nfs-client-provisioner-runner
rules:
  - apiGroups: [""]
    resources: ["persistentvolumes"]
    verbs: ["get", "list", "watch", "create", "delete"]
  - apiGroups: [""]
    resources: ["persistentvolumeclaims"]
    verbs: ["get", "list", "watch", "update"]
  - apiGroups: ["storage.k8s.io"]
    resources: ["storageclasses"]
    verbs: ["get", "list", "watch"]
  - apiGroups: [""]
    resources: ["events"]
    verbs: ["create", "update", "patch"]
---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: managed-run-nfs-client-provisioner
subjects:
  - kind: ServiceAccount
    name: nfs-client-provisioner
    namespace: liuzh
roleRef:
  kind: ClusterRole
  name: nfs-client-provisioner-runner
  apiGroup: rbac.authorization.k8s.io
---
apiVersion: rbac.authorization.k8s.io/v1
kind: Role
metadata:
  name: leader-locking-nfs-client-provisioner
  namespace: liuzh
rules:
  - apiGroups: [""]
    resources: ["endpoints"]
    verbs: ["get", "list", "watch", "create", "update", "patch"]
---
apiVersion: rbac.authorization.k8s.io/v1
kind: RoleBinding
metadata:
  name: leader-locking-nfs-client-provisioner
  namespace: liuzh
subjects:
  - kind: ServiceAccount
    name: nfs-client-provisioner
    namespace: liuzh
roleRef:
  kind: Role
  name: leader-locking-nfs-client-provisioner
  apiGroup: rbac.authorization.k8s.io
```
`kubectl apply -f nfs-rbac.yaml`  
3.创建provisioner  
`vi nfs-provisioner.yaml`  
```
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: managed-nfs-storage
  namespace: liuzh
#nfs-client-provisioner.spec.template.spec.containers.env.PROVISIONER_NAME.value
provisioner: provisioner-nfs-storage
parameters:
  archiveOnDelete: "false"
---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nfs-client-provisioner
  labels:
    app: nfs-client-provisioner
  namespace: liuzh
spec:
  replicas: 1
  selector:
    matchLabels:
      app: nfs-client-provisioner
  strategy:
    type: Recreate
  selector:
    matchLabels:
      app: nfs-client-provisioner
  template:
    metadata:
      labels:
        app: nfs-client-provisioner
    spec:
      serviceAccountName: nfs-client-provisioner
      containers:
        - name: nfs-client-provisioner
          image: dockerhub.kubekey.local/kubesphereio/nfs-subdir-external-provisioner:v4.0.2
          volumeMounts:
            - name: nfs-client-root
              mountPath: /persistentvolumes
          env:
            - name: PROVISIONER_NAME
              #managed-nfs-storage.provisioner
              value: provisioner-nfs-storage
            - name: NFS_SERVER
              value: 10.10.10.3
            - name: NFS_PATH
              value: /StorageShare
      volumes:
        - name: nfs-client-root
          nfs:
            server: 10.10.10.3
            path: /StorageShare
```
`kubectl apply -f nfs-provisioner.yaml`  
#### 3. 创建Nginx+Ingress统一IP访问服务
1.Kubesphere开启网关服务，记录http端口。  
2.创建tomcat示例工作负载，虚拟IP+不开放外部访问。  
3.创建tomcat-svc服务，将tomcat工作负载的8080端口映射到服务8080端口。  
4.创建应用路由，域名`liuzh.ingress`，路由规则/tomcat对应tomcat-svc服务，此时访问`liuzh.ingress:网关端口/tomcat`即可访问tomcat的8080端口。  
5.在1.1.1.1服务器使用`echo "1.1.1.1    liuzh.ingress" >> /etc/hosts`命令创建域名解析。  
6.在1.1.1.1服务器安装Nginx，将`liuzh.ingress:网关端口/`代理到`1.1.1.1:80`，此时访问`1.1.1.1:80/tomcat`即可访问tomcat的8080端口。  

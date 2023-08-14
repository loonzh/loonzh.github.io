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
2.创建命名空间  
```
cat <<EOF | kubectl apply -f -
apiVersion: v1
kind: Namespace
metadata:
  name: model
EOF
```
3.创建RBAC(Role-Based Access Control)  
`vi rbac.yaml`  
```
apiVersion: v1
kind: ServiceAccount
metadata:
  name: nfs-client-provisioner
  namespace: model
---
kind: ClusterRole
apiVersion: rbac.authorization.k8s.io/v1
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
kind: ClusterRoleBinding
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: managed-run-nfs-client-provisioner
subjects:
  - kind: ServiceAccount
    name: nfs-client-provisioner
    namespace: model
roleRef:
  kind: ClusterRole
  name: nfs-client-provisioner-runner
  apiGroup: rbac.authorization.k8s.io
---
kind: Role
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: leader-locking-nfs-client-provisioner
  namespace: model
rules:
  - apiGroups: [""]
    resources: ["endpoints"]
    verbs: ["get", "list", "watch", "create", "update", "patch"]
---
kind: RoleBinding
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: leader-locking-nfs-client-provisioner
  namespace: model
subjects:
  - kind: ServiceAccount
    name: nfs-client-provisioner
    namespace: model
roleRef:
  kind: Role
  name: leader-locking-nfs-client-provisioner
  apiGroup: rbac.authorization.k8s.io
```
`kubectl apply -f rbac.yaml`  
4.创建StorageClass  
```
cat <<EOF | kubectl apply -f -
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: managed-nfs-storage
  namespace: model
provisioner: provisioner-nfs-storage
#这里的名称要和provisioner配置文件中的环境变量PROVISIONER_NAME保持一致
parameters:
  archiveOnDelete: "false"
EOF
```
5.创建provisioner  
`vi nfs-provisioner.yaml`  
```
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nfs-client-provisioner
  labels:
    app: nfs-client-provisioner
  namespace: model
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
              value: provisioner-nfs-storage
              #provisioner名称,请确保该名称与StorageClass中的provisioner名称保持一致
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

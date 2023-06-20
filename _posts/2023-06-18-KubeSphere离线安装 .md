---
layout: post
title: KubeSphere离线安装
categories: [Kubernetes]
tags: [Kubernetes]
---
#### 1. 前期准备(互联网Linux机器)
[使用本地yum源安装wget]  
`yum -y install wget`  
[下载并更换阿里yum源]  
`wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo`  
<!-- more -->  
[更新yum源缓存]  
`yum clean all && yum makecache`   
[安装yum-utils]  
`yum -y install yum-utils`  
[下载conntrack-tools和socat离线依赖包]  
`repotrack conntrack-tools openssl socat ipset ebtables chrony ipvsadm`  
[将下载的依赖包复制到离线服务器，一次性安装]  
`rpm -Uvh --force --nodeps *.rpm`  
#### 2.	制作manifest(互联网Linux机器)
KubeSphere离线安装使用Kubekey工具完成，离线安装前先要生成制品(artifact)，而制品包含的内容由清单(manifest)指定。  
`vi manifest.yaml`  
```
apiVersion: kubekey.kubesphere.io/v1alpha2
kind: Manifest
metadata:
  name: liuzh
spec:
  arches:
  - amd64
  operatingSystems:
  - arch: amd64
    type: linux
    id: centos
    version: "7"
    repository:
      iso:
        localPath: "/root/KubeSphere/centos7-rpms-amd64.iso"
        url: 
  kubernetesDistributions:
  - type: kubernetes
    version: v1.22.12
  components:
    helm:
      version: v3.9.0
    cni:
      version: v0.9.1
    etcd:
      version: v3.4.13
    containerRuntimes:
    - type: docker
      version: 20.10.8
    crictl:
      version: v1.24.0
    docker-registry:
      version: "2"
    harbor:
      version: v2.5.3
    docker-compose:
      version: v2.2.2
  images:
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-apiserver:v1.22.12
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-controller-manager:v1.22.12
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-proxy:v1.22.12
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-scheduler:v1.22.12
  - registry.cn-beijing.aliyuncs.com/kubesphereio/pause:3.5
  - registry.cn-beijing.aliyuncs.com/kubesphereio/coredns:1.8.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/cni:v3.23.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-controllers:v3.23.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/node:v3.23.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/pod2daemon-flexvol:v3.23.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/typha:v3.23.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/flannel:v0.12.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/provisioner-localpv:3.3.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/linux-utils:3.3.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/haproxy:2.3
  - registry.cn-beijing.aliyuncs.com/kubesphereio/nfs-subdir-external-provisioner:v4.0.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/k8s-dns-node-cache:1.15.12
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-installer:v3.3.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-apiserver:v3.3.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-console:v3.3.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-controller-manager:v3.3.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kubectl:v1.20.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kubefed:v0.8.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/tower:v0.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/minio:RELEASE.2019-08-07T01-59-21Z
  - registry.cn-beijing.aliyuncs.com/kubesphereio/mc:RELEASE.2019-08-07T23-14-43Z
  - registry.cn-beijing.aliyuncs.com/kubesphereio/snapshot-controller:v4.0.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/nginx-ingress-controller:v1.1.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/defaultbackend-amd64:1.4
  - registry.cn-beijing.aliyuncs.com/kubesphereio/metrics-server:v0.4.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/redis:5.0.14-alpine
  - registry.cn-beijing.aliyuncs.com/kubesphereio/haproxy:2.0.25-alpine
  - registry.cn-beijing.aliyuncs.com/kubesphereio/alpine:3.14
  - registry.cn-beijing.aliyuncs.com/kubesphereio/openldap:1.3.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/netshoot:v1.0
```
#### 3. 生成制品(互联网Linux机器)
[下载工具和资源，解压到/root/KubeSphere]  
[centos7-rpms-amd64.iso和kubekey.tar.gz下载](https://github.com/kubesphere/kubekey/releases)  
[切换到中文下载地址]  
`export KKZONE=cn`  
[赋予kubekey执行权限]  
`chmod +x kk`  
[开始生成制品]  
`./kk artifact export -m manifest.yaml -o KubeSphere.tar.gz`  
**生成制品时需要访问GitHub/Googleapis，下载速度较慢**  
#### 4. 制作离线集群配置文件(离线Linux机器)
将kk和KubeSphere.tar.gz复制到master节点。  
`vi KubeSphere.yaml`  
```
apiVersion: kubekey.kubesphere.io/v1alpha2
kind: Cluster
metadata:
  name: liuzh
spec:
  hosts:
  #address是节点的对外地址，internalAddress是节点的集群内通信地址。
  - {name: ks-master-0, address: 192.168.220.100, internalAddress: 10.10.10.100, user: root, password: "123456"}
  - {name: ks-master-1, address: 192.168.220.101, internalAddress: 10.10.10.101, user: root, password: "123456"}
  - {name: ks-node-0, address: 192.168.220.102, internalAddress: 10.10.10.102, user: root, password: "123456"}
   
  roleGroups:
    #分布式存储组件，所有节点
    etcd:
    - ks-master-0
    - ks-master-1
    - ks-node-0
    #控制节点，master节点
    control-plane:
    - ks-master-0
    - ks-master-1
    #工作节点，运行pod的节点
    worker:
    - ks-master-0
    - ks-master-1
    - ks-node-0
    #镜像仓库节点，运行harbor的节点
    registry:
    - ks-node-0
  controlPlaneEndpoint:
   
    domain: lb.kubesphere.local
    address: ""
    port: 6443
  kubernetes:
    version: v1.22.12
    clusterName: cluster.local
  network:
    plugin: calico
    kubePodsCIDR: 10.233.64.0/18
    kubeServiceCIDR: 10.233.0.0/18
    multusCNI:
      enabled: false
  registry:
    type: harbor
    auths:
      #harbor域名，建议不要改，自定义会出错
      "dockerhub.kubekey.local":
        username: admin
        password: Harbor12345
    privateRegistry: "dockerhub.kubekey.local"
    namespaceOverride: "kubesphereio"
    registryMirrors: []
    insecureRegistries: []
  addons: []
---
apiVersion: installer.kubesphere.io/v1alpha1
kind: ClusterConfiguration
metadata:
  name: ks-installer
  namespace: kubesphere-system
  labels:
    version: v3.3.1
spec:
  persistence:
    storageClass: ""
  authentication:
    jwtSecret: ""
  zone: ""
  local_registry: ""
  namespace_override: ""
  etcd:
    monitoring: false
    endpointIps: localhost
    port: 2379
    tlsEnable: true
  common:
    core:
      console:
        enableMultiLogin: true
        #可视化界面端口，范围30005-32767
        port: 31313
        type: NodePort
    redis:
      enabled: false
      volumeSize: 2Gi
    openldap:
      enabled: false
      volumeSize: 2Gi
    minio:
      volumeSize: 20Gi
    monitoring:
      endpoint: http://prometheus-operated.kubesphere-monitoring-system.svc:9090
      GPUMonitoring:
        enabled: false
    gpu:
      kinds:
      - resourceName: "nvidia.com/gpu"
        resourceType: "GPU"
        default: true
    es:
      logMaxAge: 7
      elkPrefix: logstash
      basicAuth:
        enabled: false
        username: ""
        password: ""
      externalElasticsearchHost: ""
      externalElasticsearchPort: ""
  alerting:
    enabled: false
  auditing:
    enabled: false
  devops:
    enabled: false
    jenkinsMemoryLim: 8Gi
    jenkinsMemoryReq: 4Gi
    jenkinsVolumeSize: 8Gi
  events:
    enabled: false
  logging:
    enabled: false
    logsidecar:
      enabled: true
      replicas: 2
  metrics_server:
    enabled: false
  monitoring:
    storageClass: ""
    node_exporter:
      port: 9100
    gpu:
      nvidia_dcgm_exporter:
        enabled: false
  multicluster:
    clusterRole: none
  network:
    networkpolicy:
      enabled: false
    ippool:
      type: none
    topology:
      type: none
  openpitrix:
    store:
      enabled: false
  servicemesh:
    enabled: false
    istio:
      components:
        ingressGateways:
        - name: istio-ingressgateway
          enabled: false
        cni:
          enabled: false
  edgeruntime:
    enabled: false
    kubeedge:
      enabled: false
      cloudCore:
        cloudHub:
          advertiseAddress:
            - ""
        service:
          cloudhubNodePort: "30000"
          cloudhubQuicNodePort: "30001"
          cloudhubHttpsNodePort: "30002"
          cloudstreamNodePort: "30003"
          tunnelNodePort: "30004"
      iptables-manager:
        enabled: true
        mode: "external"
  terminal:
    timeout: 600
```
#### 5. 安装Harbor镜像仓库(离线Linux机器)
[赋予kubekey执行权限]  
`chmod +x kk`  
[使用制品安装Harbor]  
`./kk init registry -f KubeSphere.yaml -a KubeSphere.tar.gz`  
[创建Harbor初始化脚本]  
`vi init_harbor.sh`  
```
#!/usr/bin/env bash
   
url="https://dockerhub.kubekey.local"
user="admin"
passwd="Harbor12345"
   
harbor_projects=(library
    kubesphereio
    kubesphere
    calico
    coredns
    openebs
    csiplugin
    minio
    mirrorgooglecontainers
    osixia
    prom
    thanosio
    jimmidyson
    grafana
    elastic
    istio
    jaegertracing
    jenkins
    weaveworks
    openpitrix
    joosthofman
    nginxdemos
    fluent
    kubeedge
)
   
for project in "${harbor_projects[@]}"; do
    echo "creating $project"
    curl -u "${user}:${passwd}" -X POST -H "Content-Type: application/json" "${url}/api/v2.0/projects" -d "{ \"project_name\": \"${project}\", \"public\": true}" -k
done
```
[执行脚本初始化Harbor]  
`sh init_harbor.sh`  
#### 6. 安装KubeSphere集群(离线Linux机器)
`./kk create cluster -f KubeSphere.yaml -a KubeSphere.tar.gz --with-packages --yes`  
[安装中遇到`Please wait for the installation to complete:`，使用以下命令查看集群日志]  
`kubectl logs -n kubesphere-system $(kubectl get pod -n kubesphere-system -l 'app in (ks-install, ks-installer)' -o jsonpath='{.items[0].metadata.name}') -f`  
**安装完成后，浏览器访问master节点`http://192.168.220.100:31313`或者`http://192.168.220.101:31313`，默认帐户/密码：`admin/P@88w0rd`，即可访问KubeSphere的可视化控制页面。**  

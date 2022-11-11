---
layout: post
title: KubeSphere离线安装
categories: [Kubernetes]
tags: [Kubernetes]
---
#### 1. 前期准备
[在互联网Linux机器使用本地yum源安装wget]  
`yum -y install wget`  
[下载并更换阿里yum源]  
`wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo`  
<!-- more -->  
[更新yum源缓存]  
`yum clean all`  
`yum makecache`  
[安装yum-utils]  
`yum -y install yum-utils`  
[下载conntrack-tools和socat离线依赖包]  
`repotrack conntrack-tools`  
`repotrack socat`  
[将下载的依赖包复制到离线服务器，一次性安装]  
`rpm -Uvh --force --nodeps *.rpm`  
#### 1.	制作manifest
KubeSphere离线安装使用Kubekey工具完成，离线安装前先要生成制品(artifact)，而制品包含的内容由清单(manifest)指定，所以需要先按照规范制作manifest.yaml文件。  
`vi manifest.yaml`  
[centos7-rpms-amd64.iso和kubekey.tar.gz
下载](https://github.com/kubesphere/kubekey/releases)  
```
apiVersion: kubekey.kubesphere.io/v1alpha2
kind: Manifest
metadata:
  name: sample
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
        localPath: "/root/centos7-rpms-amd64.iso"
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
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-installer:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-apiserver:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-console:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-controller-manager:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-upgrade:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kubectl:v1.22.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kubectl:v1.21.0
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
  - registry.cn-beijing.aliyuncs.com/kubesphereio/cloudcore:v1.9.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/iptables-manager:v1.9.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/edgeservice:v0.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/gatekeeper:v3.5.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/openpitrix-jobs:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/devops-apiserver:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/devops-controller:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/devops-tools:v3.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/ks-jenkins:v3.3.0-2.319.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/inbound-agent:4.10-2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-base:v3.2.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-nodejs:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-maven:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-maven:v3.2.1-jdk11
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-python:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.2-1.16
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.2-1.17
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.2-1.18
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-base:v3.2.2-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-nodejs:v3.2.0-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-maven:v3.2.0-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-maven:v3.2.1-jdk11-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-python:v3.2.0-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.0-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.2-1.16-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.2-1.17-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/builder-go:v3.2.2-1.18-podman
  - registry.cn-beijing.aliyuncs.com/kubesphereio/s2ioperator:v3.2.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/s2irun:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/s2i-binary:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/tomcat85-java11-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/tomcat85-java11-runtime:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/tomcat85-java8-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/tomcat85-java8-runtime:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/java-11-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/java-8-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/java-8-runtime:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/java-11-runtime:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/nodejs-8-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/nodejs-6-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/nodejs-4-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/python-36-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/python-35-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/python-34-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/python-27-centos7:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/argocd:v2.3.3
  - registry.cn-beijing.aliyuncs.com/kubesphereio/argocd-applicationset:v0.4.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/dex:v2.30.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/redis:6.2.6-alpine
  - registry.cn-beijing.aliyuncs.com/kubesphereio/configmap-reload:v0.5.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/prometheus:v2.34.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/prometheus-config-reloader:v0.55.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/prometheus-operator:v0.55.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-rbac-proxy:v0.11.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-state-metrics:v2.5.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/node-exporter:v1.3.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/alertmanager:v0.23.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/thanos:v0.25.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/grafana:8.3.3
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-rbac-proxy:v0.8.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/notification-manager-operator:v1.4.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/notification-manager:v1.4.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/notification-tenant-sidecar:v3.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/elasticsearch-curator:v5.7.6
  - registry.cn-beijing.aliyuncs.com/kubesphereio/elasticsearch-oss:6.8.22
  - registry.cn-beijing.aliyuncs.com/kubesphereio/fluentbit-operator:v0.13.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/docker:19.03
  - registry.cn-beijing.aliyuncs.com/kubesphereio/fluent-bit:v1.8.11
  - registry.cn-beijing.aliyuncs.com/kubesphereio/log-sidecar-injector:1.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/filebeat:6.7.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-events-operator:v0.4.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-events-exporter:v0.4.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-events-ruler:v0.4.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-auditing-operator:v0.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kube-auditing-webhook:v0.2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/pilot:1.11.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/proxyv2:1.11.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/jaeger-operator:1.27
  - registry.cn-beijing.aliyuncs.com/kubesphereio/jaeger-agent:1.27
  - registry.cn-beijing.aliyuncs.com/kubesphereio/jaeger-collector:1.27
  - registry.cn-beijing.aliyuncs.com/kubesphereio/jaeger-query:1.27
  - registry.cn-beijing.aliyuncs.com/kubesphereio/jaeger-es-index-cleaner:1.27
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kiali-operator:v1.38.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/kiali:v1.38
  - registry.cn-beijing.aliyuncs.com/kubesphereio/busybox:1.31.1
  - registry.cn-beijing.aliyuncs.com/kubesphereio/nginx:1.14-alpine
  - registry.cn-beijing.aliyuncs.com/kubesphereio/wget:1.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/hello:plain-text
  - registry.cn-beijing.aliyuncs.com/kubesphereio/wordpress:4.8-apache
  - registry.cn-beijing.aliyuncs.com/kubesphereio/hpa-example:latest
  - registry.cn-beijing.aliyuncs.com/kubesphereio/fluentd:v1.4.2-2.0
  - registry.cn-beijing.aliyuncs.com/kubesphereio/perl:latest
  - registry.cn-beijing.aliyuncs.com/kubesphereio/examples-bookinfo-productpage-v1:1.16.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/examples-bookinfo-reviews-v1:1.16.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/examples-bookinfo-reviews-v2:1.16.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/examples-bookinfo-details-v1:1.16.2
  - registry.cn-beijing.aliyuncs.com/kubesphereio/examples-bookinfo-ratings-v1:1.16.3
  - registry.cn-beijing.aliyuncs.com/kubesphereio/scope:1.13.0
```
#### 2. 生成制品
[切换到中文下载地址]  
`export KKZONE=cn`  
[开始生成制品]  
`./kk artifact export -m manifest.yaml -o kubesphere.tar.gz`  
**生成制品时需要访问GitHub/Googleapis，下载速度较慢**  
#### 3. 制作离线集群配置文件
将kk和kubesphere.tar.gz复制到离线环境安装节点。  
`vi config-sample.yaml`  
```
apiVersion: kubekey.kubesphere.io/v1alpha2
kind: Cluster
metadata:
  name: sample
spec:
  hosts:
  - {name: master, address: 10.10.10.1, internalAddress: 10.10.10.1, user: root, password: "123456"}
  - {name: node1, address: 10.10.10.2, internalAddress: 10.10.10.2, user: root, password: "123456"}
   
  roleGroups:
    etcd:
    - master
    control-plane:
    - master
    worker:
    - node1
    registry:
    - node1
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
        port: 30880
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
#### 4. 安装Harbor镜像仓库
[赋予执行权限]  
`chmod +x kk`  
[使用制品安装Harbor]  
`./kk init registry -f config-sample.yaml -a kubesphere.tar.gz`  
[创建Harbor初始化脚本]  
`vi create_project_harbor.sh`  
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
`sh create_project_harbor.sh`  
#### 5. 安装KubeSphere集群
`./kk create cluster -f config-sample.yaml -a kubesphere.tar.gz --with-packages
`  
[查看集群状态]  
`kubectl logs -n kubesphere-system $(kubectl get pod -n kubesphere-system -l 'app in (ks-install, ks-installer)' -o jsonpath='{.items[0].metadata.name}') -f`  
**通过http://{IP}:30880 使用默认帐户和密码admin/P@88w0rd即可访问KubeSphere的Web控制台。**  

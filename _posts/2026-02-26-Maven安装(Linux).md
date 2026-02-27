---
layout: post
title: Maven安装(Linux)
categories: [Maven]
tags: [Maven]
---
#### 1. 安装JDK
下载[JDK8u202源码](https://repo.huaweicloud.com/java/jdk/8u202-b08/)并上传到目标机器。  
`tar -zxvf jdk-8u202-linux-x64.tar.gz -C /usr/local/`  
`cd /usr/local`  
`mv jdk1.8.0_202/ jdk`  
<!-- more -->  
#### 2. 安装Maven
下载[Maven源码](https://maven.apache.org/download.cgi)并上传到目标机器。  
`tar -zxvf apache-maven-3.9.12-bin.tar.gz -C /usr/local/`  
`cd /usr/local`  
`mv apache-maven-3.9.12/ maven`  
#### 3. 配置Maven镜像仓库
`vi maven/conf/settings.xml`  
搜索第一个`<mirrors>`，在下一行插入如下内容：  
```
	<mirror>
		<id>maven-default-http-blocker</id>
		<mirrorOf>external:http:*</mirrorOf>
		<name>Pseudo repository to mirror external repositories initially using HTTP.</name>
		<url>http://0.0.0.0/</url>
		<blocked>true</blocked>
	</mirror>

	<!-- 阿里云镜像central库 -->
	<!-- central Maven中央库（默认仓库），加速访问Java标准库（如commons-*、log4j等），最常用的公共依赖仓库，包含绝大多数开源Java库 -->
	<mirror>
		<id>aliYunMaven</id>
		<name>aliyun maven</name>
		<mirrorOf>central</mirrorOf>
		<url>https://maven.aliyun.com/repository/central</url>
	</mirror>

	<!-- 阿里云镜像public库 -->
	<!-- public 同时代理两个仓库的依赖：central + jcenter聚合仓，简化配置，一次引用多个源（但可能增加依赖冲突风险）     -->
	<mirror>
		<id>aliyunmaven</id>
		<mirrorOf>*</mirrorOf>
		<name>阿里云公共仓库</name>
		<url>https://maven.aliyun.com/repository/public</url>
	</mirror>

	<!--清华大学镜像 -->
	<mirror>
		<id>tsinghuaUniversityMaven</id>
		<name>tsinghuaUniversity Maven</name>
		<mirrorOf>external:http:*</mirrorOf>
		<url>https://repo.maven.apache.org/maven2/</url>
	</mirror>

	<!--华为镜像 -->
	<mirror>
		<id>huaWeiMaven</id>
		<name>huaWei Maven</name>
		<mirrorOf>external:http:*</mirrorOf>
		<url>https://repo.huaweicloud.com/repository/maven/</url>
	</mirror>        
```
#### 4. 配置配置profiles
`vi maven/conf/settings.xml`  
搜索第一个`<profiles>`，在下一行插入如下内容：  
```
	<profile>
		<!-- 定义 profile 的唯一 id -->
		<id>jdk-1.8</id>
		<!-- 定义 profile 的激活条件 -->
		<activation>
			<!-- 设置为默认激活 -->
			<activeByDefault>true</activeByDefault>
			<!-- 当 JDK 版本为 1.8 时激活 -->
			<jdk>1.8</jdk>
		</activation>
		<!-- 定义 profile 的属性配置 -->
		<properties>
			<!-- 设置 Java 源代码版本为 1.8 -->
			<maven.compiler.source>1.8</maven.compiler.source>
			<!-- 设置 Java 目标版本为 1.8 -->
			<maven.compiler.target>1.8</maven.compiler.target>
			<!-- 设置编译器版本为 1.8 -->
			<maven.compiler.compilerVersion>1.8</maven.compiler.compilerVersion>
		</properties>
	</profile>
```
搜索`</settings>`，在上一行插入如下内容：  
```
  <activeProfiles>
    <activeProfile>jdk-1.8</activeProfile>
  </activeProfiles>
```

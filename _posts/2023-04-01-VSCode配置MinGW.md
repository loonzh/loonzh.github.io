---
layout: post
title: VSCode配置MinGW
categories: [开发环境]
tags: [VSCode]
---
#### 1. 下载安装
[从官网下载安装VSCode]  
[Visual Studio Code](https://code.visualstudio.com/)  
*安装出现警告弹窗时点击【确定】*  
[从官网下载安装MinGW]  
[MinGW](https://github.com/niXman/mingw-builds-binaries/releases)  
*压缩包名称类似 x86_64-12.2.0-release-win32-seh-ucrt-rt_v10-rev2.7z ，解压到一个英文目录。*  
<!-- more -->  
#### 2. 配置MinGW环境变量
1.在【计算机】图标上右键【属性】。  
2.在弹出的新窗口选择【高级系统设置】-->【高级】-->【环境变量(N)…】。  
3.在弹出的新窗口上方的【用户变量】或下方的【系统变量】中找到【Path】，点击【编辑】。  
*如果原变量值只有一行，在尾部加入英语分号;后，写入你在安装时自定义的 英文路径\bin*  
*如果原变量值为多行，点击【新建(N)】，并写入你在安装时自定义的 英文路径\bin*  
#### 3. 配置VSCode
[VSCode安装插件]  
`Chinese (Simplified) (简体中文) Language Pack for Visual Studio Code`  
`C/C++`  
`C/C++ Snippets`  
`Code Runner`  
`Include Autocomplete`  
[配置C语言编译器文件]  
1.点击右上角三角形选择【调试 C/C++ 文件】。  
2.在弹出的窗口选择【C/C++:gcc.exe生成和调试活动文件(下边会显示你MinGW的路径)】  
3.再次点击右上角三角形选择【调试 C/C++ 文件】就可以开始C语言代码的调试了。  

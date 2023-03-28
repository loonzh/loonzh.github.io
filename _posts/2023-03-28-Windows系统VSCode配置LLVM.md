---
layout: post
title: Windows系统VSCode配置LLVM
categories: [开发环境]
tags: [VSCode]
---
#### 1. 前期准备
[从官网下载安装VSCode]  
[Visual Studio Code](https://code.visualstudio.com/)  
*安装出现警告弹窗时点击【确定】*  
[从官网下载安装MSYS2]  
[MSYS2](https://www.msys2.org/)  
*安装程序名称类似 msys2-x86_64-20230318.exe*  
<!-- more -->  
#### 2. 安装编译器
[配置环境变量]  
1.在【计算机】图标上右键【属性】。  
2.在弹出的新窗口选择【高级系统设置】-->【高级】-->【环境变量(N)…】。  
3.在弹出的新窗口上方的【用户变量】或下方的【系统变量】中找到【Path】，点击【编辑】。  
*如果原变量值只有一行，在尾部加入英语分号;后，写入你在安装时自定义的 MSYS2路径\clang64\bin*  
*如果原变量值为多行，点击【新建(N)】，并写入你在安装时自定义的 MSYS2路径\clang64\bin）*  
[使用MSYS2安装LLVM]  
1.在MSYS2安装路径找到clang64.exe，右键【以管理员身份运行】。  
2.在打开的窗口输入以下命令：  
```
pacman -S mingw-w64-clang-x86_64-toolchain mingw-w64-clang-x86_64-cninja mingw-w64-clang-x86_64-python-six mingw-w64-clang-x86_64-python-pip
```
*提示需要选择时有【Y】按【Y】，没有就直接【Enter】选择默认配置。*  
3.执行完成后执行：`pip install cmakelang`  
*这个命令执行过程中可能会连接超时，使用`外网连接工具`切换网络即可。*  
4.执行完成后执行：`pacman -Syu`  
*`pacman -Syu`执行过程中 clang64.exe可能会退出，重新打开执行`pacman -Syu`即可，直到没有需要更新的内容为止。*  
#### 3. 配置编辑器
[VSCode安装插件]  
`Chinese (Simplified) (简体中文) Language Pack for Visual Studio Code`  
`clangd`  
`CodeLLDB`  
`CMake`  
`CMake Tools`  
`Code Runner`   

# 保利威 PC 点播 SDK Demo

本文为您介绍保利威 C++/C# 播放 SDK 本地集成的操作步骤和说明。

## clone 工程

使用 git clone 工程

```bash
git clone https://github.com/polyv/polyv-player-sdk-demo.git

```

## 外部依赖

* 本 demo UI 依赖 Qt 框架，请自行到 Qt 分发站下载。
  
 > [下载](http://download.qt.io/)
  
* Python 编译处理脚本，请自行到官网下载安装。
  
 > [下载](https://www.python.org/downloads/)

* CMake

 > [下载](https://cmake.org/download/)

* OpenSSL 1.0.2 以上

 > [下载](https://oomake.com/download/openssl)

* curl
  
 > [下载](https://github.com/curl/curl/)

* 注意其中 openssl curl 本仓库 thirdparty 自带，如果有冲突，请自行替换。

## 本地集成

* plv-player-sdk 文件夹：包含 SDK 头文件以及 lib、dll、dylib 文件，使用 PlvPlayerSDK.cmake 拉取。
  * mac
    * include
    * lib
  * windows
    * x86
      * include
      * lib
    * x64
      * include
      * lib

## 编码规范

注意，Windows 平台的条件编译宏是 `_WIN32` 而不是 `WIN32`。

| Platform | Marco       |
| -------- | ----------- |
| Windows  | `_WIN32`    |
| Mac      | `__APPLE__` |

## C++ Demo 使用 CMake 构建

* 推荐使用 Visual Studio 2022 开发，本 Demo 使用 Visual Studio 2022 开发。

### CMake GUI

* 使用 `CMake GUI` 配置 `build目录`、配置所需的依赖。

 > ![cmake_build.png](https://help.polyv.net/img/vod/pc_player/CMAKE.png)

* 设置 QTDIR，指定 QT 路径。（如：C:\Qt\Qt5.15.2\5.15.2\msvc2019）
  
 > ![QTDIR.png](https://help.polyv.net/img/vod/pc_player/QTDIR.png)

### CMake Bash

```bash
mkdir build
cd build

# 以下指令请根据平台选择执行
# Demo工程使用了QT库, 须指定QTDIR
# build in Windows
cmake .. -G "Visual Studio 17 2022" -A x64 -DQTDIR=C:\Qt\5.15.2\msvc2019_64

# build in Macos
cmake -S .. -B . -G "Xcode" -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.15 -DQTDIR:PATH=/Users/polyv/Qt/5.15.2/clang_64

```

## C# Demo 使用 Visual Studio 打开

1. C# Demo 只支持 Windows。
2. 推荐使用 Visual Studio 2022 开发。
3. test/C# 直接打开 polyv-player-demo-csharp.sln。

## 接口文档

[官网接口文档](https://help.polyv.net/#/vod/pc_player/Interface)

## 更新记录（版本变更）

[版本变更说明](RELEASE-NOTES.md)

# easyTC

[TOC]

## 简介

这是一个学习项目，主要是为了实现各个内网设备能够自由通信，实现的手段主要是p2p加服务器中转。

为了应用层无感，会引入一些虚拟组网的东西。可能最后像zerotier一样的实现。

目前会依赖一些库，网络底层会依赖asio, http服务器方面会依赖crow, log方面会依赖spdlog，这些库都是only-header的库，并不会对编译过程造成明显影响

C++版本 C++17

## 第三方库仓库
可以在git里设置子仓库

```bash
# spdlog
https://github.com/gabime/spdlog.git

# asio
https://github.com/chriskohlhoff/asio.git

## crow
https://github.com/CrowCpp/Crow.git

```

## 常用命令
```bash
cd build
rm -rf * Remove-Item * -Recurse
cmake ..
cmake --build . --config Release --target install

```

## 概述
这个项目为了练手和学一些东西，所以会慢慢实现一些东西。这些东西，目前会有
* 可靠的udp协议
* 内存池
* lock-free 无锁队列
* 环形队列
* html排版
* todo
* 自定义STL容器内存分配器allcator

这些都有工业级实现，后面也会整理出对应的工业库，其中一部分库已经有了源码，目前连源码都看不懂，自己写的玩具和这些成熟库相去甚远。会学习他们，但什么时候能到他们的水准目前不知道。如有需要尽量使用成熟的工业级库。

**对应工具的开源库**

|工具|成熟开源库|
|:---:|:---:|
|无锁队列||


## 文档跳转



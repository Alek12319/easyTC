# easyTC

## 简介

这是一个学习项目，主要是为了实现各个内网设备能够自由通信，实现的手段主要是p2p加服务器中转。

为了应用层无感，会引入一些虚拟组网的东西。可能最后像zerotier一样的实现。

目前会依赖一些库，网络底层会依赖asio, http服务器方面会依赖crow, log方面会依赖spdlog，这些库都是only-header的库，并不会对编译过程造成明显影响

C++版本 C++17

## 第三方库仓库

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
rm -rf * rm *
cmake ..
cmake --build . --config Release --target install

```
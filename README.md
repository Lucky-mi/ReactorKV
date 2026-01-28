# 基于 Reactor 模式的高性能 KV 存储引擎

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](BUILD)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 项目简介

ReactorKV 是一个基于 **Epoll (ET模式) + Reactor 事件驱动模型** 的高性能 KV 存储引擎，结合 **跳表 (SkipList)** 作为内存索引结构，支持高并发连接和 O(logN) 复杂度的数据读写。

## 技术栈

- **语言**: C++14
- **平台**: Linux (Ubuntu 20.04+)
- **核心技术**: Epoll / 多线程 / Socket / GDB / Valgrind
- **数据结构**: 跳表 (SkipList)
- **设计模式**: Reactor 模式、半同步/半反应堆线程池

## 快速开始

### 编译

```bash
./scripts/build.sh Debug
```

### 运行服务端

```bash
./build/bin/kvserver --port 8888
```

### 运行客户端

```bash
./build/bin/kvclient --host 127.0.0.1 --port 8888
```

## 项目结构

```
ReactorKV/
├── src/
│   ├── base/       # 基础工具库 (日志、锁、线程池)
│   ├── net/        # 网络层 (Reactor, EventLoop, Buffer)
│   ├── storage/    # 存储引擎 (SkipList, KVStore)
│   ├── protocol/   # 协议层 (编解码)
│   └── server/     # 服务器主程序
├── tests/          # 单元测试
├── client/         # 客户端
└── docs/           # 文档
```

## 性能指标

| 指标 | 数值 |
|------|------|
| 顺序写入 QPS | ~42 万 |
| 随机写入 QPS | ~30 万 |
| 顺序读取 QPS | ~60 万 |
| 随机读取 QPS | ~29 万 |
| 混合读写 (90% 读) | ~27 万 |
| 并发连接数 | 10000+ |

> 注：性能数据基于 SkipList 基准测试，实际性能取决于硬件配置和数据特征。

## License

MIT License

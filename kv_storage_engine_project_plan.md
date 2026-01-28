# 基于 Reactor 模式的高性能 KV 存储引擎 - 完整项目计划

## 📋 项目概述

本项目旨在构建一个基于 **Epoll (ET模式) + Reactor 事件驱动模型**的高性能 KV 存储引擎，结合**跳表（SkipList）**作为内存索引结构，支持高并发连接和 O(logN) 复杂度的数据读写。

### 技术栈
- **语言**: C++11/14
- **平台**: Linux (Ubuntu 20.04+)
- **核心技术**: Epoll / 多线程 / Socket / GDB / Valgrind
- **数据结构**: 跳表（SkipList）
- **设计模式**: Reactor 模式、半同步/半反应堆线程池

---

## 🏗️ 完整项目架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Client Layer                                 │
│                    (多个客户端并发连接)                               │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      Network Layer (网络层)                          │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    Main Reactor (主 Reactor)                  │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │   │
│  │  │  Acceptor   │  │   Epoll     │  │   EventLoop         │  │   │
│  │  │(监听新连接)  │  │  (ET模式)   │  │   (事件循环)        │  │   │
│  │  └─────────────┘  └─────────────┘  └─────────────────────┘  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                │                                     │
│                    (新连接分发给 Sub Reactors)                       │
│                                ▼                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              Sub Reactors (从 Reactor 线程池)                │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐   │   │
│  │  │SubReactor1│ │SubReactor2│ │SubReactor3│ │SubReactor4│   │   │
│  │  │EventLoop  │ │EventLoop  │ │EventLoop  │ │EventLoop  │   │   │
│  │  │  Thread   │ │  Thread   │ │  Thread   │ │  Thread   │   │   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────┘   │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     Protocol Layer (协议层)                          │
│  ┌───────────────┐  ┌───────────────┐  ┌───────────────────────┐   │
│  │ Application   │  │    Codec      │  │     Buffer            │   │
│  │   Buffer      │  │  (编解码器)   │  │  (环形缓冲区)         │   │
│  │ (粘包处理)    │  │ (协议解析)    │  │  (读写分离)           │   │
│  └───────────────┘  └───────────────┘  └───────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                   Business Layer (业务逻辑层)                        │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                 Thread Pool (业务线程池)                     │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │              Task Queue (任务队列)                   │   │   │
│  │  │         Mutex + Condition Variable                  │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐         │   │
│  │  │Worker 1 │ │Worker 2 │ │Worker 3 │ │Worker N │         │   │
│  │  └─────────┘ └─────────┘ └─────────┘ └─────────┘         │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    Storage Layer (存储引擎层)                        │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    KV Store Engine                           │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │              SkipList (跳表索引结构)                    │ │   │
│  │  │         O(logN) 插入/删除/查询                         │ │   │
│  │  │    ┌─────┐                                             │ │   │
│  │  │    │Head │ ─────────────────────────────────→ NIL     │ │   │
│  │  │    │  L3 │ ──────────→ [30] ─────────────────→ NIL    │ │   │
│  │  │    │  L2 │ ──→ [10] ──→ [30] ────→ [50] ─────→ NIL    │ │   │
│  │  │    │  L1 │ ─→[5]→[10]→[20]→[30]→[40]→[50]→[60]→ NIL  │ │   │
│  │  │    └─────┘                                             │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │              Memory Management                         │ │   │
│  │  │     shared_ptr + RAII (自动内存管理)                   │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    Persistence (持久化)                      │   │
│  │           数据落盘 / 文件加载 / 快照机制                     │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 📁 项目目录结构

```
KVStorageEngine/
├── CMakeLists.txt                 # CMake 构建配置
├── README.md                      # 项目说明文档
├── build/                         # 构建输出目录
├── bin/                           # 可执行文件目录
├── lib/                           # 库文件目录
├── docs/                          # 文档目录
│   ├── design.md                  # 设计文档
│   ├── api.md                     # API 文档
│   └── benchmark.md               # 性能测试报告
│
├── src/                           # 源代码目录
│   ├── base/                      # 基础工具类
│   │   ├── noncopyable.h          # 不可拷贝基类
│   │   ├── timestamp.h/.cpp       # 时间戳类
│   │   ├── logger.h/.cpp          # 日志系统
│   │   ├── mutex.h                # 互斥锁封装
│   │   ├── condition.h            # 条件变量封装
│   │   ├── thread.h/.cpp          # 线程封装
│   │   ├── threadpool.h/.cpp      # 线程池实现
│   │   └── blockingqueue.h        # 阻塞队列
│   │
│   ├── net/                       # 网络层
│   │   ├── socket.h/.cpp          # Socket 封装
│   │   ├── inetaddress.h/.cpp     # 网络地址封装
│   │   ├── channel.h/.cpp         # 事件通道
│   │   ├── poller.h/.cpp          # IO 多路复用抽象基类
│   │   ├── epollpoller.h/.cpp     # Epoll 实现
│   │   ├── eventloop.h/.cpp       # 事件循环
│   │   ├── eventloopthread.h/.cpp # 事件循环线程
│   │   ├── eventloopthreadpool.h/.cpp  # 事件循环线程池
│   │   ├── acceptor.h/.cpp        # 连接接受器
│   │   ├── tcpserver.h/.cpp       # TCP 服务器
│   │   ├── tcpconnection.h/.cpp   # TCP 连接
│   │   ├── buffer.h/.cpp          # 应用层缓冲区
│   │   └── callbacks.h            # 回调函数类型定义
│   │
│   ├── protocol/                  # 协议层
│   │   ├── codec.h/.cpp           # 编解码器
│   │   ├── kvprotocol.h/.cpp      # KV 协议定义
│   │   └── message.h              # 消息结构定义
│   │
│   ├── storage/                   # 存储引擎层
│   │   ├── skiplist.h             # 跳表数据结构 (模板类)
│   │   ├── kvstore.h/.cpp         # KV 存储引擎
│   │   └── persistence.h/.cpp     # 数据持久化
│   │
│   └── server/                    # 服务器主程序
│       ├── kvserver.h/.cpp        # KV 服务器实现
│       └── main.cpp               # 程序入口
│
├── test/                          # 测试代码
│   ├── test_skiplist.cpp          # 跳表单元测试
│   ├── test_buffer.cpp            # Buffer 单元测试
│   ├── test_eventloop.cpp         # EventLoop 测试
│   ├── test_tcpserver.cpp         # TCP 服务器测试
│   └── benchmark.cpp              # 性能压测程序
│
├── client/                        # 客户端
│   ├── kvclient.h/.cpp            # KV 客户端实现
│   └── client_main.cpp            # 客户端程序入口
│
└── scripts/                       # 脚本工具
    ├── build.sh                   # 构建脚本
    ├── run.sh                     # 运行脚本
    └── stress_test.sh             # 压力测试脚本
```

---

## 🔧 核心模块详细设计

### 1. 基础模块 (base/)

#### 1.1 NonCopyable 基类
```cpp
// noncopyable.h
class NonCopyable {
public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};
```

#### 1.2 日志系统
```cpp
// logger.h
enum LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };

#define LOG_DEBUG(fmt, ...) Logger::log(DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger::log(INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::log(ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
```

#### 1.3 互斥锁与条件变量封装
```cpp
// mutex.h
class MutexLock : NonCopyable {
public:
    MutexLock() { pthread_mutex_init(&mutex_, nullptr); }
    ~MutexLock() { pthread_mutex_destroy(&mutex_); }
    void lock() { pthread_mutex_lock(&mutex_); }
    void unlock() { pthread_mutex_unlock(&mutex_); }
    pthread_mutex_t* getPthreadMutex() { return &mutex_; }
private:
    pthread_mutex_t mutex_;
};

class MutexLockGuard : NonCopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) { mutex_.lock(); }
    ~MutexLockGuard() { mutex_.unlock(); }
private:
    MutexLock& mutex_;
};

// condition.h
class Condition : NonCopyable {
public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, nullptr);
    }
    ~Condition() { pthread_cond_destroy(&cond_); }
    void wait() { pthread_cond_wait(&cond_, mutex_.getPthreadMutex()); }
    void notify() { pthread_cond_signal(&cond_); }
    void notifyAll() { pthread_cond_broadcast(&cond_); }
private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};
```

### 2. 网络模块 (net/)

#### 2.1 Channel (事件通道)
```cpp
// channel.h
class Channel : NonCopyable {
public:
    using EventCallback = std::function<void()>;
    
    Channel(EventLoop* loop, int fd);
    
    void handleEvent();                          // 处理事件
    void setReadCallback(EventCallback cb);      // 设置读回调
    void setWriteCallback(EventCallback cb);     // 设置写回调
    void setErrorCallback(EventCallback cb);     // 设置错误回调
    void setCloseCallback(EventCallback cb);     // 设置关闭回调
    
    void enableReading();                        // 注册读事件
    void enableWriting();                        // 注册写事件
    void disableWriting();                       // 取消写事件
    void disableAll();                           // 取消所有事件
    
    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    
private:
    EventLoop* loop_;
    const int fd_;
    int events_;      // 关注的事件
    int revents_;     // 实际发生的事件
    
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};
```

#### 2.2 Poller (Epoll 封装)
```cpp
// epollpoller.h
class EpollPoller : NonCopyable {
public:
    using ChannelList = std::vector<Channel*>;
    
    EpollPoller(EventLoop* loop);
    ~EpollPoller();
    
    // epoll_wait，返回活跃的 Channel
    void poll(int timeoutMs, ChannelList* activeChannels);
    
    // 更新 Channel 的事件
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    
private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels);
    void update(int operation, Channel* channel);
    
    using EventList = std::vector<struct epoll_event>;
    
    int epollfd_;
    EventList events_;
    EventLoop* ownerLoop_;
    std::map<int, Channel*> channels_;
};
```

#### 2.3 EventLoop (事件循环 - Reactor 核心)
```cpp
// eventloop.h
class EventLoop : NonCopyable {
public:
    using Functor = std::function<void()>;
    
    EventLoop();
    ~EventLoop();
    
    void loop();                            // 开始事件循环
    void quit();                            // 退出事件循环
    
    void runInLoop(Functor cb);             // 在当前循环执行
    void queueInLoop(Functor cb);           // 加入队列稍后执行
    
    void updateChannel(Channel* channel);   // 更新 Channel
    void removeChannel(Channel* channel);   // 移除 Channel
    
    void wakeup();                          // 唤醒阻塞的 epoll_wait
    
    bool isInLoopThread() const;            // 是否在当前线程
    
private:
    void handleRead();                      // 处理 wakeupFd 的读事件
    void doPendingFunctors();               // 执行待处理的回调
    
    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    std::atomic<bool> callingPendingFunctors_;
    
    const pid_t threadId_;
    std::unique_ptr<EpollPoller> poller_;
    
    int wakeupFd_;                          // eventfd，用于唤醒
    std::unique_ptr<Channel> wakeupChannel_;
    
    std::vector<Channel*> activeChannels_;
    
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;  // 待执行的回调
};
```

#### 2.4 Buffer (应用层缓冲区 - 解决粘包/半包)
```cpp
// buffer.h
class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    
    explicit Buffer(size_t initialSize = kInitialSize);
    
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }
    
    const char* peek() const { return begin() + readerIndex_; }
    
    void retrieve(size_t len);
    void retrieveAll();
    std::string retrieveAsString(size_t len);
    std::string retrieveAllAsString();
    
    void append(const char* data, size_t len);
    void append(const std::string& str);
    
    ssize_t readFd(int fd, int* savedErrno);   // 从 fd 读取数据
    ssize_t writeFd(int fd, int* savedErrno);  // 向 fd 写入数据
    
private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    void makeSpace(size_t len);
    
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};
```

#### 2.5 TcpConnection
```cpp
// tcpconnection.h
class TcpConnection : NonCopyable, 
                      public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
    
    TcpConnection(EventLoop* loop, const std::string& name, 
                  int sockfd, const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();
    
    void send(const std::string& message);
    void send(Buffer* buf);
    void shutdown();
    
    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);
    
    void connectEstablished();
    void connectDestroyed();
    
private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    
    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();
    
    EventLoop* loop_;
    std::string name_;
    std::atomic<int> state_;
    
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    
    InetAddress localAddr_;
    InetAddress peerAddr_;
    
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};
```

### 3. 存储引擎模块 (storage/)

#### 3.1 SkipList (跳表实现)
```cpp
// skiplist.h
template<typename K, typename V>
class SkipList {
public:
    SkipList(int maxLevel = 16);
    ~SkipList();
    
    // 基本操作
    bool insert(const K& key, const V& value);
    bool remove(const K& key);
    bool search(const K& key, V& value);
    bool contains(const K& key);
    
    // 辅助操作
    int size() const { return elementCount_; }
    void clear();
    void display() const;
    
    // 持久化
    void dumpFile(const std::string& filepath);
    void loadFile(const std::string& filepath);
    
private:
    struct Node {
        K key;
        V value;
        int level;
        std::vector<std::shared_ptr<Node>> forward;  // 使用智能指针
        
        Node(K k, V v, int level) 
            : key(k), value(v), level(level), forward(level + 1, nullptr) {}
    };
    
    int getRandomLevel();  // 随机生成节点层数
    std::shared_ptr<Node> createNode(K key, V value, int level);
    
    int maxLevel_;
    int currentLevel_;
    int elementCount_;
    std::shared_ptr<Node> header_;
    
    mutable MutexLock mutex_;  // 线程安全
};
```

#### 3.2 KVStore (KV 存储引擎)
```cpp
// kvstore.h
class KVStore {
public:
    KVStore();
    ~KVStore();
    
    // CRUD 操作
    bool put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    
    // 管理操作
    int size() const;
    void clear();
    
    // 持久化
    void save(const std::string& filepath);
    void load(const std::string& filepath);
    
private:
    SkipList<std::string, std::string> skiplist_;
};
```

### 4. 协议层 (protocol/)

#### 4.1 KV 协议设计
```
消息格式:
+--------+--------+--------+---------+---------+
| Magic  | Version| CmdType| KeyLen  | ValueLen|
| 2 bytes| 1 byte | 1 byte | 4 bytes | 4 bytes |
+--------+--------+--------+---------+---------+
|           Key Data                           |
+----------------------------------------------+
|           Value Data                         |
+----------------------------------------------+

Command Types:
- 0x01: PUT
- 0x02: GET  
- 0x03: DEL
- 0x04: EXISTS
- 0x05: SIZE
- 0x06: CLEAR

Response Format:
+--------+--------+--------+---------+
| Magic  | Version| Status | DataLen |
| 2 bytes| 1 byte | 1 byte | 4 bytes |
+--------+--------+--------+---------+
|           Data                     |
+------------------------------------+
```

```cpp
// kvprotocol.h
struct KVRequest {
    uint16_t magic = 0xBEEF;
    uint8_t version = 1;
    uint8_t cmdType;
    uint32_t keyLen;
    uint32_t valueLen;
    std::string key;
    std::string value;
    
    std::string encode() const;
    static bool decode(Buffer* buf, KVRequest& req);
};

struct KVResponse {
    uint16_t magic = 0xBEEF;
    uint8_t version = 1;
    uint8_t status;  // 0: success, 1: not found, 2: error
    uint32_t dataLen;
    std::string data;
    
    std::string encode() const;
    static bool decode(Buffer* buf, KVResponse& resp);
};
```

---

## 📅 开发计划 (8-10 周)

### 第一阶段：基础设施 (第 1-2 周)

| 周次 | 任务 | 产出 |
|------|------|------|
| Week 1 | 1. 搭建项目框架和 CMake 构建系统<br>2. 实现基础工具类 (NonCopyable, Timestamp, Logger)<br>3. 实现线程同步原语 (MutexLock, Condition) | CMakeLists.txt<br>base/ 目录完成 |
| Week 2 | 1. 实现线程封装 Thread 类<br>2. 实现阻塞队列 BlockingQueue<br>3. 实现线程池 ThreadPool | 可编译运行的基础框架<br>线程池单元测试通过 |

### 第二阶段：网络核心 (第 3-5 周)

| 周次 | 任务 | 产出 |
|------|------|------|
| Week 3 | 1. 实现 Socket 封装<br>2. 实现 Channel 类<br>3. 实现 EpollPoller (ET 模式) | net/ 基础组件 |
| Week 4 | 1. 实现 EventLoop (Reactor 核心)<br>2. 实现 wakeup 机制 (eventfd)<br>3. 实现 EventLoopThread | 单线程 Reactor 运行 |
| Week 5 | 1. 实现 Buffer (粘包/半包处理)<br>2. 实现 Acceptor<br>3. 实现 TcpConnection<br>4. 实现 TcpServer | 完整 Echo Server |

### 第三阶段：存储引擎 (第 6-7 周)

| 周次 | 任务 | 产出 |
|------|------|------|
| Week 6 | 1. 实现 SkipList 跳表数据结构<br>2. 使用 RAII + shared_ptr 管理内存<br>3. 跳表单元测试 | 线程安全的 SkipList |
| Week 7 | 1. 实现 KVStore 封装<br>2. 实现数据持久化 (落盘/加载)<br>3. 存储引擎单元测试 | 完整 KVStore |

### 第四阶段：协议与集成 (第 8-9 周)

| 周次 | 任务 | 产出 |
|------|------|------|
| Week 8 | 1. 设计并实现 KV 协议<br>2. 实现 Codec 编解码器<br>3. 集成网络层和存储层 | KVServer 基本功能 |
| Week 9 | 1. 实现 KVClient 客户端<br>2. 实现半同步/半反应堆模型<br>3. 端到端功能测试 | 完整客户端/服务端 |

### 第五阶段：优化与测试 (第 10 周)

| 周次 | 任务 | 产出 |
|------|------|------|
| Week 10 | 1. 性能压测 (QPS 测试)<br>2. 使用 GDB 调试<br>3. 使用 Valgrind 检测内存泄漏<br>4. 文档编写 | 压测报告<br>README<br>API 文档 |

---

## 🎯 关键技术点详解

### 1. Epoll ET 模式关键点

```cpp
// 必须配合非阻塞 IO 使用
int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// ET 模式下必须一次性读完所有数据
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);
    
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}
```

### 2. 半同步/半反应堆模型

```cpp
// 主 Reactor 负责 accept，Sub Reactor 负责 IO
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    // 选择一个 Sub Reactor (round-robin)
    EventLoop* ioLoop = threadPool_->getNextLoop();
    
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(
        ioLoop, connName, sockfd, localAddr_, peerAddr);
    
    // 注册回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    // 在 IO 线程中建立连接
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}
```

### 3. 粘包/半包处理策略

```cpp
// 使用 "长度前缀" 方案
void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    while (buf->readableBytes() >= kHeaderLen) {
        // 1. 先读取包头，获取数据长度
        const int32_t len = buf->peekInt32();
        
        if (len > kMaxMessageLen || len < 0) {
            // 非法长度，关闭连接
            conn->shutdown();
            break;
        }
        
        if (buf->readableBytes() >= kHeaderLen + len) {
            // 2. 数据完整，取出完整消息处理
            buf->retrieve(kHeaderLen);
            std::string message = buf->retrieveAsString(len);
            processMessage(conn, message);
        } else {
            // 3. 数据不完整，等待更多数据
            break;
        }
    }
}
```

### 4. 跳表随机层数生成

```cpp
int SkipList::getRandomLevel() {
    int level = 0;
    // p = 0.25，期望层数约为 1.33
    while ((rand() & 0xFFFF) < (0xFFFF >> 2)) {
        level++;
    }
    return std::min(level, maxLevel_ - 1);
}
```

---

## 📚 学习资源与参考项目

### 核心学习资料

| 资源 | 说明 | 链接 |
|------|------|------|
| 《Linux多线程服务端编程》 | 陈硕著，muduo 作者，必读 | 电子工业出版社 |
| muduo 网络库 | 最佳参考实现 | https://github.com/chenshuo/muduo |
| muduo 源码剖析 | 中文详解 | https://zhuanlan.zhihu.com/p/85101271 |
| Skiplist-CPP | 跳表 KV 存储参考 | https://github.com/youngyangyang04/Skiplist-CPP |
| Redis 设计与实现 | 跳表在 Redis 中的应用 | https://redisbook.readthedocs.io |

### 相似开源项目

| 项目 | 描述 | GitHub |
|------|------|--------|
| A-Tiny-Network-Library | C++11 重构 muduo | https://github.com/Shangyizhou/A-Tiny-Network-Library |
| KVstorageBaseRaft-cpp | 基于 Raft 的分布式 KV | https://github.com/youngyangyang04/KVstorageBaseRaft-cpp |
| cyclone | 跨平台 C++ 网络库 | https://github.com/thejinchao/cyclone |
| evpp | 360 开源，参考 muduo | https://github.com/Qihoo360/evpp |
| Redis-SkipList | 跳表实现参考 | https://github.com/Shy2593666979/Redis-SkipList |

### 技术文章

| 主题 | 链接 |
|------|------|
| Reactor 模式详解 | https://www.modernescpp.com/index.php/reactor/ |
| TCP 粘包解决方案 | https://balloonwj.github.io/cpp-guide-web/articles/网络编程/TCP协议如何解决粘包、半包问题.html |
| Redis 跳表详解 | http://zhangtielei.com/posts/blog-redis-skiplist.html |

---

## 🧪 测试与压测

### 功能测试用例

```cpp
// test_skiplist.cpp
TEST(SkipListTest, BasicOperations) {
    SkipList<int, std::string> sl;
    
    // 插入测试
    EXPECT_TRUE(sl.insert(1, "one"));
    EXPECT_TRUE(sl.insert(3, "three"));
    EXPECT_TRUE(sl.insert(2, "two"));
    
    // 查询测试
    std::string value;
    EXPECT_TRUE(sl.search(2, value));
    EXPECT_EQ(value, "two");
    
    // 删除测试
    EXPECT_TRUE(sl.remove(2));
    EXPECT_FALSE(sl.search(2, value));
}
```

### 压力测试

```cpp
// benchmark.cpp
void benchmark_write(KVStore& store, int count) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < count; i++) {
        store.put("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Write QPS: " << count * 1000 / duration.count() << std::endl;
}
```

### 预期性能指标

- 写入 QPS: ~24 万
- 读取 QPS: ~18 万
- 并发连接: 10000+
- 内存泄漏: 0 (Valgrind 验证)

---

## 📝 简历项目描述建议

```
基于 Reactor 模式的高性能 KV 存储引擎
技术栈: C++11 / Linux / Epoll / 多线程 / Socket / GDB

• 架构设计：基于 Epoll (ET模式) 与 Reactor 事件驱动模型，封装 EventLoop 核心网络库；
  实现了半同步/半反应堆线程池模型，有效将 IO 事件与业务逻辑解耦，支撑万级并发连接。

• 存储引擎：深入理解 Redis 底层原理，手写跳表 (SkipList) 数据结构作为内存索引引擎，
  支持 O(logN) 复杂度的 KV 数据读写，并结合 RAII 机制与智能指针管理内存资源。

• 性能优化：采用非阻塞 IO 配合应用层 Buffer 处理 TCP 粘包/半包问题；
  通过 Mutex 与 Condition Variable 实现线程间高效同步；
  压测下写入 QPS 达 24 万，读取 QPS 达 18 万。

• 质量保障：使用 GDB 进行调试，Valgrind 检测内存泄漏，实现数据落盘与快照恢复功能。
```

---

## ✅ 项目检查清单

- [ ] 项目可以正常编译运行
- [ ] 单元测试覆盖核心功能
- [ ] 压测达到预期 QPS
- [ ] Valgrind 无内存泄漏
- [ ] GDB 调试无异常
- [ ] README 文档完善
- [ ] 代码符合 Google C++ Style
- [ ] Git 提交记录规范

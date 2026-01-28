// src/net/tcp_server.h
#ifndef KVSTORE_NET_TCP_SERVER_H
#define KVSTORE_NET_TCP_SERVER_H

#include "base/noncopyable.h"
#include "net/callbacks.h"
#include "net/tcp_connection.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace kvstore {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

/**
 * @brief TCP 服务器
 *
 * TcpServer 是网络库对外的主要接口。
 * 封装了 Acceptor 和连接管理，用户只需设置回调函数即可。
 *
 * 架构：
 * - Main Reactor (baseLoop): 负责接受新连接
 * - Sub Reactors (IO 线程池): 负责处理连接 IO
 *
 * 使用示例：
 *   EventLoop loop;
 *   TcpServer server(&loop, InetAddress(8080), "EchoServer");
 *   server.setConnectionCallback(onConnection);
 *   server.setMessageCallback(onMessage);
 *   server.setThreadNum(4);  // 4 个 IO 线程
 *   server.start();
 *   loop.loop();
 */
class TcpServer : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option { kNoReusePort, kReusePort };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr,
              const std::string& name, Option option = kReusePort);
    ~TcpServer();

    // ==================== Getters ====================

    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    // ==================== 配置 ====================

    /// 设置 IO 线程数量（必须在 start() 前调用）
    void setThreadNum(int numThreads);

    /// 设置线程初始化回调
    void setThreadInitCallback(const ThreadInitCallback& cb) {
        threadInitCallback_ = cb;
    }

    // ==================== 回调设置 ====================

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

    // ==================== 控制 ====================

    /// 启动服务器（线程安全，可多次调用）
    void start();

private:
    /// 新连接到来时的回调（由 Acceptor 调用）
    void newConnection(int sockfd, const InetAddress& peerAddr);

    /// 连接关闭时的回调
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;  // Main Reactor
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    std::atomic<int> started_;
    int nextConnId_;
    ConnectionMap connections_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_TCP_SERVER_H

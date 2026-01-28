// src/net/tcp_server.cpp
#include "net/tcp_server.h"
#include "net/acceptor.h"
#include "net/eventloop.h"
#include "net/eventloop_thread_pool.h"
#include "base/logger.h"

#include <cstdio>

namespace kvstore {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                     const std::string& name, Option option)
    : loop_(loop),
      ipPort_(listenAddr.toIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(),
      messageCallback_(),
      started_(0),
      nextConnId_(1) {
    // 设置 Acceptor 的新连接回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_.fetch_add(1) == 0) {
        threadPool_->start(threadInitCallback_);

        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
        LOG_INFO << "TcpServer [" << name_ << "] started on " << ipPort_;
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();

    // 选择一个 IO 线程
    EventLoop* ioLoop = threadPool_->getNextLoop();

    // 生成连接名称
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
             << connName << "] from " << peerAddr.toIpPort();

    // 获取本地地址
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    ::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen);
    InetAddress localAddr(localaddr);

    // 创建 TcpConnection
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));

    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    // 在 IO 线程中建立连接
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->name();

    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

}  // namespace kvstore

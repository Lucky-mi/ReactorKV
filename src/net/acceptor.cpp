// src/net/acceptor.cpp
#include "net/acceptor.h"
#include "net/eventloop.h"
#include "net/inet_address.h"
#include "base/logger.h"

#include <unistd.h>
#include <fcntl.h>

namespace kvstore {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(Socket::createNonblockingSocket()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);

    // 设置读回调：有新连接时触发
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
    LOG_INFO << "Acceptor listening on fd=" << acceptSocket_.fd();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);

    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR << "Acceptor::handleRead accept failed";
        // 如果 fd 用尽，需要特殊处理
        if (errno == EMFILE) {
            LOG_ERROR << "File descriptors exhausted!";
        }
    }
}

}  // namespace kvstore

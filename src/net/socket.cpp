// src/net/socket.cpp
#include "net/socket.h"
#include "net/inet_address.h"
#include "base/logger.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

namespace kvstore {

Socket::~Socket() {
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr) {
    int ret = ::bind(sockfd_, localaddr.getSockAddr(), sizeof(struct sockaddr_in));
    if (ret < 0) {
        LOG_FATAL << "Socket::bindAddress failed, errno=" << errno;
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        LOG_FATAL << "Socket::listen failed, errno=" << errno;
    }
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = sizeof(addr);

    // 使用 accept4 直接设置非阻塞和 close-on-exec
    int connfd = ::accept4(sockfd_, reinterpret_cast<struct sockaddr*>(&addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd >= 0) {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR << "Socket::shutdownWrite failed";
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (ret < 0 && on) {
        LOG_ERROR << "SO_REUSEPORT not supported";
    }
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

int Socket::createNonblockingSocket() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL << "Socket::createNonblockingSocket failed";
    }
    return sockfd;
}

}  // namespace kvstore

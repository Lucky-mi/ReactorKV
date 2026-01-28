// src/net/socket.h
#ifndef KVSTORE_NET_SOCKET_H
#define KVSTORE_NET_SOCKET_H

#include "base/noncopyable.h"

namespace kvstore {

class InetAddress;

/**
 * @brief Socket 封装
 *
 * 对 socket fd 的 RAII 封装，析构时自动关闭。
 * 封装了常用的 socket 操作：bind, listen, accept, shutdown 等。
 *
 * 使用示例：
 *   Socket sock(Socket::createNonblockingSocket());
 *   sock.bindAddress(InetAddress(8080));
 *   sock.listen();
 *   int connfd = sock.accept(&peerAddr);
 */
class Socket : noncopyable {
public:
    /// 从已有 fd 构造
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    /// 析构时关闭 socket
    ~Socket();

    /// 获取 socket fd
    int fd() const { return sockfd_; }

    // ==================== Socket 操作 ====================

    /// 绑定地址
    void bindAddress(const InetAddress& localaddr);

    /// 开始监听
    void listen();

    /// 接受连接，返回连接 fd，peeraddr 存储对端地址
    int accept(InetAddress* peeraddr);

    /// 关闭写端（半关闭）
    void shutdownWrite();

    // ==================== Socket 选项 ====================

    /// 设置 TCP_NODELAY（禁用 Nagle 算法）
    void setTcpNoDelay(bool on);

    /// 设置 SO_REUSEADDR
    void setReuseAddr(bool on);

    /// 设置 SO_REUSEPORT
    void setReusePort(bool on);

    /// 设置 SO_KEEPALIVE
    void setKeepAlive(bool on);

    // ==================== 静态工具方法 ====================

    /// 创建非阻塞 socket
    static int createNonblockingSocket();

private:
    const int sockfd_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_SOCKET_H

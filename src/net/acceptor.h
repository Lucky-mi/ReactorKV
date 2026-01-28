// src/net/acceptor.h
#ifndef KVSTORE_NET_ACCEPTOR_H
#define KVSTORE_NET_ACCEPTOR_H

#include "base/noncopyable.h"
#include "net/socket.h"
#include "net/channel.h"

#include <functional>

namespace kvstore {

class EventLoop;
class InetAddress;

/**
 * @brief 连接接受器
 *
 * 封装了服务器监听 socket 的创建和新连接的接受。
 * 当有新连接到来时，调用用户设置的回调函数。
 *
 * 使用示例：
 *   Acceptor acceptor(loop, InetAddress(8080));
 *   acceptor.setNewConnectionCallback([](int sockfd, const InetAddress& addr) {
 *       // 处理新连接
 *   });
 *   acceptor.listen();
 */
class Acceptor : noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport = true);
    ~Acceptor();

    /// 设置新连接回调
    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    /// 开始监听
    void listen();

    /// 是否正在监听
    bool listening() const { return listening_; }

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_ACCEPTOR_H

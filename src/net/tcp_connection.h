// src/net/tcp_connection.h
#ifndef KVSTORE_NET_TCP_CONNECTION_H
#define KVSTORE_NET_TCP_CONNECTION_H

#include "base/noncopyable.h"
#include "base/timestamp.h"
#include "net/callbacks.h"
#include "net/buffer.h"
#include "net/inet_address.h"

#include <atomic>
#include <memory>
#include <string>

namespace kvstore {

class Channel;
class EventLoop;
class Socket;

/**
 * @brief TCP 连接
 *
 * TcpConnection 代表一个 TCP 连接，由 TcpServer 创建和管理。
 * 使用 shared_ptr 管理生命周期，因为连接可能被多个地方引用。
 *
 * 主要职责：
 * 1. 管理连接的 socket 和 Channel
 * 2. 处理连接的读写事件
 * 3. 维护发送和接收缓冲区
 * 4. 处理连接的建立和销毁
 *
 * 状态转换：
 * Connecting -> Connected -> Disconnecting -> Disconnected
 */
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    // ==================== Getters ====================

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    // ==================== 发送数据 ====================

    void send(const std::string& message);
    void send(Buffer* buf);

    // ==================== 连接控制 ====================

    /// 关闭连接（半关闭，等待对端关闭）
    void shutdown();

    /// 强制关闭
    void forceClose();

    /// 设置 TCP_NODELAY
    void setTcpNoDelay(bool on);

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
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }
    void setCloseCallback(const CloseCallback& cb) {
        closeCallback_ = cb;
    }

    // ==================== 内部使用 ====================

    /// 连接建立，由 TcpServer 调用
    void connectEstablished();

    /// 连接销毁，由 TcpServer 调用
    void connectDestroyed();

    /// 获取输入缓冲区
    Buffer* inputBuffer() { return &inputBuffer_; }

    /// 获取输出缓冲区
    Buffer* outputBuffer() { return &outputBuffer_; }

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    void setState(StateE s) { state_ = s; }
    const char* stateToString() const;

    EventLoop* loop_;
    const std::string name_;
    std::atomic<StateE> state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_TCP_CONNECTION_H

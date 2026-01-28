// src/net/callbacks.h
#ifndef KVSTORE_NET_CALLBACKS_H
#define KVSTORE_NET_CALLBACKS_H

#include "base/timestamp.h"

#include <functional>
#include <memory>

namespace kvstore {

// 前向声明
class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

/// 连接建立/断开回调
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

/// 消息到达回调
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

/// 写完成回调
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

/// 高水位回调（发送缓冲区积压过多时触发）
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

/// 连接关闭回调（内部使用）
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

/// 定时器回调 
using TimerCallback = std::function<void()>;

}  // namespace kvstore

#endif  // KVSTORE_NET_CALLBACKS_H

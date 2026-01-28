// src/net/channel.h
#ifndef KVSTORE_NET_CHANNEL_H
#define KVSTORE_NET_CHANNEL_H

#include "base/noncopyable.h"
#include "base/timestamp.h"

#include <functional>
#include <memory>

namespace kvstore {

class EventLoop;

/**
 * @brief 事件通道
 *
 * Channel 是 fd 和 EventLoop 之间的桥梁。
 * 一个 Channel 对象负责一个 fd 的事件分发，但不拥有 fd。
 *
 * 核心职责：
 * 1. 保存 fd 关注的事件（读/写）
 * 2. 保存实际发生的事件
 * 3. 根据事件类型调用对应的回调函数
 *
 * 生命周期：
 * - Channel 不负责关闭 fd，由 fd 的拥有者（如 Socket、TcpConnection）负责
 * - TcpConnection 中的 Channel 由 TcpConnection 持有
 */
class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    /// 处理事件（由 EventLoop::loop() 调用）
    void handleEvent(Timestamp receiveTime);

    // ==================== 设置回调 ====================

    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // ==================== 防止 TcpConnection 被提前析构 ====================

    /// 绑定 TcpConnection 的弱引用
    void tie(const std::shared_ptr<void>& obj);

    // ==================== Getters ====================

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }  // Poller 设置实际发生的事件

    // ==================== 事件控制 ====================

    /// 注册读事件
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    /// 取消读事件
    void disableReading() {
        events_ &= ~kReadEvent;
        update();
    }
    /// 注册写事件
    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }
    /// 取消写事件
    void disableWriting() {
        events_ &= ~kWriteEvent;
        update();
    }
    /// 取消所有事件
    void disableAll() {
        events_ = kNoneEvent;
        update();
    }

    /// 是否注册了写事件
    bool isWriting() const { return events_ & kWriteEvent; }
    /// 是否注册了读事件
    bool isReading() const { return events_ & kReadEvent; }
    /// 是否没有关注任何事件
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    // ==================== for Poller ====================

    int index() const { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }

    /// 从 EventLoop 中移除自己
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;    // 所属的 EventLoop
    const int fd_;       // 监听的文件描述符
    int events_;         // 关注的事件
    int revents_;        // 实际发生的事件
    int index_;          // poller 使用的状态标识

    std::weak_ptr<void> tie_;  // 绑定的对象（通常是 TcpConnection）
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_CHANNEL_H

// src/net/poller.h
#ifndef KVSTORE_NET_POLLER_H
#define KVSTORE_NET_POLLER_H

#include "base/noncopyable.h"
#include "base/timestamp.h"

#include <vector>
#include <map>

namespace kvstore {

class Channel;
class EventLoop;

/**
 * @brief IO 多路复用抽象基类
 *
 * Poller 是对 IO 多路复用的抽象（epoll/poll/select）。
 * 本项目仅实现 EpollPoller。
 */
class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    /// 等待事件发生，填充 activeChannels
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    /// 更新 Channel 的事件
    virtual void updateChannel(Channel* channel) = 0;

    /// 移除 Channel
    virtual void removeChannel(Channel* channel) = 0;

    /// 判断 Channel 是否在 Poller 中
    virtual bool hasChannel(Channel* channel) const;

    /// 获取所属 EventLoop
    EventLoop* ownerLoop() const { return ownerLoop_; }

protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;  // fd -> Channel*

private:
    EventLoop* ownerLoop_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_POLLER_H

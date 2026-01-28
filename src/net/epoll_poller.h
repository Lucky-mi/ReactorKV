// src/net/epoll_poller.h
#ifndef KVSTORE_NET_EPOLL_POLLER_H
#define KVSTORE_NET_EPOLL_POLLER_H

#include "net/poller.h"

#include <sys/epoll.h>
#include <vector>

namespace kvstore {

/**
 * @brief Epoll 实现
 *
 * 使用 epoll 实现 IO 多路复用，采用 ET（边缘触发）模式。
 *
 * ET 模式特点：
 * 1. 高效：只在状态变化时通知
 * 2. 必须配合非阻塞 IO 使用
 * 3. 必须一次性读取/写入所有数据
 */
class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    /// 调用 epoll_wait，返回活跃的 Channel
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

    /// 更新 Channel 的事件
    void updateChannel(Channel* channel) override;

    /// 移除 Channel
    void removeChannel(Channel* channel) override;

private:
    /// 填充活跃的 Channel 列表
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    /// 调用 epoll_ctl
    void update(int operation, Channel* channel);

    static const int kInitEventListSize = 16;

    int epollfd_;
    std::vector<struct epoll_event> events_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_EPOLL_POLLER_H

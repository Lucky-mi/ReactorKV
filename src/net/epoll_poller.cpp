// src/net/epoll_poller.cpp
#include "net/epoll_poller.h"
#include "net/channel.h"
#include "base/logger.h"

#include <unistd.h>
#include <cstring>

namespace kvstore {

// Channel 在 Poller 中的状态
const int kNew = -1;     // 新的，还未添加到 epoll
const int kAdded = 1;    // 已添加到 epoll
const int kDeleted = 2;  // 已从 epoll 删除

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL << "EpollPoller::EpollPoller epoll_create1 failed";
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, events_.data(),
                                  static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);

        // 如果事件数量达到上限，扩容
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_ERROR << "EpollPoller::poll() error";
        }
    }

    return now;
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (int i = 0; i < numEvents; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    int index = channel->index();
    int fd = channel->fd();

    if (index == kNew || index == kDeleted) {
        // 新的 Channel，添加到 epoll
        if (index == kNew) {
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // 已在 epoll 中，修改或删除
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    int index = channel->index();

    channels_.erase(fd);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events() | EPOLLET;  // 使用 ET 模式
    event.data.ptr = channel;
    int fd = channel->fd();

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl DEL error, fd=" << fd;
        } else {
            LOG_FATAL << "epoll_ctl ADD/MOD error, fd=" << fd;
        }
    }
}

}  // namespace kvstore

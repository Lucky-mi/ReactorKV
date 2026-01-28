// src/net/poller.cpp
#include "net/poller.h"
#include "net/channel.h"

namespace kvstore {

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {}

bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

}  // namespace kvstore

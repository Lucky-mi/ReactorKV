// src/net/eventloop_thread_pool.cpp
#include "net/eventloop_thread_pool.h"
#include "net/eventloop_thread.h"
#include "net/eventloop.h"

namespace kvstore {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      numThreads_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
    // EventLoopThread 会自动清理
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    started_ = true;

    for (int i = 0; i < numThreads_; i++) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    // 如果没有创建额外线程，使用 baseLoop
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();

    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_];
        next_ = (next_ + 1) % loops_.size();
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();

    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, baseLoop_);
    } else {
        return loops_;
    }
}

}  // namespace kvstore

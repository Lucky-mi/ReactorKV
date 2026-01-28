// src/base/threadpool.cpp
#include "base/threadpool.h"
#include "base/logger.h"

#include <cassert>
#include <cstdio>

namespace kvstore {

ThreadPool::ThreadPool(const std::string& name)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(name),
      maxQueueSize_(0),
      running_(false) {}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    
    for (int i = 0; i < numThreads; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        threads_.emplace_back(new Thread(
            std::bind(&ThreadPool::runInThread, this),
            name_ + id
        ));
        threads_[i]->start();
    }
    
    // 如果线程数为 0，直接在当前线程运行初始化回调
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    {
        MutexLockGuard lock(mutex_);
        running_ = false;
        notEmpty_.notifyAll();
        notFull_.notifyAll();
    }
    
    for (auto& thr : threads_) {
        thr->join();
    }
}

size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task task) {
    if (threads_.empty()) {
        // 没有工作线程，直接在当前线程执行
        task();
    } else {
        MutexLockGuard lock(mutex_);
        while (isFull() && running_) {
            notFull_.wait();
        }
        if (!running_) {
            return;
        }
        assert(!isFull());
        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    while (queue_.empty() && running_) {
        notEmpty_.wait();
    }
    
    Task task;
    if (!queue_.empty()) {
        task = std::move(queue_.front());
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            notFull_.notify();
        }
    }
    return task;
}

bool ThreadPool::isFull() const {
    // 调用者需持有锁
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() {
    try {
        if (threadInitCallback_) {
            threadInitCallback_();
        }
        
        while (running_) {
            Task task(take());
            if (task) {
                task();
            }
        }
    } catch (const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    } catch (...) {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw;
    }
}

}  // namespace kvstore

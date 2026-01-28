// src/base/blocking_queue.h
#ifndef KVSTORE_BASE_BLOCKING_QUEUE_H
#define KVSTORE_BASE_BLOCKING_QUEUE_H

#include "base/mutex.h"

#include <deque>
#include <cassert>

namespace kvstore {

/**
 * @brief 无界阻塞队列
 * 
 * 线程安全的生产者-消费者队列。
 * 生产者调用 put() 放入数据，消费者调用 take() 取出数据。
 * 如果队列为空，take() 会阻塞等待。
 * 
 * @tparam T 队列元素类型
 */
template <typename T>
class BlockingQueue : noncopyable {
public:
    BlockingQueue()
        : mutex_(),
          notEmpty_(mutex_),
          queue_() {}

    /// 放入元素（生产者）
    void put(const T& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify();
    }

    /// 放入元素（移动语义）
    void put(T&& x) {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }

    /// 取出元素（消费者，阻塞）
    T take() {
        MutexLockGuard lock(mutex_);
        while (queue_.empty()) {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return front;
    }

    /// 获取队列长度
    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    /// 队列是否为空
    bool empty() const {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

private:
    mutable MutexLock mutex_;
    Condition notEmpty_;
    std::deque<T> queue_;
};

/**
 * @brief 有界阻塞队列
 * 
 * 带容量限制的阻塞队列。
 * 当队列满时，put() 会阻塞等待。
 * 
 * @tparam T 队列元素类型
 */
template <typename T>
class BoundedBlockingQueue : noncopyable {
public:
    explicit BoundedBlockingQueue(size_t maxSize)
        : mutex_(),
          notEmpty_(mutex_),
          notFull_(mutex_),
          queue_(),
          maxSize_(maxSize) {}

    /// 放入元素（阻塞直到有空间）
    void put(const T& x) {
        MutexLockGuard lock(mutex_);
        while (queue_.size() >= maxSize_) {
            notFull_.wait();
        }
        assert(queue_.size() < maxSize_);
        queue_.push_back(x);
        notEmpty_.notify();
    }

    /// 放入元素（移动语义）
    void put(T&& x) {
        MutexLockGuard lock(mutex_);
        while (queue_.size() >= maxSize_) {
            notFull_.wait();
        }
        assert(queue_.size() < maxSize_);
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }

    /// 取出元素（阻塞直到有数据）
    T take() {
        MutexLockGuard lock(mutex_);
        while (queue_.empty()) {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        notFull_.notify();
        return front;
    }

    /// 队列是否为空
    bool empty() const {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    /// 队列是否已满
    bool full() const {
        MutexLockGuard lock(mutex_);
        return queue_.size() >= maxSize_;
    }

    /// 获取队列长度
    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    /// 获取容量
    size_t capacity() const {
        return maxSize_;
    }

private:
    mutable MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    std::deque<T> queue_;
    const size_t maxSize_;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_BLOCKING_QUEUE_H

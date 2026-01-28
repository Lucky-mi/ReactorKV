// src/base/count_down_latch.h
#ifndef KVSTORE_BASE_COUNT_DOWN_LATCH_H
#define KVSTORE_BASE_COUNT_DOWN_LATCH_H

#include "base/mutex.h"

namespace kvstore {

/**
 * @brief 倒计时门闩
 * 
 * 用于线程同步，等待一个或多个线程完成操作。
 * 典型用法：主线程等待子线程初始化完成。
 */
class CountDownLatch : noncopyable {
public:
    explicit CountDownLatch(int count)
        : mutex_(),
          condition_(mutex_),
          count_(count) {}

    /// 等待计数归零
    void wait() {
        MutexLockGuard lock(mutex_);
        while (count_ > 0) {
            condition_.wait();
        }
    }

    /// 减少计数
    void countDown() {
        MutexLockGuard lock(mutex_);
        --count_;
        if (count_ == 0) {
            condition_.notifyAll();
        }
    }

    /// 获取当前计数
    int getCount() const {
        MutexLockGuard lock(mutex_);
        return count_;
    }

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_COUNT_DOWN_LATCH_H

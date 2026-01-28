// src/base/mutex.h
#ifndef KVSTORE_BASE_MUTEX_H
#define KVSTORE_BASE_MUTEX_H

#include "base/noncopyable.h"
#include "base/current_thread.h"

#include <pthread.h>
#include <cassert>
#include <ctime>
#include <cerrno>
#include <cstdint>

namespace kvstore {

/**
 * @brief 互斥锁封装
 * 
 * 对 pthread_mutex_t 的 RAII 封装。
 * 配合 MutexLockGuard 使用，确保锁一定会被释放。
 * 
 * 特点：
 * 1. 记录持有者线程 ID，支持 isLockedByThisThread() 检查
 * 2. 析构时断言没有线程持有锁
 */
class MutexLock : noncopyable {
public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~MutexLock() {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    /// 是否被当前线程持有
    bool isLockedByThisThread() const {
        return holder_ == CurrentThread::tid();
    }

    /// 断言被当前线程持有（调试用）
    void assertLocked() const {
        assert(isLockedByThisThread());
    }

    /// 加锁
    void lock() {
        pthread_mutex_lock(&mutex_);
        assignHolder();
    }

    /// 解锁
    void unlock() {
        unassignHolder();
        pthread_mutex_unlock(&mutex_);
    }

    /// 获取底层 pthread_mutex_t 指针（仅供 Condition 使用）
    pthread_mutex_t* getPthreadMutex() {
        return &mutex_;
    }

private:
    friend class Condition;

    /**
     * @brief UnassignGuard - 仅供 Condition 使用
     * 
     * 在 Condition::wait() 中，pthread_cond_wait 会原子地释放锁，
     * 所以需要临时清除 holder_。
     */
    class UnassignGuard : noncopyable {
    public:
        explicit UnassignGuard(MutexLock& owner) : owner_(owner) {
            owner_.unassignHolder();
        }

        ~UnassignGuard() {
            owner_.assignHolder();
        }

    private:
        MutexLock& owner_;
    };

    void assignHolder() {
        holder_ = CurrentThread::tid();
    }

    void unassignHolder() {
        holder_ = 0;
    }

    pthread_mutex_t mutex_;
    pid_t holder_;  // 持有锁的线程 ID
};

/**
 * @brief RAII 风格的锁守卫
 * 
 * 使用方式：
 *   MutexLock mutex;
 *   {
 *       MutexLockGuard lock(mutex);
 *       // 临界区代码
 *   }  // 自动释放锁
 */
class MutexLockGuard : noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock& mutex_;
};

// 防止误用：MutexLockGuard(mutex); 临时对象会立即析构
// 使用负数 sizeof 触发编译错误，兼容所有 C++ 版本
#define MutexLockGuard(x) \
    static_assert(sizeof(x) == -1, "Missing variable name for MutexLockGuard")

/**
 * @brief 条件变量封装
 * 
 * 对 pthread_cond_t 的封装，必须配合 MutexLock 使用。
 * 
 * 典型用法（生产者-消费者）：
 *   MutexLock mutex;
 *   Condition cond(mutex);
 * 
 *   // 消费者
 *   MutexLockGuard lock(mutex);
 *   while (queue.empty()) {
 *       cond.wait();  // 等待数据
 *   }
 *   // 处理数据
 * 
 *   // 生产者
 *   MutexLockGuard lock(mutex);
 *   queue.push(data);
 *   cond.notify();  // 通知消费者
 */
class Condition : noncopyable {
public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, nullptr);
    }

    ~Condition() {
        pthread_cond_destroy(&cond_);
    }

    /// 等待条件满足
    void wait() {
        MutexLock::UnassignGuard ug(mutex_);
        pthread_cond_wait(&cond_, mutex_.getPthreadMutex());
    }

    /// 等待指定秒数，返回 true 表示被唤醒，false 表示超时
    bool waitForSeconds(double seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        abstime.tv_nsec += static_cast<long>(nanoseconds % kNanoSecondsPerSecond);

        if (abstime.tv_nsec >= kNanoSecondsPerSecond) {
            abstime.tv_sec++;
            abstime.tv_nsec -= kNanoSecondsPerSecond;
        }

        MutexLock::UnassignGuard ug(mutex_);
        return ETIMEDOUT != pthread_cond_timedwait(&cond_, mutex_.getPthreadMutex(), &abstime);
    }

    /// 唤醒一个等待线程
    void notify() {
        pthread_cond_signal(&cond_);
    }

    /// 唤醒所有等待线程
    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }

private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_MUTEX_H

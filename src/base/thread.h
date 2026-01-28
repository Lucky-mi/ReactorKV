// src/base/thread.h
#ifndef KVSTORE_BASE_THREAD_H
#define KVSTORE_BASE_THREAD_H

#include "base/noncopyable.h"
#include "base/count_down_latch.h"

#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <pthread.h>

namespace kvstore {

/**
 * @brief 线程封装类
 * 
 * 对 pthread 的 RAII 封装，支持：
 * 1. 线程函数和名称
 * 2. 等待线程启动完成
 * 3. join 等待线程结束
 */
class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();

    /// 启动线程
    void start();
    
    /// 等待线程结束
    void join();

    /// 是否已启动
    bool started() const { return started_; }
    
    /// 获取线程 ID
    pid_t tid() const { return tid_; }
    
    /// 获取线程名称
    const std::string& name() const { return name_; }

    /// 获取已创建线程数
    static int numCreated() { return numCreated_.load(); }

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;

    static std::atomic<int> numCreated_;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_THREAD_H

// src/base/threadpool.h
#ifndef KVSTORE_BASE_THREADPOOL_H
#define KVSTORE_BASE_THREADPOOL_H

#include "base/noncopyable.h"
#include "base/mutex.h"
#include "base/thread.h"

#include <functional>
#include <memory>
#include <vector>
#include <deque>
#include <string>

namespace kvstore {

/**
 * @brief 线程池
 * 
 * 固定大小的线程池，支持任务队列。
 * 
 * 使用方式：
 *   ThreadPool pool("WorkerPool");
 *   pool.setMaxQueueSize(100);  // 可选：设置任务队列大小
 *   pool.start(4);              // 启动 4 个工作线程
 *   pool.run(task);             // 提交任务
 *   pool.stop();                // 停止线程池
 */
class ThreadPool : noncopyable {
public:
    using Task = std::function<void()>;

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    /// 设置任务队列最大长度（0 表示无限制）
    void setMaxQueueSize(size_t maxSize) { maxQueueSize_ = maxSize; }

    /// 设置线程初始化回调
    void setThreadInitCallback(const Task& cb) { threadInitCallback_ = cb; }

    /// 启动线程池
    void start(int numThreads);

    /// 停止线程池
    void stop();

    /// 获取线程池名称
    const std::string& name() const { return name_; }

    /// 获取任务队列长度
    size_t queueSize() const;

    /// 提交任务
    void run(Task task);

private:
    bool isFull() const;
    void runInThread();
    Task take();

    mutable MutexLock mutex_;
    Condition notEmpty_;
    Condition notFull_;
    std::string name_;
    Task threadInitCallback_;
    std::vector<std::unique_ptr<Thread>> threads_;
    std::deque<Task> queue_;
    size_t maxQueueSize_;
    bool running_;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_THREADPOOL_H

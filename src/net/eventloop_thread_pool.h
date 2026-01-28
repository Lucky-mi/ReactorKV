// src/net/eventloop_thread_pool.h
#ifndef KVSTORE_NET_EVENTLOOP_THREAD_POOL_H
#define KVSTORE_NET_EVENTLOOP_THREAD_POOL_H

#include "base/noncopyable.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace kvstore {

class EventLoop;
class EventLoopThread;

/**
 * @brief 事件循环线程池
 *
 * 用于创建多个 SubReactor 线程。
 * 主 Reactor（baseLoop）负责接受连接，SubReactor 负责处理连接 IO。
 *
 * 使用示例：
 *   EventLoopThreadPool pool(loop, "IOPool");
 *   pool.setThreadNum(4);  // 4 个 SubReactor
 *   pool.start();
 *   EventLoop* ioLoop = pool.getNextLoop();  // round-robin
 */
class EventLoopThreadPool : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();

    /// 设置线程数量
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    /// 启动线程池
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    /// 获取下一个 EventLoop（round-robin）
    EventLoop* getNextLoop();

    /// 获取所有 EventLoop
    std::vector<EventLoop*> getAllLoops();

    /// 是否已启动
    bool started() const { return started_; }

    /// 获取名称
    const std::string& name() const { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_EVENTLOOP_THREAD_POOL_H

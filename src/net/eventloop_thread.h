// src/net/eventloop_thread.h
#ifndef KVSTORE_NET_EVENTLOOP_THREAD_H
#define KVSTORE_NET_EVENTLOOP_THREAD_H

#include "base/noncopyable.h"
#include "base/mutex.h"
#include "base/thread.h"

#include <functional>

namespace kvstore {

class EventLoop;

/**
 * @brief 事件循环线程
 *
 * 封装了一个运行 EventLoop 的线程。
 * 启动后会创建 EventLoop 并在新线程中运行。
 *
 * 使用示例：
 *   EventLoopThread loopThread;
 *   EventLoop* loop = loopThread.startLoop();
 *   // loop 在新线程中运行
 */
class EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    /// 启动线程，返回线程中的 EventLoop 指针
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    ThreadInitCallback callback_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_EVENTLOOP_THREAD_H

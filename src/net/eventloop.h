// src/net/eventloop.h
#ifndef KVSTORE_NET_EVENTLOOP_H
#define KVSTORE_NET_EVENTLOOP_H

#include "base/noncopyable.h"
#include "base/mutex.h"
#include "base/current_thread.h"
#include "base/timestamp.h"

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace kvstore {

class Channel;
class Poller;

/**
 * @brief 事件循环 (Reactor 核心)
 *
 * EventLoop 是 Reactor 模式的核心，每个线程最多一个 EventLoop。
 * 主要职责：
 * 1. 运行事件循环，调用 Poller::poll() 等待事件
 * 2. 处理活跃的 Channel 上的事件
 * 3. 处理跨线程调用（通过 runInLoop/queueInLoop）
 *
 * 线程安全：
 * - loop() 必须在创建 EventLoop 的线程中调用
 * - runInLoop()/queueInLoop() 可以跨线程调用
 *
 * wakeup 机制：
 * - 使用 eventfd 唤醒可能阻塞在 poll() 中的线程
 * - 当有跨线程任务提交时，需要唤醒以及时执行
 */
class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    /// 开始事件循环，必须在创建 EventLoop 的线程中调用
    void loop();

    /// 退出事件循环
    void quit();

    /// 获取 poll 返回的时间戳
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // ==================== 跨线程任务调度 ====================

    /**
     * @brief 在 EventLoop 所在线程执行回调
     *
     * 如果在当前线程调用，立即执行；
     * 否则加入队列，等待 EventLoop 执行。
     */
    void runInLoop(Functor cb);

    /**
     * @brief 将回调加入待执行队列
     *
     * 可以跨线程调用。
     */
    void queueInLoop(Functor cb);

    /// 唤醒阻塞在 poll() 中的 EventLoop
    void wakeup();

    // ==================== Channel 管理 ====================

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // ==================== 线程检查 ====================

    /// 是否在创建 EventLoop 的线程中
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    /// 断言在 EventLoop 线程中（调试用）
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    /// 获取当前线程的 EventLoop
    static EventLoop* getEventLoopOfCurrentThread();

private:
    void abortNotInLoopThread();
    void handleRead();  // 处理 wakeupFd_ 的可读事件
    void doPendingFunctors();  // 执行待处理的回调

    using ChannelList = std::vector<Channel*>;

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    std::atomic<bool> eventHandling_;
    std::atomic<bool> callingPendingFunctors_;

    const pid_t threadId_;  // 创建 EventLoop 的线程 ID
    Timestamp pollReturnTime_;

    std::unique_ptr<Poller> poller_;

    int wakeupFd_;  // eventfd，用于唤醒
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;  // 待执行的回调
};

}  // namespace kvstore

#endif  // KVSTORE_NET_EVENTLOOP_H

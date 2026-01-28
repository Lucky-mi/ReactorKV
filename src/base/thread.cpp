// src/base/thread.cpp
#include "base/thread.h"
#include "base/current_thread.h"

#include <cassert>
#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

namespace kvstore {

std::atomic<int> Thread::numCreated_(0);

namespace {

/// 线程数据，传递给 pthread_create
struct ThreadData {
    using ThreadFunc = Thread::ThreadFunc;
    
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(ThreadFunc func, const std::string& name, pid_t* tid, CountDownLatch* latch)
        : func_(std::move(func)),
          name_(name),
          tid_(tid),
          latch_(latch) {}

    void runInThread() {
        *tid_ = static_cast<pid_t>(::syscall(SYS_gettid));
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;

        CurrentThread::t_threadName = name_.empty() ? "kvstoreThread" : name_.c_str();

        try {
            func_();
            CurrentThread::t_threadName = "finished";
        } catch (const std::exception& ex) {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        } catch (...) {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw;
        }
    }
};

/// pthread 启动函数
void* startThread(void* obj) {
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return nullptr;
}

}  // namespace

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(std::move(func)),
      name_(name),
      latch_(1) {
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    
    if (pthread_create(&pthreadId_, nullptr, &startThread, data)) {
        started_ = false;
        delete data;
        // LOG_FATAL << "Failed in pthread_create";
        abort();
    } else {
        latch_.wait();  // 等待线程启动完成
        assert(tid_ > 0);
    }
}

void Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    pthread_join(pthreadId_, nullptr);
}

}  // namespace kvstore

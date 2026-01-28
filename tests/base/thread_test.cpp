// tests/base/thread_test.cpp
#include "base/thread.h"
#include "base/current_thread.h"
#include "base/count_down_latch.h"

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>

using namespace kvstore;

class ThreadTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试线程基本功能
TEST_F(ThreadTest, BasicThread) {
    std::atomic<int> counter{0};
    
    Thread t([&counter]() {
        counter.fetch_add(1);
    }, "TestThread");
    
    EXPECT_FALSE(t.started());
    t.start();
    EXPECT_TRUE(t.started());
    t.join();
    
    EXPECT_EQ(counter.load(), 1);
}

// 测试线程 ID
TEST_F(ThreadTest, ThreadId) {
    pid_t tid = 0;
    
    Thread t([&tid]() {
        tid = CurrentThread::tid();
    }, "TidTestThread");
    
    t.start();
    t.join();
    
    EXPECT_GT(tid, 0);
    EXPECT_NE(tid, CurrentThread::tid());  // 不同于当前线程
}

// 测试多个线程
TEST_F(ThreadTest, MultipleThreads) {
    const int kNumThreads = 5;
    std::atomic<int> counter{0};
    std::vector<std::unique_ptr<Thread>> threads;
    
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back(new Thread([&counter]() {
            counter.fetch_add(1);
        }, "Worker" + std::to_string(i)));
    }
    
    for (auto& t : threads) {
        t->start();
    }
    
    for (auto& t : threads) {
        t->join();
    }
    
    EXPECT_EQ(counter.load(), kNumThreads);
}

// 测试 CountDownLatch
TEST_F(ThreadTest, CountDownLatch) {
    CountDownLatch latch(3);
    std::atomic<int> counter{0};
    
    std::vector<std::unique_ptr<Thread>> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(new Thread([&latch, &counter]() {
            counter.fetch_add(1);
            latch.countDown();
        }, "LatchThread" + std::to_string(i)));
    }
    
    for (auto& t : threads) {
        t->start();
    }
    
    latch.wait();  // 等待所有线程完成
    EXPECT_EQ(counter.load(), 3);
    
    for (auto& t : threads) {
        t->join();
    }
}

// 测试线程名称
TEST_F(ThreadTest, ThreadName) {
    Thread t1([]() {}, "MyThread");
    EXPECT_EQ(t1.name(), "MyThread");
    
    Thread t2([]() {});  // 默认名称
    EXPECT_TRUE(t2.name().find("Thread") != std::string::npos);
}

// 测试 numCreated 计数
TEST_F(ThreadTest, NumCreated) {
    int before = Thread::numCreated();
    
    Thread t1([]() {}, "T1");
    Thread t2([]() {}, "T2");
    
    EXPECT_EQ(Thread::numCreated(), before + 2);
}

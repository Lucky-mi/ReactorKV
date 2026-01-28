// tests/base/threadpool_test.cpp
#include "base/threadpool.h"
#include "base/count_down_latch.h"

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace kvstore;

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试基本任务执行
TEST_F(ThreadPoolTest, BasicExecution) {
    ThreadPool pool("TestPool");
    pool.start(4);
    
    std::atomic<int> counter{0};
    CountDownLatch latch(10);
    
    for (int i = 0; i < 10; ++i) {
        pool.run([&counter, &latch]() {
            counter.fetch_add(1);
            latch.countDown();
        });
    }
    
    latch.wait();
    EXPECT_EQ(counter.load(), 10);
    
    pool.stop();
}

// 测试任务队列大小限制
TEST_F(ThreadPoolTest, QueueSize) {
    ThreadPool pool("QueuePool");
    pool.setMaxQueueSize(5);
    pool.start(2);
    
    std::atomic<int> counter{0};
    CountDownLatch latch(10);
    
    for (int i = 0; i < 10; ++i) {
        pool.run([&counter, &latch]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter.fetch_add(1);
            latch.countDown();
        });
    }
    
    latch.wait();
    EXPECT_EQ(counter.load(), 10);
    
    pool.stop();
}

// 测试空线程池（直接在当前线程执行）
TEST_F(ThreadPoolTest, ZeroThreads) {
    ThreadPool pool("EmptyPool");
    pool.start(0);
    
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 5; ++i) {
        pool.run([&counter]() {
            counter.fetch_add(1);
        });
    }
    
    EXPECT_EQ(counter.load(), 5);  // 应该立即执行完成
    
    pool.stop();
}

// 测试初始化回调
TEST_F(ThreadPoolTest, InitCallback) {
    ThreadPool pool("InitPool");
    std::atomic<int> initCount{0};
    
    pool.setThreadInitCallback([&initCount]() {
        initCount.fetch_add(1);
    });
    
    pool.start(4);
    
    // 给线程一点时间执行初始化
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_EQ(initCount.load(), 4);
    
    pool.stop();
}

// 测试并发执行
TEST_F(ThreadPoolTest, ConcurrentExecution) {
    ThreadPool pool("ConcurrentPool");
    pool.start(4);
    
    std::atomic<int> maxConcurrent{0};
    std::atomic<int> currentConcurrent{0};
    CountDownLatch latch(20);
    
    for (int i = 0; i < 20; ++i) {
        pool.run([&]() {
            int cur = currentConcurrent.fetch_add(1) + 1;
            
            // 更新最大并发数
            int max = maxConcurrent.load();
            while (cur > max && !maxConcurrent.compare_exchange_weak(max, cur)) {
                // retry
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            currentConcurrent.fetch_sub(1);
            latch.countDown();
        });
    }
    
    latch.wait();
    
    // 最大并发数应该接近线程池大小
    EXPECT_GE(maxConcurrent.load(), 2);
    EXPECT_LE(maxConcurrent.load(), 4);
    
    pool.stop();
}

// 测试线程池名称
TEST_F(ThreadPoolTest, PoolName) {
    ThreadPool pool("MyTestPool");
    EXPECT_EQ(pool.name(), "MyTestPool");
}

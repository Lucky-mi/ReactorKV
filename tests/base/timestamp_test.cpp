// tests/base/timestamp_test.cpp
#include "base/timestamp.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace kvstore;

class TimestampTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试默认构造
TEST_F(TimestampTest, DefaultConstructor) {
    Timestamp ts;
    EXPECT_FALSE(ts.valid());
    EXPECT_EQ(ts.microSecondsSinceEpoch(), 0);
}

// 测试 now()
TEST_F(TimestampTest, Now) {
    Timestamp ts = Timestamp::now();
    EXPECT_TRUE(ts.valid());
    EXPECT_GT(ts.microSecondsSinceEpoch(), 0);
}

// 测试时间差计算
TEST_F(TimestampTest, TimeDifference) {
    Timestamp t1 = Timestamp::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Timestamp t2 = Timestamp::now();
    
    double diff = timeDifference(t2, t1);
    // 允许 10ms 误差
    EXPECT_GE(diff, 0.09);
    EXPECT_LE(diff, 0.15);
}

// 测试时间加法
TEST_F(TimestampTest, AddTime) {
    Timestamp t1 = Timestamp::now();
    Timestamp t2 = addTime(t1, 1.5);  // 加 1.5 秒
    
    double diff = timeDifference(t2, t1);
    EXPECT_DOUBLE_EQ(diff, 1.5);
}

// 测试比较运算符
TEST_F(TimestampTest, Comparison) {
    Timestamp t1 = Timestamp::now();
    Timestamp t2 = addTime(t1, 1.0);
    
    EXPECT_TRUE(t1 < t2);
    EXPECT_FALSE(t1 == t2);
    EXPECT_TRUE(t1 != t2);
}

// 测试字符串转换
TEST_F(TimestampTest, ToString) {
    Timestamp ts = Timestamp::now();
    std::string str = ts.toString();
    
    // 应该包含小数点
    EXPECT_NE(str.find('.'), std::string::npos);
}

// 测试格式化字符串
TEST_F(TimestampTest, ToFormattedString) {
    Timestamp ts = Timestamp::now();
    
    std::string withMicro = ts.toFormattedString(true);
    std::string withoutMicro = ts.toFormattedString(false);
    
    // 带微秒的更长
    EXPECT_GT(withMicro.length(), withoutMicro.length());
    
    // 应该包含日期分隔符
    EXPECT_NE(withMicro.find('-'), std::string::npos);
    EXPECT_NE(withMicro.find(':'), std::string::npos);
}

// 并发测试：多线程调用 now()
TEST_F(TimestampTest, ConcurrentNow) {
    const int kNumThreads = 10;
    const int kIterations = 1000;
    
    std::vector<std::thread> threads;
    std::vector<Timestamp> timestamps(kNumThreads * kIterations);
    
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&timestamps, i, kIterations]() {
            for (int j = 0; j < kIterations; ++j) {
                timestamps[i * kIterations + j] = Timestamp::now();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // 所有时间戳都应该有效
    for (const auto& ts : timestamps) {
        EXPECT_TRUE(ts.valid());
    }
}

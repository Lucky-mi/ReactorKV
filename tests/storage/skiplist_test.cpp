// tests/storage/skiplist_test.cpp
#include "storage/skiplist.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace kvstore;

class SkipListTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试用例前清空
        skiplist_.clear();
    }

    SkipList<std::string, std::string> skiplist_;
};

// ==================== 基本功能测试 ====================

TEST_F(SkipListTest, InsertAndSearch) {
    // 插入
    EXPECT_TRUE(skiplist_.insert("key1", "value1"));
    EXPECT_TRUE(skiplist_.insert("key2", "value2"));
    EXPECT_TRUE(skiplist_.insert("key3", "value3"));

    EXPECT_EQ(skiplist_.size(), 3);

    // 查询
    std::string value;
    EXPECT_TRUE(skiplist_.search("key1", value));
    EXPECT_EQ(value, "value1");

    EXPECT_TRUE(skiplist_.search("key2", value));
    EXPECT_EQ(value, "value2");

    EXPECT_TRUE(skiplist_.search("key3", value));
    EXPECT_EQ(value, "value3");
}

TEST_F(SkipListTest, UpdateExistingKey) {
    // 插入
    EXPECT_TRUE(skiplist_.insert("key1", "value1"));
    EXPECT_EQ(skiplist_.size(), 1);

    // 更新同一个 key
    EXPECT_FALSE(skiplist_.insert("key1", "new_value1"));  // 返回 false 表示更新
    EXPECT_EQ(skiplist_.size(), 1);  // 数量不变

    // 验证更新后的值
    std::string value;
    EXPECT_TRUE(skiplist_.search("key1", value));
    EXPECT_EQ(value, "new_value1");
}

TEST_F(SkipListTest, Remove) {
    skiplist_.insert("key1", "value1");
    skiplist_.insert("key2", "value2");
    skiplist_.insert("key3", "value3");

    EXPECT_EQ(skiplist_.size(), 3);

    // 删除存在的 key
    EXPECT_TRUE(skiplist_.remove("key2"));
    EXPECT_EQ(skiplist_.size(), 2);

    // 确认已删除
    std::string value;
    EXPECT_FALSE(skiplist_.search("key2", value));

    // 其他 key 不受影响
    EXPECT_TRUE(skiplist_.search("key1", value));
    EXPECT_TRUE(skiplist_.search("key3", value));
}

TEST_F(SkipListTest, RemoveNonExistent) {
    skiplist_.insert("key1", "value1");

    // 删除不存在的 key
    EXPECT_FALSE(skiplist_.remove("nonexistent"));
    EXPECT_EQ(skiplist_.size(), 1);
}

TEST_F(SkipListTest, Contains) {
    skiplist_.insert("key1", "value1");

    EXPECT_TRUE(skiplist_.contains("key1"));
    EXPECT_FALSE(skiplist_.contains("key2"));
}

TEST_F(SkipListTest, Clear) {
    skiplist_.insert("key1", "value1");
    skiplist_.insert("key2", "value2");
    skiplist_.insert("key3", "value3");

    EXPECT_EQ(skiplist_.size(), 3);

    skiplist_.clear();

    EXPECT_EQ(skiplist_.size(), 0);
    EXPECT_FALSE(skiplist_.contains("key1"));
}

// ==================== 边界情况测试 ====================

TEST_F(SkipListTest, EmptySkipList) {
    EXPECT_EQ(skiplist_.size(), 0);

    std::string value;
    EXPECT_FALSE(skiplist_.search("any", value));
    EXPECT_FALSE(skiplist_.remove("any"));
    EXPECT_FALSE(skiplist_.contains("any"));
}

TEST_F(SkipListTest, SingleElement) {
    skiplist_.insert("only", "one");

    EXPECT_EQ(skiplist_.size(), 1);

    std::string value;
    EXPECT_TRUE(skiplist_.search("only", value));
    EXPECT_EQ(value, "one");

    EXPECT_TRUE(skiplist_.remove("only"));
    EXPECT_EQ(skiplist_.size(), 0);
}

// ==================== 大数据量测试 ====================

TEST_F(SkipListTest, LargeDataSet) {
    const int count = 10000;

    // 插入大量数据
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        skiplist_.insert(key, value);
    }

    EXPECT_EQ(skiplist_.size(), count);

    // 验证所有数据
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(i);
        std::string expected = "value" + std::to_string(i);
        std::string value;
        ASSERT_TRUE(skiplist_.search(key, value)) << "Failed to find key: " << key;
        EXPECT_EQ(value, expected);
    }

    // 删除一半数据
    for (int i = 0; i < count; i += 2) {
        std::string key = "key" + std::to_string(i);
        EXPECT_TRUE(skiplist_.remove(key));
    }

    EXPECT_EQ(skiplist_.size(), count / 2);
}

TEST_F(SkipListTest, RandomOrder) {
    std::vector<int> numbers;
    for (int i = 0; i < 1000; i++) {
        numbers.push_back(i);
    }

    // 随机打乱顺序
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(numbers.begin(), numbers.end(), g);

    // 随机顺序插入
    for (int n : numbers) {
        std::string key = "key" + std::to_string(n);
        std::string value = "value" + std::to_string(n);
        skiplist_.insert(key, value);
    }

    EXPECT_EQ(skiplist_.size(), 1000);

    // 验证
    for (int n : numbers) {
        std::string key = "key" + std::to_string(n);
        std::string expected = "value" + std::to_string(n);
        std::string value;
        EXPECT_TRUE(skiplist_.search(key, value));
        EXPECT_EQ(value, expected);
    }
}

// ==================== 多线程测试 ====================

TEST_F(SkipListTest, ConcurrentInsert) {
    const int numThreads = 4;
    const int numPerThread = 1000;

    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; t++) {
        threads.emplace_back([this, t, numPerThread]() {
            for (int i = 0; i < numPerThread; i++) {
                std::string key = "thread" + std::to_string(t) + "_key" + std::to_string(i);
                std::string value = "value" + std::to_string(i);
                skiplist_.insert(key, value);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(skiplist_.size(), numThreads * numPerThread);
}

TEST_F(SkipListTest, ConcurrentReadWrite) {
    // 预先插入一些数据
    for (int i = 0; i < 100; i++) {
        skiplist_.insert("key" + std::to_string(i), "value" + std::to_string(i));
    }

    std::atomic<int> readCount(0);
    std::atomic<int> writeCount(0);

    std::vector<std::thread> threads;

    // 读线程
    for (int t = 0; t < 2; t++) {
        threads.emplace_back([this, &readCount]() {
            for (int i = 0; i < 1000; i++) {
                std::string key = "key" + std::to_string(i % 100);
                std::string value;
                if (skiplist_.search(key, value)) {
                    readCount++;
                }
            }
        });
    }

    // 写线程
    for (int t = 0; t < 2; t++) {
        threads.emplace_back([this, &writeCount, t]() {
            for (int i = 0; i < 500; i++) {
                std::string key = "new_key_" + std::to_string(t) + "_" + std::to_string(i);
                std::string value = "new_value" + std::to_string(i);
                skiplist_.insert(key, value);
                writeCount++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // 验证没有崩溃，数据一致
    EXPECT_GT(readCount.load(), 0);
    EXPECT_EQ(writeCount.load(), 1000);
}

// ==================== 持久化测试 ====================

TEST_F(SkipListTest, DumpAndLoad) {
    // 插入数据
    skiplist_.insert("name", "Alice");
    skiplist_.insert("age", "25");
    skiplist_.insert("city", "Beijing");

    // 保存到文件
    std::string filepath = "/tmp/skiplist_test.db";
    EXPECT_TRUE(skiplist_.dumpFile(filepath));

    // 创建新的跳表并加载
    SkipList<std::string, std::string> newSkiplist;
    EXPECT_TRUE(newSkiplist.loadFile(filepath));

    EXPECT_EQ(newSkiplist.size(), 3);

    std::string value;
    EXPECT_TRUE(newSkiplist.search("name", value));
    EXPECT_EQ(value, "Alice");

    EXPECT_TRUE(newSkiplist.search("age", value));
    EXPECT_EQ(value, "25");

    EXPECT_TRUE(newSkiplist.search("city", value));
    EXPECT_EQ(value, "Beijing");

    // 清理测试文件
    std::remove(filepath.c_str());
}

// ==================== 整数键跳表测试 ====================

TEST(IntSkipListTest, IntegerKeys) {
    SkipList<int, std::string> intSkiplist;

    intSkiplist.insert(3, "three");
    intSkiplist.insert(1, "one");
    intSkiplist.insert(2, "two");

    EXPECT_EQ(intSkiplist.size(), 3);

    std::string value;
    EXPECT_TRUE(intSkiplist.search(1, value));
    EXPECT_EQ(value, "one");

    EXPECT_TRUE(intSkiplist.search(2, value));
    EXPECT_EQ(value, "two");

    EXPECT_TRUE(intSkiplist.remove(2));
    EXPECT_FALSE(intSkiplist.search(2, value));
}

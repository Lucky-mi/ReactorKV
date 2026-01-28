// tests/storage/kvstore_test.cpp
#include "storage/kvstore.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

using namespace kvstore;

class KVStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        store_.clear();
    }

    KVStore store_;
};

// ==================== CRUD 测试 ====================

TEST_F(KVStoreTest, PutAndGet) {
    EXPECT_TRUE(store_.put("name", "Alice"));
    EXPECT_TRUE(store_.put("age", "25"));

    EXPECT_EQ(store_.size(), 2);

    std::string value;
    EXPECT_TRUE(store_.get("name", value));
    EXPECT_EQ(value, "Alice");

    EXPECT_TRUE(store_.get("age", value));
    EXPECT_EQ(value, "25");
}

TEST_F(KVStoreTest, UpdateExistingKey) {
    store_.put("key", "old_value");

    EXPECT_FALSE(store_.put("key", "new_value"));  // 更新返回 false

    std::string value;
    EXPECT_TRUE(store_.get("key", value));
    EXPECT_EQ(value, "new_value");
}

TEST_F(KVStoreTest, Delete) {
    store_.put("key1", "value1");
    store_.put("key2", "value2");

    EXPECT_TRUE(store_.del("key1"));
    EXPECT_EQ(store_.size(), 1);

    std::string value;
    EXPECT_FALSE(store_.get("key1", value));
    EXPECT_TRUE(store_.get("key2", value));
}

TEST_F(KVStoreTest, DeleteNonExistent) {
    store_.put("key", "value");

    EXPECT_FALSE(store_.del("nonexistent"));
    EXPECT_EQ(store_.size(), 1);
}

TEST_F(KVStoreTest, Exists) {
    store_.put("key", "value");

    EXPECT_TRUE(store_.exists("key"));
    EXPECT_FALSE(store_.exists("nonexistent"));
}

TEST_F(KVStoreTest, Clear) {
    store_.put("key1", "value1");
    store_.put("key2", "value2");

    store_.clear();

    EXPECT_EQ(store_.size(), 0);
    EXPECT_FALSE(store_.exists("key1"));
}

// ==================== 边界情况测试 ====================

TEST_F(KVStoreTest, EmptyKey) {
    EXPECT_FALSE(store_.put("", "value"));  // 空 key 不允许

    std::string value;
    EXPECT_FALSE(store_.get("", value));
    EXPECT_FALSE(store_.del(""));
    EXPECT_FALSE(store_.exists(""));
}

TEST_F(KVStoreTest, EmptyValue) {
    EXPECT_TRUE(store_.put("key", ""));  // 空 value 允许

    std::string value;
    EXPECT_TRUE(store_.get("key", value));
    EXPECT_EQ(value, "");
}

TEST_F(KVStoreTest, LongKeyValue) {
    std::string longKey(1000, 'k');
    std::string longValue(10000, 'v');

    EXPECT_TRUE(store_.put(longKey, longValue));

    std::string value;
    EXPECT_TRUE(store_.get(longKey, value));
    EXPECT_EQ(value, longValue);
}

// ==================== 持久化测试 ====================

TEST_F(KVStoreTest, SaveAndLoad) {
    store_.put("name", "Bob");
    store_.put("city", "Shanghai");
    store_.put("job", "Engineer");

    std::string filepath = "/tmp/kvstore_test.db";

    // 保存
    EXPECT_TRUE(store_.save(filepath));

    // 创建新的 store 并加载
    KVStore newStore;
    EXPECT_TRUE(newStore.load(filepath));

    EXPECT_EQ(newStore.size(), 3);

    std::string value;
    EXPECT_TRUE(newStore.get("name", value));
    EXPECT_EQ(value, "Bob");

    EXPECT_TRUE(newStore.get("city", value));
    EXPECT_EQ(value, "Shanghai");

    EXPECT_TRUE(newStore.get("job", value));
    EXPECT_EQ(value, "Engineer");

    // 清理测试文件
    std::remove(filepath.c_str());
}

TEST_F(KVStoreTest, LoadClearsExistingData) {
    store_.put("old_key", "old_value");

    // 创建测试文件
    std::string filepath = "/tmp/kvstore_load_test.db";
    {
        KVStore tempStore;
        tempStore.put("new_key", "new_value");
        tempStore.save(filepath);
    }

    // 加载会清空现有数据
    store_.load(filepath);

    EXPECT_EQ(store_.size(), 1);
    EXPECT_FALSE(store_.exists("old_key"));
    EXPECT_TRUE(store_.exists("new_key"));

    std::remove(filepath.c_str());
}

TEST_F(KVStoreTest, SaveEmptyStore) {
    std::string filepath = "/tmp/kvstore_empty_test.db";

    EXPECT_TRUE(store_.save(filepath));

    KVStore newStore;
    EXPECT_TRUE(newStore.load(filepath));
    EXPECT_EQ(newStore.size(), 0);

    std::remove(filepath.c_str());
}

TEST_F(KVStoreTest, LoadNonExistentFile) {
    EXPECT_FALSE(store_.load("/tmp/nonexistent_file_12345.db"));
}

TEST_F(KVStoreTest, SaveEmptyFilepath) {
    EXPECT_FALSE(store_.save(""));
    EXPECT_FALSE(store_.load(""));
}

// ==================== 压力测试 ====================

TEST_F(KVStoreTest, LargeDataSet) {
    const int count = 5000;

    // 写入
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        store_.put(key, value);
    }

    EXPECT_EQ(store_.size(), count);

    // 读取验证
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(i);
        std::string expected = "value" + std::to_string(i);
        std::string value;
        ASSERT_TRUE(store_.get(key, value));
        EXPECT_EQ(value, expected);
    }

    // 持久化后重新加载
    std::string filepath = "/tmp/kvstore_large_test.db";
    EXPECT_TRUE(store_.save(filepath));

    KVStore newStore;
    EXPECT_TRUE(newStore.load(filepath));
    EXPECT_EQ(newStore.size(), count);

    std::remove(filepath.c_str());
}

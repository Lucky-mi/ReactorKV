// src/storage/kvstore.h
#ifndef KVSTORE_STORAGE_KVSTORE_H
#define KVSTORE_STORAGE_KVSTORE_H

#include "storage/skiplist.h"

#include <string>

namespace kvstore {

/**
 * @brief KV 存储引擎
 *
 * 对 SkipList 的封装，提供简洁的 KV 操作接口。
 * 底层使用跳表作为内存索引结构，支持：
 * - O(log N) 的读写操作
 * - 数据持久化和加载
 * - 线程安全
 *
 * 使用示例：
 *   KVStore store;
 *   store.put("name", "Alice");
 *
 *   std::string value;
 *   if (store.get("name", value)) {
 *       std::cout << value << std::endl;  // 输出: Alice
 *   }
 *
 *   store.save("data.db");  // 持久化
 */
class KVStore : noncopyable {
public:
    /**
     * @brief 构造函数
     * @param maxLevel 跳表最大层数，默认 16
     */
    explicit KVStore(int maxLevel = 16);

    /**
     * @brief 析构函数
     */
    ~KVStore();

    // ==================== CRUD 操作 ====================

    /**
     * @brief 写入键值对
     *
     * 如果 key 已存在，则更新 value。
     *
     * @param key 键
     * @param value 值
     * @return true 新增，false 更新
     */
    bool put(const std::string& key, const std::string& value);

    /**
     * @brief 读取键对应的值
     * @param key 键
     * @param value 输出参数
     * @return true 成功，false 键不存在
     */
    bool get(const std::string& key, std::string& value) const;

    /**
     * @brief 删除键值对
     * @param key 键
     * @return true 成功，false 键不存在
     */
    bool del(const std::string& key);

    /**
     * @brief 判断键是否存在
     * @param key 键
     * @return true 存在，false 不存在
     */
    bool exists(const std::string& key) const;

    // ==================== 管理操作 ====================

    /**
     * @brief 获取存储的键值对数量
     * @return 数量
     */
    int size() const;

    /**
     * @brief 清空所有数据
     */
    void clear();

    // ==================== 持久化 ====================

    /**
     * @brief 将数据保存到文件
     * @param filepath 文件路径
     * @return true 成功，false 失败
     */
    bool save(const std::string& filepath) const;

    /**
     * @brief 从文件加载数据
     *
     * 注意：会清空当前数据后再加载
     *
     * @param filepath 文件路径
     * @return true 成功，false 失败
     */
    bool load(const std::string& filepath);

    /**
     * @brief 打印存储结构（调试用）
     */
    void dump() const;

private:
    SkipList<std::string, std::string> skiplist_;
};

}  // namespace kvstore

#endif  // KVSTORE_STORAGE_KVSTORE_H

// src/storage/skiplist.h
#ifndef KVSTORE_STORAGE_SKIPLIST_H
#define KVSTORE_STORAGE_SKIPLIST_H

#include "base/mutex.h"
#include "base/noncopyable.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>

namespace kvstore {

/**
 * @brief 跳表数据结构
 *
 * 跳表是一种概率性数据结构，支持 O(log N) 的查找、插入和删除操作。
 * 相比于红黑树等平衡树，跳表实现简单，且在并发环境下更容易实现无锁或细粒度锁。
 *
 * 结构示意：
 *   Level 3:  head ─────────────────────────────────→ 30 ────────────────────→ NIL
 *   Level 2:  head ────────→ 10 ────────────────────→ 30 ──────→ 50 ─────────→ NIL
 *   Level 1:  head → 5 → 10 → 15 → 20 → 25 → 30 → 40 → 50 → 60 → NIL
 *
 * 特点：
 * 1. 使用 MutexLock 保证线程安全
 * 2. 使用 shared_ptr 管理节点内存，避免内存泄漏
 * 3. 支持持久化到文件和从文件加载
 *
 * @tparam K 键类型，需要支持 < 运算符
 * @tparam V 值类型
 */
template <typename K, typename V>
class SkipList : noncopyable {
public:
    /**
     * @brief 构造函数
     * @param maxLevel 跳表最大层数，默认 16
     */
    explicit SkipList(int maxLevel = kDefaultMaxLevel);

    /**
     * @brief 析构函数
     */
    ~SkipList();

    /**
     * @brief 插入键值对
     *
     * 如果 key 已存在，则更新对应的 value。
     *
     * @param key 键
     * @param value 值
     * @return true 插入成功（新键），false 更新已存在的键
     */
    bool insert(const K& key, const V& value);

    /**
     * @brief 查询键对应的值
     * @param key 键
     * @param value 输出参数，存储查询结果
     * @return true 查询成功，false 键不存在
     */
    bool search(const K& key, V& value) const;

    /**
     * @brief 删除键值对
     * @param key 键
     * @return true 删除成功，false 键不存在
     */
    bool remove(const K& key);

    /**
     * @brief 判断键是否存在
     * @param key 键
     * @return true 存在，false 不存在
     */
    bool contains(const K& key) const;

    /**
     * @brief 获取跳表中元素个数
     * @return 元素个数
     */
    int size() const;

    /**
     * @brief 清空跳表
     */
    void clear();

    /**
     * @brief 将跳表数据持久化到文件
     * @param filepath 文件路径
     * @return true 成功，false 失败
     */
    bool dumpFile(const std::string& filepath) const;

    /**
     * @brief 从文件加载数据到跳表
     * @param filepath 文件路径
     * @return true 成功，false 失败
     */
    bool loadFile(const std::string& filepath);

    /**
     * @brief 打印跳表结构（调试用）
     */
    void displayList() const;

private:
    // ==================== 内部类型定义 ====================

    /**
     * @brief 跳表节点
     */
    struct Node {
        K key;
        V value;
        int nodeLevel;  // 节点层数

        // forward[i] 指向第 i 层的下一个节点
        std::vector<std::shared_ptr<Node>> forward;

        Node(const K& k, const V& v, int level)
            : key(k), value(v), nodeLevel(level), forward(level + 1, nullptr) {}

        // 空头节点构造
        Node(int level) : key(), value(), nodeLevel(level), forward(level + 1, nullptr) {}
    };

    using NodePtr = std::shared_ptr<Node>;

    // ==================== 私有方法 ====================

    /**
     * @brief 随机生成节点层数
     *
     * 使用概率 p = 0.25，期望层数约为 1.33。
     * 每一层有 25% 的概率向上扩展。
     *
     * @return 随机层数 [0, maxLevel_ - 1]
     */
    int getRandomLevel() const;

    /**
     * @brief 创建新节点
     */
    NodePtr createNode(const K& key, const V& value, int level);

    /**
     * @brief 解析一行数据
     * @param line 格式："key:value"
     * @param key 输出键
     * @param value 输出值
     * @return true 解析成功
     */
    bool parseString(const std::string& line, std::string& key, std::string& value) const;

    // ==================== 成员变量 ====================

    static constexpr int kDefaultMaxLevel = 16;     // 默认最大层数
    static constexpr double kProbability = 0.25;    // 层数扩展概率
    static constexpr char kDelimiter = ':';         // 持久化分隔符

    int maxLevel_;          // 最大层数
    int currentLevel_;      // 当前最高层数
    int elementCount_;      // 元素个数
    NodePtr header_;        // 头节点

    mutable MutexLock mutex_;  // 线程安全锁
};

// ==================== 模板类实现 ====================

template <typename K, typename V>
SkipList<K, V>::SkipList(int maxLevel)
    : maxLevel_(maxLevel),
      currentLevel_(0),
      elementCount_(0),
      header_(std::make_shared<Node>(maxLevel)),
      mutex_() {
    // 随机数生成器已改为 thread_local，无需初始化种子
}

template <typename K, typename V>
SkipList<K, V>::~SkipList() {
    // shared_ptr 自动管理内存，无需手动释放
    // 但为了安全，我们显式清空
    clear();
}

template <typename K, typename V>
int SkipList<K, V>::getRandomLevel() const {
    // 使用 thread_local 随机数生成器，线程安全且性能更好
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    thread_local std::uniform_real_distribution<> dis(0.0, 1.0);

    int level = 0;
    // 概率 p = 0.25，每层有 25% 的概率向上扩展
    while (dis(gen) < kProbability && level < maxLevel_ - 1) {
        level++;
    }
    return level;
}

template <typename K, typename V>
typename SkipList<K, V>::NodePtr SkipList<K, V>::createNode(const K& key, const V& value,
                                                             int level) {
    return std::make_shared<Node>(key, value, level);
}

template <typename K, typename V>
bool SkipList<K, V>::insert(const K& key, const V& value) {
    MutexLockGuard lock(mutex_);

    // update[i] 记录第 i 层需要更新 forward 指针的节点
    std::vector<NodePtr> update(maxLevel_ + 1, nullptr);
    NodePtr current = header_;

    // 从最高层向下搜索插入位置
    for (int i = currentLevel_; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    // current 现在指向第 0 层中 key 的前驱节点
    current = current->forward[0];

    // 检查 key 是否已存在
    if (current != nullptr && current->key == key) {
        // key 已存在，更新 value
        current->value = value;
        return false;  // 返回 false 表示是更新而非新插入
    }

    // 生成新节点的随机层数
    int randomLevel = getRandomLevel();

    // 如果新节点层数超过当前最高层，初始化新层的 update
    if (randomLevel > currentLevel_) {
        for (int i = currentLevel_ + 1; i <= randomLevel; i++) {
            update[i] = header_;
        }
        currentLevel_ = randomLevel;
    }

    // 创建新节点
    NodePtr newNode = createNode(key, value, randomLevel);

    // 插入节点：更新每一层的 forward 指针
    for (int i = 0; i <= randomLevel; i++) {
        newNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = newNode;
    }

    elementCount_++;
    return true;
}

template <typename K, typename V>
bool SkipList<K, V>::search(const K& key, V& value) const {
    MutexLockGuard lock(mutex_);

    NodePtr current = header_;

    // 从最高层向下搜索
    for (int i = currentLevel_; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->key < key) {
            current = current->forward[i];
        }
    }

    // 移动到第 0 层的下一个节点
    current = current->forward[0];

    // 检查是否找到
    if (current != nullptr && current->key == key) {
        value = current->value;
        return true;
    }

    return false;
}

template <typename K, typename V>
bool SkipList<K, V>::remove(const K& key) {
    MutexLockGuard lock(mutex_);

    std::vector<NodePtr> update(maxLevel_ + 1, nullptr);
    NodePtr current = header_;

    // 从最高层向下搜索
    for (int i = currentLevel_; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    // 检查 key 是否存在
    if (current == nullptr || current->key != key) {
        return false;
    }

    // 从每一层中删除节点
    for (int i = 0; i <= currentLevel_; i++) {
        if (update[i]->forward[i] != current) {
            break;
        }
        update[i]->forward[i] = current->forward[i];
    }

    // 更新当前最高层数（如果删除后某些层变空）
    while (currentLevel_ > 0 && header_->forward[currentLevel_] == nullptr) {
        currentLevel_--;
    }

    elementCount_--;
    return true;
}

template <typename K, typename V>
bool SkipList<K, V>::contains(const K& key) const {
    V dummy;
    return search(key, dummy);
}

template <typename K, typename V>
int SkipList<K, V>::size() const {
    MutexLockGuard lock(mutex_);
    return elementCount_;
}

template <typename K, typename V>
void SkipList<K, V>::clear() {
    MutexLockGuard lock(mutex_);

    // 清空所有 forward 指针
    for (int i = 0; i <= currentLevel_; i++) {
        header_->forward[i] = nullptr;
    }

    currentLevel_ = 0;
    elementCount_ = 0;
}

template <typename K, typename V>
bool SkipList<K, V>::dumpFile(const std::string& filepath) const {
    MutexLockGuard lock(mutex_);

    std::ofstream outFile(filepath);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }

    // 遍历第 0 层，写入所有键值对
    NodePtr current = header_->forward[0];
    while (current != nullptr) {
        outFile << current->key << kDelimiter << current->value << "\n";
        current = current->forward[0];
    }

    outFile.flush();
    outFile.close();
    return true;
}

template <typename K, typename V>
bool SkipList<K, V>::loadFile(const std::string& filepath) {
    std::ifstream inFile(filepath);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << filepath << std::endl;
        return false;
    }

    // 先读取所有数据到临时容器，避免在持有锁时调用 insert 导致递归锁问题
    std::vector<std::pair<std::string, std::string>> data;
    std::string line;
    std::string key;
    std::string value;

    while (std::getline(inFile, line)) {
        if (parseString(line, key, value)) {
            data.emplace_back(key, value);
        }
    }
    inFile.close();

    // 批量插入（insert 内部会加锁）
    for (const auto& kv : data) {
        // 注意：这里假设 K 和 V 都是 std::string
        // 对于其他类型，需要进行类型转换
        insert(static_cast<K>(kv.first), static_cast<V>(kv.second));
    }

    return true;
}

template <typename K, typename V>
bool SkipList<K, V>::parseString(const std::string& line, std::string& key,
                                  std::string& value) const {
    if (line.empty()) {
        return false;
    }

    size_t pos = line.find(kDelimiter);
    if (pos == std::string::npos) {
        return false;
    }

    key = line.substr(0, pos);
    value = line.substr(pos + 1);

    if (key.empty()) {
        return false;
    }

    return true;
}

template <typename K, typename V>
void SkipList<K, V>::displayList() const {
    MutexLockGuard lock(mutex_);

    std::cout << "\n========== Skip List ==========" << std::endl;
    std::cout << "Element count: " << elementCount_ << std::endl;
    std::cout << "Current level: " << currentLevel_ << std::endl;

    for (int i = currentLevel_; i >= 0; i--) {
        std::cout << "Level " << i << ": ";
        NodePtr current = header_->forward[i];
        while (current != nullptr) {
            std::cout << current->key << ":" << current->value << " -> ";
            current = current->forward[i];
        }
        std::cout << "NIL" << std::endl;
    }
    std::cout << "================================\n" << std::endl;
}

}  // namespace kvstore

#endif  // KVSTORE_STORAGE_SKIPLIST_H

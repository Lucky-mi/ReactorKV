// client/kvclient.h
#ifndef KVSTORE_CLIENT_KVCLIENT_H
#define KVSTORE_CLIENT_KVCLIENT_H

#include <string>
#include <cstdint>

namespace kvstore {

/**
 * @brief KV 客户端
 *
 * 同步阻塞式客户端，用于连接 KVServer 并执行命令。
 *
 * 使用示例：
 *   KVClient client("127.0.0.1", 6379);
 *   if (client.connect()) {
 *       client.put("name", "Alice");
 *       auto result = client.get("name");
 *       if (result.first) {
 *           std::cout << result.second << std::endl;
 *       }
 *       client.disconnect();
 *   }
 */
class KVClient {
public:
    /**
     * @brief 构造函数
     * @param host 服务器地址
     * @param port 服务器端口
     */
    KVClient(const std::string& host, uint16_t port);

    /**
     * @brief 析构函数
     */
    ~KVClient();

    // 禁止拷贝
    KVClient(const KVClient&) = delete;
    KVClient& operator=(const KVClient&) = delete;

    // ==================== 连接管理 ====================

    /**
     * @brief 连接到服务器
     * @return true 成功，false 失败
     */
    bool connect();

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 检查是否已连接
     */
    bool isConnected() const { return sockfd_ >= 0; }

    // ==================== KV 操作 ====================

    /**
     * @brief 存储键值对
     * @param key 键
     * @param value 值
     * @return true 成功，false 失败
     */
    bool put(const std::string& key, const std::string& value);

    /**
     * @brief 获取键对应的值
     * @param key 键
     * @return pair<是否成功, 值>
     */
    std::pair<bool, std::string> get(const std::string& key);

    /**
     * @brief 删除键
     * @param key 键
     * @return true 成功，false 失败
     */
    bool del(const std::string& key);

    /**
     * @brief 检查键是否存在
     * @param key 键
     * @return true 存在，false 不存在
     */
    bool exists(const std::string& key);

    /**
     * @brief 获取存储大小
     * @return pair<是否成功, 数量>
     */
    std::pair<bool, int> size();

    /**
     * @brief 清空所有数据
     * @return true 成功
     */
    bool clear();

    /**
     * @brief 心跳检测
     * @return true 服务器在线
     */
    bool ping();

    /**
     * @brief 获取最后的错误信息
     */
    const std::string& lastError() const { return lastError_; }

private:
    /**
     * @brief 发送命令并接收响应
     * @param command 命令字符串
     * @return 响应字符串
     */
    std::string sendCommand(const std::string& command);

    /**
     * @brief 解析响应
     * @param response 响应字符串
     * @param value 输出值
     * @return true 成功，false 失败或 NOT_FOUND
     */
    bool parseResponse(const std::string& response, std::string* value = nullptr);

    std::string host_;
    uint16_t port_;
    int sockfd_;
    std::string lastError_;
};

}  // namespace kvstore

#endif  // KVSTORE_CLIENT_KVCLIENT_H

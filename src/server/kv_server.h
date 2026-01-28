// src/server/kv_server.h
#ifndef KVSTORE_SERVER_KV_SERVER_H
#define KVSTORE_SERVER_KV_SERVER_H

#include "base/noncopyable.h"
#include "net/tcp_server.h"
#include "net/eventloop.h"
#include "storage/kvstore.h"
#include "protocol/message.h"
#include "protocol/codec.h"

#include <string>
#include <memory>

namespace kvstore {

/**
 * @brief KV 存储服务器
 *
 * 整合 TcpServer 和 KVStore，提供完整的 KV 存储服务。
 *
 * 支持的命令：
 *   PUT key value   - 存储键值对
 *   GET key         - 获取值
 *   DEL key         - 删除键
 *   EXISTS key      - 判断键是否存在
 *   SIZE            - 获取存储数量
 *   CLEAR           - 清空所有数据
 *   PING            - 心跳检测
 *   QUIT            - 断开连接
 *
 * 使用示例：
 *   EventLoop loop;
 *   KVServer server(&loop, 8080);
 *   server.setThreadNum(4);
 *   server.start();
 *   loop.loop();
 */
class KVServer : noncopyable {
public:
    KVServer(EventLoop* loop, uint16_t port,
             const std::string& name = "KVServer");
    ~KVServer();

    /// 设置 IO 线程数量
    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    /// 启动服务器
    void start();

    /// 获取底层 KVStore
    KVStore& store() { return store_; }

    /// 加载数据文件
    bool loadData(const std::string& filepath);

    /// 保存数据文件
    bool saveData(const std::string& filepath);

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

    Response handleRequest(const Request& request);

    EventLoop* loop_;
    TcpServer server_;
    KVStore store_;
    std::string dataFile_;
};

}  // namespace kvstore

#endif  // KVSTORE_SERVER_KV_SERVER_H

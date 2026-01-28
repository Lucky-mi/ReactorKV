// src/net/inet_address.h
#ifndef KVSTORE_NET_INET_ADDRESS_H
#define KVSTORE_NET_INET_ADDRESS_H

#include <netinet/in.h>
#include <string>

namespace kvstore {

/**
 * @brief 网络地址封装
 *
 * 对 sockaddr_in 的封装，支持 IPv4 地址。
 * 提供 IP 和端口的解析、格式化功能。
 *
 * 使用示例：
 *   InetAddress addr(8080);                    // 监听所有接口的 8080 端口
 *   InetAddress addr("127.0.0.1", 8080);       // 指定 IP 和端口
 *   InetAddress addr("192.168.1.100:9000");    // 解析 "IP:Port" 格式
 */
class InetAddress {
public:
    /**
     * @brief 构造函数：仅指定端口（用于服务器监听）
     * @param port 端口号
     * @param loopbackOnly 是否仅监听 loopback (127.0.0.1)
     */
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    /**
     * @brief 构造函数：指定 IP 和端口
     * @param ip IP 地址字符串
     * @param port 端口号
     */
    InetAddress(const std::string& ip, uint16_t port);

    /**
     * @brief 构造函数：从 sockaddr_in 构造
     * @param addr sockaddr_in 结构
     */
    explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

    // ==================== Getters ====================

    /// 获取 IP 地址字符串
    std::string toIp() const;

    /// 获取 "IP:Port" 格式字符串
    std::string toIpPort() const;

    /// 获取端口号
    uint16_t port() const;

    /// 获取 sockaddr 指针（用于 bind/connect）
    const struct sockaddr* getSockAddr() const {
        return reinterpret_cast<const struct sockaddr*>(&addr_);
    }

    /// 获取 sockaddr_in 引用
    const struct sockaddr_in& getSockAddrInet() const { return addr_; }

    /// 设置 sockaddr_in
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

    // ==================== 静态方法 ====================

    /// 解析 "host:port" 格式字符串
    static bool resolve(const std::string& hostname, InetAddress* result);

private:
    struct sockaddr_in addr_;
};

}  // namespace kvstore

#endif  // KVSTORE_NET_INET_ADDRESS_H

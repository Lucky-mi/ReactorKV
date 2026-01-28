// src/net/inet_address.cpp
#include "net/inet_address.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>

namespace kvstore {

InetAddress::InetAddress(uint16_t port, bool loopbackOnly) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        // 解析失败，使用 0.0.0.0
        addr_.sin_addr.s_addr = INADDR_ANY;
    }
}

std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const {
    char buf[64];
    char ip[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip));
    snprintf(buf, sizeof(buf), "%s:%u", ip, ntohs(addr_.sin_port));
    return buf;
}

uint16_t InetAddress::port() const {
    return ntohs(addr_.sin_port);
}

bool InetAddress::resolve(const std::string& hostname, InetAddress* result) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int ret = ::getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if (ret != 0 || res == nullptr) {
        return false;
    }

    struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    result->addr_.sin_addr = addr->sin_addr;
    ::freeaddrinfo(res);
    return true;
}

}  // namespace kvstore

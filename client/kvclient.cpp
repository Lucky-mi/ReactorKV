// client/kvclient.cpp
#include "kvclient.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace kvstore {

KVClient::KVClient(const std::string& host, uint16_t port)
    : host_(host), port_(port), sockfd_(-1) {}

KVClient::~KVClient() {
    disconnect();
}

bool KVClient::connect() {
    if (isConnected()) {
        return true;
    }

    // 创建 socket
    sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        lastError_ = "Failed to create socket: " + std::string(strerror(errno));
        return false;
    }

    // 解析地址
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);

    if (inet_pton(AF_INET, host_.c_str(), &serverAddr.sin_addr) <= 0) {
        // 尝试 DNS 解析
        struct hostent* he = gethostbyname(host_.c_str());
        if (he == nullptr) {
            lastError_ = "Failed to resolve host: " + host_;
            ::close(sockfd_);
            sockfd_ = -1;
            return false;
        }
        memcpy(&serverAddr.sin_addr, he->h_addr, he->h_length);
    }

    // 连接服务器
    if (::connect(sockfd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        lastError_ = "Failed to connect: " + std::string(strerror(errno));
        ::close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    return true;
}

void KVClient::disconnect() {
    if (sockfd_ >= 0) {
        // 发送 QUIT 命令
        sendCommand("QUIT\r\n");
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

std::string KVClient::sendCommand(const std::string& command) {
    if (!isConnected()) {
        lastError_ = "Not connected";
        return "";
    }

    // 发送命令
    ssize_t sent = ::send(sockfd_, command.c_str(), command.size(), 0);
    if (sent != static_cast<ssize_t>(command.size())) {
        lastError_ = "Failed to send command";
        return "";
    }

    // 接收响应
    char buffer[4096];
    ssize_t received = ::recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        lastError_ = "Failed to receive response";
        return "";
    }

    buffer[received] = '\0';
    return std::string(buffer, received);
}

bool KVClient::parseResponse(const std::string& response, std::string* value) {
    if (response.empty()) {
        return false;
    }

    // 响应格式：+OK [value]\r\n 或 -ERROR message\r\n
    if (response[0] == '+') {
        // 成功
        if (value) {
            // 提取值（跳过 "+OK " 前缀）
            size_t start = 0;
            if (response.size() > 3 && response.substr(0, 3) == "+OK") {
                start = 3;
                if (start < response.size() && response[start] == ' ') {
                    start++;
                }
            }
            // 去掉尾部的 \r\n
            size_t end = response.find_last_not_of("\r\n");
            if (end != std::string::npos && start <= end) {
                *value = response.substr(start, end - start + 1);
            } else {
                *value = "";
            }
        }
        return true;
    } else if (response[0] == '-') {
        // 错误或 NOT_FOUND
        if (response.find("NOT_FOUND") != std::string::npos) {
            lastError_ = "Key not found";
        } else {
            lastError_ = response.substr(1);
            // 去掉尾部的 \r\n
            size_t end = lastError_.find_last_not_of("\r\n");
            if (end != std::string::npos) {
                lastError_ = lastError_.substr(0, end + 1);
            }
        }
        return false;
    }

    lastError_ = "Unknown response format";
    return false;
}

bool KVClient::put(const std::string& key, const std::string& value) {
    std::string command = "PUT " + key + " " + value + "\r\n";
    std::string response = sendCommand(command);
    return parseResponse(response);
}

std::pair<bool, std::string> KVClient::get(const std::string& key) {
    std::string command = "GET " + key + "\r\n";
    std::string response = sendCommand(command);
    std::string value;
    bool success = parseResponse(response, &value);
    return {success, value};
}

bool KVClient::del(const std::string& key) {
    std::string command = "DEL " + key + "\r\n";
    std::string response = sendCommand(command);
    return parseResponse(response);
}

bool KVClient::exists(const std::string& key) {
    std::string command = "EXISTS " + key + "\r\n";
    std::string response = sendCommand(command);
    std::string value;
    if (parseResponse(response, &value)) {
        return value == "1" || value == "true";
    }
    return false;
}

std::pair<bool, int> KVClient::size() {
    std::string command = "SIZE\r\n";
    std::string response = sendCommand(command);
    std::string value;
    if (parseResponse(response, &value)) {
        try {
            return {true, std::stoi(value)};
        } catch (...) {
            return {false, 0};
        }
    }
    return {false, 0};
}

bool KVClient::clear() {
    std::string command = "CLEAR\r\n";
    std::string response = sendCommand(command);
    return parseResponse(response);
}

bool KVClient::ping() {
    std::string command = "PING\r\n";
    std::string response = sendCommand(command);
    return response.find("PONG") != std::string::npos;
}

}  // namespace kvstore

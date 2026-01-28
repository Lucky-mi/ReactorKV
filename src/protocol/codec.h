// src/protocol/codec.h
#ifndef KVSTORE_PROTOCOL_CODEC_H
#define KVSTORE_PROTOCOL_CODEC_H

#include "protocol/message.h"
#include "net/buffer.h"
#include "net/tcp_connection.h"

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace kvstore {

/**
 * @brief 协议编解码器
 *
 * 负责解析客户端请求和编码服务器响应。
 * 采用简单的文本协议，每行以 \r\n 结尾。
 *
 * 请求格式：
 *   COMMAND [key] [value]\r\n
 *
 * 响应格式：
 *   +OK [value]\r\n    成功
 *   -ERROR message\r\n 失败
 */
class Codec {
public:
    /**
     * @brief 尝试从 Buffer 解析一个请求
     * @param buf 输入缓冲区
     * @param request 输出请求
     * @return true 解析成功，false 数据不完整需要继续等待
     */
    static bool parseRequest(Buffer* buf, Request* request);

    /**
     * @brief 编码响应到字符串
     * @param response 响应对象
     * @return 编码后的字符串
     */
    static std::string encodeResponse(const Response& response);

    /**
     * @brief 发送响应
     * @param conn TCP 连接
     * @param response 响应对象
     */
    static void sendResponse(const TcpConnectionPtr& conn, const Response& response);

private:
    /// 解析命令行
    static bool parseLine(const std::string& line, Request* request);

    /// 分割字符串
    static std::vector<std::string> split(const std::string& str);

    /// 转换为大写
    static std::string toUpper(const std::string& str);
};

// ==================== 实现 ====================

inline bool Codec::parseRequest(Buffer* buf, Request* request) {
    // 查找行结束符：支持 \r\n 和 \n
    const char* lineEnd = nullptr;
    const char* crlf = buf->findCRLF();

    // 也查找单独的 \n（兼容 nc 等工具）
    const char* lf = static_cast<const char*>(
        memchr(buf->peek(), '\n', buf->readableBytes()));

    // 取最早出现的换行符
    if (crlf && lf) {
        lineEnd = (crlf < lf) ? crlf : lf;
    } else if (crlf) {
        lineEnd = crlf;
    } else if (lf) {
        lineEnd = lf;
    }

    if (lineEnd == nullptr) {
        // 没有找到完整的一行
        return false;
    }

    // 提取一行（去掉可能的 \r）
    size_t lineLen = lineEnd - buf->peek();
    std::string line(buf->peek(), lineLen);

    // 去掉行尾的 \r（如果有）
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    // 计算需要跳过的字节数
    // 如果 lineEnd 指向 \r，且下一个字符是 \n，跳过 2 字节
    // 否则只跳过 1 字节（lineEnd 指向 \n）
    size_t bytesToSkip = lineLen + 1;  // 默认跳过到 lineEnd 后一个字符
    if (*lineEnd == '\r') {
        // lineEnd 指向 \r，检查是否还有 \n
        if (lineEnd + 1 < buf->peek() + buf->readableBytes() && *(lineEnd + 1) == '\n') {
            bytesToSkip = lineLen + 2;  // 跳过 \r\n
        }
    }
    buf->retrieve(bytesToSkip);

    return parseLine(line, request);
}

inline bool Codec::parseLine(const std::string& line, Request* request) {
    if (line.empty()) {
        request->command = CommandType::kUnknown;
        return true;
    }

    std::vector<std::string> parts = split(line);
    if (parts.empty()) {
        request->command = CommandType::kUnknown;
        return true;
    }

    std::string cmd = toUpper(parts[0]);

    if (cmd == "PUT" || cmd == "SET") {
        if (parts.size() >= 3) {
            request->command = CommandType::kPut;
            request->key = parts[1];
            // value 可能包含空格，取剩余所有部分
            size_t valueStart = line.find(parts[1]) + parts[1].size();
            while (valueStart < line.size() && std::isspace(line[valueStart])) {
                valueStart++;
            }
            request->value = line.substr(valueStart);
        } else {
            request->command = CommandType::kUnknown;
        }
    } else if (cmd == "GET") {
        if (parts.size() >= 2) {
            request->command = CommandType::kGet;
            request->key = parts[1];
        } else {
            request->command = CommandType::kUnknown;
        }
    } else if (cmd == "DEL" || cmd == "DELETE") {
        if (parts.size() >= 2) {
            request->command = CommandType::kDel;
            request->key = parts[1];
        } else {
            request->command = CommandType::kUnknown;
        }
    } else if (cmd == "EXISTS") {
        if (parts.size() >= 2) {
            request->command = CommandType::kExists;
            request->key = parts[1];
        } else {
            request->command = CommandType::kUnknown;
        }
    } else if (cmd == "SIZE" || cmd == "DBSIZE") {
        request->command = CommandType::kSize;
    } else if (cmd == "CLEAR" || cmd == "FLUSHDB") {
        request->command = CommandType::kClear;
    } else if (cmd == "PING") {
        request->command = CommandType::kPing;
    } else if (cmd == "QUIT" || cmd == "EXIT") {
        request->command = CommandType::kQuit;
    } else {
        request->command = CommandType::kUnknown;
    }

    return true;
}

inline std::string Codec::encodeResponse(const Response& response) {
    std::ostringstream oss;

    switch (response.status) {
        case StatusCode::kOk:
            oss << "+OK";
            if (!response.message.empty()) {
                oss << " " << response.message;
            }
            break;
        case StatusCode::kNotFound:
            oss << "-NOT_FOUND";
            break;
        case StatusCode::kError:
            oss << "-ERROR";
            if (!response.message.empty()) {
                oss << " " << response.message;
            }
            break;
        case StatusCode::kPong:
            oss << "+PONG";
            break;
        case StatusCode::kBye:
            oss << "+BYE";
            break;
    }

    oss << "\r\n";
    return oss.str();
}

inline void Codec::sendResponse(const TcpConnectionPtr& conn, const Response& response) {
    conn->send(encodeResponse(response));
}

inline std::vector<std::string> Codec::split(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

inline std::string Codec::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

}  // namespace kvstore

#endif  // KVSTORE_PROTOCOL_CODEC_H

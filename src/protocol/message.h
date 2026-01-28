// src/protocol/message.h
#ifndef KVSTORE_PROTOCOL_MESSAGE_H
#define KVSTORE_PROTOCOL_MESSAGE_H

#include <string>
#include <cstdint>

namespace kvstore {

/**
 * @brief 命令类型
 */
enum class CommandType : uint8_t {
    kUnknown = 0,
    kPut = 1,      // PUT key value
    kGet = 2,      // GET key
    kDel = 3,      // DEL key
    kExists = 4,   // EXISTS key
    kSize = 5,     // SIZE
    kClear = 6,    // CLEAR
    kPing = 7,     // PING
    kQuit = 8,     // QUIT
};

/**
 * @brief 响应状态
 */
enum class StatusCode : uint8_t {
    kOk = 0,          // 成功
    kNotFound = 1,    // 键不存在
    kError = 2,       // 错误
    kPong = 3,        // PING 响应
    kBye = 4,         // QUIT 响应
};

/**
 * @brief 请求消息
 *
 * 协议格式（文本协议，类似 Redis）：
 *   PUT key value\r\n
 *   GET key\r\n
 *   DEL key\r\n
 *   EXISTS key\r\n
 *   SIZE\r\n
 *   CLEAR\r\n
 *   PING\r\n
 *   QUIT\r\n
 */
struct Request {
    CommandType command;
    std::string key;
    std::string value;

    Request() : command(CommandType::kUnknown) {}

    Request(CommandType cmd, const std::string& k = "", const std::string& v = "")
        : command(cmd), key(k), value(v) {}
};

/**
 * @brief 响应消息
 *
 * 响应格式：
 *   +OK\r\n                   // 成功（无值）
 *   +OK value\r\n             // 成功（有值）
 *   -NOT_FOUND\r\n            // 键不存在
 *   -ERROR message\r\n        // 错误
 *   +PONG\r\n                 // PING 响应
 *   +BYE\r\n                  // QUIT 响应
 */
struct Response {
    StatusCode status;
    std::string message;   // 错误信息或返回值

    Response() : status(StatusCode::kOk) {}

    Response(StatusCode s, const std::string& msg = "")
        : status(s), message(msg) {}

    // 便捷构造方法
    static Response ok(const std::string& msg = "") {
        return Response(StatusCode::kOk, msg);
    }

    static Response notFound() {
        return Response(StatusCode::kNotFound);
    }

    static Response error(const std::string& msg) {
        return Response(StatusCode::kError, msg);
    }

    static Response pong() {
        return Response(StatusCode::kPong);
    }

    static Response bye() {
        return Response(StatusCode::kBye);
    }
};

/**
 * @brief 命令类型转字符串（调试用）
 */
inline const char* commandToString(CommandType cmd) {
    switch (cmd) {
        case CommandType::kPut: return "PUT";
        case CommandType::kGet: return "GET";
        case CommandType::kDel: return "DEL";
        case CommandType::kExists: return "EXISTS";
        case CommandType::kSize: return "SIZE";
        case CommandType::kClear: return "CLEAR";
        case CommandType::kPing: return "PING";
        case CommandType::kQuit: return "QUIT";
        default: return "UNKNOWN";
    }
}

}  // namespace kvstore

#endif  // KVSTORE_PROTOCOL_MESSAGE_H

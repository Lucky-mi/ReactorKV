// src/server/kv_server.cpp
#include "server/kv_server.h"
#include "base/logger.h"

namespace kvstore {

KVServer::KVServer(EventLoop* loop, uint16_t port, const std::string& name)
    : loop_(loop),
      server_(loop, InetAddress(port), name),
      store_() {
    // 设置回调
    server_.setConnectionCallback(
        std::bind(&KVServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&KVServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
}

KVServer::~KVServer() {
    // 保存数据
    if (!dataFile_.empty()) {
        store_.save(dataFile_);
    }
}

void KVServer::start() {
    LOG_INFO << "KVServer starting...";
    server_.start();
}

bool KVServer::loadData(const std::string& filepath) {
    dataFile_ = filepath;
    return store_.load(filepath);
}

bool KVServer::saveData(const std::string& filepath) {
    dataFile_ = filepath;
    return store_.save(filepath);
}

void KVServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO << "Client connected: " << conn->peerAddress().toIpPort();
        // 发送欢迎消息
        conn->send("+WELCOME ReactorKV Server\r\n");
    } else {
        LOG_INFO << "Client disconnected: " << conn->peerAddress().toIpPort();
    }
}

void KVServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    // 可能一次收到多个请求
    while (buf->readableBytes() > 0) {
        Request request;
        if (!Codec::parseRequest(buf, &request)) {
            // 数据不完整，等待更多数据
            break;
        }

        // 处理请求
        Response response = handleRequest(request);

        // 发送响应
        Codec::sendResponse(conn, response);

        // QUIT 命令：关闭连接
        if (request.command == CommandType::kQuit) {
            conn->shutdown();
            break;
        }
    }
}

Response KVServer::handleRequest(const Request& request) {
    LOG_DEBUG << "Handling command: " << commandToString(request.command)
              << " key=" << request.key;

    switch (request.command) {
        case CommandType::kPut: {
            if (request.key.empty()) {
                return Response::error("Key cannot be empty");
            }
            bool isNew = store_.put(request.key, request.value);
            return Response::ok(isNew ? "CREATED" : "UPDATED");
        }

        case CommandType::kGet: {
            std::string value;
            if (store_.get(request.key, value)) {
                return Response::ok(value);
            } else {
                return Response::notFound();
            }
        }

        case CommandType::kDel: {
            if (store_.del(request.key)) {
                return Response::ok("DELETED");
            } else {
                return Response::notFound();
            }
        }

        case CommandType::kExists: {
            if (store_.exists(request.key)) {
                return Response::ok("1");
            } else {
                return Response::ok("0");
            }
        }

        case CommandType::kSize: {
            return Response::ok(std::to_string(store_.size()));
        }

        case CommandType::kClear: {
            store_.clear();
            return Response::ok("CLEARED");
        }

        case CommandType::kPing: {
            return Response::pong();
        }

        case CommandType::kQuit: {
            return Response::bye();
        }

        default: {
            return Response::error("Unknown command");
        }
    }
}

}  // namespace kvstore

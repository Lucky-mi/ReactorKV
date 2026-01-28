// examples/echo_server.cpp
// Echo Server 示例 - 验证网络模块

#include "net/tcp_server.h"
#include "net/eventloop.h"
#include "net/inet_address.h"
#include "net/buffer.h"
#include "base/logger.h"

#include <iostream>
#include <csignal>

using namespace kvstore;

EventLoop* g_loop = nullptr;

void signalHandler(int sig) {
    if (g_loop) {
        g_loop->quit();
    }
}

// 连接建立/断开回调
void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOG_INFO << "New connection: " << conn->name()
                 << " from " << conn->peerAddress().toIpPort();
    } else {
        LOG_INFO << "Connection closed: " << conn->name();
    }
}

// 消息到达回调 - Echo: 收到什么就发回什么
void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg = buf->retrieveAllAsString();
    LOG_INFO << "Received " << msg.size() << " bytes from " << conn->name()
             << " at " << time.toFormattedString();
    conn->send(msg);  // Echo back
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    std::cout << "========================================\n";
    std::cout << " ReactorKV Echo Server\n";
    std::cout << " Port: " << port << "\n";
    std::cout << " Press Ctrl+C to stop\n";
    std::cout << "========================================\n";

    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    EventLoop loop;
    g_loop = &loop;

    InetAddress listenAddr(port);
    TcpServer server(&loop, listenAddr, "EchoServer");

    // 设置回调
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    // 设置 IO 线程数（0 表示单线程模式）
    server.setThreadNum(2);

    server.start();
    loop.loop();

    std::cout << "Server stopped.\n";
    return 0;
}

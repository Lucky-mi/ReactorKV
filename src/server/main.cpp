// src/server/main.cpp
// ReactorKV 主程序

#include "server/kv_server.h"
#include "net/eventloop.h"
#include "base/logger.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <getopt.h>
#include <atomic>

using namespace kvstore;

EventLoop* g_loop = nullptr;
KVServer* g_server = nullptr;
std::string g_dataFile;  // 全局数据文件路径

// 信号处理函数 - 只做异步安全的操作
void signalHandler(int /*sig*/) {
    // 只调用异步安全的 quit()（设置 atomic flag）
    // 避免在信号处理中调用 cout、saveData 等非异步安全的函数
    if (g_loop) {
        g_loop->quit();
    }
}

void printUsage(const char* progname) {
    std::cout << "Usage: " << progname << " [options]\n"
              << "Options:\n"
              << "  -p, --port PORT      Server port (default: 6379)\n"
              << "  -t, --threads NUM    IO threads (default: 4)\n"
              << "  -d, --data FILE      Data file path (default: data.db)\n"
              << "  -h, --help           Show this help\n";
}

int main(int argc, char* argv[]) {
    int port = 6379;
    int threads = 4;
    std::string dataFile = "data.db";

    // 解析命令行参数
    static struct option longOptions[] = {
        {"port", required_argument, nullptr, 'p'},
        {"threads", required_argument, nullptr, 't'},
        {"data", required_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "p:t:d:h", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 't':
                threads = atoi(optarg);
                break;
            case 'd':
                dataFile = optarg;
                break;
            case 'h':
            default:
                printUsage(argv[0]);
                return 0;
        }
    }

    // 保存全局数据文件路径
    g_dataFile = dataFile;

    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);

    std::cout << "========================================\n";
    std::cout << "        ReactorKV Server v1.0\n";
    std::cout << "========================================\n";
    std::cout << "  Port:      " << port << "\n";
    std::cout << "  Threads:   " << threads << "\n";
    std::cout << "  Data File: " << dataFile << "\n";
    std::cout << "========================================\n";
    std::cout << "Press Ctrl+C to stop\n\n";

    EventLoop loop;
    g_loop = &loop;

    KVServer server(&loop, static_cast<uint16_t>(port), "ReactorKV");
    g_server = &server;

    server.setThreadNum(threads);

    // 尝试加载数据
    if (!dataFile.empty()) {
        if (server.loadData(dataFile)) {
            LOG_INFO << "Loaded " << server.store().size() << " keys from " << dataFile;
        } else {
            LOG_INFO << "No existing data file, starting fresh";
        }
    }

    server.start();
    loop.loop();

    // 主循环退出后保存数据（这里是安全的）
    std::cout << "\nShutting down...\n";
    if (g_server && !g_dataFile.empty()) {
        if (g_server->saveData(g_dataFile)) {
            std::cout << "Data saved to " << g_dataFile << "\n";
        } else {
            std::cerr << "Failed to save data\n";
        }
    }

    std::cout << "Server stopped.\n";
    return 0;
}


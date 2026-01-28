// benchmarks/kvserver_bench.cpp
// KVServer 网络性能测试客户端

#include "base/timestamp.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <iomanip>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace kvstore;

std::atomic<int> g_successCount(0);
std::atomic<int> g_failCount(0);

// 简单的同步客户端
class SyncClient {
public:
    SyncClient(const std::string& host, uint16_t port)
        : host_(host), port_(port), sockfd_(-1) {}

    ~SyncClient() {
        disconnect();
    }

    bool connect() {
        sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0) {
            std::cerr << "socket() failed" << std::endl;
            return false;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &addr.sin_addr);

        // 设置较短的超时（1秒）
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        if (::connect(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "connect() failed: " << strerror(errno) << std::endl;
            ::close(sockfd_);
            sockfd_ = -1;
            return false;
        }

        // 读取欢迎消息（非阻塞，只等一小段时间）
        char buf[256];
        ssize_t n = recv(sockfd_, buf, sizeof(buf), 0);
        if (n <= 0) {
            // 即使读不到欢迎消息也继续
            std::cerr << "Warning: no welcome message" << std::endl;
        }
        return true;
    }

    void disconnect() {
        if (sockfd_ >= 0) {
            ::close(sockfd_);
            sockfd_ = -1;
        }
    }

    bool sendCommand(const std::string& cmd, std::string& response) {
        if (sockfd_ < 0) return false;

        std::string request = cmd + "\n";
        if (::send(sockfd_, request.c_str(), request.size(), 0) < 0) {
            return false;
        }

        char buf[4096];
        ssize_t n = ::recv(sockfd_, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            return false;
        }
        buf[n] = '\0';
        response = buf;
        return true;
    }

private:
    std::string host_;
    uint16_t port_;
    int sockfd_;
};

void runBenchmark(const std::string& host, uint16_t port,
                  int numClients, int requestsPerClient,
                  const std::string& testType) {
    std::vector<std::thread> threads;
    g_successCount = 0;
    g_failCount = 0;

    Timestamp start = Timestamp::now();

    for (int c = 0; c < numClients; c++) {
        // 使用值捕获避免悬空引用问题
        threads.emplace_back([host, port, c, requestsPerClient, testType]() {
            SyncClient client(host, port);
            if (!client.connect()) {
                std::cerr << "Client " << c << " connect failed!" << std::endl;
                g_failCount += requestsPerClient;
                return;
            }

            std::string response;
            for (int i = 0; i < requestsPerClient; i++) {
                std::string key = "bench_" + std::to_string(c) + "_" + std::to_string(i);

                if (testType == "PUT" || testType == "MIXED") {
                    if (client.sendCommand("PUT " + key + " value_" + std::to_string(i), response)) {
                        g_successCount++;
                    } else {
                        g_failCount++;
                    }
                }

                if (testType == "GET" || testType == "MIXED") {
                    if (client.sendCommand("GET " + key, response)) {
                        g_successCount++;
                    } else {
                        g_failCount++;
                    }
                }
            }

            client.sendCommand("QUIT", response);
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    Timestamp end = Timestamp::now();
    double seconds = timeDifference(end, start);
    int total = g_successCount.load();
    int failed = g_failCount.load();
    double qps = (seconds > 0) ? (total / seconds) : 0;

    std::cout << std::left << std::setw(15) << testType
              << std::right << std::setw(8) << numClients << " clients, "
              << std::setw(10) << total << " ops, "
              << std::setw(6) << failed << " fails, "
              << std::fixed << std::setprecision(3) << std::setw(8) << seconds << " sec, "
              << std::setprecision(0) << std::setw(10) << qps << " QPS"
              << std::endl;
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 6379;
    int numClients = 10;
    int requestsPerClient = 1000;

    // 解析参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            host = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            numClients = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            requestsPerClient = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -h HOST     Server host (default: 127.0.0.1)\n"
                      << "  -p PORT     Server port (default: 6379)\n"
                      << "  -c NUM      Number of clients (default: 10)\n"
                      << "  -n NUM      Requests per client (default: 1000)\n";
            return 0;
        }
    }

    std::cout << "========================================\n";
    std::cout << "    KVServer Benchmark\n";
    std::cout << "========================================\n";
    std::cout << "Server:   " << host << ":" << port << "\n";
    std::cout << "Clients:  " << numClients << "\n";
    std::cout << "Requests: " << requestsPerClient << " per client\n";
    std::cout << "----------------------------------------\n";

    // PUT 测试
    runBenchmark(host, port, numClients, requestsPerClient, "PUT");

    // GET 测试 (重用刚才 PUT 的 key)
    runBenchmark(host, port, numClients, requestsPerClient, "GET");

    // 混合测试
    runBenchmark(host, port, numClients, requestsPerClient / 2, "MIXED");

    std::cout << "========================================\n";

    return 0;
}

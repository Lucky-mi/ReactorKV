// benchmarks/simple_bench.cpp
// 单线程 KVServer 性能测试

#include "base/timestamp.h"

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace kvstore;

class SimpleClient {
public:
    SimpleClient(const char* host, int port) : sockfd_(-1) {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0) {
            perror("socket");
            return;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host, &addr.sin_addr);

        if (connect(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("connect");
            close(sockfd_);
            sockfd_ = -1;
            return;
        }

        // 读取欢迎消息
        char buf[256];
        ::recv(sockfd_, buf, sizeof(buf), 0);
        std::cout << "Connected! Welcome: " << buf;
    }

    ~SimpleClient() {
        if (sockfd_ >= 0) {
            send("QUIT\n");
            close(sockfd_);
        }
    }

    bool isConnected() { return sockfd_ >= 0; }

    bool send(const std::string& cmd) {
        return ::send(sockfd_, cmd.c_str(), cmd.size(), 0) > 0;
    }

    std::string recv() {
        char buf[4096];
        ssize_t n = ::recv(sockfd_, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            return buf;
        }
        return "";
    }

    bool command(const std::string& cmd) {
        if (!send(cmd + "\n")) return false;
        std::string resp = recv();
        return !resp.empty();
    }

private:
    int sockfd_;
};

int main(int argc, char* argv[]) {
    const char* host = "127.0.0.1";
    int port = 6379;
    int count = 1000;

    if (argc > 1) count = atoi(argv[1]);
    if (argc > 2) host = argv[2];
    if (argc > 3) port = atoi(argv[3]);

    std::cout << "========================================\n";
    std::cout << "  Simple Benchmark (Single Thread)\n";
    std::cout << "========================================\n";
    std::cout << "  Server: " << host << ":" << port << "\n";
    std::cout << "  Operations: " << count << "\n";
    std::cout << "========================================\n";

    SimpleClient client(host, port);
    if (!client.isConnected()) {
        std::cerr << "Failed to connect!\n";
        return 1;
    }

    // PUT 测试
    std::cout << "\nPUT test...\n";
    Timestamp start = Timestamp::now();
    int success = 0;
    for (int i = 0; i < count; i++) {
        std::string cmd = "PUT key" + std::to_string(i) + " value" + std::to_string(i);
        if (client.command(cmd)) success++;
    }
    Timestamp end = Timestamp::now();
    double sec = timeDifference(end, start);
    std::cout << "PUT: " << success << "/" << count << " in " << sec << "s = "
              << (int)(success / sec) << " QPS\n";

    // GET 测试
    std::cout << "\nGET test...\n";
    start = Timestamp::now();
    success = 0;
    for (int i = 0; i < count; i++) {
        std::string cmd = "GET key" + std::to_string(i);
        if (client.command(cmd)) success++;
    }
    end = Timestamp::now();
    sec = timeDifference(end, start);
    std::cout << "GET: " << success << "/" << count << " in " << sec << "s = "
              << (int)(success / sec) << " QPS\n";

    std::cout << "\n========================================\n";
    return 0;
}

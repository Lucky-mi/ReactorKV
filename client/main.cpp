// client/main.cpp
// ReactorKV 客户端程序

#include "kvclient.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <getopt.h>

using namespace kvstore;

void printUsage(const char* progname) {
    std::cout << "Usage: " << progname << " [options]\n"
              << "Options:\n"
              << "  -h, --host HOST      Server host (default: 127.0.0.1)\n"
              << "  -p, --port PORT      Server port (default: 6379)\n"
              << "  --help               Show this help\n";
}

void printHelp() {
    std::cout << "\nAvailable commands:\n"
              << "  PUT key value   - Store a key-value pair\n"
              << "  GET key         - Get value by key\n"
              << "  DEL key         - Delete a key\n"
              << "  EXISTS key      - Check if key exists\n"
              << "  SIZE            - Get number of stored keys\n"
              << "  CLEAR           - Clear all data\n"
              << "  PING            - Test server connection\n"
              << "  QUIT            - Exit the client\n"
              << "  HELP            - Show this help\n\n";
}

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 6379;

    // 解析命令行参数
    static struct option longOptions[] = {
        {"host", required_argument, nullptr, 'h'},
        {"port", required_argument, nullptr, 'p'},
        {"help", no_argument, nullptr, 0},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    int optionIndex = 0;
    while ((opt = getopt_long(argc, argv, "h:p:", longOptions, &optionIndex)) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 0:
                if (std::string(longOptions[optionIndex].name) == "help") {
                    printUsage(argv[0]);
                    return 0;
                }
                break;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    std::cout << "========================================\n";
    std::cout << "        ReactorKV Client v1.0\n";
    std::cout << "========================================\n";
    std::cout << "  Connecting to " << host << ":" << port << "...\n";

    KVClient client(host, static_cast<uint16_t>(port));

    if (!client.connect()) {
        std::cerr << "Failed to connect: " << client.lastError() << "\n";
        return 1;
    }

    // 测试连接
    if (!client.ping()) {
        std::cerr << "Server not responding\n";
        return 1;
    }

    std::cout << "  Connected!\n";
    std::cout << "========================================\n";
    printHelp();

    std::string line;
    std::cout << "reactorkv> ";

    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            std::cout << "reactorkv> ";
            continue;
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        // 转换为大写
        for (auto& c : cmd) {
            c = toupper(c);
        }

        if (cmd == "QUIT" || cmd == "EXIT") {
            std::cout << "Bye!\n";
            break;
        } else if (cmd == "HELP") {
            printHelp();
        } else if (cmd == "PING") {
            if (client.ping()) {
                std::cout << "PONG\n";
            } else {
                std::cout << "(error) " << client.lastError() << "\n";
            }
        } else if (cmd == "PUT" || cmd == "SET") {
            std::string key, value;
            iss >> key;
            std::getline(iss, value);
            // 去掉前导空格
            size_t start = value.find_first_not_of(" \t");
            if (start != std::string::npos) {
                value = value.substr(start);
            }

            if (key.empty() || value.empty()) {
                std::cout << "(error) Usage: PUT key value\n";
            } else if (client.put(key, value)) {
                std::cout << "OK\n";
            } else {
                std::cout << "(error) " << client.lastError() << "\n";
            }
        } else if (cmd == "GET") {
            std::string key;
            iss >> key;
            if (key.empty()) {
                std::cout << "(error) Usage: GET key\n";
            } else {
                auto result = client.get(key);
                if (result.first) {
                    std::cout << "\"" << result.second << "\"\n";
                } else {
                    std::cout << "(nil)\n";
                }
            }
        } else if (cmd == "DEL" || cmd == "DELETE") {
            std::string key;
            iss >> key;
            if (key.empty()) {
                std::cout << "(error) Usage: DEL key\n";
            } else if (client.del(key)) {
                std::cout << "(integer) 1\n";
            } else {
                std::cout << "(integer) 0\n";
            }
        } else if (cmd == "EXISTS") {
            std::string key;
            iss >> key;
            if (key.empty()) {
                std::cout << "(error) Usage: EXISTS key\n";
            } else if (client.exists(key)) {
                std::cout << "(integer) 1\n";
            } else {
                std::cout << "(integer) 0\n";
            }
        } else if (cmd == "SIZE" || cmd == "DBSIZE") {
            auto result = client.size();
            if (result.first) {
                std::cout << "(integer) " << result.second << "\n";
            } else {
                std::cout << "(error) " << client.lastError() << "\n";
            }
        } else if (cmd == "CLEAR" || cmd == "FLUSHDB") {
            if (client.clear()) {
                std::cout << "OK\n";
            } else {
                std::cout << "(error) " << client.lastError() << "\n";
            }
        } else {
            std::cout << "(error) Unknown command '" << cmd << "'. Type HELP for available commands.\n";
        }

        std::cout << "reactorkv> ";
    }

    client.disconnect();
    return 0;
}

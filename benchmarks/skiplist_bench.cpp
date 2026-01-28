// benchmarks/skiplist_bench.cpp
// SkipList 存储引擎性能测试

#include "storage/skiplist.h"
#include "base/timestamp.h"

#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <iomanip>
#include <thread>

using namespace kvstore;

// 生成随机字符串
std::string randomString(int len) {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);

    std::string result;
    result.reserve(len);
    for (int i = 0; i < len; i++) {
        result += chars[dis(gen)];
    }
    return result;
}

void printResult(const std::string& testName, int count, double seconds) {
    double qps = count / seconds;
    std::cout << std::left << std::setw(30) << testName
              << std::right << std::setw(10) << count << " ops, "
              << std::fixed << std::setprecision(3) << std::setw(8) << seconds << " sec, "
              << std::setprecision(0) << std::setw(12) << qps << " QPS"
              << std::endl;
}

void benchSequentialInsert(SkipList<std::string, std::string>& sl, int count) {
    Timestamp start = Timestamp::now();
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        sl.insert(key, value);
    }
    Timestamp end = Timestamp::now();
    printResult("Sequential Insert", count, timeDifference(end, start));
}

void benchRandomInsert(SkipList<std::string, std::string>& sl, int count) {
    std::vector<std::pair<std::string, std::string>> data;
    for (int i = 0; i < count; i++) {
        data.push_back({randomString(16), randomString(32)});
    }

    Timestamp start = Timestamp::now();
    for (const auto& kv : data) {
        sl.insert(kv.first, kv.second);
    }
    Timestamp end = Timestamp::now();
    printResult("Random Insert", count, timeDifference(end, start));
}

void benchSequentialSearch(SkipList<std::string, std::string>& sl, int count) {
    std::string value;
    Timestamp start = Timestamp::now();
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(i % sl.size());
        sl.search(key, value);
    }
    Timestamp end = Timestamp::now();
    printResult("Sequential Search", count, timeDifference(end, start));
}

void benchRandomSearch(SkipList<std::string, std::string>& sl, int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sl.size() - 1);

    std::string value;
    Timestamp start = Timestamp::now();
    for (int i = 0; i < count; i++) {
        std::string key = "key" + std::to_string(dis(gen));
        sl.search(key, value);
    }
    Timestamp end = Timestamp::now();
    printResult("Random Search", count, timeDifference(end, start));
}

void benchMixedReadWrite(SkipList<std::string, std::string>& sl, int count, int readRatio) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDis(1, 100);
    std::uniform_int_distribution<> keyDis(0, sl.size() - 1);

    std::string value;
    int reads = 0, writes = 0;

    Timestamp start = Timestamp::now();
    for (int i = 0; i < count; i++) {
        if (opDis(gen) <= readRatio) {
            // 读操作
            std::string key = "key" + std::to_string(keyDis(gen));
            sl.search(key, value);
            reads++;
        } else {
            // 写操作
            std::string key = "key" + std::to_string(sl.size() + i);
            sl.insert(key, "value");
            writes++;
        }
    }
    Timestamp end = Timestamp::now();

    std::string name = "Mixed " + std::to_string(readRatio) + "% Read";
    printResult(name, count, timeDifference(end, start));
}

void benchConcurrentInsert(int numThreads, int countPerThread) {
    SkipList<std::string, std::string> sl;
    std::vector<std::thread> threads;

    Timestamp start = Timestamp::now();
    for (int t = 0; t < numThreads; t++) {
        threads.emplace_back([&sl, t, countPerThread]() {
            for (int i = 0; i < countPerThread; i++) {
                std::string key = "t" + std::to_string(t) + "_key" + std::to_string(i);
                sl.insert(key, "value");
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }
    Timestamp end = Timestamp::now();

    int total = numThreads * countPerThread;
    printResult("Concurrent Insert (" + std::to_string(numThreads) + " threads)",
                total, timeDifference(end, start));
}

int main(int argc, char* argv[]) {
    int count = 100000;
    if (argc > 1) {
        count = atoi(argv[1]);
    }

    std::cout << "========================================\n";
    std::cout << "    SkipList Benchmark\n";
    std::cout << "========================================\n";
    std::cout << "Operations per test: " << count << "\n";
    std::cout << "----------------------------------------\n";

    // 单线程测试
    {
        SkipList<std::string, std::string> sl;
        benchSequentialInsert(sl, count);
    }

    {
        SkipList<std::string, std::string> sl;
        benchRandomInsert(sl, count);
    }

    {
        SkipList<std::string, std::string> sl;
        // 先插入数据
        for (int i = 0; i < count; i++) {
            sl.insert("key" + std::to_string(i), "value" + std::to_string(i));
        }
        benchSequentialSearch(sl, count);
        benchRandomSearch(sl, count);
        benchMixedReadWrite(sl, count, 90);  // 90% 读
        benchMixedReadWrite(sl, count, 50);  // 50% 读
    }

    std::cout << "----------------------------------------\n";

    // 多线程测试
    benchConcurrentInsert(2, count / 2);
    benchConcurrentInsert(4, count / 4);
    benchConcurrentInsert(8, count / 8);

    std::cout << "========================================\n";

    return 0;
}

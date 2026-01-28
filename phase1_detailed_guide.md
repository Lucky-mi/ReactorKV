# 第一阶段：环境搭建与工程化基础 - 详细实施指南

> **目标**：建立完整的项目骨架，包括构建系统、日志、测试框架、编码规范
> **时间**：5-7 天
> **产出**：可编译运行的项目框架 + 基础工具类 + 单元测试示例

---

## 📁 Day 1: 项目目录结构与 CMake 构建系统

### 1.1 创建完整目录结构

```bash
# 在终端执行以下命令创建项目骨架
mkdir -p KVStorageEngine/{src/{base,net,storage,protocol,server},tests,include,lib,bin,build,docs,scripts,third_party}

cd KVStorageEngine

# 创建必要的空文件占位
touch CMakeLists.txt
touch src/CMakeLists.txt
touch src/base/CMakeLists.txt
touch tests/CMakeLists.txt
touch README.md
touch .gitignore
touch .clang-format
```

最终目录结构应该是：
```
KVStorageEngine/
├── CMakeLists.txt              # 顶层 CMake 配置
├── README.md                   # 项目说明
├── .gitignore                  # Git 忽略文件
├── .clang-format               # 代码格式化配置
│
├── include/                    # 公共头文件（对外暴露的接口）
│   └── kvstore/
│       └── kvstore.h
│
├── src/                        # 源代码
│   ├── CMakeLists.txt
│   ├── base/                   # 基础工具库
│   │   ├── CMakeLists.txt
│   │   ├── noncopyable.h
│   │   ├── types.h
│   │   ├── timestamp.h
│   │   ├── timestamp.cpp
│   │   ├── logger.h
│   │   ├── logger.cpp
│   │   ├── mutex.h
│   │   └── current_thread.h
│   │
│   ├── net/                    # 网络库（阶段三实现）
│   ├── storage/                # 存储引擎（阶段二实现）
│   ├── protocol/               # 协议层（阶段五实现）
│   └── server/                 # 服务器主程序
│
├── tests/                      # 单元测试
│   ├── CMakeLists.txt
│   ├── base/
│   │   ├── timestamp_test.cpp
│   │   └── logger_test.cpp
│   └── storage/
│       └── skiplist_test.cpp   # 阶段二添加
│
├── third_party/                # 第三方库
│   └── googletest/             # GTest (git submodule)
│
├── scripts/                    # 脚本工具
│   ├── build.sh
│   ├── run_tests.sh
│   └── format.sh
│
├── docs/                       # 文档
│   └── design.md
│
├── bin/                        # 可执行文件输出
├── lib/                        # 库文件输出
└── build/                      # CMake 构建目录
```

### 1.2 顶层 CMakeLists.txt

```cmake
# CMakeLists.txt (项目根目录)
cmake_minimum_required(VERSION 3.14)

project(KVStorageEngine
    VERSION 1.0.0
    DESCRIPTION "A High-Performance KV Storage Engine based on Reactor Pattern"
    LANGUAGES CXX
)

# ============== 编译选项 ==============
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# 默认 Debug 模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")

# ============== 依赖查找 ==============
find_package(Threads REQUIRED)

# ============== 子目录 ==============
add_subdirectory(src)

# ============== 测试 ==============
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(third_party/googletest)
    add_subdirectory(tests)
endif()

# ============== 安装配置 ==============
# install(TARGETS kvstore_base DESTINATION lib)
# install(DIRECTORY include/ DESTINATION include)
```

### 1.3 src/CMakeLists.txt

```cmake
# src/CMakeLists.txt
add_subdirectory(base)
# add_subdirectory(net)        # 阶段三开启
# add_subdirectory(storage)    # 阶段二开启
# add_subdirectory(server)     # 阶段六开启
```

### 1.4 src/base/CMakeLists.txt

```cmake
# src/base/CMakeLists.txt

# 收集所有源文件
set(BASE_SOURCES
    timestamp.cpp
    logger.cpp
)

# 创建静态库
add_library(kvstore_base STATIC ${BASE_SOURCES})

# 设置头文件搜索路径
target_include_directories(kvstore_base
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(kvstore_base
    PUBLIC
        Threads::Threads
)

# 设置编译特性
target_compile_features(kvstore_base PUBLIC cxx_std_14)
```

### 1.5 .gitignore

```gitignore
# Build directories
build/
cmake-build-*/
bin/
lib/

# IDE
.idea/
.vscode/
*.swp
*.swo
*~

# Compiled files
*.o
*.a
*.so
*.dylib

# Test output
Testing/
*.log

# OS files
.DS_Store
Thumbs.db
```

### 1.6 .clang-format (代码格式化配置)

```yaml
# .clang-format
# 基于 Google 风格，略作调整
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
AccessModifierOffset: -4
AlignAfterOpenBracket: Align
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: Never
AllowShortLoopsOnASingleLine: false
BreakBeforeBraces: Attach
IncludeBlocks: Preserve
SortIncludes: true
```

---

## 📁 Day 2: 基础工具类实现

### 2.1 src/base/noncopyable.h

```cpp
// src/base/noncopyable.h
#ifndef KVSTORE_BASE_NONCOPYABLE_H
#define KVSTORE_BASE_NONCOPYABLE_H

namespace kvstore {

/**
 * @brief 不可拷贝基类
 * 
 * 继承此类的派生类将禁止拷贝构造和拷贝赋值
 * 使用方式: class MyClass : noncopyable { ... };
 */
class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_NONCOPYABLE_H
```

### 2.2 src/base/types.h

```cpp
// src/base/types.h
#ifndef KVSTORE_BASE_TYPES_H
#define KVSTORE_BASE_TYPES_H

#include <stdint.h>
#include <string>

namespace kvstore {

// 使用标准整数类型
using std::string;

// 隐式类型转换检查（编译期）
template <typename To, typename From>
inline To implicit_cast(From const& f) {
    return f;
}

// 向下转型（调试时检查）
template <typename To, typename From>
inline To down_cast(From* f) {
    // 只在 Debug 模式下检查
#if !defined(NDEBUG)
    assert(f == nullptr || dynamic_cast<To>(f) != nullptr);
#endif
    return static_cast<To>(f);
}

}  // namespace kvstore

#endif  // KVSTORE_BASE_TYPES_H
```

### 2.3 src/base/timestamp.h

```cpp
// src/base/timestamp.h
#ifndef KVSTORE_BASE_TIMESTAMP_H
#define KVSTORE_BASE_TIMESTAMP_H

#include <stdint.h>
#include <string>

namespace kvstore {

/**
 * @brief 时间戳类
 * 
 * 基于 UTC 时间，精度为微秒
 * 使用值语义，可以安全拷贝
 */
class Timestamp {
public:
    Timestamp() : microSecondsSinceEpoch_(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

    // 获取当前时间
    static Timestamp now();

    // 获取一个无效的时间戳
    static Timestamp invalid() { return Timestamp(); }

    // 是否有效
    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // 获取微秒数
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    // 获取秒数
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    // 转换为字符串 (格式: "YYYY-MM-DD HH:MM:SS.ssssss")
    std::string toString() const;

    // 转换为格式化字符串 (格式: "YYYYMMDD HH:MM:SS.ssssss")
    std::string toFormattedString(bool showMicroseconds = true) const;

    // 常量
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

// 比较运算符
inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator!=(Timestamp lhs, Timestamp rhs) {
    return !(lhs == rhs);
}

// 计算时间差（秒）
inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

// 时间加法
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace kvstore

#endif  // KVSTORE_BASE_TIMESTAMP_H
```

### 2.4 src/base/timestamp.cpp

```cpp
// src/base/timestamp.cpp
#include "base/timestamp.h"

#include <sys/time.h>
#include <stdio.h>
#include <inttypes.h>

namespace kvstore {

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string Timestamp::toString() const {
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroseconds) {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

}  // namespace kvstore
```

### 2.5 src/base/current_thread.h

```cpp
// src/base/current_thread.h
#ifndef KVSTORE_BASE_CURRENT_THREAD_H
#define KVSTORE_BASE_CURRENT_THREAD_H

#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace kvstore {
namespace CurrentThread {

// 线程局部变量，缓存线程ID
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();

inline int tid() {
    // 使用 __builtin_expect 优化分支预测
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* tidString() {
    return t_tidString;
}

inline int tidStringLength() {
    return t_tidStringLength;
}

inline const char* name() {
    return t_threadName;
}

}  // namespace CurrentThread
}  // namespace kvstore

#endif  // KVSTORE_BASE_CURRENT_THREAD_H
```

---

## 📁 Day 3: 日志系统实现

### 3.1 src/base/logger.h

```cpp
// src/base/logger.h
#ifndef KVSTORE_BASE_LOGGER_H
#define KVSTORE_BASE_LOGGER_H

#include "base/noncopyable.h"
#include "base/timestamp.h"

#include <string>
#include <sstream>
#include <functional>

namespace kvstore {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS
};

/**
 * @brief 日志类
 * 
 * 使用方式：
 *   LOG_INFO << "message " << value;
 * 
 * 特点：
 * 1. 线程安全
 * 2. 支持自定义输出目标
 * 3. 编译期可关闭低级别日志
 */
class Logger : noncopyable {
public:
    using OutputFunc = std::function<void(const char* msg, int len)>;
    using FlushFunc = std::function<void()>;

    // 构造函数
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    // 获取日志流
    std::ostringstream& stream() { return stream_; }

    // 静态方法：设置日志级别
    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    // 静态方法：设置输出函数
    static void setOutput(OutputFunc func);
    static void setFlush(FlushFunc func);

private:
    void formatTime();
    void finish();

    Timestamp time_;
    std::ostringstream stream_;
    LogLevel level_;
    int line_;
    const char* basename_;
};

// 全局日志级别
extern LogLevel g_logLevel;

inline LogLevel Logger::logLevel() {
    return g_logLevel;
}

// ==================== 日志宏定义 ====================

#define LOG_TRACE                                          \
    if (kvstore::Logger::logLevel() <= kvstore::LogLevel::TRACE) \
    kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::TRACE, __func__).stream()

#define LOG_DEBUG                                          \
    if (kvstore::Logger::logLevel() <= kvstore::LogLevel::DEBUG) \
    kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::DEBUG, __func__).stream()

#define LOG_INFO                                           \
    if (kvstore::Logger::logLevel() <= kvstore::LogLevel::INFO) \
    kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::INFO).stream()

#define LOG_WARN kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::WARN).stream()

#define LOG_ERROR kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::ERROR).stream()

#define LOG_FATAL kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::FATAL).stream()

// 条件检查宏
#define CHECK_NOTNULL(val) \
    kvstore::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template <typename T>
T* CheckNotNull(const char* file, int line, const char* names, T* ptr) {
    if (ptr == nullptr) {
        Logger(file, line, LogLevel::FATAL).stream() << names;
    }
    return ptr;
}

}  // namespace kvstore

#endif  // KVSTORE_BASE_LOGGER_H
```

### 3.2 src/base/logger.cpp

```cpp
// src/base/logger.cpp
#include "base/logger.h"
#include "base/current_thread.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace kvstore {

// 全局日志级别，默认 INFO
LogLevel g_logLevel = LogLevel::INFO;

// 日志级别名称
const char* LogLevelName[static_cast<int>(LogLevel::NUM_LOG_LEVELS)] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

// 默认输出函数：输出到 stdout
void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;  // 忽略返回值
}

// 默认刷新函数
void defaultFlush() {
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

// 获取文件名（去掉路径）
const char* getBaseName(const char* filepath) {
    const char* slash = strrchr(filepath, '/');
    if (slash) {
        return slash + 1;
    }
    return filepath;
}

Logger::Logger(const char* file, int line, LogLevel level)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(getBaseName(file)) {
    formatTime();
    stream_ << CurrentThread::tidString() << " ";
    stream_ << LogLevelName[static_cast<int>(level)];
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(getBaseName(file)) {
    formatTime();
    stream_ << CurrentThread::tidString() << " ";
    stream_ << LogLevelName[static_cast<int>(level)];
    stream_ << func << " ";
}

Logger::~Logger() {
    finish();
    const std::string& buf = stream_.str();
    g_output(buf.c_str(), static_cast<int>(buf.size()));

    if (level_ == LogLevel::FATAL) {
        g_flush();
        abort();
    }
}

void Logger::formatTime() {
    stream_ << time_.toFormattedString() << " ";
}

void Logger::finish() {
    stream_ << " - " << basename_ << ":" << line_ << "\n";
}

void Logger::setLogLevel(LogLevel level) {
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc func) {
    g_output = func;
}

void Logger::setFlush(FlushFunc func) {
    g_flush = func;
}

// CurrentThread 实现
namespace CurrentThread {

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "unknown";

void cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        t_tidStringLength =
            snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cachedTid);
    }
}

}  // namespace CurrentThread

}  // namespace kvstore
```

---

## 📁 Day 4: 互斥锁封装

### 4.1 src/base/mutex.h

```cpp
// src/base/mutex.h
#ifndef KVSTORE_BASE_MUTEX_H
#define KVSTORE_BASE_MUTEX_H

#include "base/noncopyable.h"
#include "base/current_thread.h"

#include <pthread.h>
#include <assert.h>

namespace kvstore {

/**
 * @brief 互斥锁封装
 * 
 * 使用 RAII 风格，配合 MutexLockGuard 使用
 */
class MutexLock : noncopyable {
public:
    MutexLock() : holder_(0) {
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~MutexLock() {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread() const {
        return holder_ == CurrentThread::tid();
    }

    void assertLocked() const {
        assert(isLockedByThisThread());
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
        assignHolder();
    }

    void unlock() {
        unassignHolder();
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t* getPthreadMutex() {
        return &mutex_;
    }

private:
    friend class Condition;

    // 仅供 Condition 使用的内部类
    class UnassignGuard : noncopyable {
    public:
        explicit UnassignGuard(MutexLock& owner) : owner_(owner) {
            owner_.unassignHolder();
        }

        ~UnassignGuard() {
            owner_.assignHolder();
        }

    private:
        MutexLock& owner_;
    };

    void assignHolder() {
        holder_ = CurrentThread::tid();
    }

    void unassignHolder() {
        holder_ = 0;
    }

    pthread_mutex_t mutex_;
    pid_t holder_;
};

/**
 * @brief RAII 风格的锁守卫
 * 
 * 使用方式：
 *   MutexLock mutex;
 *   {
 *       MutexLockGuard lock(mutex);
 *       // 临界区代码
 *   }  // 自动释放锁
 */
class MutexLockGuard : noncopyable {
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock& mutex_;
};

// 防止误用：MutexLockGuard(mutex); 这样的临时对象会立即析构
#define MutexLockGuard(x) static_assert(false, "Missing variable name for MutexLockGuard")

/**
 * @brief 条件变量封装
 */
class Condition : noncopyable {
public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, nullptr);
    }

    ~Condition() {
        pthread_cond_destroy(&cond_);
    }

    void wait() {
        MutexLock::UnassignGuard ug(mutex_);
        pthread_cond_wait(&cond_, mutex_.getPthreadMutex());
    }

    // 返回 true 表示被唤醒，false 表示超时
    bool waitForSeconds(double seconds);

    void notify() {
        pthread_cond_signal(&cond_);
    }

    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }

private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};

}  // namespace kvstore

#endif  // KVSTORE_BASE_MUTEX_H
```

---

## 📁 Day 5: 引入 GTest 并编写单元测试

### 5.1 添加 GoogleTest

```bash
# 在项目根目录执行
cd third_party
git clone https://github.com/google/googletest.git --depth=1
cd ..
```

或者使用 git submodule:
```bash
git submodule add https://github.com/google/googletest.git third_party/googletest
```

### 5.2 tests/CMakeLists.txt

```cmake
# tests/CMakeLists.txt

# 包含 GTest
include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include)

# ==================== Timestamp 测试 ====================
add_executable(timestamp_test
    base/timestamp_test.cpp
)

target_link_libraries(timestamp_test
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME timestamp_test COMMAND timestamp_test)

# ==================== Logger 测试 ====================
add_executable(logger_test
    base/logger_test.cpp
)

target_link_libraries(logger_test
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME logger_test COMMAND logger_test)
```

### 5.3 tests/base/timestamp_test.cpp

```cpp
// tests/base/timestamp_test.cpp
#include "base/timestamp.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace kvstore;

class TimestampTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试用例开始前执行
    }

    void TearDown() override {
        // 每个测试用例结束后执行
    }
};

// 测试默认构造
TEST_F(TimestampTest, DefaultConstructor) {
    Timestamp ts;
    EXPECT_FALSE(ts.valid());
    EXPECT_EQ(ts.microSecondsSinceEpoch(), 0);
}

// 测试 now()
TEST_F(TimestampTest, Now) {
    Timestamp ts = Timestamp::now();
    EXPECT_TRUE(ts.valid());
    EXPECT_GT(ts.microSecondsSinceEpoch(), 0);
}

// 测试时间差计算
TEST_F(TimestampTest, TimeDifference) {
    Timestamp t1 = Timestamp::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Timestamp t2 = Timestamp::now();
    
    double diff = timeDifference(t2, t1);
    // 允许 10ms 误差
    EXPECT_GE(diff, 0.09);
    EXPECT_LE(diff, 0.15);
}

// 测试时间加法
TEST_F(TimestampTest, AddTime) {
    Timestamp t1 = Timestamp::now();
    Timestamp t2 = addTime(t1, 1.5);  // 加 1.5 秒
    
    double diff = timeDifference(t2, t1);
    EXPECT_DOUBLE_EQ(diff, 1.5);
}

// 测试比较运算符
TEST_F(TimestampTest, Comparison) {
    Timestamp t1 = Timestamp::now();
    Timestamp t2 = addTime(t1, 1.0);
    
    EXPECT_TRUE(t1 < t2);
    EXPECT_FALSE(t1 == t2);
    EXPECT_TRUE(t1 != t2);
}

// 测试字符串转换
TEST_F(TimestampTest, ToString) {
    Timestamp ts = Timestamp::now();
    std::string str = ts.toString();
    
    // 应该包含小数点
    EXPECT_NE(str.find('.'), std::string::npos);
}

// 测试格式化字符串
TEST_F(TimestampTest, ToFormattedString) {
    Timestamp ts = Timestamp::now();
    
    std::string withMicro = ts.toFormattedString(true);
    std::string withoutMicro = ts.toFormattedString(false);
    
    // 带微秒的更长
    EXPECT_GT(withMicro.length(), withoutMicro.length());
    
    // 应该包含日期分隔符
    EXPECT_NE(withMicro.find('-'), std::string::npos);
    EXPECT_NE(withMicro.find(':'), std::string::npos);
}

// 并发测试：多线程调用 now()
TEST_F(TimestampTest, ConcurrentNow) {
    const int kNumThreads = 10;
    const int kIterations = 1000;
    
    std::vector<std::thread> threads;
    std::vector<Timestamp> timestamps(kNumThreads * kIterations);
    
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&timestamps, i, kIterations]() {
            for (int j = 0; j < kIterations; ++j) {
                timestamps[i * kIterations + j] = Timestamp::now();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // 所有时间戳都应该有效
    for (const auto& ts : timestamps) {
        EXPECT_TRUE(ts.valid());
    }
}
```

### 5.4 tests/base/logger_test.cpp

```cpp
// tests/base/logger_test.cpp
#include "base/logger.h"

#include <gtest/gtest.h>
#include <sstream>
#include <string>

using namespace kvstore;

class LoggerTest : public ::testing::Test {
protected:
    std::ostringstream output_;
    Logger::OutputFunc originalOutput_;
    LogLevel originalLevel_;

    void SetUp() override {
        originalLevel_ = Logger::logLevel();
        
        // 重定向日志输出到 stringstream
        Logger::setOutput([this](const char* msg, int len) {
            output_.write(msg, len);
        });
        
        Logger::setLogLevel(LogLevel::TRACE);
    }

    void TearDown() override {
        Logger::setLogLevel(originalLevel_);
        Logger::setOutput([](const char* msg, int len) {
            fwrite(msg, 1, len, stdout);
        });
    }
    
    std::string getOutput() {
        return output_.str();
    }
    
    void clearOutput() {
        output_.str("");
        output_.clear();
    }
};

// 测试基本日志输出
TEST_F(LoggerTest, BasicOutput) {
    LOG_INFO << "Hello, World!";
    
    std::string out = getOutput();
    EXPECT_NE(out.find("Hello, World!"), std::string::npos);
    EXPECT_NE(out.find("INFO"), std::string::npos);
}

// 测试日志级别
TEST_F(LoggerTest, LogLevel) {
    Logger::setLogLevel(LogLevel::WARN);
    
    LOG_INFO << "This should not appear";
    EXPECT_TRUE(getOutput().empty());
    
    clearOutput();
    
    LOG_WARN << "This should appear";
    EXPECT_FALSE(getOutput().empty());
}

// 测试不同级别
TEST_F(LoggerTest, DifferentLevels) {
    clearOutput();
    LOG_TRACE << "trace message";
    EXPECT_NE(getOutput().find("TRACE"), std::string::npos);
    
    clearOutput();
    LOG_DEBUG << "debug message";
    EXPECT_NE(getOutput().find("DEBUG"), std::string::npos);
    
    clearOutput();
    LOG_ERROR << "error message";
    EXPECT_NE(getOutput().find("ERROR"), std::string::npos);
}

// 测试多种类型输出
TEST_F(LoggerTest, MultipleTypes) {
    int num = 42;
    double pi = 3.14159;
    std::string str = "test";
    
    LOG_INFO << "int=" << num << " double=" << pi << " str=" << str;
    
    std::string out = getOutput();
    EXPECT_NE(out.find("42"), std::string::npos);
    EXPECT_NE(out.find("3.14159"), std::string::npos);
    EXPECT_NE(out.find("test"), std::string::npos);
}

// 测试文件名和行号
TEST_F(LoggerTest, FileAndLine) {
    LOG_INFO << "test";  // 这是第 N 行
    
    std::string out = getOutput();
    // 应该包含文件名
    EXPECT_NE(out.find("logger_test.cpp"), std::string::npos);
}
```

---

## 📁 Day 6-7: 构建脚本与完整测试

### 6.1 scripts/build.sh

```bash
#!/bin/bash
# scripts/build.sh

set -e  # 遇到错误立即退出

PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="${PROJECT_ROOT}/build"
BUILD_TYPE=${1:-Debug}

echo "========================================"
echo "Building KVStorageEngine..."
echo "Build Type: ${BUILD_TYPE}"
echo "Build Directory: ${BUILD_DIR}"
echo "========================================"

# 创建构建目录
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# 运行 CMake
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..

# 编译 (使用所有 CPU 核心)
make -j$(nproc)

echo "========================================"
echo "Build completed successfully!"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo "========================================"
```

### 6.2 scripts/run_tests.sh

```bash
#!/bin/bash
# scripts/run_tests.sh

set -e

PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="${PROJECT_ROOT}/build"

echo "========================================"
echo "Running all tests..."
echo "========================================"

cd "${BUILD_DIR}"

# 使用 CTest 运行所有测试
ctest --output-on-failure --verbose

echo "========================================"
echo "All tests passed!"
echo "========================================"
```

### 6.3 scripts/format.sh

```bash
#!/bin/bash
# scripts/format.sh
# 格式化所有 C++ 代码

PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)

find "${PROJECT_ROOT}/src" "${PROJECT_ROOT}/tests" \
    -name "*.cpp" -o -name "*.h" | \
    xargs clang-format -i -style=file

echo "Code formatting completed!"
```

### 6.4 完整构建与测试

```bash
# 赋予脚本执行权限
chmod +x scripts/*.sh

# 构建项目
./scripts/build.sh Debug

# 运行测试
./scripts/run_tests.sh

# 格式化代码
./scripts/format.sh
```

---

## ✅ 第一阶段检查清单

完成第一阶段后，你应该有：

- [ ] 完整的目录结构
- [ ] CMake 构建系统正常工作
- [ ] `kvstore_base` 静态库可编译
- [ ] `Timestamp` 类及其单元测试（全部通过）
- [ ] `Logger` 类及其单元测试（全部通过）
- [ ] `MutexLock` / `MutexLockGuard` / `Condition` 封装
- [ ] `noncopyable` 基类
- [ ] `.clang-format` 代码格式化配置
- [ ] 构建脚本（build.sh, run_tests.sh）
- [ ] Git 初始化并提交

### 验证命令

```bash
# 构建
cd build && cmake .. && make -j$(nproc)

# 测试
ctest --output-on-failure

# 预期输出
# [==========] 2 tests from 2 test suites ran.
# [  PASSED  ] 2 tests.
```

---

## 🚀 下一步：阶段二预告

完成第一阶段后，进入**跳表实现**：

```
Week 2: 存储引擎核心
├── src/storage/
│   ├── skiplist.h          # 跳表模板类
│   ├── skiplist_test.cpp   # 跳表单元测试
│   └── kvstore.h           # KV 封装
└── 目标：百万级数据读写性能测试
```

需要我继续生成第二阶段的详细指导吗？

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
    TRACE,   // 最详细的跟踪信息
    DBG,     // 调试信息 (不能用DEBUG，会与-DDEBUG宏冲突)    
    INFO,    // 普通信息
    WARN,    // 警告
    ERROR,   // 错误
    FATAL,   // 致命错误，会导致程序终止
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
 * 2. 支持自定义输出目标（默认 stdout）
 * 3. 编译期可关闭低级别日志（通过日志级别控制）
 * 4. 临时对象生命周期结束时自动输出
 */
class Logger : noncopyable {
public:
    using OutputFunc = std::function<void(const char* msg, int len)>;
    using FlushFunc = std::function<void()>;

    /// 构造函数
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    /// 获取日志流（用于 << 输出）
    std::ostringstream& stream() { return stream_; }

    // ==================== 静态方法 ====================

    /// 获取当前日志级别
    static LogLevel logLevel();
    
    /// 设置日志级别
    static void setLogLevel(LogLevel level);

    /// 设置输出函数（可重定向到文件）
    static void setOutput(OutputFunc func);
    
    /// 设置刷新函数
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
    if (kvstore::Logger::logLevel() <= kvstore::LogLevel::DBG) \
    kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::DBG, __func__).stream()

#define LOG_INFO                                           \
    if (kvstore::Logger::logLevel() <= kvstore::LogLevel::INFO) \
    kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::INFO).stream()

#define LOG_WARN kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::WARN).stream()

#define LOG_ERROR kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::ERROR).stream()

#define LOG_FATAL kvstore::Logger(__FILE__, __LINE__, kvstore::LogLevel::FATAL).stream()

// ==================== 条件检查宏 ====================

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

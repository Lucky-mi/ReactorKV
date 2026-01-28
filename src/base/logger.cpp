// src/base/logger.cpp
#include "base/logger.h"
#include "base/current_thread.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace kvstore {

// 全局日志级别，默认 INFO
LogLevel g_logLevel = LogLevel::INFO;

// 日志级别名称
static const char* LogLevelName[static_cast<int>(LogLevel::NUM_LOG_LEVELS)] = {
    "TRACE ",
    "DEBUG ",  // 显示仍用DEBUG，只是enum名改为DBG
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

// 默认输出函数：输出到 stdout
static void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, static_cast<size_t>(len), stdout);
    (void)n;  // 忽略返回值
}

// 默认刷新函数
static void defaultFlush() {
    fflush(stdout);
}

static Logger::OutputFunc g_output = defaultOutput;
static Logger::FlushFunc g_flush = defaultFlush;

// 获取文件名（去掉路径）
static const char* getBaseName(const char* filepath) {
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
    stream_ << LogLevelName[static_cast<int>(level_)];
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(getBaseName(file)) {
    formatTime();
    stream_ << CurrentThread::tidString() << " ";
    stream_ << LogLevelName[static_cast<int>(level_)];
    stream_ << func << " ";
}

Logger::~Logger() {
    finish();
    const std::string buf = stream_.str();
    g_output(buf.c_str(), static_cast<int>(buf.size()));

    // FATAL 级别日志会触发程序终止
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

// ==================== CurrentThread 实现 ====================

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

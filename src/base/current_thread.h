// src/base/current_thread.h
#ifndef KVSTORE_BASE_CURRENT_THREAD_H
#define KVSTORE_BASE_CURRENT_THREAD_H

#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace kvstore {

/**
 * @brief 当前线程信息
 * 
 * 使用线程局部变量 (__thread) 缓存线程 ID，
 * 避免每次调用 syscall(SYS_gettid) 的开销。
 * 
 * 用途：
 * 1. 日志打印线程 ID
 * 2. EventLoop 验证当前线程
 */
namespace CurrentThread {

// 线程局部变量，缓存线程ID
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();

/// 获取当前线程 ID
inline int tid() {
    // 使用 __builtin_expect 优化分支预测
    // 大多数情况下 t_cachedTid 已经被缓存
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

/// 获取线程 ID 字符串
inline const char* tidString() {
    return t_tidString;
}

/// 获取线程 ID 字符串长度
inline int tidStringLength() {
    return t_tidStringLength;
}

/// 获取线程名称
inline const char* name() {
    return t_threadName;
}

}  // namespace CurrentThread
}  // namespace kvstore

#endif  // KVSTORE_BASE_CURRENT_THREAD_H

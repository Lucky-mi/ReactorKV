// src/base/timestamp.h
#ifndef KVSTORE_BASE_TIMESTAMP_H
#define KVSTORE_BASE_TIMESTAMP_H

#include <stdint.h>
#include <ctime>
#include <string>

namespace kvstore {

/**
 * @brief 时间戳类
 * 
 * 基于 UTC 时间，精度为微秒。
 * 使用值语义，可以安全拷贝。
 * 
 * 用途：
 * 1. 日志打印时间
 * 2. 定时器管理
 * 3. 性能统计
 */
class Timestamp {
public:
    Timestamp() : microSecondsSinceEpoch_(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

    /// 获取当前时间
    static Timestamp now();

    /// 获取一个无效的时间戳
    static Timestamp invalid() { return Timestamp(); }

    /// 是否有效
    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    /// 获取微秒数
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    /// 获取秒数
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    /// 转换为字符串 (格式: "seconds.microseconds")
    std::string toString() const;

    /// 转换为格式化字符串 (格式: "YYYY-MM-DD HH:MM:SS.ssssss")
    std::string toFormattedString(bool showMicroseconds = true) const;

    /// 常量
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

// ==================== 比较运算符 ====================

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator!=(Timestamp lhs, Timestamp rhs) {
    return !(lhs == rhs);
}

// ==================== 辅助函数 ====================

/// 计算时间差（返回秒）
inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

/// 时间加法
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace kvstore

#endif  // KVSTORE_BASE_TIMESTAMP_H

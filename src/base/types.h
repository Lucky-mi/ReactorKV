// src/base/types.h
#ifndef KVSTORE_BASE_TYPES_H
#define KVSTORE_BASE_TYPES_H

#include <stdint.h>
#include <string>
#include <cassert>

namespace kvstore {

// 使用标准整数类型
using std::string;

/**
 * @brief 隐式类型转换检查（编译期）
 * 
 * 安全地进行隐式类型转换，如果转换不合法会在编译期报错。
 */
template <typename To, typename From>
inline To implicit_cast(From const& f) {
    return f;
}

/**
 * @brief 向下转型（调试时检查）
 * 
 * 在 Debug 模式下使用 dynamic_cast 验证转换的正确性，
 * Release 模式下使用 static_cast 提高性能。
 */
template <typename To, typename From>
inline To down_cast(From* f) {
#if !defined(NDEBUG)
    assert(f == nullptr || dynamic_cast<To>(f) != nullptr);
#endif
    return static_cast<To>(f);
}

}  // namespace kvstore

#endif  // KVSTORE_BASE_TYPES_H

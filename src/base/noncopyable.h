// src/base/noncopyable.h
#ifndef KVSTORE_BASE_NONCOPYABLE_H
#define KVSTORE_BASE_NONCOPYABLE_H

namespace kvstore {

/**
 * @brief 不可拷贝基类
 * 
 * 继承此类的派生类将禁止拷贝构造和拷贝赋值。
 * 这是资源管理类（如锁、文件句柄、socket）的基本要求，
 * 避免浅拷贝导致的双重释放问题。
 * 
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

// src/net/buffer.h
#ifndef KVSTORE_NET_BUFFER_H
#define KVSTORE_NET_BUFFER_H

#include <algorithm>
#include <string>
#include <vector>
#include <cstring>

namespace kvstore {

/**
 * @brief 应用层缓冲区
 *
 * 解决 TCP 粘包/半包问题的关键组件。
 * 采用 vector<char> 作为底层存储，支持自动扩容。
 *
 * 结构：
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 *
 * prependable: 预留空间，用于在数据前面添加长度等头部信息
 * readable: 可读取的有效数据
 * writable: 可写入的空闲空间
 */
class Buffer {
public:
    static const size_t kCheapPrepend = 8;    // 预留 8 字节用于 prepend
    static const size_t kInitialSize = 1024;  // 初始缓冲区大小

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {}

    // ==================== 容量查询 ====================

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // ==================== 读取操作 ====================

    /// 获取可读数据的起始地址
    const char* peek() const { return begin() + readerIndex_; }

    /// 查找 "\r\n"
    const char* findCRLF() const {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    /// 取走 len 字节数据（移动 readerIndex）
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    /// 取走所有数据
    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    /// 取走数据并返回 string
    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    /// 取走所有数据并返回 string
    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    // ==================== 写入操作 ====================

    /// 获取可写区域的起始地址
    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }

    /// 确保有足够的可写空间
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    /// 写入数据
    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    /// 更新 writerIndex
    void hasWritten(size_t len) { writerIndex_ += len; }

    /// 回退 writerIndex
    void unwrite(size_t len) { writerIndex_ -= len; }

    // ==================== Prepend ====================

    /// 在数据前面添加内容
    void prepend(const void* data, size_t len) {
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    // ==================== 网络 IO ====================

    /// 从 fd 读取数据
    ssize_t readFd(int fd, int* savedErrno);

    /// 向 fd 写入数据
    ssize_t writeFd(int fd, int* savedErrno);

    // ==================== 32位整数操作（网络序）====================

    void appendInt32(int32_t x) {
        int32_t be32 = htobe32(x);
        append(&be32, sizeof(be32));
    }

    int32_t peekInt32() const {
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof(be32));
        return be32toh(be32);
    }

    int32_t readInt32() {
        int32_t result = peekInt32();
        retrieve(sizeof(int32_t));
        return result;
    }

    void prependInt32(int32_t x) {
        int32_t be32 = htobe32(x);
        prepend(&be32, sizeof(be32));
    }

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    void makeSpace(size_t len) {
        // 如果前面的空闲空间 + 后面的空闲空间 不够，就扩容
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        } else {
            // 内部腾挪：把数据移到前面
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};

}  // namespace kvstore

#endif  // KVSTORE_NET_BUFFER_H

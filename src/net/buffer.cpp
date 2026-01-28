// src/net/buffer.cpp
#include "net/buffer.h"

#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

namespace kvstore {

const char Buffer::kCRLF[] = "\r\n";

/**
 * @brief 从 fd 读取数据到 Buffer
 *
 * 使用 readv 实现 scatter-gather IO：
 * 1. 先尝试读到 Buffer 的可写空间
 * 2. 如果数据过多，使用栈上的 extrabuf 临时存储
 * 3. 最后把 extrabuf 的数据 append 到 Buffer
 *
 * 优点：
 * - 一次 readv 调用可以读取大量数据
 * - 避免预先分配大缓冲区
 * - 适配 ET 模式需要一次性读完的要求
 */
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    // 栈上的临时缓冲区，64KB
    char extrabuf[65536];

    struct iovec vec[2];
    const size_t writable = writableBytes();

    // 第一块：Buffer 的可写空间
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;

    // 第二块：栈上的临时缓冲区
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    // 如果 Buffer 可写空间小于 extrabuf，就用两块；否则只用一块
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;

    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        // 数据全部读到了 Buffer 中
        writerIndex_ += n;
    } else {
        // Buffer 写满了，剩余数据在 extrabuf 中
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int* savedErrno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *savedErrno = errno;
    }
    return n;
}

}  // namespace kvstore

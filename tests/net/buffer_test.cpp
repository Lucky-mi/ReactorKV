// tests/net/buffer_test.cpp
#include "net/buffer.h"

#include <gtest/gtest.h>
#include <string>

using namespace kvstore;

class BufferTest : public ::testing::Test {
protected:
    Buffer buffer_;
};

// 测试初始状态
TEST_F(BufferTest, InitialState) {
    EXPECT_EQ(buffer_.readableBytes(), 0);
    EXPECT_GT(buffer_.writableBytes(), 0);
    // 避免直接使用 Buffer::kCheapPrepend（ODR 问题）
    size_t expectedPrepend = 8;  // kCheapPrepend 的值
    EXPECT_EQ(buffer_.prependableBytes(), expectedPrepend);
}

// 测试 append 和 retrieve
TEST_F(BufferTest, AppendAndRetrieve) {
    std::string data = "Hello, World!";
    buffer_.append(data);
    
    EXPECT_EQ(buffer_.readableBytes(), data.size());
    EXPECT_EQ(buffer_.retrieveAllAsString(), data);
    EXPECT_EQ(buffer_.readableBytes(), 0);
}

// 测试 peek
TEST_F(BufferTest, Peek) {
    std::string data = "Test Data";
    buffer_.append(data);
    
    const char* peek = buffer_.peek();
    EXPECT_EQ(std::string(peek, data.size()), data);
    
    // peek 不应该改变 buffer 状态
    EXPECT_EQ(buffer_.readableBytes(), data.size());
}

// 测试 retrieve 部分数据
TEST_F(BufferTest, RetrievePartial) {
    std::string data = "Hello, World!";
    buffer_.append(data);
    
    std::string part = buffer_.retrieveAsString(5);
    EXPECT_EQ(part, "Hello");
    EXPECT_EQ(buffer_.readableBytes(), data.size() - 5);
}

// 测试 findCRLF
TEST_F(BufferTest, FindCRLF) {
    buffer_.append("Line1\r\nLine2\r\n");
    
    const char* crlf = buffer_.findCRLF();
    ASSERT_NE(crlf, nullptr);
    
    size_t lineLen = crlf - buffer_.peek();
    EXPECT_EQ(lineLen, 5);  // "Line1"
}

// 测试多次 append
TEST_F(BufferTest, MultipleAppend) {
    buffer_.append("Part1");
    buffer_.append(" ");
    buffer_.append("Part2");
    
    EXPECT_EQ(buffer_.retrieveAllAsString(), "Part1 Part2");
}

// 测试空 buffer
TEST_F(BufferTest, EmptyRetrieve) {
    std::string result = buffer_.retrieveAllAsString();
    EXPECT_TRUE(result.empty());
}

// 测试大数据
TEST_F(BufferTest, LargeData) {
    std::string largeData(10000, 'X');
    buffer_.append(largeData);
    
    EXPECT_EQ(buffer_.readableBytes(), 10000);
    EXPECT_EQ(buffer_.retrieveAllAsString(), largeData);
}

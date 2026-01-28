// tests/base/logger_test.cpp
#include "base/logger.h"

#include <gtest/gtest.h>
#include <sstream>
#include <string>

using namespace kvstore;

class LoggerTest : public ::testing::Test {
protected:
    std::ostringstream output_;
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

// 测试日志级别过滤
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
    LOG_INFO << "test";
    
    std::string out = getOutput();
    // 应该包含文件名
    EXPECT_NE(out.find("logger_test.cpp"), std::string::npos);
}

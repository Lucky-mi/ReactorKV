# tests/CMakeLists.txt

# 包含 GTest
include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include)

# ==================== Timestamp 测试 ====================
add_executable(timestamp_test
    base/timestamp_test.cpp
)

target_link_libraries(timestamp_test
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME timestamp_test COMMAND timestamp_test)

# ==================== Logger 测试 ====================
add_executable(logger_test
    base/logger_test.cpp
)

target_link_libraries(logger_test
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME logger_test COMMAND logger_test)

# ==================== SkipList 测试 ====================
add_executable(skiplist_test
    storage/skiplist_test.cpp
)

target_link_libraries(skiplist_test
    kvstore_storage
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME skiplist_test COMMAND skiplist_test)

# ==================== KVStore 测试 ====================
add_executable(kvstore_test
    storage/kvstore_test.cpp
)

target_link_libraries(kvstore_test
    kvstore_storage
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME kvstore_test COMMAND kvstore_test)

# ==================== Thread 测试 ====================
add_executable(thread_test
    base/thread_test.cpp
)

target_link_libraries(thread_test
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME thread_test COMMAND thread_test)

# ==================== ThreadPool 测试 ====================
add_executable(threadpool_test
    base/threadpool_test.cpp
)

target_link_libraries(threadpool_test
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME threadpool_test COMMAND threadpool_test)

# ==================== Buffer 测试 ====================
add_executable(buffer_test
    net/buffer_test.cpp
)

target_link_libraries(buffer_test
    kvstore_net
    kvstore_base
    gtest
    gtest_main
    pthread
)

add_test(NAME buffer_test COMMAND buffer_test)


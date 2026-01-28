# src/base/CMakeLists.txt

# 收集所有源文件
set(BASE_SOURCES
    timestamp.cpp
    logger.cpp
    thread.cpp
    threadpool.cpp
)

# 创建静态库
add_library(kvstore_base STATIC ${BASE_SOURCES})

# 设置头文件搜索路径
target_include_directories(kvstore_base
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(kvstore_base
    PUBLIC
        Threads::Threads
)

# 设置编译特性
target_compile_features(kvstore_base PUBLIC cxx_std_14)

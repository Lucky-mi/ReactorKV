# src/storage/CMakeLists.txt

# 收集所有源文件
set(STORAGE_SOURCES
    kvstore.cpp
)

# 创建静态库
add_library(kvstore_storage STATIC ${STORAGE_SOURCES})

# 设置头文件搜索路径
target_include_directories(kvstore_storage
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(kvstore_storage
    PUBLIC
        kvstore_base
)

# 设置编译特性
target_compile_features(kvstore_storage PUBLIC cxx_std_14)

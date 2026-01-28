# src/net/CMakeLists.txt

# 收集所有源文件
set(NET_SOURCES
    inet_address.cpp
    socket.cpp
    channel.cpp
    poller.cpp
    epoll_poller.cpp
    eventloop.cpp
    buffer.cpp
    acceptor.cpp
    tcp_connection.cpp
    eventloop_thread.cpp
    eventloop_thread_pool.cpp
    tcp_server.cpp
)

# 创建静态库
add_library(kvstore_net STATIC ${NET_SOURCES})

# 设置头文件搜索路径
target_include_directories(kvstore_net
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖
target_link_libraries(kvstore_net
    PUBLIC
        kvstore_base
)

# 设置编译特性
target_compile_features(kvstore_net PUBLIC cxx_std_14)

# examples/CMakeLists.txt

# Echo Server 示例
add_executable(echo_server
    echo_server.cpp
)

target_link_libraries(echo_server
    kvstore_net
    kvstore_base
)

# src/protocol/CMakeLists.txt

# Protocol 模块只有头文件，创建 INTERFACE 库
add_library(kvstore_protocol INTERFACE)

target_include_directories(kvstore_protocol
    INTERFACE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(kvstore_protocol
    INTERFACE
        kvstore_net
)

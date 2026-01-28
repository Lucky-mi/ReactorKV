# src/server/CMakeLists.txt

set(SERVER_SOURCES
    kv_server.cpp
)

add_library(kvstore_server STATIC ${SERVER_SOURCES})

target_include_directories(kvstore_server
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(kvstore_server
    PUBLIC
        kvstore_net
        kvstore_storage
        kvstore_protocol
)

target_compile_features(kvstore_server PUBLIC cxx_std_14)

# 主程序
add_executable(kvserver main.cpp)

target_link_libraries(kvserver
    kvstore_server
)


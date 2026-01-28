# client/CMakeLists.txt

# KVClient 库
add_library(kvstore_client
    kvclient.cpp
)

target_include_directories(kvstore_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# KVClient 可执行程序
add_executable(kvclient
    main.cpp
)

target_link_libraries(kvclient
    kvstore_client
    pthread
)

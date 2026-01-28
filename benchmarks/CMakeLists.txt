# benchmarks/CMakeLists.txt

# SkipList 性能测试
add_executable(skiplist_bench
    skiplist_bench.cpp
)

target_link_libraries(skiplist_bench
    kvstore_storage
    kvstore_base
)

# KVServer 网络性能测试（多线程）
add_executable(kvserver_bench
    kvserver_bench.cpp
)

target_link_libraries(kvserver_bench
    kvstore_net
    kvstore_base
)

# 简单性能测试（单线程）
add_executable(simple_bench
    simple_bench.cpp
)

target_link_libraries(simple_bench
    kvstore_base
)

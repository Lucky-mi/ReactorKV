#!/bin/bash
# benchmarks/simple_bench.sh
# 简单的 KVServer 性能测试脚本

HOST=${1:-127.0.0.1}
PORT=${2:-6379}
COUNT=${3:-1000}

echo "=========================================="
echo "  Simple KVServer Benchmark"
echo "=========================================="
echo "  Server: $HOST:$PORT"
echo "  Operations: $COUNT"
echo "=========================================="

# 创建临时文件存放命令
TMPFILE=$(mktemp)

# 生成 PUT 命令
echo "Generating $COUNT PUT commands..."
for i in $(seq 1 $COUNT); do
    echo "PUT key$i value$i"
done > $TMPFILE
echo "QUIT" >> $TMPFILE

# 执行 PUT 测试
echo ""
echo "Running PUT test..."
START=$(date +%s.%N)
nc -q 1 $HOST $PORT < $TMPFILE > /dev/null
END=$(date +%s.%N)
DURATION=$(echo "$END - $START" | bc)
QPS=$(echo "scale=0; $COUNT / $DURATION" | bc)
echo "PUT: $COUNT ops in ${DURATION}s = $QPS QPS"

# 生成 GET 命令
for i in $(seq 1 $COUNT); do
    echo "GET key$i"
done > $TMPFILE
echo "QUIT" >> $TMPFILE

# 执行 GET 测试
echo ""
echo "Running GET test..."
START=$(date +%s.%N)
nc -q 1 $HOST $PORT < $TMPFILE > /dev/null
END=$(date +%s.%N)
DURATION=$(echo "$END - $START" | bc)
QPS=$(echo "scale=0; $COUNT / $DURATION" | bc)
echo "GET: $COUNT ops in ${DURATION}s = $QPS QPS"

# 清理
rm -f $TMPFILE

echo ""
echo "=========================================="
echo "  Benchmark Complete"
echo "=========================================="

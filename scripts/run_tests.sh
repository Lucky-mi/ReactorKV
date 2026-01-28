#!/bin/bash
# scripts/run_tests.sh

set -e

PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="${PROJECT_ROOT}/build"

echo "========================================"
echo "Running all tests..."
echo "========================================"

cd "${BUILD_DIR}"

# 使用 CTest 运行所有测试
ctest --output-on-failure --verbose

echo "========================================"
echo "All tests passed!"
echo "========================================"

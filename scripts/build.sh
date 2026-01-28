#!/bin/bash
# scripts/build.sh

set -e  # 遇到错误立即退出

PROJECT_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="${PROJECT_ROOT}/build"
BUILD_TYPE=${1:-Debug}

echo "========================================"
echo "Building ReactorKV..."
echo "Build Type: ${BUILD_TYPE}"
echo "Build Directory: ${BUILD_DIR}"
echo "========================================"

# 创建构建目录
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# 运行 CMake
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..

# 编译 (使用所有 CPU 核心)
make -j$(nproc)

echo "========================================"
echo "Build completed successfully!"
echo "Binaries are in: ${BUILD_DIR}/bin"
echo "========================================"

#!/bin/bash

GCC_COMPILER_PATH=/path/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/

#BUILD_TYPE=Release
BUILD_TYPE=Debug

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

BUILD_DIR=${ROOT_PWD}/build/build_rk3399pro_linux_aarch64

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
    -DCMAKE_TOOLCHAIN_FILE=${ROOT_PWD}/cmake/rk3399pro.linux.aarch64.cmake \
    -DCMAKE_C_COMPILER=${GCC_COMPILER_PATH}/bin/aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=${GCC_COMPILER_PATH}/bin/aarch64-linux-gnu-g++ \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${ROOT_PWD}/install/rockx_rk3399pro_linux_aarch64
make -j4
make install
cd -

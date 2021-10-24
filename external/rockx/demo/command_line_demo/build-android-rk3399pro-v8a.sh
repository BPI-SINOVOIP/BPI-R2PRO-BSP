#!/bin/bash

set -e

ANDROID_NDK_PATH=/path/android-ndk-r16b

#BUILD_TYPE=Release
BUILD_TYPE=Debug

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

BUILD_DIR=${ROOT_PWD}/build/build_rk3399pro_android_v8a

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
        -DCMAKE_TOOLCHAIN_FILE=${ROOT_PWD}/cmake/rk3399pro.android.arm64v8a.cmake \
        -DCMAKE_ANDROID_NDK=${ANDROID_NDK_PATH} \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_INSTALL_PREFIX=${ROOT_PWD}/install/rockx_rk3399pro_android_arm64-v8a
make -j4
make install
cd -


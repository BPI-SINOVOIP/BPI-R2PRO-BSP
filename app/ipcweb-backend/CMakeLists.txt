#
# Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(version)
project(ipcweb-backend
    LANGUAGES CXX
    VERSION ${PROJECT_VERSION_FULL})
set(DESCRIPTION "IP Camera web backend cgi projects")

option(IPCWEBBACKEND_BUILD_TESTS  "Build tests executables" ON)
option(ENABLE_JWT  "enbale jwt" ON)
option(MEDIASERVER_ROCKFACE  "enbale mediaserver rockface" OFF)

set(PROJECT_THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/thirdparty")
include(clang-format)
include(cppcheck)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    message(FATAL_ERROR "This project can only be built for linux")
endif()

set(TARGET_HOST ${CMAKE_SYSTEM_PROCESSOR}-linux)

find_package(MiniLogger REQUIRED)
find_package(Cgicc)
find_package(nlohmann_json REQUIRED)
find_package(jwt_cpp REQUIRED)
find_package(OpenSSL REQUIRED)

include(GNUInstallDirs)
if (${IPCWEBBACKEND_BUILD_TESTS})
    enable_testing()
    add_subdirectory(thirdparty/googletest)
    add_subdirectory(test)
endif()
if (${ENABLE_JWT})
    add_definitions(-DENABLE_JWT)
endif()
if (${MEDIASERVER_ROCKFACE})
    add_definitions(-DMEDIASERVER_ROCKFACE)
endif()
add_subdirectory(src)

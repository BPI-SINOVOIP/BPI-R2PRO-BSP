cmake_minimum_required(VERSION 3.10)

project(rockface_demo)

message("rockface_demo")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

# rockface
set(RockFace_DIR ${CMAKE_SOURCE_DIR}/../../sdk/rockface-${TARGET_SOC}-${CMAKE_SYSTEM_NAME})
find_package(RockFace REQUIRED)
include_directories(${RockFace_INCLUDE_DIRS})

# set install prefix
if (NOT CMAKE_INSTALL_PREFIX)
    if (CMAKE_SYSTEM_NAME MATCHES "Android")
        set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rockface-${TARGET_SOC}-${CMAKE_SYSTEM_NAME}-${CMAKE_ANDROID_ARCH_ABI})
    else()
        set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rockface-${TARGET_SOC}-${CMAKE_SYSTEM_NAME})
    endif ()
endif()

# demos
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
add_subdirectory(rockface_analyze)
add_subdirectory(rockface_recognition_1v1)
add_subdirectory(rockface_recognition_1vn)
add_subdirectory(rockface_liveness)
add_subdirectory(rockface_mask_recognition_1v1)
add_subdirectory(rockface_mask_detect)

install(PROGRAMS ${RockFace_LIBS} DESTINATION lib)
install(PROGRAMS ${RockFace_DATA} DESTINATION lib)
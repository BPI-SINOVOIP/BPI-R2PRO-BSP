cmake_minimum_required(VERSION 3.10)

project(rockx_demo)

message("rockx_demo")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

# rockx
set(RockX_DIR ${CMAKE_SOURCE_DIR}/../../sdk/rockx-${TARGET_SOC}-${CMAKE_SYSTEM_NAME})
find_package(RockX REQUIRED)
include_directories(${RockX_INCLUDE_DIRS})

# set install prefix
if (NOT CMAKE_INSTALL_PREFIX)
    if (CMAKE_SYSTEM_NAME MATCHES "Android")
        set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rockx-${TARGET_SOC}-${CMAKE_SYSTEM_NAME}-${CMAKE_ANDROID_ARCH_ABI})
    else()
        set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rockx-${TARGET_SOC}-${CMAKE_SYSTEM_NAME})
    endif ()
endif()

# demos
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
add_subdirectory(rockx_face_detection_demo)
add_subdirectory(rockx_face_landmark_demo)
add_subdirectory(rockx_object_detection_demo)
add_subdirectory(rockx_pose_body_demo)
add_subdirectory(rockx_pose_finger_demo)
add_subdirectory(rockx_face_recognition_demo)
add_subdirectory(rockx_face_attribute_demo)
add_subdirectory(rockx_head_detection_demo)
add_subdirectory(rockx_carplate_demo)
add_subdirectory(rockx_object_track_demo)
add_subdirectory(rockx_face_masks_detection_demo)
add_subdirectory(rockx_body_mask_demo)
add_subdirectory(rockx_person_detection_demo)
add_subdirectory(rockx_face_capture_demo)

install(PROGRAMS ${RockX_LIBS} DESTINATION lib)
install(PROGRAMS ${RockX_BINS} DESTINATION lib)
install(PROGRAMS ${RockX_DATA} DESTINATION lib)

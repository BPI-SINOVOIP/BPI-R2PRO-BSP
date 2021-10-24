cmake_minimum_required(VERSION 3.10)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(ARCH_DIR x86)
else()
    set(ARCH_DIR arm)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/opencv/${ARCH_DIR}/include
    ${CMAKE_SOURCE_DIR}/rknn_api/${ARCH_DIR}/include
)

set(link_libs  ${CMAKE_SOURCE_DIR}/3rdparty/opencv/${ARCH_DIR}/lib64/libopencv_core.so
               ${CMAKE_SOURCE_DIR}/3rdparty/opencv/${ARCH_DIR}/lib64/libopencv_highgui.so
               ${CMAKE_SOURCE_DIR}/3rdparty/opencv/${ARCH_DIR}/lib64/libopencv_imgcodecs.so
               ${CMAKE_SOURCE_DIR}/3rdparty/opencv/${ARCH_DIR}/lib64/libopencv_imgproc.so
               ${CMAKE_SOURCE_DIR}/rknn_api/${ARCH_DIR}/lib64/librknn_api.so
               pthread
)

add_executable(rknn_mobilenet  rknn_mobilenet.cpp)
target_link_libraries(rknn_mobilenet ${link_libs})

add_executable(rknn_ssd  rknn_ssd.cpp)
target_link_libraries(rknn_ssd ${link_libs})
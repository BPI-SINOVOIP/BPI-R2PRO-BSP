cmake_minimum_required(VERSION 3.10)

project(rockx_face_capture)

set(OUT_PATH ${PROJECT_NAME}_demo)

# file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB IMGS ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg)

add_executable(rockx_face_capture
        rockx_face_capture.cpp)

target_link_libraries(rockx_face_capture
        ${RockX_LIBS})

install(TARGETS rockx_face_capture DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})

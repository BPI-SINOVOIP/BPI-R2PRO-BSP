cmake_minimum_required(VERSION 3.10)

project(rockface_recognition_mask_1v1)

set(OUT_PATH ${PROJECT_NAME}_demo)

file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB IMGS ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg)

add_executable(recognition_mask_1v1
        ${SRCS})

target_link_libraries(recognition_mask_1v1
        ${RockFace_LIBS})

install(TARGETS recognition_mask_1v1 DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})
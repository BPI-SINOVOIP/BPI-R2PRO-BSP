cmake_minimum_required(VERSION 3.10)

project(rockface_mask_det)

set(OUT_PATH ${PROJECT_NAME}_demo)

file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB IMGS ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg)

add_executable(face_mask_det
        ${SRCS})

target_link_libraries(face_mask_det
        ${RockFace_LIBS})

install(TARGETS face_mask_det DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})
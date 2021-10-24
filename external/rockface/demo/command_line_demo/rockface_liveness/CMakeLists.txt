cmake_minimum_required(VERSION 3.10)

project(rockface_liveness)

set(OUT_PATH ${PROJECT_NAME}_demo)

file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB IMGS ${CMAKE_CURRENT_SOURCE_DIR}/*.png)

add_executable(face_liveness
        ${SRCS})

target_link_libraries(face_liveness
        ${RockFace_LIBS})

install(TARGETS face_liveness DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})
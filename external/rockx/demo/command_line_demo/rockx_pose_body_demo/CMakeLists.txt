cmake_minimum_required(VERSION 3.10)

project(rockx_pose_body)

set(OUT_PATH ${PROJECT_NAME}_demo)

file(GLOB IMGS ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg)

add_executable(rockx_pose_body
        rockx_pose_body.cpp)

target_link_libraries(rockx_pose_body
        ${RockX_LIBS})

add_executable(rockx_pose_body_async
        rockx_pose_body_async.cpp)

target_link_libraries(rockx_pose_body_async
        ${RockX_LIBS})

install(TARGETS rockx_pose_body DESTINATION ${OUT_PATH})
install(TARGETS rockx_pose_body_async DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})
cmake_minimum_required(VERSION 3.10)

project(rockx_face_detection)

set(OUT_PATH ${PROJECT_NAME}_demo)

file(GLOB IMGS ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg)

add_executable(rockx_face_detection
        rockx_face_detection.cpp)

target_link_libraries(rockx_face_detection
        ${RockX_LIBS})

add_executable(rockx_face_detection_async
        rockx_face_detection_async.cpp)

target_link_libraries(rockx_face_detection_async
        ${RockX_LIBS})

install(TARGETS rockx_face_detection DESTINATION ${OUT_PATH})
install(TARGETS rockx_face_detection_async DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})

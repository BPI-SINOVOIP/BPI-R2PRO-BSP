cmake_minimum_required(VERSION 3.10)

project(rockx_object_track)

set(OUT_PATH ${PROJECT_NAME}_demo)

file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME}
        ${SRCS})

target_link_libraries(${PROJECT_NAME}
        ${RockX_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})
install(DIRECTORY frame_in DESTINATION ${OUT_PATH})
install(FILES video_frame_list.txt DESTINATION ${OUT_PATH})
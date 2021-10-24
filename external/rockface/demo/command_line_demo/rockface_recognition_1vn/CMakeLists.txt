cmake_minimum_required(VERSION 3.10)

project(rockface_recognition_1vn)

set(OUT_PATH ${PROJECT_NAME}_demo)

# sqlite
set(SQLITE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3)
include_directories(${SQLITE_PATH})
add_library(sqlite3-static STATIC 
        ${SQLITE_PATH}/sqlite3.c
        ${SQLITE_PATH}/sqlite3.h
)

target_link_libraries(sqlite3-static dl)
if (NOT CMAKE_SYSTEM_NAME MATCHES "Android")
target_link_libraries(sqlite3-static pthread)
endif ()

set(COMMON_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/face_db.c
        ${CMAKE_CURRENT_SOURCE_DIR}/rockface_recognition.c)

add_executable(import_face_library
        ${CMAKE_CURRENT_SOURCE_DIR}/import_face_library.cpp
        ${COMMON_SRCS})

target_link_libraries(import_face_library
        sqlite3-static
        ${RockFace_LIBS}
        )

add_executable(recognition_1vn
        ${CMAKE_CURRENT_SOURCE_DIR}/recognition_1vn.cpp
        ${COMMON_SRCS})

target_link_libraries(recognition_1vn
        sqlite3-static
        ${RockFace_LIBS}
        )

install(TARGETS recognition_1vn DESTINATION ${OUT_PATH})
install(TARGETS import_face_library DESTINATION ${OUT_PATH})
install(FILES ${IMGS} DESTINATION ${OUT_PATH})
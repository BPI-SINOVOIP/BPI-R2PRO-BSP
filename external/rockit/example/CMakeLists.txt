cmake_minimum_required( VERSION 2.8.8 )
add_definitions(-fno-rtti)

add_compile_options(-std=c++11)
add_definitions(-std=c++11 -Wno-attributes -Wno-deprecated-declarations -DANDROID_STL=c++_shared)

include_directories(include)
set(RK_DEP_COMMON_LIBS
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/libavcodec.so.58
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/libavformat.so.58
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/libavutil.so.56
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/libswresample.so.3
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/libgraphic_lsf.so
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/libmali.so.1
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/deps/lib/librockchip_mpp.so.1
    ${CMAKE_CURRENT_LIST_DIR}/../sdk/lib64/librockit.so
)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(RK_DEP_COMMON_LIBS
        ${RK_DEP_COMMON_LIBS}
        -lpthread
    )
    message(STATUS "Build WITH linker libs for Linux")
endif()

add_subdirectory(common)
add_subdirectory(mod)
add_subdirectory(app)
add_subdirectory(sundry)

set(ROCKIT_DUMPSYS_FILE ${CMAKE_CURRENT_LIST_DIR}/bin/dumpsys )
install(FILES ${ROCKIT_DUMPSYS_FILE} DESTINATION "bin")


set(RockFace_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)

if (CMAKE_SYSTEM_NAME MATCHES "Android")
    set(RockFace_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/${CMAKE_ANDROID_ARCH_ABI}/librknn_api.so
        ${CMAKE_CURRENT_LIST_DIR}/${CMAKE_ANDROID_ARCH_ABI}/librockface.so
    )
else ()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (TARGET_LIB_ARCH lib64)
    else()
        set (TARGET_LIB_ARCH lib)
    endif()
    set(RockFace_LIBS
        ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/librknn_api.so
        ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/librockface.so
    )
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../rockface-data")
    file(GLOB ROCKFACE_DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/../rockface-data/*")
endif()

set(RockFace_DATA ${ROCKFACE_DATA_FILES})
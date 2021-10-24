
set(ROCKIT_FILE_CONFIGS ${CMAKE_CURRENT_LIST_DIR}/conf)
set(ROCKIT_FILE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/headers)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (TARGET_LIB_ARCH lib64)
else()
    set (TARGET_LIB_ARCH lib32)
endif()


option(ENABLE_UAC  "enable uac" ON)
if (${ENABLE_UAC})
    set(ROCKIT_FILE_LIBS
       ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/libavutil.so.56
       ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/libswresample.so.3
       ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/librockit.so)
else()
    set(ROCKIT_FILE_LIBS ${CMAKE_CURRENT_LIST_DIR}/${TARGET_LIB_ARCH}/uaccut/librockit.so)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/configs")
    file(GLOB ROCKIT_FILE_CONFIGS "${CMAKE_CURRENT_LIST_DIR}/configs/*")
endif()

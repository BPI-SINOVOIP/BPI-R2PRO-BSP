option(ENABLE_MEDIASERVER_LIB_EXAMPLE  "enbale libmediaserver exmaple" ON)
if (ENABLE_MEDIASERVER_LIB_EXAMPLE)
aux_source_directory(example MEDIASEVER_LIB_EXAMPLE_SRC)
add_executable(libmediaserver_test ${MEDIASEVER_LIB_EXAMPLE_SRC})
target_include_directories(libmediaserver_test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_SYSROOT}/usr/include/rkaiq/
    ${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI/
    ${CMAKE_SYSROOT}/usr/include/rkaiq/algos
    ${CMAKE_SYSROOT}/usr/include/rkaiq/common
    ${CMAKE_SYSROOT}/usr/include/rkaiq/xcore
    ${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser
)
target_link_libraries(libmediaserver_test ${MEDIASEVER_LIB} mediaserver stdc++ rkaiq)
install(TARGETS libmediaserver_test RUNTIME DESTINATION bin)
endif()
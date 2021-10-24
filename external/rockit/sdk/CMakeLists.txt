cmake_minimum_required( VERSION 2.8.8 )

set(ROCKIT_FILE_MPI_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include)
set(ROCKIT_DEPS_HEADERS ${CMAKE_CURRENT_LIST_DIR}/deps/include)
set(ROCKIT_FILE_LIBS ${CMAKE_CURRENT_LIST_DIR}/lib64/librockit.so)
set(ROCKIT_DEPEND_LIBS
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libavcodec.so.58
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libavformat.so.58
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libavutil.so.56
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libswresample.so.3
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libgraphic_lsf.so
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libmali.so.1
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/libmali.so.0
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/librockchip_mpp.so.1
    ${CMAKE_CURRENT_LIST_DIR}/deps/lib/librga.so.2
)

install(FILES ${ROCKIT_FILE_LIBS} DESTINATION "lib")
install(FILES ${ROCKIT_DEPEND_LIBS} DESTINATION "lib")
install(DIRECTORY ${ROCKIT_DEPS_HEADERS}/ DESTINATION "include")
install(DIRECTORY ${ROCKIT_FILE_MPI_HEADERS}/ DESTINATION "include/rockit")

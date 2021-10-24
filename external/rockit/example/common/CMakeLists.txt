cmake_minimum_required( VERSION 2.8.8 )
add_definitions(-fno-rtti)

add_compile_options(-std=c++11)
add_definitions(-std=c++11 -Wno-attributes -Wno-deprecated-declarations -DANDROID_STL=c++_shared)

set(RK_MPI_TEST_COMMON_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/argparse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mpi_test_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loadbmp.cpp
    PARENT_SCOPE
)


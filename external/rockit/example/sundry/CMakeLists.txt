cmake_minimum_required( VERSION 2.8.8 )
add_definitions(-fno-rtti)

add_compile_options(-std=c++11)
add_definitions(-std=c++11 -Wno-attributes -Wno-deprecated-declarations -DANDROID_STL=c++_shared)

set(RK_MPI_TEST_CLK_SRC
    test_sys_clk.cpp
)

set(RK_MPI_TEST_AVIO_SRC
    test_sys_avio.cpp
)

#--------------------------
# rk_mpi_clk_test
#--------------------------
add_executable(rk_mpi_clk_test ${RK_MPI_TEST_CLK_SRC} ${RK_MPI_TEST_COMMON_SRC})
target_link_libraries(rk_mpi_clk_test ${RK_DEP_COMMON_LIBS})
install(TARGETS rk_mpi_clk_test RUNTIME DESTINATION "bin/rockit")

#--------------------------
# rk_mpi_avio_test
#--------------------------
add_executable(rk_mpi_avio_test ${RK_MPI_TEST_AVIO_SRC} ${RK_MPI_TEST_COMMON_SRC})
target_link_libraries(rk_mpi_avio_test ${RK_DEP_COMMON_LIBS})
install(TARGETS rk_mpi_avio_test RUNTIME DESTINATION "bin/rockit")


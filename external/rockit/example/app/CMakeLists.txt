cmake_minimum_required( VERSION 2.8.8 )
add_definitions(-fno-rtti)

add_compile_options(-std=c++11)
add_definitions(-std=c++11 -Wno-attributes -Wno-deprecated-declarations -DANDROID_STL=c++_shared)

set(RK_MPI_TEST_APLAY_SRC
    rk_mpi_aplay_test.cpp
)

set(RK_MPI_TEST_VPLAY_SRC
    rk_mpi_vplay_test.cpp
)

set(RK_MPI_TEST_ACAPTURE_SRC
    rk_mpi_acapture_test.cpp
)

set(RK_MPI_TEST_CAM2HDMI_SRC
    rk_mpi_cam2hdmi.cpp
)

#--------------------------
# rk_mpi_aplay_test
#--------------------------
add_executable(rk_mpi_aplay_test ${RK_MPI_TEST_APLAY_SRC} ${RK_MPI_TEST_COMMON_SRC})
target_link_libraries(rk_mpi_aplay_test ${RK_DEP_COMMON_LIBS})
install(TARGETS rk_mpi_aplay_test RUNTIME DESTINATION "bin/rockit")

#--------------------------
# rk_mpi_vplay_test
#--------------------------
option(ENABLE_VPLAY  "enable vplay" ON)
if (${ENABLE_VPLAY})
    add_executable(rk_mpi_vplay_test ${RK_MPI_TEST_VPLAY_SRC} ${RK_MPI_TEST_COMMON_SRC})
    target_link_libraries(rk_mpi_vplay_test ${RK_DEP_COMMON_LIBS})
    install(TARGETS rk_mpi_vplay_test RUNTIME DESTINATION "bin/rockit")
endif()

#--------------------------
# rk_mpi_acapture_test
#--------------------------
add_executable(rk_mpi_acapture_test ${RK_MPI_TEST_ACAPTURE_SRC} ${RK_MPI_TEST_COMMON_SRC})
target_link_libraries(rk_mpi_acapture_test ${RK_DEP_COMMON_LIBS})
install(TARGETS rk_mpi_acapture_test RUNTIME DESTINATION "bin/rockit")

#--------------------------
# rk_mpi_cam2hdmi test
#--------------------------
add_executable(rk_mpi_cam2hdmi_test ${RK_MPI_TEST_CAM2HDMI_SRC} ${RK_MPI_TEST_COMMON_SRC})
target_link_libraries(rk_mpi_cam2hdmi_test ${RK_DEP_COMMON_LIBS})
install(TARGETS rk_mpi_cam2hdmi_test RUNTIME DESTINATION "bin/rockit")

cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

set(SRC_FILES
    DeviceIOTest.cpp
    bt_test.cpp
    bt_test_1s2.cpp
    rk_ble_app.c
    rk_wifi_test.c
)

if(BLUEZ)
add_definitions(-DBLUEZ_USE)
elseif(BSA)
add_definitions(-DBSA_USE)
endif()

add_definitions("-fpermissive")

add_executable(deviceio_test ${SRC_FILES})
target_include_directories(deviceio_test PUBLIC
        "${deviceio_test_SOURCE_DIR}/DeviceIO/include" )
target_link_libraries(deviceio_test pthread DeviceIo asound)

install(TARGETS deviceio_test DESTINATION bin)

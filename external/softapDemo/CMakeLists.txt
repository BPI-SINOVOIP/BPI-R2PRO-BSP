cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

PROJECT (softapDemo)

AUX_SOURCE_DIRECTORY(src SOFT_AP_SOURCE)

#set(CMAKE_C_COMPILER arm-rockchip-linux-gnueabihf-gcc)
#set(CMAKE_CXX_COMPILER arm-rockchip-linux-gnueabihf-g++)

#set(CMAKE_C_FLAGS "-static ${CMAKE_C_FLAGS}")

ADD_EXECUTABLE(softapDemo ${SOFT_AP_SOURCE})
install(TARGETS softapDemo DESTINATION bin)

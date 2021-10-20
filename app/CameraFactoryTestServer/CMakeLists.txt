cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)

project(CameraFactoryTestServer)
include(GNUInstallDirs)

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)

set(CMAKE_CXX_STANDARD 11)
#add_definitions(-fno-rtti)
#add_definitions(-fexceptions -finstrument-functions -funwind-tables -g -rdynamic -O0)
add_definitions(-O2)
add_definitions("-fpermissive -g")

include_directories(./)

option(NDEBUG OFF)
if(NDEBUG)
  add_definitions("-DNDEBUG")
endif()

set(CAMERA_FACTORY_TEST_SRC
      CameraFactoryTestServer.cpp
    )

add_executable(CameraFactoryTestServer ${CAMERA_FACTORY_TEST_SRC})
target_link_libraries(CameraFactoryTestServer pthread crypto event_core event_extra event_pthreads PkgConfig::GLIB)
install(TARGETS CameraFactoryTestServer RUNTIME DESTINATION "bin")


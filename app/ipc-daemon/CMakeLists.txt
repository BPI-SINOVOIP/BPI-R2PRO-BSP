cmake_minimum_required(VERSION 3.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(ipc-daemon LANGUAGES CXX C)

include(clang-format)
include(cppcheck)

include(FindPkgConfig)
pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

include_directories(
	${PROJECT_SOURCE_DIR}/./)

find_package(minilogger REQUIRED)

aux_source_directory(./ SRCS)
add_executable(${PROJECT_NAME}
	${SRCS})

target_link_libraries(${PROJECT_NAME}
	PkgConfig::GLIB
	PkgConfig::DBUS
	PkgConfig::JSON-C
	gdbus
	MiniLogger::MiniLogger
	pthread
	unwind
)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin)
install(FILES dbusconfig/system.conf
	DESTINATION share/dbus-1/system.d)

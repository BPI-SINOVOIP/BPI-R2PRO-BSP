#1.cmake version
cmake_minimum_required(VERSION 3.2)

#2.project name
project(HDAEC)

# ----------------------------------------------------------------------------
#  install headers
# ----------------------------------------------------------------------------
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
	DESTINATION "include/HDAEC"
	FILES_MATCHING PATTERN "*.h"
)

# ----------------------------------------------------------------------------
#  install libs
# ----------------------------------------------------------------------------
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/libRK_HDAEC_V6_01_04.so
	DESTINATION "lib"
)

add_subdirectory(test)

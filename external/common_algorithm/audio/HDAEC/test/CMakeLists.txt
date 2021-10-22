cmake_minimum_required(VERSION 3.2)

project(test_hdaec_file)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_executable(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/test_hdaec_file.c)

target_link_libraries(${PROJECT_NAME}
	RK_HDAEC_V6_01_04
	m)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin)

#add_executable(test_hdaec_alsa
#	${CMAKE_CURRENT_SOURCE_DIR}/alsa_reader.c
#	${CMAKE_CURRENT_SOURCE_DIR}/test_hdaec_alsa.c)

#target_link_libraries(test_hdaec_alsa
#	RK_HDAEC_V6_01_04
#	m)
#install(TARGETS test_hdaec_alsa
#	RUNTIME DESTINATION bin)


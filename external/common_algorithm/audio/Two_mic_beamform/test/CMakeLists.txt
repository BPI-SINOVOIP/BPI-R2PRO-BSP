cmake_minimum_required(VERSION 3.2)

project(test_beamform)

include_directories(
	${PROJECT_SOURCE_DIR}/../include)

link_directories(
	${PROJECT_SOURCE_DIR}/../lib)

add_executable(${PROJECT_NAME}
	${PROJECT_SOURCE_DIR}/alsa_reader.c
	${PROJECT_SOURCE_DIR}/alsa_test.cpp)

target_link_libraries(${PROJECT_NAME}
	two_mic_beamform
	asound
	m)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin)

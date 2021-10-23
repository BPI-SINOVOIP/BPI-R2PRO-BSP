cmake_minimum_required(VERSION 3.8)

project(RK_PCBA_ADB)

message(${PROJECT_SOURCE_DIR})
message(${PROJECT_BINARY_DIR})

set(INSTALL_TARGET_DIR /usr/bin/)

set(SRC_LIST 
    FactoryTestMain.cpp
    FactoryTest-wlan.cpp
    FactoryTest-usbhost.cpp
    FactoryTest-sdcard.cpp
    FactoryTest-lan.cpp
    FactoryTest-ircut.cpp
    FactoryTest-key.cpp
    FactoryTest-audioplay.cpp
    FactoryTest-mic.cpp
    # NewTest.cpp
)
foreach(obj_source ${SRC_LIST})
	STRING(REGEX REPLACE "(.*)(\\..*)$" "\\1" obj_target ${obj_source})
	add_executable(${obj_target} ${obj_source})
	install(TARGETS ${obj_target} DESTINATION ${INSTALL_TARGET_DIR})
endforeach(obj_source)

set(SCRIPT_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/FactoryTest-sdcard.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/FactoryTest-usbhost.sh
)
foreach(obj_source ${SCRIPT_LIST})
	install(
	    PROGRAMS ${obj_source}
	    DESTINATION ${INSTALL_TARGET_DIR}
	)
endforeach(obj_source)


MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "==CMAKE_CURRENT_SOURCE_DIR=")
MESSAGE(${CMAKE_STRIP}  "==CMAKE_STRIP")
MESSAGE(${CMAKE_C_COMPILER} "==CMAKE_C_COMPILER")

project(RK_PCBA)

message(${PROJECT_SOURCE_DIR})
message(${PROJECT_BINARY_DIR})

set(INSTALL_TARGET_DIR /usr/bin/)

set(SRC_LIST 
    echo_audio_play_test.c
	#echo_audio_record_test.c
    #echo_audio_test.c
    #echo_bt_test.c
    #echo_cpu_test.c
    #echo_ddr_test.c
    #echo_emmc_test.c
    #echo_led_test.c
    #echo_rotary_test.c
    #echo_rtc_test.c
    echo_wlan_test.c
    Wlan_test.c
    Bt_test.c
    echo_laohua_test.c
    #echo_sdcard_test.c
    #echo_usbhost_test.c
    #echo_auto_test.c
    #echo_ir_test.c
    Write_BTmac.c
    Read_BTmac.c
    Write_WiFimac.c
    Read_WiFimac.c
    Write_SN.c
    Read_SN.c
    fa.c
    led_show.c
    Version.c
    Battery_test.c
    Read_Size.c
    #Ringmic_test.c
    Debug_Ringmic.c
    SIM_test.c
    Tube_test.c
    #Button_test.c
)

foreach(obj_source ${SRC_LIST})
	STRING( REGEX REPLACE "(.*)(\\..*)$" "\\1" obj_target ${obj_source} )
add_executable( ${obj_target} ${obj_source} )
install(TARGETS ${obj_target} DESTINATION ${INSTALL_TARGET_DIR})
endforeach(obj_source)

set(SRC_LIST Button_test.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( Button_test ${SRC_LIST} )

set(SRC_LIST echo_pcbatest_server.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( echo_pcbatest_server ${SRC_LIST} )

set(SRC_LIST Ringmic_test.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( Ringmic_test ${SRC_LIST} )

set(SRC_LIST echo_audio_record_test.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( echo_audio_record_test ${SRC_LIST} )

set(SRC_LIST mic_test_Linux/record_test.c mic_test_Linux/vibrate_test.c)
add_library(mictest ${SRC_LIST})
set_property(TARGET mictest PROPERTY POSITION_INDEPENDENT_CODE ON)

set(SRC_LIST echo_ringmic_test.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( echo_ringmic_test ${SRC_LIST} )
target_link_libraries(echo_ringmic_test mictest)

set(SRC_LIST echo_ringmic_pdm_test.c)
add_executable( echo_ringmic_pdm_test ${SRC_LIST} )
target_link_libraries(echo_ringmic_pdm_test mictest)

set(SRC_LIST echo_key_test.c cJSON/cJSON.c)
include_directories(cJSON)
link_libraries(m)
add_executable( echo_key_test ${SRC_LIST} )

MESSAGE(${CMAKE_CURRENT_SOURCE_DIR} "==CMAKE_CURRENT_SOURCE_DIR=")
MESSAGE(${CMAKE_STRIP}  "== CMAKE_STRIP")
MESSAGE(${CMAKE_C_COMPILER} "==CMAKE_C_COMPILER")

install(TARGETS Button_test Ringmic_test echo_pcbatest_server echo_audio_record_test echo_key_test echo_ringmic_test echo_ringmic_pdm_test mictest DESTINATION ${INSTALL_TARGET_DIR})

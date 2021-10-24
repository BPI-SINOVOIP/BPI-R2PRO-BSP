project (rockx)
cmake_minimum_required(VERSION 3.8)

include(sdk/rockx-${COMPILE_PLATFORM}/RockXConfig.cmake)

install(DIRECTORY ${RockX_INCLUDE_DIRS}/ DESTINATION "include/rockx")
install(FILES ${RockX_LIBS} DESTINATION "lib")

if(${ENABLE_ROCKX_TB})
    install(FILES ${RockX_DATA} DESTINATION  "/../oem/usr/lib")
else()
    install(FILES ${RockX_DATA} DESTINATION "lib")
endif()

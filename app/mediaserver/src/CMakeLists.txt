add_definitions(-DDBUS_HAS_RECURSIVE_MUTEX)
add_definitions(-fPIC)
add_compile_options(-std=c++11 -Wno-psabi)

find_package(Threads REQUIRED)
find_package(EasyMedia REQUIRED)

option(ENABLE_DBUS  "enbale dbus" OFF)
option(ENABLE_MINILOGGER  "enbale minilogger" OFF)
option(ENABLE_OSD_SERVER  "enbale osd server" OFF)
option(ENABLE_SHM_SERVER  "enbale shm server" OFF)
option(ENABLE_EXIV2_LIB  "enbale exiv2 library" OFF)
option(USE_ROCKFACE  "enbale rockface" OFF)
option(USE_ROCKX  "enbale rockx" OFF)
option(ENABLE_SCHEDULES_SERVER  "enbale schedules" OFF)
option(ENABLE_LINK_SUPPORT  "Build link api" OFF)
option(ENABLE_LINK_SUPPORT_LINKKIT  "Build ALiBaBa linkkit api" OFF)
option(ENABLE_LINK_SUPPORT_TUYA  "Build Tuya api" OFF)
option(ENABLE_MEDIASERVER_BIN  "enbale mediaserver add_executable" ON)
option(ENABLE_ZBAR  "enbale zbar scan QR code" OFF)
option(ENABLE_CY43438  "enbale cy43438" OFF)

if (${ENABLE_DBUS})
set(CMAKE_DBUSXX_XML2CPP_TOOLS ${BR2_SDK_PATH}/bin/dbusxx-xml2cpp)
add_custom_command(OUTPUT dbus_meida_control_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_media_control.xml
                   --proxy=dbus_media_control_proxy.h
                   --adaptor=dbus_media_control_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/control
                   COMMENT "build dbus_meida_control xml2cpp head file")
add_custom_target(dbus_meida_control_xml2cppTask ALL DEPENDS dbus_meida_control_xml2cpp)
add_custom_command(OUTPUT dbus_dbserver_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_dbserver.xml
                   --proxy=dbus_dbserver_proxy.h
                   --adaptor=dbus_dbserver_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database
                   COMMENT "build dbus_dbserver xml2cpp head file")
add_custom_target(dbus_dbserver_xml2cppTask ALL DEPENDS dbus_dbserver_xml2cpp)
add_custom_command(OUTPUT dbus_storage_manager_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_storage_manager.xml
                   --proxy=dbus_storage_manager_proxy.h
                   --adaptor=dbus_storage_manager_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/storage_manager
                   COMMENT "build dbus_storage_manager xml2cpp head file")
add_custom_target(dbus_storage_manager_xml2cppTask ALL DEPENDS dbus_storage_manager_xml2cpp)
add_custom_command(OUTPUT dbus_ispserver_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_ispserver.xml
                   --proxy=dbus_ispserver_proxy.h
                   --adaptor=dbus_ispserver_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/ispserver
                   COMMENT "build dbus_ispserver xml2cpp head file")
add_custom_target(dbus_ispserver_xml2cppTask ALL DEPENDS dbus_ispserver_xml2cpp)
find_package(Dbus REQUIRED)
find_package(DbusC++ REQUIRED)
find_package(GLIB2 REQUIRED)
add_definitions(-DENABLE_DBUS)
add_definitions(-DDBUS_HAS_RECURSIVE_MUTEX)
set(MEDIASEVER_LIB ${MEDIASEVER_LIB} dbus-1 glib_2::glib_2 dbus_cpp::dbus_cpp IPCProtocol)
set(MEDIASEVER_INC
    ${MEDIASEVER_INC}
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/control/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/storage_manager/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/ispserver/)
aux_source_directory(dbus MEDIASEVER_SRC)
aux_source_directory(dbus/control MEDIASEVER_SRC)
aux_source_directory(dbus/database/ MEDIASEVER_SRC)
aux_source_directory(dbus/storage_manager/ MEDIASEVER_SRC)
aux_source_directory(dbus/ispserver/ MEDIASEVER_SRC)
if (${ENABLE_SCHEDULES_SERVER})
    add_definitions(-DENABLE_SCHEDULES_SERVER)
    set(MEDIASEVER_INC ${MEDIASEVER_INC} ${CMAKE_CURRENT_SOURCE_DIR}/utils/schedules/)
    aux_source_directory(utils/schedules/ MEDIASEVER_SRC)
endif() # ENABLE_SCHEDULES_SERVER
endif() # ENABLE_DBUS

if (${ENABLE_MINILOGGER})
    find_package(MiniLogger REQUIRED)
    add_definitions(-DENABLE_MINILOGGER)
    set(MEDIASEVER_LIB ${MEDIASEVER_LIB} MiniLogger::MiniLogger)
endif()

if (${ENABLE_OSD_SERVER})
    find_package(FreeType2 REQUIRED)
    add_definitions(-DENABLE_OSD_SERVER)
    set(MEDIASEVER_LIB ${MEDIASEVER_LIB} Freetype_2::Freetype_2)
    set(MEDIASEVER_INC
        ${MEDIASEVER_INC}
        ${FREEFTYPE_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/osd/
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database/)
    aux_source_directory(utils/osd/ MEDIASEVER_SRC)
endif()

if (${ENABLE_SHM_SERVER})
    add_definitions(-DENABLE_SHM_SERVER)
    set(MEDIASEVER_INC
        ${MEDIASEVER_INC}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/private/shmc/)
    aux_source_directory(utils/private/shmc/ MEDIASEVER_SRC)
    find_package(Protobuf REQUIRED)
    if(PROTOBUF_FOUND)
        message(STATUS "protobuf library found")
        set(MEDIASEVER_LIB ${MEDIASEVER_LIB} protobuf)
        include_directories(${PROTOBUF_INCLUDE_DIRS})
        include_directories(${CMAKE_CURRENT_BINARY_DIR})
        protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/utils/private/shmc/nn_data.proto)
        set(MEDIASEVER_INC ${MEDIASEVER_INC} ${PROTO_HDRS})
        set(MEDIASEVER_SRC ${MEDIASEVER_SRC} ${PROTO_SRCS} ${PROTO_HDRS})
    endif()
endif()

if (${ENABLE_EXIV2_LIB})
    find_package(Exiv2 REQUIRED)
    add_definitions(-DENABLE_EXIV2_LIB)
    set(MEDIASEVER_LIB ${MEDIASEVER_LIB} Exiv2::Exiv2)
    set(MEDIASEVER_INC ${MEDIASEVER_INC} utils/exiv2/)
    aux_source_directory(utils/exiv2/ MEDIASEVER_SRC)
endif()

if (${USE_ROCKFACE})
    add_definitions(-DUSE_ROCKFACE)
if (${USE_ROCKFACE_REG})
    add_definitions(-DUSE_ROCKFACE_REG)
endif()
endif()

if (${USE_ROCKX})
    add_definitions(-DUSE_ROCKX)
    set(MEDIASEVER_LIB ${MEDIASEVER_LIB} rockx)
endif()

if (${ENABLE_TB})
    add_definitions(-DTHUNDER_BOOT)
endif()

if (${ENABLE_LINK_SUPPORT})
    find_package(CJSON REQUIRED)
    add_definitions(-DLINK_API_ENABLE)
    set(MEDIASEVER_INC ${MEDIASEVER_INC} ${CMAKE_CURRENT_SOURCE_DIR}/link/)
    set(MEDIASEVER_SRC ${MEDIASEVER_SRC} link/link_manager.cpp)
    if (${ENABLE_LINK_SUPPORT_LINKKIT})
        add_definitions(-DLINK_API_ENABLE_LINKKIT)
        set(LINKKIT_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/link/plugin/linkkit/)
        set(LINKKIT_C_LIB ${CMAKE_CURRENT_SOURCE_DIR}/link/plugin/linkkit/lib/linkkit-sdk-c)
        set(LINKKIT_VISUAL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/link/plugin/linkkit/lib/linkvisual)
        add_library(iot_hal STATIC IMPORTED)
        add_library(iot_sdk STATIC IMPORTED)
        add_library(iot_tls STATIC IMPORTED)
        add_library(link_visual_device STATIC IMPORTED)
        set_target_properties(iot_hal PROPERTIES IMPORTED_LOCATION ${LINKKIT_C_LIB}/libiot_hal.a)
        set_target_properties(iot_sdk PROPERTIES IMPORTED_LOCATION ${LINKKIT_C_LIB}/libiot_sdk.a)
        set_target_properties(iot_tls PROPERTIES IMPORTED_LOCATION ${LINKKIT_C_LIB}/libiot_tls.a)
        set_target_properties(link_visual_device PROPERTIES IMPORTED_LOCATION ${LINKKIT_VISUAL_LIB}/liblink_visual_device.a)
        set(MEDIASEVER_INC
            ${MEDIASEVER_INC}
            ${LINKKIT_PLUGIN_DIR}/
            ${LINKKIT_PLUGIN_DIR}/include/linkvisual/
            ${LINKKIT_PLUGIN_DIR}/include/linkkit-sdk-c/
            ${LINKKIT_PLUGIN_DIR}/include/linkkit-sdk-c/exports/
            ${LINKKIT_PLUGIN_DIR}/include/linkkit-sdk-c/imports/)
        set(MEDIASEVER_SRC ${MEDIASEVER_SRC} ${LINKKIT_PLUGIN_DIR}/linkkit_api.cpp)
        set(MEDIASEVER_LIB ${MEDIASEVER_LIB} link_visual_device iot_sdk iot_hal iot_tls cjson rt)
    endif()
    if (${ENABLE_LINK_SUPPORT_TUYA})
        add_definitions(-DLINK_API_ENABLE_TUYA)
        set(TUYA_PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/link/plugin/tuya/)
        set(TUYA_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/link/plugin/tuya/libs)
        add_library(mbedtls STATIC IMPORTED)
        add_library(tuya_ipc STATIC IMPORTED)
        set_target_properties(mbedtls PROPERTIES IMPORTED_LOCATION ${TUYA_LIBS}/libmbedtls.a)
        set_target_properties(tuya_ipc PROPERTIES IMPORTED_LOCATION ${TUYA_LIBS}/libtuya_ipc.a)
        set(MEDIASEVER_INC
            ${MEDIASEVER_INC}
            ${TUYA_PLUGIN_DIR}/
            ${TUYA_PLUGIN_DIR}/include)
        set(MEDIASEVER_SRC ${MEDIASEVER_SRC} ${TUYA_PLUGIN_DIR}/tuya_api.cpp)
        # generate hwl_wf.a for tuya_ipc.a
        set(LIBHWL_WF_SRC ${TUYA_PLUGIN_DIR}/hwl_wf.c)
        add_library(hwl_wf STATIC ${LIBHWL_WF_SRC})
        target_include_directories(hwl_wf PUBLIC ${MEDIASEVER_INC})
        set(MEDIASEVER_LIB ${MEDIASEVER_LIB} tuya_ipc mbedtls hwl_wf)
    endif()
endif()

if (${ENABLE_LINK_VENDOR})
    add_definitions(-DLINK_VENDOR)
    set(MEDIASEVER_LIB ${MEDIASEVER_LIB} vendor_storage)
endif()

if (${ENABLE_ZBAR})
    add_definitions(-DENABLE_ZBAR)
    aux_source_directory(utils/zbar/ MEDIASEVER_SRC)
    add_library(rkbar STATIC IMPORTED)
    set_target_properties(rkbar PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/utils/zbar/librkbar.a)
    set(MEDIASEVER_LIB ${MEDIASEVER_LIB} rkbar)
endif()

if (${ENABLE_CY43438})
    add_definitions(-DENABLE_CY43438)
    aux_source_directory(utils/CY_WL_API/  MEDIASEVER_SRC)
    aux_source_directory(utils/CY_WL_API/libwl  MEDIASEVER_SRC)
endif()

set(MEDIASEVER_INC
    ${MEDIASEVER_INC}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/server/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/thread/
    ${CMAKE_CURRENT_SOURCE_DIR}/flows/)

set(MEDIASEVER_LIB
    ${MEDIASEVER_LIB}
    EasyMedia::EasyMedia
    ${CMAKE_THREAD_LIBS_INIT})

aux_source_directory(utils MEDIASEVER_SRC)
aux_source_directory(utils/thread/ MEDIASEVER_SRC)
aux_source_directory(utils/server/ MEDIASEVER_SRC)
aux_source_directory(utils/logger/ MEDIASEVER_SRC)
aux_source_directory(flows MEDIASEVER_SRC)

if (DEFINED MEDIASERVER_CONF_PREFIX)
add_definitions(-DMEDIASERVER_CONF_PREFIX=${MEDIASERVER_CONF_PREFIX})
endif()

if (ENABLE_MEDIASERVER_BIN)
    add_definitions(-DENABLE_MEDIASERVER_BIN)
    set(MEDIASEVER_SRC ${MEDIASEVER_SRC} mediaserver.cpp)
    add_executable(mediaserver ${MEDIASEVER_SRC})
    target_include_directories(mediaserver PUBLIC ${MEDIASEVER_INC})
    target_link_libraries(mediaserver ${MEDIASEVER_LIB})
    if (${ENABLE_DBUS})
        add_dependencies(mediaserver dbus_meida_control_xml2cppTask)
        add_dependencies(mediaserver dbus_dbserver_xml2cppTask)
        add_dependencies(mediaserver dbus_storage_manager_xml2cppTask)
        add_dependencies(mediaserver dbus_ispserver_xml2cppTask)
        install(FILES dbus/rockchip.mediaserver.control.conf DESTINATION share/dbus-1/system.d)
    endif()
    install(TARGETS mediaserver RUNTIME DESTINATION bin)
    if (DEFINED MEDIASERVE_CONF)
        install(FILES conf/${COMPILE_PLATFORM}/${MEDIASERVE_CONF} DESTINATION share/mediaserver/)
    else()
        install(DIRECTORY conf/ DESTINATION share/mediaserver FILES_MATCHING PATTERN "*.conf")
    endif()
    if (${ENABLE_OSD_SERVER})
        if (${ENABLE_TB})
            install(DIRECTORY resource/ DESTINATION /../oem/usr/share/mediaserver FILES_MATCHING PATTERN "*.bmp")
            install(DIRECTORY resource/ DESTINATION /../oem/usr/share/mediaserver FILES_MATCHING PATTERN "*.ttc")
            install(DIRECTORY resource/ DESTINATION /../oem/usr/share/mediaserver FILES_MATCHING PATTERN "*.ttf")
        else()
            install(DIRECTORY resource/ DESTINATION share/mediaserver FILES_MATCHING PATTERN "*.bmp")
            install(DIRECTORY resource/ DESTINATION share/mediaserver FILES_MATCHING PATTERN "*.ttc")
            install(DIRECTORY resource/ DESTINATION share/mediaserver FILES_MATCHING PATTERN "*.ttf")
        endif()
    endif()
    if (${ENABLE_TB})
        if (${ENABLE_LINK_SUPPORT_LINKKIT})
            install(PROGRAMS link/plugin/mqtt-rrpc-demo DESTINATION bin)
        endif()
    endif()
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/mediaserver.h DESTINATION include/mediaserver)
else()
    aux_source_directory(export MEDIASEVER_SRC)
    set(MEDIASEVER_INC ${MEDIASEVER_INC} ${CMAKE_CURRENT_SOURCE_DIR}/export/ ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database/)
    add_library(mediaserver SHARED ${MEDIASEVER_SRC})
    target_include_directories(mediaserver PUBLIC ${MEDIASEVER_INC})
    target_link_libraries(mediaserver ${MEDIASEVER_LIB})
    install(TARGETS mediaserver LIBRARY DESTINATION lib)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/export/libmediaserver_api.h DESTINATION include/mediaserver)
    install(DIRECTORY conf/libmediaserver/ DESTINATION share/mediaserver/ FILES_MATCHING PATTERN "*.conf")
	add_subdirectory(export)
endif()

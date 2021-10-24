# vim: syntax=cmake
cmake_minimum_required(VERSION 2.8.11)
add_compile_options(-g -rdynamic -ldl -funwind-tables -fPIC -O0)

include_directories(.)
include_directories(./include)
include_directories(./common)
include_directories(./ui)
include_directories(./rknn/rknn_api)
include_directories(./rknn/ssd)
set(RKNN_DEMO_SRC
      rknn_camera.c
      common/rknn_msg.c
      common/v4l2camera.c
      common/yuv.c
      common/buffer.c
      common/device_name.c
      ui/ui_res.c
    )

add_definitions(-DNEED_RKNNAPI=${NEED_RKNNAPI})
if(NEED_RKNNAPI)
    include_directories(./rknn/ssd/ssd_3399pro)
    set(RKNNAPI_SRC rknn/ssd/ssd_3399pro/ssd.c
        rknn/ssd/ssd_3399pro/ssd_post.cpp)
else(NEED_RKNNAPI)
    include_directories(./rknn/ssd/ssd_1808)
    set(RKNNAPI_SRC rknn/ssd/ssd_1808/ssd.c
        rknn/ssd/ssd_1808/ssd_post.c)
endif(NEED_RKNNAPI)

if(DEFINED ENABLE_SSD)
  add_definitions(-DENABLE_SSD=${ENABLE_SSD})
  include_directories(./ui/ssd)
  set(RKNN_MODEL_SRC
      rknn/ssd/tracker/objects_tracker.cpp
      rknn/ssd/tracker/objects_update.cpp
      rknn/ssd/tracker/track_c_link_c++.cpp
      ui/ssd/ssd_ui.c
      ${RKNNAPI_SRC}
    )
endif(DEFINED ENABLE_SSD)

if(DEFINED ENABLE_JOINT)
  add_definitions(-DENABLE_JOINT=${ENABLE_JOINT})
  include_directories(./rknn/joint)
  include_directories(./ui/joint)
  set(RKNN_MODEL_SRC
    rknn/joint/joint.c
    rknn/joint/joint_post.c
    ui/joint/joint_ui.c
  )
endif(DEFINED ENABLE_JOINT)

if(DEFINED ENABLE_FRG)
  add_definitions(-DENABLE_FRG=${ENABLE_FRG})
  include_directories(./rknn/frg)
  include_directories(./ui/frg)
  set(RKNN_MODEL_SRC
    rknn/frg/frg.c
    ui/frg/frg_ui.c
  )
  link_libraries("${CMAKE_SOURCE_DIR}/rknn/frg/frgsdk_rk1808/so_file/librkfacerecg.so"
                 "${CMAKE_SOURCE_DIR}/rknn/frg/frgsdk_rk1808/so_file/libopencv_core.so"
                 "${CMAKE_SOURCE_DIR}/rknn/frg/frgsdk_rk1808/so_file/libopencv_imgproc.so")
endif()

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fsanitize-recover=address -static-libasan")

# import rknn
set(RKNPU_PATH ${CMAKE_SOURCE_DIR}/../../../../../external/rknpu)
include_directories(${RKNPU_PATH}/rknn/include)

if(DEFINED NEED_RKNNAPI)
	if(NEED_RKNNAPI)
		link_libraries("librknn_api.so")
	else(NEED_RKNNAPI)
		link_libraries("librknn_runtime.so")
	endif(NEED_RKNNAPI)
endif(DEFINED NEED_RKNNAPI)
link_libraries("libv4l2.so")
link_libraries("libjpeg.so")
link_libraries("libm.so")
link_libraries("libdrm.so")
link_libraries("libminigui_ths.so")
link_libraries("libpng12.so")
link_libraries("libpthread.so")
link_libraries("librga.so")
link_libraries("libfreetype.so")
link_libraries("librkuvc.so")

add_executable(rknn_demo ${RKNN_DEMO_SRC} ${RKNN_MODEL_SRC})

if (DEFINED ENABLE_FRG)
install(FILES rknn/frg/frgsdk_rk1808/so_file/librkfacerecg.so DESTINATION lib)
install(FILES rknn/frg/frgsdk_rk1808/so_file/libopencv_core.so
        DESTINATION lib
        RENAME libopencv_core.so.3.4)
install(FILES rknn/frg/frgsdk_rk1808/so_file/libopencv_imgproc.so
        DESTINATION lib
        RENAME libopencv_imgproc.so.3.4)
install(FILES rknn/frg/frgsdk_rk1808/box_priors.txt DESTINATION bin)
install(FILES rknn/frg/start_rknn_frg.sh DESTINATION bin)
endif()

if (DEFINED MODEL_RESOURCE_FILES)
separate_arguments(MODEL_RESOURCE_FILES)
install(FILES ${MODEL_RESOURCE_FILES} DESTINATION share/rknn_demo)
endif()

install(FILES ${MINIGUI_CFG} DESTINATION ../etc RENAME MiniGUI.cfg)
install(DIRECTORY minigui DESTINATION local/share)
install(DIRECTORY resource DESTINATION local/share/rknn_demo)

#for qt desktop
install(FILES ui/qt/terminator.desktop DESTINATION /usr/share/applications/)

install(TARGETS rknn_demo RUNTIME DESTINATION bin)

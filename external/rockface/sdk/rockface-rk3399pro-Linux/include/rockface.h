/****************************************************************************
*
*    Copyright (c) 2017 - 2019 by Rockchip Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rockchip Corporation. This is proprietary information owned by
*    Rockchip Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Rockchip Corporation.
*
*****************************************************************************/

#ifndef _ROCKFACE_H
#define _ROCKFACE_H

#include "rockface_type.h"
#include "rockface_image.h"
#include "rockface_detect.h"
#include "rockface_analyze.h"
#include "rockface_recog.h"
#include "rockface_liveness.h"

/**
 * @mainpage 首页
 *
 * @section 概述
 *
 * RockFace SDK提供一系列人脸识别分析相关的功能，充分利用RK180X/RK3399Pro平台的NPU来对算法模型进行加速。SDK提供人脸检测、人脸属性分析、人脸识别、人脸关键点标记等API函数，能够帮助开发者快速开发人脸AI应用。
 *
 * @section 如何使用
 *
 * @subsection SDK库导入
 *
 * 开发者可以在自己的CMakeLists.txt中引用库
 * 
 * ```
 * # Find RockFace Package
 * set(RockFace_DIR <path-to-rockface-sdk>/sdk/rockface-rk1808-Linux)
 * find_package(RockFace REQUIRED)
 *
 * # Include RockFace Header
 * include_directories(${RockFace_INCLUDE_DIRS})
 *
 * # Link RockFace Libraries
 * target_link_libraries(target_name ${RockFace_LIBS})
 * 
 * # Install RockFace
 * install(PROGRAMS ${RockFace_LIBS} DESTINATION lib)
 * install(PROGRAMS ${RockFace_DATA} DESTINATION lib)
 * ```
 *
 * @subsection 创建、销毁和初始化
 *
 * RockFace通过`rockface_create_handle`函数来创建一个句柄对象，示意代码如下所示：
 * 
 * ```
 *  rockface_ret_t ret;
 *  rockface_handle_t face_handle = rockface_create_handle();
 *  
 * ```
 *
 * 创建完之后可以通过调用`rockface_set_licence`函数设置licence文件（licence文件的获取请参见下面的授权说明一节），示意代码如下所示：
 * 
 * ```
 *  ret = rockface_set_licence(face_handle, licence_path);
 *  if (ret != ROCKFACE_RET_SUCCESS) {
 *      printf("Error: authorization error %d!", ret);
 *      return ret;
 *  }
 * ```
 * 
 * 接下来可以通过调用`rockface_set_data_path`来设置数据文件(sdk/rockface-data)在设备的路径，示意代码如下所示：
 * 
 * ```
 * rockface_set_data_path(face_handle, "/usr/share/rockface-data")
 * ```
 * 
 * 以上成功设置后，可以根据需要使用的模块来调用不同的初始化函数进行初始化，示意代码如下所示:
 * ```
 * ret = rockface_init_detector(face_handle);
 * if (ret != ROCKFACE_RET_SUCCESS) {
 *     printf("Error: init detector error %d!", ret);
 *     return ret;
 * }
 * ret = rockface_init_recognizer(face_handle);
 * if (ret != ROCKFACE_RET_SUCCESS) {
 *     printf("Error: init recognizer error %d!", ret);
 *     return ret;
 * }
 * ```
 * 
 * 最后如果不需要继续使用，可以调用`rockface_release_handle`函数进行释放，示意代码如下:
 * 
 * ```
 * rockface_release_handle(face_handle);
 * ```
 *
 * @subsection 授权说明
 * 
 * SDK需要获得授权后才能使用，客户需要向对应业务提出申请，获得授权使用的用户名和密码，然后通过sdk包中auth目录下的工具进行授权即可进行使用。授权流程如下所示：
 * 
 * @image html res/auth.png Figure 1 授权流程
 * 
 * 授权工具具体使用方法请参考其目录下的说明文档，得到授权文件之后，用户可以通过`rockface_set_data_path`函数设置授权文件路径。
 * 
 * @subsection API函数
 *
 * SDK提供的人脸API函数有:
 *
 * 函数                               | 初始化函数                              | 描述               | 定义位置            |
 * ----------------------------------|---------------------------------------|--------------------|--------------------|
 * @ref rockface_detect              | @ref rockface_init_detector           | 人脸检测             | face.h             |
 * @ref rockface_track               | @ref rockface_init_detector           | 人脸跟踪             | object_track.h     |
 * @ref rockface_align               | @ref rockface_init_detector           | 人脸对齐             | face.h             |
 * @ref rockface_landmark5           | @ref rockface_init_detector           | 人脸关键点标记(5点)   | face.h             |
 * @ref rockface_landmark            | @ref rockface_init_analyzer           | 人脸关键点标记(68点)  | face.h             |
 * @ref rockface_angle               | @ref rockface_init_analyzer           | 人脸角度             | face.h             |
 * @ref rockface_attribute           | @ref rockface_init_analyzer           | 人脸属性分析          | face.h            |
 * @ref rockface_feature_extract     | @ref rockface_init_recognizer         | 人脸特征提取          | face.h            |
 * @ref rockface_feature_compare     | @ref rockface_init_recognizer         | 人脸特征比对          | face.h            |
 * @ref rockface_liveness_detect     | @ref rockface_init_liveness_detector  | 活体检测             | face.h             |
 *
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 创建Handle
 * 
 * @return @ref rockface_handle_t 
 */
rockface_handle_t rockface_create_handle();

/**
 * 释放Handle
 * 
 * @param handle [in] 需要释放的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_release_handle(rockface_handle_t handle);

/**
 * 授权
 *
 * @param handle [in] 需要授权的handle
 * @param lic_path [in] 授权文件路径
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_set_licence(rockface_handle_t handle, const char* lic_path);

/**
 * 设置数据文件路径（如果所有的data文件与librockface.so放在相同目录下，可以不需要设置）
 * 
 * @param handle [in] 需要设置的Handle
 * @param data_path [in] 数据文件的路径
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_set_data_path(rockface_handle_t handle, const char* data_path);


#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_H
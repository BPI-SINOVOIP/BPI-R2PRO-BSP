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

#ifndef _ROCKFACE_DETECT_H
#define _ROCKFACE_DETECT_H

#include "rockface_type.h"
#include "rockface_image.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化人脸检测器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_detector(rockface_handle_t handle);

/**
 * 初始化人脸检测器，支持选择不同人脸检测模型
 *
 * @param handle [in] 需要初始化的Handle
 * @param version [in] 需要初始化的人脸检测模型版本
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_init_detector2(rockface_handle_t handle, int version);

/**
 * 初始化人形检测器
 * 
 * @param handle 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_person_detector(rockface_handle_t handle);

/**
 * 人脸检测
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_detector 函数初始化）
 * @param in_img [in] 输入图像
 * @param face_array [out] 人脸检测结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_detect(rockface_handle_t handle, rockface_image_t *in_img, rockface_det_array_t *face_array);

/**
 * 人形检测
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_person_detector 函数初始化）
 * @param in_img [in] 输入图像
 * @param face_array [out] 人形检测结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_person_detect(rockface_handle_t handle, rockface_image_t *in_img, rockface_det_person_array_t *person_array);

/**
 * 人脸跟踪，在连续视频帧时使用，会对当前场景的检测的结果分配跟踪ID，连续帧下对同一目标保持相同的跟踪ID
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_detector 函数初始化）
 * @param in_img [in] 输入图像
 * @param max_track_time [in] 最大跟踪时间（避免偶然漏检导致目标丢失）
 * @param in_track_objects [in] 人脸检测结果
 * @param out_track_objects [out] 已跟踪的人脸列表
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_track(rockface_handle_t handle, rockface_image_t *in_img, int max_track_time,
        rockface_det_array_t* in_track_face, rockface_det_array_t* out_track_face);

/**
 * 人脸跟踪，在连续视频帧时使用，会对当前场景的检测的结果分配跟踪ID，连续帧下对同一目标保持相同的跟踪ID
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_detector 函数初始化）
 * @param in_img [in] 输入图像
 * @param max_track_time [in] 最大跟踪时间（避免偶然漏检导致目标丢失）
 * @param in_track_objects [in] 人脸检测结果
 * @param out_track_objects [out] 已跟踪的人脸列表
 * @param enable_autotrack [in] 是否启用自动跟踪，如果启用自动跟踪，则将使用CPU进行跟踪，不需要传入原始人脸坐标参数
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_autotrack(rockface_handle_t handle, rockface_image_t *in_img, int max_track_time,
        rockface_det_array_t* in_track_face, rockface_det_array_t* out_track_face, bool enable_autotrack);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_DETECT_H
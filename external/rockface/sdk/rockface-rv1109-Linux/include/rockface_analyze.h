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

#ifndef _ROCKFACE_ANALYZE_H
#define _ROCKFACE_ANALYZE_H

#include "rockface_type.h"
#include "rockface_image.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 初始化人脸分析器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_analyzer(rockface_handle_t handle);

/**
 * 初始化人脸关键点检测
 * 
 * @param handle 需要初始化的Handle
 * @param landmark_count 人脸关键点数（支持5/68/98点）
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_landmark(rockface_handle_t handle, int landmark_count);

/**
 * 初始化人脸质量分析器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_quality(rockface_handle_t handle);

/**
 * 初始化口罩检测器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_mask_classifier(rockface_handle_t handle);

/**
 * 初始化口罩检测器(可以检测口罩人脸位置)
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_mask_detector(rockface_handle_t handle);

/**
 * 检测人脸关键点（68点）
 *
 * 68关键点如图1所示：
 * @image html res/face_landmark68.png Figure 1 人脸68关键点示意图
 *
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_landmark(handle, 68) 函数初始化）
 * @param in_img [in] 输入图像
 * @param in_box [in] 人脸区域
 * @param out_landmark [out] 人脸关键点结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_landmark(rockface_handle_t handle, rockface_image_t* in_img, rockface_rect_t *in_box,
        rockface_landmark_t *out_landmark);

/**
 * 检测人脸关键点（5点），结果可用于计算人脸角度/人脸对齐
 *
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_landmark(handle, 5) 函数初始化）
 * @param in_img [in] 输入图像
 * @param in_box [in] 人脸区域
 * @param out_landmark [out] 人脸关键点结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_landmark5(rockface_handle_t handle, rockface_image_t* in_img, rockface_rect_t *in_box,
        rockface_landmark_t *out_landmark);

/**
 * 检测人脸关键点（106点），结果可用于计算人脸角度/人脸对齐
 *
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_landmark(handle, 106) 函数初始化）
 * @param in_img [in] 输入图像
 * @param in_box [in] 人脸区域
 * @param in_landmark [in] 人脸关键点(5点)结果
 * @param out_landmark [out] 人脸关键点(106点)结果
 * @param out_angle [out] 人脸角度
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_landmark106(rockface_handle_t handle, rockface_image_t* in_img, rockface_rect_t *in_box, 
        rockface_landmark_t *in_landmark, rockface_landmark_t *out_landmark, rockface_angle_t *out_angle);

/**
 * 人脸误检过滤
 *
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_landmark(handle, 5) 函数初始化）
 * @param in_img [in] 输入图像
 * @param in_box [in] 人脸区域
 * @param is_false_face [out] 是否为人脸
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_face_filter(rockface_handle_t handle, rockface_image_t* in_img, rockface_rect_t *in_box,
        int *is_false_face);

/**
 * 获取人脸角度
 * 
 * @param in_landmark [in] 人脸关键点结果
 * @param out_angle [out] 人脸角度
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_angle(rockface_handle_t handle, rockface_landmark_t *in_landmark, rockface_angle_t *out_angle);

/**
 * 获取人脸属性（性别和年龄）
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_analyzer 函数初始化）
 * @param in_img [in] 对齐后的人脸图像
 * @param attr [out] 人脸属性
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_attribute(rockface_handle_t handle, rockface_image_t *in_img, rockface_attribute_t *attr);

/**
 * 人脸矫正对齐
 *
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_landmark(handle, 5) 函数初始化）
 * @param in_img [in] 输入图像（需要原始图像）
 * @param in_box [in] 人脸区域
 * @param in_landmark [in] 人脸关键点（5点）。如果为NULL，函数内部会自己获取
 * @param out_img [out] 对齐后的人脸图像（用完后需要调用 @ref rockface_image_release 释放）
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_align(rockface_handle_t handle, rockface_image_t *in_img, rockface_rect_t *in_box, rockface_landmark_t *in_landmark, rockface_image_t *out_img);

/**
 * 人脸矫正对齐2
 *
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_landmark(handle, 5) 函数初始化）
 * @param in_img [in] 输入图像（需要原始图像）
 * @param in_box [in] 人脸区域
 * @param in_landmark [in] 人脸关键点（5点）。如果为NULL，函数内部会自己获取
 * @param out_img [out] 对齐后的人脸图像（用完后需要调用 @ref rockface_image_release 释放）
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_align2(rockface_handle_t handle, rockface_image_t *in_img, rockface_rect_t *in_box, rockface_landmark_t *in_landmark, rockface_image_t *out_img);

/**
 * 人脸过曝检测
 * 
 * @param in_face_img [in] 输入对齐人脸图像
 * @param out_isOverexpose [out] 输出是否过曝，1为过曝，0为非过曝
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_overexpose_detect(rockface_image_t *in_face_img , int* out_isOverexpose );

/**
 * 人脸模糊检测
 * @param in_img [in] 输入图像（需要原始图像）
 * @param in_box [in] 人脸区域
 * @param blur [out] 输出为人脸模糊度，值越大越模糊，建议阈值为0.82
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_blur(rockface_image_t *in_img, rockface_rect_t *in_box, float *blur);

/**
 * 人脸亮度
 * 
 * @param in_face_img [in] 输入对齐人脸图像
 * @param bright_level [out] 输出为人脸亮度等级，范围0.0～255.0，小于60为欠曝，大于210为过曝
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_brightlevel(rockface_image_t *in_face_img, float *bright_level);

/**
 * 口罩检测
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_mask_classifier 函数初始化）
 * @param input_image [in] 原始图像
 * @param face_box [in] 人脸检测结果
 * @param out_score [out] 口罩分数
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_mask_classifier(rockface_handle_t handle, rockface_image_t* input_image, rockface_rect_t *face_box, float *out_score);

/**
 * 人脸口罩检测
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_mask_detector 函数初始化）
 * @param in_img [in] 原始图像
 * @param face_mask_array [out] 口罩检测结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_mask_detect(rockface_handle_t handle, rockface_image_t *in_img, rockface_mask_array_t *face_mask_array);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_ANALYZE_H
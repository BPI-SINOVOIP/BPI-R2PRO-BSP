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

#ifndef _ROCKFACE_LIVENESS_H
#define _ROCKFACE_LIVENESS_H

#include "rockface_type.h"
#include "rockface_image.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化活体检测器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_liveness_detector(rockface_handle_t handle);

/**
 * 活体检测
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_liveness_detector 函数初始化）
 * @param in_ir_img [in] 输入图像（需要特定红外摄像头，如需使用请与我们联系）
 * @param in_box [in] 人脸检测区域
 * @param out_liveness_result [out] 活体结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_liveness_detect(rockface_handle_t handle, rockface_image_t* in_ir_img, rockface_rect_t *in_box, rockface_liveness_t *out_liveness_result);


#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_LIVENESS_H
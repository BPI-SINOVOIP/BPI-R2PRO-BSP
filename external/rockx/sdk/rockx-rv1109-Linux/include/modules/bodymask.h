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

#ifndef _ROCKX_BODY_MASK_API_H
#define _ROCKX_BODY_MASK_API_H

#include "../rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BodyMask Array
 */
typedef struct rockx_bodymask_array_t {
	uint8_t *mask;        /// [out] output mask, size = width * height
	size_t width;     /// [in] mask width
	size_t height;    /// [in] mask height
	float threshold;  /// [in] mask threshold
} rockx_bodymask_array_t;

/**
 * Get Mask of Human Body (Multi Person)
 * @param handle [in] Handle of a created ROCKX_MODULE_BODY_MASK module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param bodymask_array [in] [out] Array of body mask
 * @param callback [in] Async callback function pointer
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_body_mask(rockx_handle_t handle, rockx_image_t *in_img, rockx_bodymask_array_t *masks_array,
		rockx_async_callback *callback);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKX_BODY_MASK_API_H

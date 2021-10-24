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

#ifndef _ROCKX_OBJECT_TRACK_H
#define _ROCKX_OBJECT_TRACK_H

#include <stddef.h>
#include "../rockx_type.h"
#include "object_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Track Object (Can use for Face, Head, CarPlate, Object, etc...)
 * @param handle [in] Handle of a created ROCKX_MODULE_OBJECT_TRACK module(created by @ref rockx_create)
 * @param width [in] Input image width
 * @param height [in] Input image height
 * @param max_track_time [in] Max track time (frame count for tracked object disappear)
 * @param in_track_objects [in] Object array need to track
 * @param out_track_objects [out] tracked object array
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_object_track(rockx_handle_t handle, int width, int height, int max_track_time,
        rockx_object_array_t* in_track_objects, rockx_object_array_t* out_track_objects);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKX_FACE_H
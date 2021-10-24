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

#ifndef _ROCKFACE_DATA_H
#define _ROCKFACE_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rockface.h"

#define MAX_SIZE_NAME 64

typedef struct {
    rockface_feature_t feature;
    char name[MAX_SIZE_NAME];
} face_data;

typedef struct {
    rockface_feature_float_t feature;
    char name[MAX_SIZE_NAME];
} mask_face_data;

rockface_ret_t run_face_detection(rockface_handle_t face_handle, rockface_image_t *in_image, rockface_det_t *out_face);

rockface_ret_t run_get_face_feature(rockface_handle_t face_handle, rockface_image_t *in_image, rockface_det_t *face_det, rockface_feature_t *out_feature);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
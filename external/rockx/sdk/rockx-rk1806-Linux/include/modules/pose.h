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

#ifndef _ROCKX_POSE_H
#define _ROCKX_POSE_H

#include "../rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pose of Body KeyPoints Name
 */
static const char *ROCKX_POSE_BODY_KEYPOINTS_NAME[] = {
	"Nose", "Neck",
	"R-Sho", "R-Elb", "R-Wr",
	"L-Sho", "L-Elb", "L-Wr",
	"R-Hip", "R-Knee", "R-Ank",
	"L-Hip", "L-Knee", "L-Ank",
	"R-Eye", "L-Eye", "R-Ear", "L-Ear"
};

/**
 * @brief KeyPoints for One Body or Finger
 */
typedef struct rockx_keypoints_t {
    int count;                  ///< key points count
    rockx_point_t points[32];   ///< key points
    float score[32];			///< Key points score
} rockx_keypoints_t;

/**
 * @brief KeyPoints Array
 */
typedef struct rockx_keypoints_array_t {
	int count;							///< Array size
	rockx_keypoints_t keypoints[32];	///< Array of rockx_keypoints_t
} rockx_keypoints_array_t;

/**
 * Get KeyPoint of Human Body (Multi Person)
 * @param handle [in] Handle of a created ROCKX_MODULE_POSE_BODY module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param keypoints_array [out] Array of pose key points
 * @param callback [in] Async callback function pointer
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_pose_body(rockx_handle_t handle, rockx_image_t *in_img, rockx_keypoints_array_t *keypoints_array,
		rockx_async_callback callback);

/**
 * Get KeyPoint of A Human Hand
 *
 * Finger 21 KeyPoint As Show in Figure 1.
 * @image html res/finger_landmark21.jpg Figure 1 Finger 21 KeyPoints Detection
 *
 * @param handle [in] Handle of a created ROCKX_MODULE_POSE_FINGER_3 or ROCKX_MODULE_POSE_FINGER_21 module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param keypoints [out] KeyPoints
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_pose_finger(rockx_handle_t handle, rockx_image_t *in_img, rockx_keypoints_t *keypoints);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKX_POSE_H
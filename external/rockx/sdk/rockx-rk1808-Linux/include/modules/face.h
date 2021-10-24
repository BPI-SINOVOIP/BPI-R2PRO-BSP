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

#ifndef _ROCKX_FACE_H
#define _ROCKX_FACE_H

#include <stddef.h>
#include "../rockx_type.h"
#include "object_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Face Landmark Result (get from @ref rockx_face_landmark)
 */
typedef struct rockx_face_landmark_t {
    int image_width;                ///< Input image width
    int image_height;               ///< Input image height
    rockx_rect_t face_box;          ///< Face region
    int landmarks_count;            ///< Landmark points count
    rockx_point_t landmarks[128];   ///< Landmark points
    float score;                    ///< Score (Only 5 points has score)
} rockx_face_landmark_t;

/**
 * @brief Face Angle Result (get from @ref rockx_face_pose)
 */
typedef struct rockx_face_angle_t {
    float pitch;        ///< Pitch angle ( < 0: Up, > 0: Down )
    float yaw;          ///< Yaw angle ( < 0: Left, > 0: Right )
    float roll;         ///< Roll angle ( < 0: Right, > 0: Left )
} rockx_face_angle_t;

/**
 * @brief Face Feature Result (get from @ref rockx_face_recognize)
 */
typedef struct rockx_face_feature_t {
    int version;            ///< Face recognition algorithm version
    int len;                ///< Feature length
    float feature[512];     ///< Feature data
} rockx_face_feature_t;

/**
 * @brief Face Attritute Result (get from @ref rockx_face_attribute)
 */
typedef struct rockx_face_attribute_t {
    int gender;         ///< Gender
    int age;            ///< Age
} rockx_face_attribute_t;

/**
 * @brief Face Antispoof Result
 */
typedef struct rockx_face_liveness_t {
    float fake_score;   ///< score of fake
    float real_score;   ///< score of real
} rockx_face_liveness_t;

/**
 * Face Detection
 * @param handle [in] Handle of a created ROCKX_MODULE_FACE_DETECTION module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param face_array [out] Face detection result array
 * @param callback [in] Async callback function pointer
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_detect(rockx_handle_t handle, rockx_image_t *in_img, rockx_object_array_t *face_array,
        rockx_async_callback callback);

/**
 * Face Landmark KeyPoint (Current can get 68 or 5 key points)
 *
 * Face Landmark 68 KeyPoint As Show In Figure 1.
 * @image html res/face_landmark68.png Figure 1 Face Landmark 68 KeyPoint
 *
 * @param handle [in] Handle of a created ROCKX_MODULE_FACE_LANDMARK_68 or ROCKX_MODULE_FACE_LANDMARK_5 module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param in_box [in] Face region(get from rockx_face_detect)
 * @param out_landmark [out] Face landmark
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_landmark(rockx_handle_t handle, rockx_image_t* in_img, rockx_rect_t *in_box,
        rockx_face_landmark_t *out_landmark);

/**
 * Face Pose
 * @param in_landmark [in] Face landmark result (get from @ref rockx_face_landmark)
 * @param out_angle [out] Angle of Face
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_pose(rockx_face_landmark_t *in_landmark, rockx_face_angle_t *out_angle);

/**
 * Face Correction Alignment
 * @param handle [in] Handle of a created ROCKX_MODULE_FACE_LANDMARK_5 module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param in_box [in] Detection Result
 * @param in_landmark [in] Face landmark result (if set NULL will call @ref rockx_face_landmark to get a landmark result)
 * @param out_img [out] Aligned face image
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_align(rockx_handle_t handle, rockx_image_t *in_img, rockx_rect_t *in_box,
                             rockx_face_landmark_t *in_landmark, rockx_image_t *out_img);

/**
 * Get Face Feature
 * @param handle [in] Handle of a created ROCKX_MODULE_FACE_RECOGNIZE module(created by @ref rockx_create)
 * @param in_img [in] Input image
 * @param out_feature [out] Face Feature
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_recognize(rockx_handle_t handle, rockx_image_t *in_img, rockx_face_feature_t *out_feature);

/**
 * Compare Two Face Feature Similarity (Use Euclidean distance). Developers can adjust thresholds(0.1~1.3) based on different face data sets and application scenarios.
 * @param in_feature1 [in] Face 1 Feature
 * @param in_feature2 [in] Face 2 Feature
 * @param out_similarity [out] Similarity (more smaller more similar)
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_feature_similarity(rockx_face_feature_t *in_feature1, rockx_face_feature_t *in_feature2, float *out_similarity);

/**
 * Face Attribute (Gender and Age)
 * @param handle [in] Handle of a created ROCKX_MODULE_FACE_ANALYZE module(created by @ref rockx_create)
 * @param in_img [in] Input Image
 * @param attr [out] Face attribute
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_attribute(rockx_handle_t handle, rockx_image_t *in_img, rockx_face_attribute_t *attr);

/**
 * Face Liveness Detection
 * @param handle [in] Handle of a created ROCKX_MODULE_FACE_LIVENESS module(created by @ref rockx_create)
 * @param in_ir_img [in] Input IR image (need specified ir camera)
 * @param box [in] Face Detect Area
 * @param out_liveness_result [out] Liveness result
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_face_liveness_detect(rockx_handle_t handle, rockx_image_t* in_ir_img, rockx_rect_t *in_box, rockx_face_liveness_t *out_liveness_result);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKX_FACE_H
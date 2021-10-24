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

#ifndef _ROCKX_H
#define _ROCKX_H

#include <stddef.h>

#include "rockx_type.h"
#include "modules/face.h"
#include "modules/pose.h"
#include "modules/object_detection.h"
#include "modules/carplate.h"
#include "modules/object_track.h"
#include "utils/rockx_tensor_util.h"
#include "utils/rockx_image_util.h"
#include "utils/rockx_config_util.h"

/**
 * @mainpage Index Page
 *
 * @section Introduction
 *
 * Rock-X SDK is a set of AI components based on the RK3399Pro/RK180X platform. Developers can quickly build AI
 * applications through the API interface provided by SDK.
 * The functions provided by the SDK are as follows:
 *
 * Classes         |              Functions                          |
 * ----------------|-------------------------------------------------|
 * Object Detection| Head Detection / 91 Classes Object Detection    |
 * Face            | Face Landmark / Face Analyze / Face Recognition |
 * CarPlate        | CarPlate Detectin / Carplate Recognition        |
 * Human Keypoint  | Body Keypoint / Finger Keypoint                 |
 *
 * @section How-to-use
 *
 * @subsection Import-Library
 *
 * Developers can refer to the following example to import the library of RockX sdk in CMakeLists.txt
 * ```
 * # Find RockX Package
 * set(RockX_DIR <path-to-rockx-sdk>/sdk/rockx-rk3399pro-Android)
 * find_package(RockX REQUIRED)
 *
 * # Include RockX Header
 * include_directories(${RockX_INCLUDE_DIRS})
 *
 * # Link RockX Libraries
 * target_link_libraries(target_name ${RockX_LIBS})
 * ```
 *
 * @subsection Create-and-Destroy-Module
 *
 * Rock-X modules are initialized by the rockx_create function, and different modules are initialized by passing in
 * different rockx_module_t enumeration values. The sample code is as follows:
 * ```
 *  rockx_ret_t ret;
 *  rockx_handle_t face_det_handle;
 *  ret = rockx_create(&face_det_handle,
 *                      ROCKX_MODULE_FACE_DETECTION,
 *                      nullptr, 0);
 *  if (ret != ROCKX_RET_SUCCESS) {
 *      printf("init rockx module error %d\n", ret);
 *  }
 * ```
 *
 * If you don't need to use this module, you can release the handle by calling the rockx_destroy function. The sample
 * code is as follows:
 * ```
 * rockx_destroy(face_det_handle);
 * ```
 *
 * @subsection Modules-Interface
 *
 * All module interfaces provided by the Rock-X SDK are as follows:
 *
 * Functions                         | The Value of rockx_module_t Used on creating                           | Description                 | Define At          |
 * ----------------------------------|------------------------------------------------------------------------|-----------------------------|--------------------|
 * @ref rockx_object_detect          | @ref ROCKX_MODULE_OBJECT_DETECTION                                     | 91 Classes Object Detection | object_detection.h |
 * @ref rockx_head_detect            | @ref ROCKX_MODULE_HEAD_DETECTION                                       | Head Detection              | object_detection.h |
 * @ref rockx_face_detect            | @ref ROCKX_MODULE_FACE_DETECTION                                       | Face Detection              | face.h             |
 * @ref rockx_face_landmark          | @ref ROCKX_MODULE_FACE_LANDMARK_68 / @ref ROCKX_MODULE_FACE_LANDMARK_5 | Face KeyPoint Landmark      | face.h             |
 * @ref rockx_face_pose              |                                                                        | Face Angle                  | face.h             |
 * @ref rockx_face_align             | @ref ROCKX_MODULE_FACE_LANDMARK_5                                      | Face Align                  | face.h             |
 * @ref rockx_face_recognize         | @ref ROCKX_MODULE_FACE_RECOGNIZE                                       | Face Recognition            | face.h             |
 * @ref rockx_face_feature_similarity|                                                                        | Compare Two Face Feature    | face.h             |
 * @ref rockx_face_attribute         | @ref ROCKX_MODULE_FACE_ANALYZE                                         | Face Attribute Analyze      | face.h             |
 * @ref rockx_carplate_detect        | @ref ROCKX_MODULE_CARPLATE_DETECTION                                   | CarPlate Detection          | carplate.h         |
 * @ref rockx_carplate_recognize     | @ref ROCKX_MODULE_CARPLATE_RECOG                                       | CarPlate Recognition        | carplate.h         |
 * @ref rockx_carplate_align         | @ref ROCKX_MODULE_CARPLATE_ALIGN                                       | Carplate Align              | carplate.h         |
 * @ref rockx_pose_body              | @ref ROCKX_MODULE_POSE_BODY                                            | Human Body Keypoint         | pose.h             |
 * @ref rockx_pose_finger            | @ref ROCKX_MODULE_POSE_FINGER_3 / @ref ROCKX_MODULE_POSE_FINGER_21     | Human Finger Keypoint       | pose.h             |
 * @ref rockx_object_track           |                                                                        | Track Detection Object      | object_track.h     |
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RockX Modules Define
 *
 * @details This is using for create a RockX module(See @ref rockx_create)
 */
typedef enum {
    ROCKX_MODULE_FACE_DETECTION      = 1,    ///< Face Detection
    ROCKX_MODULE_FACE_LANDMARK_68    = 2,    ///< Face Landmark (68 KeyPoints)
    ROCKX_MODULE_FACE_RECOGNIZE      = 3,    ///< Face Recognition
    ROCKX_MODULE_FACE_ANALYZE        = 4,    ///< Face Attribute(Gender and Age) Analyze
    ROCKX_MODULE_OBJECT_DETECTION    = 5,    ///< Object Detection(91 Classes)
    ROCKX_MODULE_POSE_BODY           = 6,    ///< Body Pose(14 KeyPoints)
    ROCKX_MODULE_POSE_FINGER_21      = 7,    ///< Finger Landmark(21 KeyPoint)
    ROCKX_MODULE_FACE_LANDMARK_5     = 8,    ///< Face Landmark(5 KeyPoints)
    ROCKX_MODULE_HEAD_DETECTION      = 9,    ///< Head Detection
    ROCKX_MODULE_CARPLATE_DETECTION  = 10,   ///< Car Plate Detection
    ROCKX_MODULE_CARPLATE_ALIGN      = 11,   ///< Car Plate Correct Alignment
    ROCKX_MODULE_CARPLATE_RECOG      = 12,   ///< Car Plate Recognition
    ROCKX_MODULE_OBJECT_TRACK        = 13,   ///< Object Track
    ROCKX_MODULE_POSE_FINGER_3       = 14,   ///< Finger Landmark(3 KeyPoint)
    ROCKX_MODULE_FACE_LIVENESS       = 15    ///< 2D Liveness
} rockx_module_t;

/// Create A Rockx Module
/// \param handle [out] The handle for created module
/// \param m [in] Enum of RockX module(@ref rockx_module_t)
/// \param config [in] Config for Rockx Module(@ref rockx_config_t)
/// \param config_size [in] Size of config
/// \return @ref rockx_ret_t
rockx_ret_t rockx_create(rockx_handle_t *handle, rockx_module_t m, void *config, size_t config_size);

/// Destroy A Rockx Module
/// \param handle [in] The handle of a created module (created by @ref rockx_create)
/// \return @ref rockx_ret_t
rockx_ret_t rockx_destroy(rockx_handle_t handle);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKX_H

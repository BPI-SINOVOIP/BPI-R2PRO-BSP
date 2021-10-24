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

#ifndef _ROCKFACE_RECOG_H
#define _ROCKFACE_RECOG_H

#include "rockface_type.h"
#include "rockface_image.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 初始化人脸识别器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_recognizer(rockface_handle_t handle);

/**
 * 初始化戴口罩人脸识别器
 * 
 * @param handle [in] 需要初始化的Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_init_mask_recognizer(rockface_handle_t handle);

/**
 * 对已对齐的人脸图像提取人脸特征
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_recognizer 函数初始化）
 * @param in_img [in] 对齐后的人脸图像
 * @param out_feature [out] 人脸特征
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_feature_extract(rockface_handle_t handle, rockface_image_t *in_img, rockface_feature_t *out_feature);

/**
 * 对佩戴口罩的人脸图像提取人脸特征
 * 
 * @param handle [in] 已初始化的Handle（调用 @ref rockface_init_mask_recognizer 函数初始化）
 * @param in_img [in] 原始图像（为了保证精度最好不要缩放）
 * @param face_box [in] 人脸检测框
 * @param mode [in] 特征提取模式（0：注册模式，输入图像是注册底库，无口罩；1：识别模式，输入图像有佩戴口罩）
 * @param out_feature [out] 人脸特征
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_mask_feature_extract(rockface_handle_t handle, rockface_image_t *in_img, rockface_rect_t *face_box, int mode, rockface_feature_float_t *out_feature);

/**
 * 比较两个人脸特征的相似度 (使用欧式距离)， 开发者可以设置阈值(建议阈值范围0.7~1.3，可根据不同人脸库和应用场景调整)，如果小于阈值可以判断为同一人
 * 
 * @param in_feature1 [in] 人脸特征1
 * @param in_feature2 [in] 人脸特征2
 * @param out_similarity [out] 相似度
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_feature_compare(rockface_feature_t *in_feature1, rockface_feature_t *in_feature2, float *out_similarity);

/**
 * 初始化人脸库
 * 
 * @param handle [in] Handle，如果该Handle之前有初始化过人脸库，需要先调用（ @ref rockface_face_library_release ）释放后再初始化
 * @param face_array [in] 人脸数组，数组元素可以是用户自定义的结构体，但必须包含一个rockface_feature_t类型的结构体成员（注意，初始化后该数组不能被释放）
 * @param num [in] 人脸数组数量
 * @param face_data_size [in] 用户定义结构体的大小
 * @param feature_offset [in] 用户定义结构体中 @ref rockface_feature_t 成员的偏移位置
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_face_library_init(rockface_handle_t handle, void *face_array, int num, size_t face_data_size, size_t feature_offset);

/**
 * 初始化人脸库
 * 
 * @param handle [in] Handle，如果该Handle之前有初始化过人脸库，需要先调用（ @ref rockface_face_library_release ）释放后再初始化
 * @param face_array [in] 人脸数组，数组元素可以是用户自定义的结构体，但必须包含一个rockface_feature_t类型的结构体成员（注意，初始化后该数组不能被释放）
 * @param num [in] 人脸数组数量
 * @param face_data_size [in] 用户定义结构体的大小
 * @param feature_offset [in] 用户定义结构体中 @ref rockface_feature_t 成员的偏移位置
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_face_library_init2(rockface_handle_t handle, rockface_recog_version version, void *face_array, int num, size_t face_data_size, size_t feature_offset);

/**
 * 释放人脸库
 * 
 * @param handle [in] Handle
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_face_library_release(rockface_handle_t handle);

/**
 * 搜索人脸，得到小于阈值情况下，和输入特征最相似的目标
 * 
 * @param handle [in] Handle
 * @param in_feature [in] 要搜索的人脸特征
 * @param threshold [in] 阈值(建议阈值范围0.7~1.3，可根据不同人脸库和应用场景调整，阈值越小越严格)
 * @param out_result [out] 搜索结果
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_feature_search(rockface_handle_t handle, rockface_feature_t* in_feature, float threshold, rockface_search_result_t *out_result);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_RECOG_H
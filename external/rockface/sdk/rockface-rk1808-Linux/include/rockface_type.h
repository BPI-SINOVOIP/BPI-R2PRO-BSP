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

#ifndef _ROCKFACE_TYPE_H
#define _ROCKFACE_TYPE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief rockface handle
 */
typedef void *rockface_handle_t;

/**
 * @brief 函数返回值
 */
typedef enum {
    ROCKFACE_RET_SUCCESS = 0,      ///< 成功
    ROCKFACE_RET_FAIL = -1,        ///< 失败
    ROCKFACE_RET_PARAM_ERR = -2,   ///< 输入参数错误
    ROCKFACE_RET_AUTH_FAIL = -99,  ///< 授权失败
    ROCKFACE_RET_NOT_SUPPORT = -98 ///< 不支持设备
} rockface_ret_t;

/**
 * @brief 图像像素格式
 */
typedef enum {
    ROCKFACE_PIXEL_FORMAT_GRAY8 = 0,       ///< Gray8
    ROCKFACE_PIXEL_FORMAT_RGB888,          ///< RGB888
    ROCKFACE_PIXEL_FORMAT_BGR888,          ///< BGR888
    ROCKFACE_PIXEL_FORMAT_RGBA8888,        ///< RGBA8888
    ROCKFACE_PIXEL_FORMAT_BGRA8888,        ///< BGRA8888
    ROCKFACE_PIXEL_FORMAT_YUV420P_YU12,    ///< YUV420P YU12: YYYYYYYYUUVV
    ROCKFACE_PIXEL_FORMAT_YUV420P_YV12,    ///< YUV420P YV12: YYYYYYYYVVUU
    ROCKFACE_PIXEL_FORMAT_YUV420SP_NV12,   ///< YUV420SP NV12: YYYYYYYYUVUV
    ROCKFACE_PIXEL_FORMAT_YUV420SP_NV21,   ///< YUV420SP NV21: YYYYYYYYVUVU
    ROCKFACE_PIXEL_FORMAT_YUV422P_YU16,    ///< YUV422P YU16: YYYYYYYYUUUUVVVV
    ROCKFACE_PIXEL_FORMAT_YUV422P_YV16,    ///< YUV422P YV16: YYYYYYYYVVVVUUUU
    ROCKFACE_PIXEL_FORMAT_YUV422SP_NV16,   ///< YUV422SP NV16: YYYYYYYYUVUVUVUV
    ROCKFACE_PIXEL_FORMAT_YUV422SP_NV61,   ///< YUV422SP NV61: YYYYYYYYVUVUVUVU
    ROCKFACE_PIXEL_FORMAT_GRAY16,          ///< Gray16
    ROCKFACE_PIXEL_FORMAT_MAX,
} rockface_pixel_format;

/**
 * @brief Image Rotate Mode
 */
typedef enum {
    ROCKFACE_IMAGE_TRANSFORM_NONE              = 0x00,  ///< Do not transform
    ROCKFACE_IMAGE_TRANSFORM_FLIP_H            = 0x01,  ///< Flip image horizontally
    ROCKFACE_IMAGE_TRANSFORM_FLIP_V            = 0x02,  ///< Flip image vertically
    ROCKFACE_IMAGE_TRANSFORM_ROTATE_90         = 0x04,  ///< Rotate image 90 degree
    ROCKFACE_IMAGE_TRANSFORM_ROTATE_180        = 0x03,  ///< Rotate image 180 degree
    ROCKFACE_IMAGE_TRANSFORM_ROTATE_270        = 0x07,  ///< Rotate image 270 defree
} rockface_image_transform_mode;

/**
 * @brief 人脸图像曝光
 */
typedef enum {
    ROCKFACE_IMAGE_ILLUMINATION_NORM = 0,
    ROCKFACE_IMAGE_ILLUMINATION_UNEVEN = 1,
    ROCKFACE_IMAGE_OVER_EXPOSURE = 2,
    ROCKFACE_IMAGE_UNDER_EXPOSURE = 3,
} rockface_image_illumination_t;

/**
 * @brief 人脸图像对比度
 */
typedef enum {
    ROCKFACE_IMAGE_CONTRAST_NORM = 0,
    ROCKFACE_IMAGE_CONTRAST_WEAK = 1,
    ROCKFACE_IMAGE_CONTRAST_STRONG = 2,
} rockface_image_contrast_t;

/**
 * @brief 表示二维图像上点的坐标
 */
typedef struct rockface_point_t {
    int x;      ///< X坐标
    int y;      ///< Y坐标
} rockface_point_t;

/**
 * @brief 颜色
 */
typedef struct rockface_color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rockface_color_t;

/**
 * @brief 表示二维图像上人脸的矩形区域
 */
typedef struct rockface_rect_t {
    int left;       ///< 矩形最左边的坐标
    int top;        ///< 矩形最上边的坐标
    int right;      ///< 矩形最右边的坐标
    int bottom;     ///< 矩形最下边的坐标
} rockface_rect_t;

/**
 * @brief 表示一个二维图像
 */
typedef struct rockface_image_t {
    uint8_t *data;                          ///< 图像数据
    uint32_t size;                          ///< 图像数据大小
    uint8_t is_prealloc_buf;                ///< 图像数据是否已预分配内存
    rockface_pixel_format pixel_format;     ///< 图像像素格式 (@ref rockface_pixel_format)
    uint32_t width;                         ///< 图像宽
    uint32_t height;                        ///< 图像高
    float original_ratio;                   ///< Image original ratio of width & height, default is 1
} rockface_image_t;

/**
 * @brief 表示人脸关键点标记结果
 */
typedef struct rockface_landmark_t {
    int image_width;                    ///< 输入图像宽度
    int image_height;                   ///< 输入图像高度
    rockface_rect_t face_box;           ///< 人脸区域
    int landmarks_count;                ///< 人脸关键点数量
    rockface_point_t landmarks[128];    ///< 人脸关键点
    float score;                        ///< 分数
} rockface_landmark_t;

/**
 * @brief 表示人脸角度结果
 */
typedef struct rockface_angle_t {
    float pitch;        ///< 俯仰角 ( < 0: 上, > 0: 下 )
    float yaw;          ///< 侧摆角 ( < 0: 左, > 0: 右 )
    float roll;         ///< 侧摇角 ( < 0: 左, > 0: 右 )
} rockface_angle_t;

/**
 * @brief 人脸识别算法版本
 * 
 */
typedef enum {
    ROCKFACE_RECOG_NORMAL = 0,  ///< 正常人脸识别
    ROCKFACE_RECOG_MASK = 11    ///< 戴口罩人脸识别
} rockface_recog_version;

/**
 * @brief 表示人脸特征结果
 */
typedef struct rockface_feature_t {
    int version;            ///< 人脸识别算法版本
    int len;                ///< 特征长度
    uint8_t feature[512];   ///< 特征数据
} rockface_feature_t;

/**
 * @brief 表示人脸特征结果
 */
typedef struct rockface_feature_float_t {
    int version;            ///< 人脸识别算法版本
    int len;                ///< 特征长度
    float feature[512];     ///< 特征数据
} rockface_feature_float_t;

/**
 * @brief 表示人脸搜索结果
 */
typedef struct rockface_search_result_t {
    float similarity;           ///< 相似度
    void *face_data;            ///< 自定义的人脸结构体指针
} rockface_search_result_t;

/**
 * @brief 表示人脸属性结果
 */
typedef struct rockface_attribute_t {
    int gender;         ///< 性别
    int age;            ///< 年龄
} rockface_attribute_t;

/**
 * @brief 表示活体检测结果
 */
typedef struct rockface_liveness_t {
    float fake_score;   ///< 假人概率
    float real_score;   ///< 真人概率
} rockface_liveness_t;

/**
 * @brief 表示一个检测的人脸
 */
typedef struct rockface_det_t {
    int id;                     ///< 跟踪ID
    int reserve;                ///< 保留字段
    rockface_rect_t box;        ///< 人脸区域
    float score;                ///< 人脸分数
} rockface_det_t;

/**
 * @brief 表示检测到人脸的数组
 */
typedef struct rockface_det_array_t {
    int count;                     ///< 数组大小 (0 <= count < 128)
    rockface_det_t face[128];      ///< 人脸数组
} rockface_det_array_t;

/**
 * @brief 表示检测到人形的数组
 */
typedef struct rockface_det_person_array_t {
    int count;                       ///< 数组大小 (0 <= count < 128)
    rockface_det_t person[128];      ///< 人形数组
} rockface_det_person_array_t;

/**
 * @brief 人脸口罩检测结果
 */
typedef struct rockface_mask_t {
    rockface_rect_t face_box;       ///< 人脸区域
    float score;                    ///< 人脸分数
    int has_mask;                   ///< 是否有佩戴口罩
} rockface_mask_t;

/**
 * @brief 人脸口罩检测结果数组
 */
typedef struct rockface_mask_array_t {
    int count;                                ///< 检测数量
    rockface_mask_t face_masks[128];          ///< 人脸口罩
} rockface_mask_array_t;

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_TYPE_H
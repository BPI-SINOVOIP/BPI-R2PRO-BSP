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

#ifndef _ROCKX_TYPE_H
#define _ROCKX_TYPE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle of a created RockX module
 */
typedef void *rockx_handle_t;

/**
 * @brief Pointer of Async Callback Function
 */
typedef void (*rockx_async_callback)(void *result, size_t result_size);

/**
 * @brief Return Value of RockX functions
 */
typedef enum {
    ROCKX_RET_SUCCESS = 0,      ///< Success
    ROCKX_RET_FAIL = -1,        ///< Fail
    ROCKX_RET_PARAM_ERR = -2    ///< Input Param error
} rockx_ret_t;

/**
 * @brief Image Pixel Format
 */
typedef enum {
    ROCKX_PIXEL_FORMAT_GRAY8 = 0,       ///< Gray8
    ROCKX_PIXEL_FORMAT_RGB888,          ///< RGB888
    ROCKX_PIXEL_FORMAT_BGR888,          ///< BGR888
    ROCKX_PIXEL_FORMAT_RGBA8888,        ///< RGBA8888
    ROCKX_PIXEL_FORMAT_BGRA8888,        ///< BGRA8888
    ROCKX_PIXEL_FORMAT_YUV420P_YU12,    ///< YUV420P YU12: YYYYYYYYUUVV
    ROCKX_PIXEL_FORMAT_YUV420P_YV12,    ///< YUV420P YV12: YYYYYYYYVVUU
    ROCKX_PIXEL_FORMAT_YUV420SP_NV12,   ///< YUV420SP NV12: YYYYYYYYUVUV
    ROCKX_PIXEL_FORMAT_YUV420SP_NV21,   ///< YUV420SP NV21: YYYYYYYYVUVU
    ROCKX_PIXEL_FORMAT_YUV422P_YU16,    ///< YUV422P YU16: YYYYYYYYUUUUVVVV
    ROCKX_PIXEL_FORMAT_YUV422P_YV16,    ///< YUV422P YV16: YYYYYYYYVVVVUUUU
    ROCKX_PIXEL_FORMAT_YUV422SP_NV16,   ///< YUV422SP NV16: YYYYYYYYUVUVUVUV
    ROCKX_PIXEL_FORMAT_YUV422SP_NV61,   ///< YUV422SP NV61: YYYYYYYYVUVUVUVU
    ROCKX_PIXEL_FORMAT_MAX,
} rockx_pixel_format;

/**
 * @brief Data Type
 */
typedef enum {
    ROCKX_DTYPE_FLOAT32 = 0,        ///< Data type is float32
    ROCKX_DTYPE_FLOAT16,            ///< Data type is float16
    ROCKX_DTYPE_INT8,               ///< Data type is int8
    ROCKX_DTYPE_UINT8,              ///< Data type is uint8
    ROCKX_DTYPE_INT16,              ///< Data type is int16
    ROCKX_DTYPE_TYPE_MAX
} rockx_data_type;

/**
 * @brief Tensor Format
 */
typedef enum {
    ROCKX_TENSOR_FORMAT_NCHW = 0,    ///< data format is NCHW (RRRGGGBBB)
    ROCKX_TENSOR_FORMAT_NHWC,        ///< data format is NHWC (RGBRGBRGB)
    ROCKX_TENSOR_FORMAT_MAX
} rockx_tensor_format;

/**
 * @brief Tensor
 */
typedef struct rockx_tensor_t {
    rockx_data_type dtype;          ///< Data type (@ref rockx_data_type)
    rockx_tensor_format fmt;        ///< Tensor Format(@ref rockx_tensor_format)
    uint8_t n_dims;                 ///< Number of tensor dimension (0 < n_dims <= 4)
    uint32_t dims[4];               ///< Tensor dimension
    void* data;                     ///< Tensor data
} rockx_tensor_t;

/**
 * @brief Point
 */
typedef struct rockx_point_t {
    int x;      ///< X Coordinate
    int y;      ///< Y Coordinate
} rockx_point_t;

/**
 * @brief Point (Float)
 */
typedef struct rockx_pointf_t {
    float x;    ///< X Coordinate
    float y;    ///< Y Coordinate
} rockx_pointf_t;

/**
 * @brief Rectangle of Object Region
 */
typedef struct rockx_rect_t {
    int left;       ///< Most left coordinate
    int top;        ///< Most top coordinate
    int right;      ///< Most right coordinate
    int bottom;     ///< Most bottom coordinate
} rockx_rect_t;

/**
 * @brief Rectangle of Object Region
 */
typedef struct rockx_rectf_t {
    float left;     ///< Most left coordinate
    float top;      ///< Most top coordinate
    float right;    ///< Most right coordinate
    float bottom;   ///< Most bottom coordinate
} rockx_rectf_t;

/**
 * @brief Image
 */
typedef struct rockx_image_t {
    uint8_t *data;                      ///< Image data
    uint32_t size;                      ///< Image data size
    uint8_t is_prealloc_buf;            ///< Image data buffer prealloc
    rockx_pixel_format pixel_format;    ///< Image pixel format (@ref rockx_pixel_format)
    uint32_t width;                     ///< Image Width
    uint32_t height;                    ///< Image Height
} rockx_image_t;

/**
 * @brief Color
 */
typedef struct rockx_color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rockx_color_t;


#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKX_TYPE_H
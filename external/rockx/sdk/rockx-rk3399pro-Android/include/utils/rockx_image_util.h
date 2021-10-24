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

#ifndef _ROCKX_IMAGE_UTIL_H
#define _ROCKX_IMAGE_UTIL_H

#include "../rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Image Rotate Mode
 */
typedef enum {
    ROCKX_IMAGE_TRANSFORM_NONE              = 0x00,  ///< Do not transform
    ROCKX_IMAGE_TRANSFORM_FLIP_H            = 0x01,  ///< Flip image horizontally
    ROCKX_IMAGE_TRANSFORM_FLIP_V            = 0x02,  ///< Flip image vertically
    ROCKX_IMAGE_TRANSFORM_ROTATE_90         = 0x04,  ///< Rotate image 90 degree
    ROCKX_IMAGE_TRANSFORM_ROTATE_180        = 0x03,  ///< Rotate image 180 degree
    ROCKX_IMAGE_TRANSFORM_ROTATE_270        = 0x07,  ///< Rotate image 270 defree
} rockx_image_transform_mode;

/**
 * Get Channels of a @ref rockx_image_t
 * @param [in] img Image
 * @return Channels
 */
int rockx_image_get_channels(rockx_image_t *img);

/**
 * Get Bugger Size of a @ref rockx_image_t
 * @param [in] img Image
 * @return Buffer size
 */
int rockx_image_get_size(rockx_image_t *img);

/**
 * Convert Image Size and Color
 * @param src [in] Source image
 * @param dst [out] Destination image
 * @param mode [in] transform mode
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_convert(rockx_image_t *src, rockx_image_t *dst, rockx_image_transform_mode mode);

/**
 * Convert Image Size(Keep Ration) and Color
 * @param src [in] Source image
 * @param dst [out] Destination image
 * @param dst_width [in] Destination Width
 * @param dst_heigh [in] Destination Height
 * @param dst_pixel_format [in] Destination image format
 * @param pad_color [in] padding color
 * @param scale_w [out] width resize scale
 * @param scale_h [out] height resize scale
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_convert_keep_ration(rockx_image_t *src, rockx_image_t *dst, int dst_width, int dst_heigh,
                                            rockx_pixel_format dst_pixel_format, int pad_color, float *scale_w,
                                            float *scale_h);

/**
 * @brief Image Clarity
 * 
 * @param in_image [in] image
 * @param clarity [out] clarity
 * @return rockx_ret_t 
 */
rockx_ret_t rockx_image_clarity(rockx_image_t *in_image, float *clarity);

/**
 * @brief Image ROI
 * 
 * @param img [in] imag
 * @param roi [in] roi
 * @param roi_img [out] roi image
 * @return rockx_ret_t 
 */
rockx_ret_t rockx_image_roi(rockx_image_t *img, rockx_rect_t *roi, rockx_image_t *roi_img);

/**
 * Read Image From File(Need Release If Not Use @ref release_rockx_image)
 * @param img_path [in] Image file path
 * @param image [out] Read Image
 * @param flag [in] 0: gray; 1: color
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_read(const char *img_path, rockx_image_t *image, int flag);

/**
 * Write Image To File
 * @param path [in] File path to write
 * @param img [in] Image to write
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_write(const char *path, rockx_image_t *img);

/**
 * Write Image Raw Data To File
 * @param path [in] File path to write
 * @param img [in] Image to write
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_write_raw(const char *path, rockx_image_t *img);

/**
 * Clone Image
 * @param img [in] Source image
 * @return cloned image
 */
rockx_image_t *rockx_image_clone(rockx_image_t *img);

/**
 * Release Image
 * @param img [in] Image to release
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_release(rockx_image_t *img);

/**
 * Draw Circle on Image
 * @param img [in] Image to Draw
 * @param point [in] Circle Center Point
 * @param radius [in] Circle Radius
 * @param color [in] Color
 * @param thickness [in] ThickNess
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_draw_circle(rockx_image_t *img, rockx_point_t point, int radius,
        rockx_color_t color, int thickness);

/**
 * Draw Line on Image
 * @param img [in] Image to Draw
 * @param pt1 [in] Start Point
 * @param pt2 [in] End Point
 * @param color [in] Color
 * @param thickness [in] ThickNess
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_draw_line(rockx_image_t *img, rockx_point_t pt1, rockx_point_t pt2,
        rockx_color_t color, int thickness);

/**
 * Draw Rect on Image
 * @param img [in] Image to Draw
 * @param pt1 [in] RectAngle Left Top Point
 * @param pt2 [in] RectAngle Right Bottom Point
 * @param color [in] Color
 * @param thickness [in] ThickNess
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_draw_rect(rockx_image_t *img, rockx_point_t pt1, rockx_point_t pt2,
        rockx_color_t color, int thickness);

/**
 * Draw Text on Image
 * @param img [in] Image to Draw
 * @param text [in] Text
 * @param pt [in] Origin Point
 * @param color [in] Color
 * @param thickness [in] ThickNess
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_draw_text(rockx_image_t *img, const char *text, rockx_point_t pt,
        rockx_color_t color, int thickness);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_ROCKX_IMAGE_UTIL_H

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
 * @brief Image Illumination Result
 */
typedef enum {
    ROCKX_IMAGE_ILLUMINATION_NORM = 0,
    ROCKX_IMAGE_ILLUMINATION_UNEVEN = 1,
    ROCKX_IMAGE_OVER_EXPOSURE = 2,
    ROCKX_IMAGE_UNDER_EXPOSURE = 3,
} rockx_image_illumination_t;

/**
 * @brief Image Contrast Result
 */
typedef enum {
    ROCKX_IMAGE_CONTRAST_NORM = 0,
    ROCKX_IMAGE_CONTRAST_WEAK = 1,
    ROCKX_IMAGE_CONTRAST_STRONG = 2,
} rockx_image_contrast_t;

/**
 * @brief Image Resize Method
 */
typedef enum {
    ROCKX_IMAGE_RESIZE_INTER_LINEAR = 0, //default resize method
    ROCKX_IMAGE_RESIZE_INTER_AREA = 1,
    ROCKX_IMAGE_RESIZE_INTER_CUBIC = 2,
    ROCKX_IMAGE_RESIZE_INTER_NEAREST = 3,
}rockx_image_resize_method_t;

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
 * @brief Convert Image With Crop
 * 
 * @param src_img [in] Source image
 * @param roi [in] Source image ROI
 * @param dst_img [out] Destination image
 * @param trans_mode [in] transform mode
 * @return @ref rockx_ret_t 
 */
rockx_ret_t rockx_image_convert_with_crop(rockx_image_t *src_img, rockx_rect_t *roi, rockx_image_t *dst_img, rockx_image_transform_mode trans_mode);

/**
 * Convert Image Size(Keep Ration) and Color
 * @param src [in] Source image (must )
 * @param dst [out] Destination image(need set width/height/pixel_fmt)
 * @param on_center [in] Is Image on target image center and border padding.
 * @param pad_color [in] padding color
 * @param scale_w [out] width resize scale
 * @param scale_h [out] height resize scale
 * @param left_offset [out] Image left padding offset
 * @param top_offset [out] Image top padding offset
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_convert_keep_ration(rockx_image_t *src, rockx_image_t *dst, int on_center, int pad_color, 
        float *scale_w, float *scale_h, float *left_offset, float *top_offset);

/**
 * Convert Image Size(Keep Ratio) and Color using different interpolation method
 * @param src [in] Source image (must )
 * @param dst [out] Destination image(need set width/height/pixel_fmt)
 * @param on_center [in] Is Image on target image center and border padding.
 * @param resize_method [in] Image interpolation method.
 * @param pad_color [in] padding color
 * @param scale_w [out] width resize scale
 * @param scale_h [out] height resize scale
 * @param left_offset [out] Image left padding offset
 * @param top_offset [out] Image top padding offset
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_convert_keep_ratio_by_method(rockx_image_t *src, rockx_image_t *dst, int on_center, int pad_color, 
        float *scale_w, float *scale_h, float *left_offset, float *top_offset,rockx_image_resize_method_t resize_method);

/**
 * @brief Image Clarity
 * 
 * @param in_image [in] image
 * @param clarity [out] clarity
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_clarity(rockx_image_t *in_image, float *clarity);

/**
 * @brief Image ROI
 * 
 * @param img [in] imag
 * @param roi [in] roi
 * @param roi_img [out] roi image
 * @return @ref rockx_ret_t 
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
 * Write Image Data To txt File
 * @param path [in] File path to write
 * @param img [in] Image to write
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_write_to_txt(const char *path, rockx_image_t *img);

/**
 * Clone Image
 * @param img [in] Source image
 * @return cloned image
 */
rockx_image_t *rockx_image_clone(rockx_image_t *img);

/**
 * Clone Image
 * @param img [in] Source image
 * @param clone_data [in] is clone image data
 * @return cloned image
 */
rockx_image_t *rockx_image_clone2(rockx_image_t *img, int clone_data);

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

/**
 * Histogram equalization of the image
 * 
 * @param src_img [in] in image
 * @param dst_img [out] out image
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_equalize_hist(rockx_image_t *src_img, rockx_image_t *dst_img);

/**
 * border on Image
 * @param img [in] Image to border
 * @param img_border [in] border Image
 * @param box [in] Box to crop RoI
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_image_border_fillblack(rockx_image_t *img, rockx_image_t *img_border, rockx_rect_t *box);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_ROCKX_IMAGE_UTIL_H

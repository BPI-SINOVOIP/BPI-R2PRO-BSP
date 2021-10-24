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

#ifndef _ROCKFACE_IMAGE_H
#define _ROCKFACE_IMAGE_H

#include "rockface_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 读取图像文件(使用后必须调用 @ref rockface_image_release 释放)
 * 
 * @param img_path [in] Image file path
 * @param image [out] Read Image
 * @param flag [in] 0: gray; 1: color
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_read(const char *img_path, rockface_image_t *image, int flag);

/**
 * 将图像保存成图像文件(支持jpg或png)
 * 
 * @param path [in] File path to write
 * @param img [in] Image to write
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_write(const char *path, rockface_image_t *img);

/**
 * 将图像的原始数据保存成RGB24文件
 * 
 * @param path [in] File path to write
 * @param img [in] Image to write
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_write_raw(const char *path, rockface_image_t *img);

/**
 * 释放rockface_image_t对象中动态分配的数据
 * 
 * @param img 要释放的rockface_image_t
 * @return @ref rockface_ret_t 
 */
rockface_ret_t rockface_image_release(rockface_image_t *img);

/**
 * 对图像进行缩放、旋转、颜色通道转换
 * 
 * @param src [in] Source image
 * @param dst [out] Destination image
 * @param mode [in] transform mode
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_convert(rockface_image_t *src, rockface_image_t *dst, rockface_image_transform_mode mode);

/**
 * 计算图像清晰度
 * 
 * @param img [in] Image
 * @param clarity [in] Clarity result
 * @return rockface_ret_t 
 */
rockface_ret_t rockface_image_clarity(rockface_image_t *img, float *clarity);

/**
 * 获取图像的内存大小
 * 
 * @param img [in] Image
 * @return @ref rockface_ret_t
 */
int rockface_image_size(rockface_image_t *img);

/**
 * 获取图像的区域子图
 * 
 * @param img [in] 输入图像
 * @param box [in] 区域
 * @param roi_img [out] 区域图像
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_roi(rockface_image_t *img, rockface_rect_t *box, rockface_image_t *roi_img);

/**
 * 在图像上绘制圆形(只支持RGB/GRAY8图像)
 * 
 * @param img [in] 输入图像
 * @param point [in] 圆心坐标
 * @param radius [in] 半径
 * @param color [in] 颜色
 * @param thickness [in] 线宽
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_draw_circle(rockface_image_t *img, rockface_point_t point, int radius, rockface_color_t color, int thickness);

/**
 * 在图像上绘制方形（只支持RGB/GRAY8图像）
 * 
 * @param img [in] 输入图像
 * @param box [in] 区域
 * @param color [in] 颜色
 * @param thickness [in] 线宽
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_draw_rect(rockface_image_t *img, rockface_rect_t *box, rockface_color_t color, int thickness);

/**
 * 在图像上绘制文字（只支持RGB/GRAY8图像）
 * 
 * @param img [in] 输入图像
 * @param text [in] 文字
 * @param pt [in] 文字起始坐标
 * @param color [in] 颜色
 * @param thickness [in] 线宽
 * @return @ref rockface_ret_t
 */
rockface_ret_t rockface_image_draw_text(rockface_image_t *img, const char *text, rockface_point_t pt,
                                         rockface_color_t color, int thickness);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // _ROCKFACE_IMAGE_H
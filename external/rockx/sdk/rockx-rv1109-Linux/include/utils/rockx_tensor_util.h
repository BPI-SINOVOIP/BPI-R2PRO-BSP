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

#ifndef _TENSOR_UTIL_H
#define _TENSOR_UTIL_H

#include "../rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get Size of Data Type
 * @param type [in] Data type
 * @return size of data type
 */
int rockx_tensor_dtype_size(rockx_data_type type);

/**
 * Get Tensor Elements Number
 * @param tensor [in] Input tensor
 * @return Number of tensor elements
 */
int rockx_tensor_elems_num(rockx_tensor_t *tensor);

/**
 * Get Tensor Data Buffer Size
 * @param tensor [in] Input tensor
 * @return Size of tensor data buffer
 */
int rockx_tensor_size(rockx_tensor_t *tensor);

/**
 * Dump Tensor Info
 * @param tensor [in] Input tensor
 */
void rockx_tensor_dump(rockx_tensor_t *tensor);

/**
 * Write Tensor to File
 * @param path [in] File path to Write
 * @param tensor [in] Tensor to Write
 */
void write_tensor(const char *path, rockx_tensor_t *tensor);

/**
 * Initial a Tensor Attribute From Image without malloc memory
 * @param img [in] Input image
 * @param tensor [out] Initialed Tensor
 * @return @ref rockx_ret_t
 */

rockx_ret_t rockx_set_tensor_attr_from_image(rockx_image_t *img,rockx_tensor_t *tensor);

/**
 * Initial a Tensor From Image
 * @param img [in] Input image
 * @param tensor [out] Initialed Tensor
 * @return @ref rockx_ret_t
 */

rockx_ret_t rockx_tensor_init_from_image(rockx_image_t *img, rockx_tensor_t *tensor);

/**
 * Clone a Tensor
 * @param tensor [in] Tensor to clone
 * @param dst_tensor [out] Cloned Tensor
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_tensor_clone(rockx_tensor_t *tensor, rockx_tensor_t *dst_tensor);

/**
 * Release Data Buffer of Tensor
 * @param tensor [in] Tensor to release
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_tensor_release(rockx_tensor_t *tensor);

/**
 * Reshape Tensor
 * @param tensor [in] Tensor to reshape
 * @param dst_dims [in] New shape
 * @param n_dims [in] Number of new tensor dimention 
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_tensor_reshape(rockx_tensor_t *tensor, uint32_t dst_dims[4], uint32_t n_dims);

/**
 * Save Tensor as string format
 * @param path [in] file path to write
 * @param tensor [in] Tensor
 * @return @ref rockx_ret_t
 */
rockx_ret_t write_tensor_to_txt(const char *path, rockx_tensor_t *tensor);

/**
 * Save Raw float buffer as string format
 * @param path [in] File path to write
 * @param data [in] Float buffer
 * @param n_elements [in] Number of elements
 * @return @ref rockx_ret_t
 */
rockx_ret_t write_raw_float32_to_txt(const char *path, float *data, unsigned int n_elements);

/**
 * Save Raw uint8 buffer as string format
 * @param path [in] File path to write
 * @param data [in] uint8 buffer
 * @param n_elements [in] Number of elements
 * @return @ref rockx_ret_t
 */
rockx_ret_t write_raw_uint8_to_txt(const char *path, uint8_t *data, unsigned int n_elements);

/**
 * Read tensor from a data file
 * @param path [in] File path to read
 * @param dtype [in] Tensor date type
 * @param out_tensor [out] Read Tensor
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_tensor_read_from_file(const char *path, rockx_data_type dtype, rockx_tensor_t *out_tensor);

/**
 * Caculate "(image - mean_value) / std_value" for each channel
 * @param in_image [in] Image to caculate
 * @param out_tensor [in] Caculate result
 * @param mean_value [in] Mean value for each channel(for example: float mean_value[3] = {0, 0, 0})
 * @param std_value [in] Std value for each channel (for example: float std_value[3] = {255.0, 255.0, 255.0})
 * @return @ref rockx_ret_t
 */
rockx_ret_t rockx_tensor_mean_std(rockx_image_t *in_image, rockx_tensor_t *out_tensor, float *mean_value, float *std_value);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_TENSOR_UTIL_H

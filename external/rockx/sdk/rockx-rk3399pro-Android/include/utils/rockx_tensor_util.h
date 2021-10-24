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

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_TENSOR_UTIL_H

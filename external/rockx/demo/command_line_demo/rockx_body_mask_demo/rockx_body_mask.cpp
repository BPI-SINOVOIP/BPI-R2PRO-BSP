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
#include <stdio.h>
#include <memory.h>
#include <sys/time.h>
#include <stdlib.h>

#include "rockx.h"

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t bodymask_handle;
    struct timeval tv;

    if (argc != 3) {
        printf("Usage %s input.jpg background.jpg\n", argv[0]);
        return -1;
    }

    const char *img_path = argv[1];
    const char *bg_img_path = argv[2];

    // create a face detection handle
    ret = rockx_create(&bodymask_handle, ROCKX_MODULE_BODY_MASK, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_BODY_MASK error %d\n", ret);
        return -1;
    }

    // read image
    rockx_image_t input_origin_image;
    rockx_image_t input_image;
    rockx_image_t background_image;

    rockx_image_read(img_path, &input_origin_image, 1); 
    rockx_image_read(bg_img_path, &background_image, 1);

    input_image.width = background_image.width;
    input_image.height = background_image.height;
    input_image.pixel_format = input_origin_image.pixel_format;

    float dummy_data;
    rockx_image_convert_keep_ration(&input_origin_image, &input_image, 0, 0, &dummy_data, &dummy_data, &dummy_data, &dummy_data);

    // create rockx_bodymask_array_t for store result
    rockx_bodymask_array_t bodymask_array;
    memset(&bodymask_array, 0, sizeof(rockx_bodymask_array_t));
    bodymask_array.width = input_image.width;
    bodymask_array.height = input_image.height;
    bodymask_array.threshold = 0.4;
    bodymask_array.mask = (uint8_t *)malloc(bodymask_array.width * bodymask_array.height);

    // get mask
    printf("Begin inference ...\n");
    ret = rockx_body_mask(bodymask_handle, &input_image, &bodymask_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_body_mask error %d\n", ret);
        return -1;
    }

    // save mask, for test
    rockx_image_t mask_image;
    mask_image.data = bodymask_array.mask;
    mask_image.height = bodymask_array.height;
    mask_image.width = bodymask_array.width;
    mask_image.is_prealloc_buf = true;
    mask_image.pixel_format = ROCKX_PIXEL_FORMAT_GRAY8;
    for (int i=0; i<mask_image.width * mask_image.height; i++) {
        bodymask_array.mask[i]  *= 255;  
    }

    printf("Write mask.jpg ...\n");
    rockx_image_write("./mask.jpg", &mask_image);

    // only for demo
    for (int i=0; i<background_image.width * background_image.height; i++) {
        if (bodymask_array.mask[i] > 0 ) {
            background_image.data[3*i] = input_image.data[3*i];
            background_image.data[3*i+1] = input_image.data[3*i+1];
            background_image.data[3*i+2] = input_image.data[3*i+2];
        }
    }

    // save image
    printf("Write out.jpg ...\n");
    rockx_image_write("./out.jpg", &background_image);

    // release
    if (bodymask_array.mask != nullptr) {
        free(bodymask_array.mask);
    }
    rockx_image_release(&mask_image);
    rockx_image_release(&input_image);
    rockx_image_release(&input_origin_image);
    rockx_image_release(&background_image);
    rockx_destroy(bodymask_handle);
}

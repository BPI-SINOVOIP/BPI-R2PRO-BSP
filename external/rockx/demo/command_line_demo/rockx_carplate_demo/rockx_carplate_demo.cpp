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
    const char *img_path = argv[1];
    struct timeval tv;

    // read image
    rockx_image_t input_image;
    rockx_image_read(img_path, &input_image, 1);

    rockx_handle_t carplate_det_handle;
    rockx_handle_t carplate_align_handle;
    rockx_handle_t carplate_recog_handle;

    ret = rockx_create(&carplate_det_handle, ROCKX_MODULE_CARPLATE_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_CARPLATE_DETECTION error %d\n", ret);
        return -1;
    }

    ret = rockx_create(&carplate_align_handle, ROCKX_MODULE_CARPLATE_ALIGN, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_CARPLATE_ALIGN error %d\n", ret);
        return -1;
    }

    ret = rockx_create(&carplate_recog_handle, ROCKX_MODULE_CARPLATE_RECOG, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
        return -1;
    }

    rockx_object_array_t carplate_array;
    memset(&carplate_array, 0, sizeof(rockx_object_array_t));

    // detect carplate
    ret = rockx_carplate_detect(carplate_det_handle, &input_image, &carplate_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_carplate_detect error %d\n", ret);
        return -1;
    }

    for (int i = 0; i < carplate_array.count; i++) {
        // create rockx_carplate_align_result_t for store result
        rockx_carplate_align_result_t result;
        memset(&result, 0, sizeof(rockx_carplate_align_result_t));

        printf("(%d %d %d %d) %f\n", carplate_array.object[i].box.left, carplate_array.object[i].box.top,
                carplate_array.object[i].box.right, carplate_array.object[i].box.bottom, carplate_array.object[i].score);

        // carplate_fmapping_
        ret = rockx_carplate_align(carplate_align_handle, &input_image, &carplate_array.object[i].box, &result);
        if (ret != ROCKX_RET_SUCCESS) {
            printf("rockx_carplate_align error %d\n", ret);
            return -1;
        }

        // save image
        rockx_image_write("./refined_img.jpg", &(result.aligned_image));

        // recognize carplate number
        rockx_carplate_recog_result_t recog_result;
        ret = rockx_carplate_recognize(carplate_recog_handle, &(result.aligned_image), &recog_result);
        
        // remember release aligned image
        rockx_image_release(&(result.aligned_image));

        if (ret != ROCKX_RET_SUCCESS) {
            printf("rockx_face_detect error %d\n", ret);
            return -1;
        }

        // process result
        char platename[32];
        memset(platename, 0, 32);
        for(int n = 0; n < recog_result.length; n++) {
            strcat(platename, CARPLATE_RECOG_CODE[recog_result.namecode[n]]);
        }
        printf("carplate: %s\n", platename);
    }

    // release
    rockx_image_release(&input_image);
    rockx_destroy(carplate_det_handle);
    rockx_destroy(carplate_align_handle);
    rockx_destroy(carplate_recog_handle);
}

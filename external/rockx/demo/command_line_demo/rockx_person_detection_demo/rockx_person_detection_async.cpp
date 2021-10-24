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

#include "rockx.h"

static rockx_image_t input_image;

void body_det_callback_func(void *result, size_t result_size, void *extra_data) {
    rockx_object_array_t *body_array = (rockx_object_array_t*)result;

    // process result
    for (int i = 0; i < body_array->count; i++) {
        int left = body_array->object[i].box.left;
        int top = body_array->object[i].box.top;
        int right = body_array->object[i].box.right;
        int bottom = body_array->object[i].box.bottom;
        float score = body_array->object[i].score;
        // printf("box=(%d %d %d %d) score=%f\n", left, top, right, bottom, score);
        // draw
        char score_str[8];
        memset(score_str, 0, 8);
        snprintf(score_str, 8, "%.3f", score);
        rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {255, 0, 0}, 3);
        rockx_image_draw_text(&input_image, score_str, {left, top-12}, {255, 0, 0}, 3);
    }
}

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t face_det_handle;
    struct timeval tv1,tv2;

    const char *img_path = argv[1];

    // create a face detection handle
    ret = rockx_create(&face_det_handle, ROCKX_MODULE_PERSON_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
    }

    // read image
    rockx_image_read(img_path, &input_image, 1);

    // create rockx_body_array_t for store result
    rockx_object_array_t body_array;
    memset(&body_array, 0, sizeof(rockx_object_array_t));

    // detect face

    rockx_async_callback callback;
    callback.callback_func = body_det_callback_func;
    callback.extra_data = nullptr;

    gettimeofday(&tv1, NULL);
    printf("%ld before rockx_body_detect\n", (tv1.tv_sec * 1000000 + tv1.tv_usec));
    ret = rockx_person_detect(face_det_handle, &input_image, &body_array, &callback);
    gettimeofday(&tv2, NULL);
    printf("%ld after rockx_body_detect\n", (tv2.tv_sec * 1000000 + tv2.tv_usec));

    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_body_detect error %d\n", ret);
        return -1;
    }

    // save image
    rockx_image_write("./out.jpg", &input_image);

    // release
    rockx_image_release(&input_image);
    rockx_destroy(face_det_handle);
}

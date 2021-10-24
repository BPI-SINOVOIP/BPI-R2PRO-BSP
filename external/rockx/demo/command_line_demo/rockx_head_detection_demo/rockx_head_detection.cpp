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

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t head_det_handle;
    struct timeval tv;

    const char *img_path = argv[1];

    // create a head detection handle
    ret = rockx_create(&head_det_handle, ROCKX_MODULE_HEAD_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_HEAD_DETECTION error %d\n", ret);
    }

    // read image
    rockx_image_t input_image;
    rockx_image_read(img_path, &input_image, 1);

    // create rockx_head_array_t for store result
    rockx_object_array_t head_array;
    memset(&head_array, 0, sizeof(rockx_object_array_t));

    // detect head
    ret = rockx_head_detect(head_det_handle, &input_image, &head_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_head_detect error %d\n", ret);
        return -1;
    }

    // process result
    for (int i = 0; i < head_array.count; i++) {
        int left = head_array.object[i].box.left;
        int top = head_array.object[i].box.top;
        int right = head_array.object[i].box.right;
        int bottom = head_array.object[i].box.bottom;
        float score = head_array.object[i].score;
        printf("box=(%d %d %d %d) score=%f\n", left, top, right, bottom, score);
        // draw
        char score_str[8];
        memset(score_str, 0, 8);
        snprintf(score_str, 8, "%.3f", score);
        rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {255, 0, 0}, 3);
    }

    // save image
    rockx_image_write("./out.jpg", &input_image);

    // release
    rockx_image_release(&input_image);
    rockx_destroy(head_det_handle);
}

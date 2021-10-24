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

using namespace std;

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t object_det_handle;
    struct timeval tv;

    const char *img_path = argv[1];

    // create a object detection handle
    ret = rockx_create(&object_det_handle, ROCKX_MODULE_OBJECT_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
    }

    // read image
    rockx_image_t input_image;
    rockx_image_read(img_path, &input_image, 1);

    // create rockx_object_array_t for store result
    rockx_object_array_t object_array;
    memset(&object_array, 0, sizeof(rockx_object_array_t));

    // detect object
    ret = rockx_object_detect(object_det_handle, &input_image, &object_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_object_detect error %d\n", ret);
        return -1;
    }

    // process result
    for (int i = 0; i < object_array.count; i++) {
        int left = object_array.object[i].box.left;
        int top = object_array.object[i].box.top;
        int right = object_array.object[i].box.right;
        int bottom = object_array.object[i].box.bottom;
        int cls_idx = object_array.object[i].cls_idx;
        const char *cls_name = OBJECT_DETECTION_LABELS_91[object_array.object[i].cls_idx];
        float score = object_array.object[i].score;
        printf("box=(%d %d %d %d) cls_name=%s, score=%f\n", left, top, right, bottom, cls_name, score);

        // draw
        rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {255, 0, 0}, 2);
        rockx_image_draw_text(&input_image, cls_name, {left, top-8}, {255, 0, 0}, 2);
    }

    // save image
    rockx_image_write("./out.jpg", &input_image);

    // release
    rockx_image_release(&input_image);
    rockx_destroy(object_det_handle);
}

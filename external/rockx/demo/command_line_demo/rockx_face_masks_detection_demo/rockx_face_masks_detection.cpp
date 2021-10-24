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
    rockx_handle_t face_masks_det_handle;
    struct timeval tv;

    const char *img_path = argv[1];
    const char *id2class[2] = {"Mask","NoMask"};

    // create a face detection handle
    ret = rockx_create(&face_masks_det_handle, ROCKX_MODULE_FACE_MASKS_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_MASKS_DETECTION error %d\n", ret);
    }

    // read image
    rockx_image_t input_image;
    rockx_image_read(img_path, &input_image, 1);

    // create rockx_face_mask_array_t for store result
    rockx_face_mask_array_t face_mask_array;
    memset(&face_mask_array, 0, sizeof(rockx_face_mask_array_t));

    // detect face
    ret = rockx_face_masks_detect(face_masks_det_handle, &input_image, &face_mask_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_face_masks_detect error %d\n", ret);
        return -1;
    }

    // process result
    for (int i = 0; i < face_mask_array.count; i++) {
        int left = face_mask_array.face_masks[i].face_box.left;
        int top = face_mask_array.face_masks[i].face_box.top;
        int right = face_mask_array.face_masks[i].face_box.right;
        int bottom = face_mask_array.face_masks[i].face_box.bottom;
        float mask_score = face_mask_array.face_masks[i].mask_score;
        int hasMask = face_mask_array.face_masks[i].hasMask;
        printf("box=(%d %d %d %d), mask_score =%f\n", left, top, right, bottom, mask_score);
        // draw
        if(hasMask){
            rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {0,255,0}, 2);
            rockx_image_draw_text(&input_image,id2class[0],{left,top},{0,255,0},0.8);
        }
        else
        {
            rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {0,0,255}, 2);
            rockx_image_draw_text(&input_image,id2class[1],{left,top},{0,0,255},0.8);
        }
    }

    // save image
    printf("write img\n");
    rockx_image_write("./out.jpg", &input_image);

    // release
    rockx_image_release(&input_image);
    rockx_destroy(face_masks_det_handle);
}

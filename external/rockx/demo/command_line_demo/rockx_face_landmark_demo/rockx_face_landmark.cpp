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
    struct timeval tv;
    rockx_module_t rockx_module;

    if (argc < 3) {
        printf("Use ./rockx_face_landmark xxx.jpg 5 or ./rockx_face_landmark xxx.jpg 68\n");
        return -1;
    }

    const char *img_path = argv[1];
    int landmark_count = atoi(argv[2]);

    if (landmark_count == 5) {
        rockx_module = ROCKX_MODULE_FACE_LANDMARK_5;
    } else if (landmark_count == 68) {
        rockx_module = ROCKX_MODULE_FACE_LANDMARK_68;
    } else {
        printf("error landmark count must 5 or 68\n");
        return -1;
    }

    rockx_handle_t face_det_handle;

    // create a face detection handle
    ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
    }

    rockx_handle_t face_landmark_handle;

    // create a face landmark handle
    ret = rockx_create(&face_landmark_handle, rockx_module, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_LANDMARK_68 error %d\n", ret);
    }

    // read image
    rockx_image_t input_image;
    rockx_image_read(img_path, &input_image, 1);

    /*************** FACE Detect ***************/

    // create rockx_face_array_t for store result
    rockx_object_array_t face_array;
    memset(&face_array, 0, sizeof(rockx_object_array_t));

    // detect face
    ret = rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_face_detect error %d\n", ret);
        return -1;
    }

    // process result
    for (int i = 0; i < face_array.count; i++) {
        int left = face_array.object[i].box.left;
        int top = face_array.object[i].box.top;
        int right = face_array.object[i].box.right;
        int bottom = face_array.object[i].box.bottom;
        float score = face_array.object[i].score;
        printf("box=(%d %d %d %d) score=%f\n", left, top, right, bottom, score);
    }

    /*************** FACE Landmark ***************/

    // detect landmark for every face
    for (int i = 0; i < face_array.count; i++) {
        rockx_face_landmark_t out_landmark;
        rockx_face_angle_t out_angle;

        memset(&out_landmark, 0, sizeof(rockx_face_landmark_t));
        memset(&out_angle, 0, sizeof(rockx_face_angle_t));

        ret = rockx_face_landmark(face_landmark_handle, &input_image, &face_array.object[i].box, &out_landmark);

        if (landmark_count == 5) {
            rockx_image_t out_img;
            memset(&out_img, 0, sizeof(rockx_image_t));
            ret = rockx_face_align(face_landmark_handle, &input_image, &face_array.object[i].box, &out_landmark, &out_img);
            rockx_image_write("./aligned.jpg", &out_img);
            rockx_image_release(&out_img);
        }

        ret = rockx_face_pose(&out_landmark, &out_angle);

        printf("face %d:\n landmarks_count=%d landmark_score=%f angle=(pitch:%f yaw:%f roll:%f)\n", i,
                out_landmark.landmarks_count, out_landmark.score,
                out_angle.pitch, out_angle.yaw, out_angle.roll);

        for(int j = 0; j < out_landmark.landmarks_count; j++) {
            printf("landmark %d %d %d\n", j, out_landmark.landmarks[j].x, out_landmark.landmarks[j].y);
            rockx_image_draw_circle(&input_image, {out_landmark.landmarks[j].x, out_landmark.landmarks[j].y}, 2, {0, 255, 0}, -1);
        }
    }
    rockx_image_write("./out_landmark.jpg", &input_image);

    rockx_image_release(&input_image);

    // release handle
    rockx_destroy(face_det_handle);
    rockx_destroy(face_landmark_handle);
}

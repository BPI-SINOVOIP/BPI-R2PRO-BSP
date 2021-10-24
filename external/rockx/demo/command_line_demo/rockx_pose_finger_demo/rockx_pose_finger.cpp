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

#include <vector>

#include "rockx.h"

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t pose_finger_handle;
    struct timeval tv;

    if (argc < 3) {
        printf("Use ./rockx_pose_finger book.jpg 3 or ./rockx_face_landmark hand_368x368.jpg 21\n");
        return -1;
    }

    const char *img_path = argv[1];
    int keypoint_num = atoi(argv[2]);

    rockx_module_t target_module = ROCKX_MODULE_POSE_FINGER_21;

    if (keypoint_num == 3) {
        target_module = ROCKX_MODULE_POSE_FINGER_3;
    }

    ret = rockx_create(&pose_finger_handle, target_module, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_POSE_FINGER error %d\n", ret);
    }

    // read image
    rockx_image_t input_image;
    rockx_image_read(img_path, &input_image, 1);

    // create rockx_face_array_t for store result
    rockx_keypoints_t finger;
    memset(&finger, 0, sizeof(rockx_keypoints_t));

    // finger pose
    ret = rockx_pose_finger(pose_finger_handle, &input_image, &finger);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_pose_finger error %d\n", ret);
        return -1;
    }

    // process result
    if(finger.count > 0) {
        for(int j = 0; j < finger.count; j++) {
            int x = finger.points[j].x;
            int y = finger.points[j].y;
            printf("  [%d, %d]\n", x, y);
            rockx_image_draw_circle(&input_image, {x, y}, 3, {255, 0, 0}, -1);
        }
        const std::vector<std::pair<int,int>> posePairs = {
            {0,1}, {1,2}, {2,3}, {3,4}, {0,5}, {5,6}, {6,7},
            {7,8}, {0,9}, {9,10}, {10,11}, {11,12}, {0,13}, {13,14},
            {14,15}, {15,16}, {0,17}, {17,18}, {18,19}, {19,20}
        };

        for(int j = 0; j < posePairs.size(); j++) {
            const std::pair<int, int> &posePair = posePairs[j];
            int x0 = finger.points[posePair.first].x;
            int y0 = finger.points[posePair.first].y;
            int x1 = finger.points[posePair.second].x;
            int y1 = finger.points[posePair.second].y;

            if(x0 > 0 && y0 > 0 && x1 > 0 && y1 > 0) {
                rockx_image_draw_line(&input_image, {x0, y0}, {x1, y1}, {0, 255, 0}, 1);
            }
        }

    }

    // save image
    rockx_image_write("./out.jpg", &input_image);

    // release
    rockx_image_release(&input_image);
    rockx_destroy(pose_finger_handle);
}

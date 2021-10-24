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
#include <unistd.h>

#include <vector>

#include "rockx.h"

static rockx_image_t input_image;

const std::vector<std::pair<int,int>> posePairs = {
        {2,3}, {3,4}, {5,6}, {6,7},
        {8,9}, {9,10}, {11,12}, {12,13},
        {1,0}, {0,14}, {14,16}, {0,15}, {15,17},
        {2,5}, {8,11}, {2,8}, {5,11}
};

void callback_func(void *result, size_t result_size, void *extra_data) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("%ld on result callback\n", (tv.tv_sec * 1000000 + tv.tv_usec));

    rockx_keypoints_array_t *body_array = (rockx_keypoints_array_t*)result;

    // process result
    for (int i = 0; i < body_array->count; i++) {
        printf("person %d:\n", i);

        for(int j = 0; j < body_array->keypoints[i].count; j++) {
            int x = body_array->keypoints[i].points[j].x;
            int y = body_array->keypoints[i].points[j].y;
            float score = body_array->keypoints[i].score[j];
            printf("  %s [%d, %d] %f\n", ROCKX_POSE_BODY_KEYPOINTS_NAME[j], x, y, score);
            rockx_image_draw_circle(&input_image, {x, y}, 3, {255, 0, 0}, -1);
        }

        for(int j = 0; j < posePairs.size(); j ++) {
            const std::pair<int,int>& posePair = posePairs[j];
            int x0 = body_array->keypoints[i].points[posePair.first].x;
            int y0 = body_array->keypoints[i].points[posePair.first].y;
            int x1 = body_array->keypoints[i].points[posePair.second].x;
            int y1 = body_array->keypoints[i].points[posePair.second].y;

            if( x0 > 0 && y0 > 0 && x1 > 0 && y1 > 0) {
                rockx_image_draw_line(&input_image, {x0, y0}, {x1, y1}, {0, 255, 0}, 1);
            }
        }
    }

    // save image
    rockx_image_write("./out.jpg", &input_image);
    rockx_image_release(&input_image);
}

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t pose_body_handle;
    struct timeval tv;

    const char *img_path = argv[1];

    // create a pose_body handle
    ret = rockx_create(&pose_body_handle, ROCKX_MODULE_POSE_BODY_V2, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_POSE_BODY error %d\n", ret);
    }

    // read image
    rockx_image_read(img_path, &input_image, 1);

    // create rockx_face_array_t for store result
    rockx_keypoints_array_t body_array;
    memset(&body_array, 0, sizeof(rockx_keypoints_array_t));

    // body pose

    rockx_async_callback callback;
    callback.callback_func = callback_func;
    callback.extra_data = nullptr;

    gettimeofday(&tv, NULL);
    printf("%ld before rockx_pose_body\n", (tv.tv_sec * 1000000 + tv.tv_usec));
    ret = rockx_pose_body(pose_body_handle, &input_image, &body_array, &callback);
    gettimeofday(&tv, NULL);
    printf("%ld after rockx_pose_body\n", (tv.tv_sec * 1000000 + tv.tv_usec));
    if (ret != ROCKX_RET_SUCCESS) {
        printf("rockx_pose_body error %d\n", ret);
        return -1;
    }

    sleep(3);

    // release handle
    rockx_destroy(pose_body_handle);
}

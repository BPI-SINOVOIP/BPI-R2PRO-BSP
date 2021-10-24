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

rockx_handle_t face_det_handle;
rockx_handle_t face_5landmarks_handle;
rockx_handle_t face_recognize_handle;

rockx_object_t *get_max_face(rockx_object_array_t *face_array) {
    if (face_array->count == 0) {
        return NULL;
    }
    rockx_object_t *max_face = NULL;
    int i;
    for (i = 0; i < face_array->count; i++) {
        rockx_object_t *cur_face = &(face_array->object[i]);
        if (max_face == NULL) {
            max_face = cur_face;
            continue;
        }
        int cur_face_box_area = (cur_face->box.right - cur_face->box.left) * (cur_face->box.bottom - cur_face->box.top);
        int max_face_box_area = (max_face->box.right - max_face->box.left) * (max_face->box.bottom - max_face->box.top);
        if (cur_face_box_area > max_face_box_area) {
            max_face = cur_face;
        }
    }
    printf("get_max_face %d\n", i-1);
    return max_face;
}

int run_face_recognize(rockx_image_t *in_image, rockx_face_feature_t *out_feature) {
    rockx_ret_t ret;

    /*************** FACE Detect ***************/
    // create rockx_face_array_t for store result
    rockx_object_array_t face_array;
    memset(&face_array, 0, sizeof(rockx_object_array_t));

    // detect face
    ret = rockx_face_detect(face_det_handle, in_image, &face_array, nullptr);
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
        printf("%d box=(%d %d %d %d) score=%f\n", i, left, top, right, bottom, score);
    }

    // Get max face
    rockx_object_t* max_face = get_max_face(&face_array);
    if (max_face == NULL) {
        printf("error no face detected\n");
        return -1;
    }

    // Face Align
    rockx_image_t out_img;
    memset(&out_img, 0, sizeof(rockx_image_t));
    ret = rockx_face_align(face_5landmarks_handle, in_image, &(max_face->box), nullptr, &out_img);
    if (ret != ROCKX_RET_SUCCESS) {
        return -1;
    }

    // Face Recognition
    rockx_face_recognize(face_recognize_handle, &out_img, out_feature);

    // Release Aligned Image
    rockx_image_release(&out_img);

    return 0;
}

int main(int argc, char** argv) {

    rockx_ret_t ret;
    struct timeval tv;

    if( argc != 3 ){
        printf("\nUsage: mser_sample <path_to_person1_image> <path_to_person2_image>\n");
        return -1;
    }
    /*************** Creat Handle ***************/
    // create a face detection handle
    ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
        return -1;
    }
    
    // create a face landmark handle
    ret = rockx_create(&face_5landmarks_handle, ROCKX_MODULE_FACE_LANDMARK_5, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_LANDMARK_68 error %d\n", ret);
        return -1;
    }

    // create a face recognize handle
    ret = rockx_create(&face_recognize_handle, ROCKX_MODULE_FACE_RECOGNIZE, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_LANDMARK_68 error %d\n", ret);
        return -1;
    }

    rockx_face_feature_t out_feature1;
    rockx_face_feature_t out_feature2;

    memset(&out_feature1, 0, sizeof(rockx_face_feature_t));
    memset(&out_feature2, 0, sizeof(rockx_face_feature_t));

    // read image
    const char *img_path1 = argv[1];
    const char *img_path2 = argv[2];

    // read image
    rockx_image_t input_image1;
    rockx_image_read(img_path1, &input_image1, 1);

    rockx_image_t input_image2;
    rockx_image_read(img_path2, &input_image2, 1);

    run_face_recognize(&input_image1, &out_feature1);
    run_face_recognize(&input_image2, &out_feature2);

    float similarity;
    ret = rockx_face_feature_similarity(&out_feature1, &out_feature2, &similarity);
    printf("similarity is %f\n",similarity);

    //release handle
    rockx_destroy(face_det_handle);
    rockx_destroy(face_5landmarks_handle);
    rockx_destroy(face_recognize_handle);
    return 0;
}

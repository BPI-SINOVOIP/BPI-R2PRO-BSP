/****************************************************************************
*
*    Copyright (c) 2017 - 2020 by Rockchip Corp.  All rights reserved.
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

#include "rockface.h"

rockface_det_t *get_max_face(rockface_det_array_t *face_array) {
    if (face_array->count == 0) {
        return NULL;
    }
    rockface_det_t *max_face = NULL;
    int i;
    for (i = 0; i < face_array->count; i++) {
        rockface_det_t *cur_face = &(face_array->face[i]);
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
    return max_face;
}

rockface_ret_t run_face_mask_recognize(rockface_handle_t face_handle, rockface_image_t *in_image, rockface_feature_float_t *out_feature,  int mode) {

    rockface_ret_t ret;
    rockface_image_t in_rgb_img;
    rockface_det_array_t face_array;
    rockface_det_t* max_face = NULL;

    memset(&in_rgb_img, 0, sizeof(rockface_image_t));
    memset(&face_array, 0, sizeof(rockface_det_array_t));

    in_rgb_img.width = in_image->width;
    in_rgb_img.height = in_image->height;
    in_rgb_img.pixel_format = ROCKFACE_PIXEL_FORMAT_RGB888;
    ret = rockface_image_convert(in_image, &in_rgb_img, ROCKFACE_IMAGE_TRANSFORM_NONE);
    if (ret != ROCKFACE_RET_SUCCESS) {
        goto error;
    }

    // detect face
    ret = rockface_detect(face_handle, &in_rgb_img, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("rockface_face_detect error %d\n", ret);
        goto error;
    }
    if (face_array.count == 0){
        ret = ROCKFACE_RET_FAIL;
        printf("no face\n");
        goto error;
    }
    
    max_face = get_max_face(&face_array);
    if (max_face == NULL) {
        ret = ROCKFACE_RET_FAIL;
        printf("error no face detected\n");
        goto error;
    }

    // Get face feature
    ret = rockface_mask_feature_extract(face_handle, &in_rgb_img, &max_face->box, mode, out_feature);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("rockface_mask_feature_extract error %d\n", ret);
        goto error;
    }
    
error:
    // Release Aligned Image
    rockface_image_release(&in_rgb_img);
    return ret;
}

int main(int argc, char** argv) {

    rockface_ret_t ret;

    char *licence_path = NULL;
    char *img_path1 = NULL;
    char *img_path2 = NULL;

    if (argc != 3 && argc != 4) {
        printf("\nUsage:\n");
        printf("\trecognition_1v1 <image file 1> <image file 2>\n");
        printf("or\n");
        printf("\trecognition_1v1 <image file 1> <image file 2> <licence file>\n");
        return -1;
    }

    img_path1 = argv[1];
    img_path2 = argv[2];

    printf("image path1: %s\n", img_path1);
    printf("image path2: %s\n", img_path2);

    if (argc == 4) {
        licence_path = argv[1];
        printf("licence path: %s\n", licence_path);
    }

    rockface_handle_t face_handle = rockface_create_handle();

    if (licence_path != NULL) {
        ret = rockface_set_licence(face_handle, licence_path);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("ERROR: authorization fail %d!\n", ret);
            return ret;
        }
    } else {
        printf("WARNING: can only try for a while without authorization\n");
    }

    // init rockface handle
    ret = rockface_init_detector(face_handle);
    ret = rockface_init_mask_recognizer(face_handle);

    rockface_feature_float_t out_feature1;
    rockface_feature_float_t out_feature2;

    memset(&out_feature1, 0, sizeof(rockface_feature_float_t));
    memset(&out_feature2, 0, sizeof(rockface_feature_float_t));

    // read gallery image
    rockface_image_t input_image1;
    rockface_image_read(img_path1, &input_image1, 1);

    // read test image
    rockface_image_t input_image2;
    rockface_image_read(img_path2, &input_image2, 1);
    
    //提取未带口罩底库人脸的特征
    ret = run_face_mask_recognize(face_handle, &input_image1, &out_feature1, 0);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("Error: get face feature error %d", ret);
        return -1;
    }

    //提取测试人脸的特征
    ret = run_face_mask_recognize(face_handle, &input_image2, &out_feature2, 1);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("Error: get face feature error %d", ret);
        return -1;
    }

    float similarity;
    rockface_feature_compare((rockface_feature_t *)&out_feature1, (rockface_feature_t *)&out_feature2, &similarity);
    printf("similarity is %f\n",similarity);

    // release image
    rockface_image_release(&input_image1);
    rockface_image_release(&input_image2);

    //release handle
    rockface_release_handle(face_handle);
    return 0;
}

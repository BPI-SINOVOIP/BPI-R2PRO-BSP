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

int main(int argc, char** argv) {

    rockface_ret_t ret;

    char *licence_path = NULL;
    char *img_path = NULL;

    if (argc != 2 && argc != 3) {
        printf("\nUsage:\n");
        printf("\tface_analyze <image file>\n");
        printf("or\n");
        printf("\tface_analyze <image file> <licence file>\n");
        return -1;
    }

    img_path = argv[1];
    printf("image path: %s\n", img_path);

    if (argc == 3) {
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

    ret = rockface_init_quality(face_handle);
    ret = rockface_init_detector(face_handle);
    ret = rockface_init_analyzer(face_handle);
    ret = rockface_init_landmark(face_handle, 5);
    ret = rockface_init_landmark(face_handle, 106);

    // read image
    rockface_image_t input_image;
    rockface_image_read(img_path, &input_image, 1);

    // detect face
    rockface_det_array_t face_array;
    memset(&face_array, 0, sizeof(rockface_det_array_t));

    ret = rockface_detect(face_handle, &input_image, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("rockface_face_detect error %d\n", ret);
        return -1;
    }

    for (int i = 0; i < face_array.count; i++) {
        rockface_det_t *det_face = &(face_array.face[i]);
        //check is_face
        int is_false_face;
        ret = rockface_face_filter(face_handle, &input_image, &(det_face->box), &is_false_face);
        printf("check false face: %d\n", is_false_face);

        // face align
        rockface_image_t aligned_img;
        memset(&aligned_img, 0, sizeof(rockface_image_t));
        ret = rockface_align(face_handle, &input_image, &(det_face->box), NULL, &aligned_img);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("error align face %d\n", ret);
            return -1;
        }
        // face attribute
        rockface_attribute_t face_attr;
        ret = rockface_attribute(face_handle, &aligned_img, &face_attr);

        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("error rockface_attribute %d\n", ret);
            return -1;
        }
        // face landmark
        rockface_landmark_t face_landmark;
        ret = rockface_landmark5(face_handle, &input_image, &(det_face->box), &face_landmark);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("error rockface_landmarke %d\n", ret);
            return -1;
        }
        // face angle
        rockface_angle_t face_angle;
        ret = rockface_angle(face_handle, &face_landmark, &face_angle);
        // output result
        printf("face index=%d box=(%d %d %d %d) score=%f landmark_score=%f age=%d gender=%d angle=(%f %f %f)\n",
            i, det_face->box.left, det_face->box.top, det_face->box.right, det_face->box.bottom, det_face->score, face_landmark.score,
            face_attr.age, face_attr.gender,
            face_angle.pitch, face_angle.roll, face_angle.yaw);

        rockface_angle_t face_angle_106;
        rockface_landmark_t face_landmark_106;
        ret = rockface_landmark106(face_handle, &input_image, &(det_face->box), &face_landmark, &face_landmark_106, &face_angle_106);
        // output result
        printf("106 landmark angle=(%f %f %f)\n", face_angle_106.pitch, face_angle_106.roll, face_angle_106.yaw);

        // face blur detect
        float blur;
        rockface_blur(&aligned_img, &blur);
        printf("face blur valu is %f\n",blur);

        //face bright level
        float bright_level;
        rockface_brightlevel(&aligned_img, &bright_level);
        printf("face bright valu is %f\n",bright_level);

        // face mask classify
        float mask_score;
        rockface_mask_classifier(face_handle, &input_image, &(det_face->box), &mask_score);
        printf("face mask score: %f\n", mask_score);

        // release aligned image first (avoid memory leak)
        rockface_image_release(&aligned_img);
    }

    // release image
    rockface_image_release(&input_image);

    //release handle
    rockface_release_handle(face_handle);
    return 0;
}

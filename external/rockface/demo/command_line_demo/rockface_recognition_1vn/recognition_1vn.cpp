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
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "rockface.h"
#include "face_db.h"
#include "rockface_recognition.h"

face_data *_face_array = NULL;
mask_face_data *_mask_face_array = NULL;

static int get_face_library(rockface_handle_t handle, const char *db_path, int is_mask_face_recog) {

    sqlite3 *db = NULL;

    int _face_num = 0;

    rockface_ret_t rockface_ret = ROCKFACE_RET_SUCCESS;
    int ret = 0;

    ret = open_db(db_path, &db);

    ret = get_face_count(db, &_face_num);
    if (ret < 0) {
        printf("error get_face_count %d\n", ret);
        goto error;
    }

    _face_array = (face_data *)malloc(_face_num*sizeof(face_data));
    ret = get_all_face(db, _face_array, _face_num);
    if (ret < 0) {
        printf("error get_all_face %d\n", ret);
        goto error;
    }

    rockface_ret = rockface_face_library_init2(handle, ROCKFACE_RECOG_NORMAL, _face_array, _face_num, sizeof(face_data), 0);
    if (rockface_ret != ROCKFACE_RET_SUCCESS) {
        printf("error rockface_face_library_init %d\n", rockface_ret);
        ret = -1;
        goto error;
    }

    if (is_mask_face_recog == 1) {
        _mask_face_array = (mask_face_data *)malloc(_face_num*sizeof(mask_face_data));
        ret = get_all_mask_face(db, _mask_face_array, _face_num);
        if (ret < 0) {
            printf("error get_all_mask_face %d\n", ret);
            goto error;
        }

        rockface_ret = rockface_face_library_init2(handle, ROCKFACE_RECOG_MASK, _mask_face_array, _face_num, sizeof(mask_face_data), 0);
        if (rockface_ret != ROCKFACE_RET_SUCCESS) {
            printf("error rockface_face_library_init %d\n", rockface_ret);
            ret = -1;
            goto error;
        }
    }

error:
    close_db(db);
    if (_face_array != NULL) {
        free(_face_array);
    }

    return ret;
}

int main(int argc, char** argv) {

    rockface_ret_t ret;
    struct timeval tv;
    char *licence_path = NULL;
    char *img_path = NULL;
    char *db_path = NULL;
    int is_mask_face_recog = 0;

    if(argc != 3 && argc != 4 ){
        printf("\nUsage:\n");
        printf("\trecognition_1vn <image path> <database path> <is_mask_face_recog>\n");
        printf("or\n");
        printf("\trecognition_1vn <image path> <database path> <is_mask_face_recog> <licence file>\n");
        return -1;
    }

    img_path = argv[1];
    db_path = argv[2];
    is_mask_face_recog = atoi(argv[3]);

    printf("image path: %s\n", img_path);
    printf("create database path: %s\n", db_path);
    printf("mask face recog: %d\n", is_mask_face_recog);

    if (argc == 5) {
        licence_path = argv[4];
        printf("licence path: %s\n", licence_path);
    }

    rockface_handle_t face_handle = rockface_create_handle();
    rockface_feature_t face_feature;
    rockface_feature_float_t mask_face_feature;
    rockface_image_t in_img;
    rockface_search_result_t search_result;
    rockface_ret_t rockface_ret;

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
    ret = rockface_init_landmark(face_handle, 5);
    ret = rockface_init_recognizer(face_handle);
    if (is_mask_face_recog == 1) {
        ret = rockface_init_mask_recognizer(face_handle);
        ret = rockface_init_mask_classifier(face_handle);
    }

    // load database
    int db_ret = get_face_library(face_handle, db_path, is_mask_face_recog);
    if (db_ret != 0) {
        printf("get face library error\n");
        goto error;
    }

    rockface_image_read(img_path, &in_img, 1);

    // run face detection
    rockface_det_t face_det;
    ret = run_face_detection(face_handle, &in_img, &face_det);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("run face detection error");
        goto error;
    }

    if (is_mask_face_recog == 1) {
        float mask_score;
        ret = rockface_mask_classifier(face_handle, &in_img, &(face_det.box), &mask_score);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("rockface_mask_classifier error");
            goto error;
        }

        printf("mask_score=%f\n", mask_score);

        if (mask_score < 0.5) {
            ret = run_get_face_feature(face_handle, &in_img, &face_det, &face_feature);
            if (ret != ROCKFACE_RET_SUCCESS) {
                printf("get face feature error");
                goto error;
            }
            rockface_ret = rockface_feature_search(face_handle, &face_feature, 1.0, &search_result);
        } else {
            ret = rockface_mask_feature_extract(face_handle, &in_img, &face_det.box, 1, &mask_face_feature);
            if (ret != ROCKFACE_RET_SUCCESS) {
                printf("get face feature error");
                goto error;
            }
            rockface_ret = rockface_feature_search(face_handle, (rockface_feature_t *)&mask_face_feature, 1.0, &search_result);
        }
    } else {
        ret = run_get_face_feature(face_handle, &in_img, &face_det, &face_feature);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("get face feature error");
            goto error;
        }
        rockface_ret = rockface_feature_search(face_handle, &face_feature, 1.0, &search_result);
    }

    if (rockface_ret == ROCKFACE_RET_SUCCESS) {
        char *name = NULL;
        if (((face_data *)search_result.face_data)->feature.version == ROCKFACE_RECOG_MASK) {
            name = ((mask_face_data *)search_result.face_data)->name;
        } else {
            name = ((face_data *)search_result.face_data)->name;
        }
        printf("search result: name=%s dist=%f\n", name, search_result.similarity);
    } else {
        printf("error rockface_feature_search %d\n", rockface_ret);
    }

error:
    rockface_image_release(&in_img);
    rockface_face_library_release(face_handle);
    rockface_release_handle(face_handle);

    return 0;
}

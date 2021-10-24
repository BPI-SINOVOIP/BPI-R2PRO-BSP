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

#include "rockface.h"
#include "face_db.h"
#include "rockface_recognition.h"

static int is_image_file(const char *filename) {
    int len = strlen(filename);
    if (strcmp(&filename[len-3], "jpg") == 0 || strcmp(&filename[len-3], "png") == 0) {
        return 1;
    }
    return 0;
}

static int import_face_library(rockface_handle_t handle, const char *image_dir_path, const char * db_path, int is_mask_face_recog) {
    int ret_code = 0;
    sqlite3 *db;
    open_db(db_path, &db);

    DIR *dir;
    struct dirent *ptr;
    dir = opendir(image_dir_path);
    struct stat statbuf;
    char image_path[256];

    int image_num = 0;
    while((ptr = readdir(dir)) != NULL) {
        if (is_image_file(ptr->d_name)) {
            image_num++;
        }
    }
    closedir(dir);

    printf("image num %d\n", image_num);

    face_data facedata;
    mask_face_data maskfacedata;

    rockface_ret_t ret;
    dir = opendir(image_dir_path);
    int imported_num = 0;
    int index = 0;
    int db_ret = 0;

    while((ptr = readdir(dir)) != NULL) {
        if (is_image_file(ptr->d_name) <= 0) {
            continue;
        }
        index++;
        snprintf(image_path, 256, "%s/%s", image_dir_path, ptr->d_name);
        printf("%s\n", image_path);

        // read image
        rockface_image_t image;
        ret = rockface_image_read(image_path, &image, 1);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("%d [%d/%d] imported %s fail\n", index, imported_num, image_num, image_path);
            continue;
        }

        // face detection
        rockface_det_t face_box;
        ret = run_face_detection(handle, &image, &face_box);
        if (ret != ROCKFACE_RET_SUCCESS) {
            rockface_image_release(&image);
            printf("%d [%d/%d] imported %s fail\n", index, imported_num, image_num, image_path);
            continue;
        }

        // register normal face
        // get feature
        rockface_feature_t feature;
        memset(&feature, 0, sizeof(rockface_feature_t));
        ret = run_get_face_feature(handle, &image, &face_box, &feature);

        if (ret != ROCKFACE_RET_SUCCESS) {
            rockface_image_release(&image);
            printf("%d [%d/%d] imported %s fail\n", index, imported_num, image_num, image_path);
            continue;
        }
        memset(&facedata, 0, sizeof(face_data));
        strncpy(facedata.name, ptr->d_name, MAX_SIZE_NAME);
        memcpy(&(facedata.feature), &feature, sizeof(rockface_feature_t));
        // save to database
        db_ret = insert_face(db, &facedata);
        if (db_ret != 0) {
            ret_code = -1;
            printf("%d [%d/%d] imported %s fail\n", index, imported_num, image_num, image_path);
            goto error;
        }

        // register mask face
        if (is_mask_face_recog == 1) {
            // get feature
            rockface_feature_float_t mask_feature;
            memset(&mask_feature, 0, sizeof(rockface_feature_float_t));
            ret = rockface_mask_feature_extract(handle, &image, &face_box.box, 0, &mask_feature);
            if (ret != ROCKFACE_RET_SUCCESS) {
                rockface_image_release(&image);
                printf("%d [%d/%d] imported %s fail\n", index, imported_num, image_num, image_path);
                continue;
            }
            memset(&maskfacedata, 0, sizeof(face_data));
            strncpy(maskfacedata.name, ptr->d_name, MAX_SIZE_NAME);
            memcpy(&(maskfacedata.feature), &mask_feature, sizeof(rockface_feature_float_t));
            // save to database
            db_ret = insert_mask_face(db, &maskfacedata);
            if (db_ret != 0) {
                ret_code = -1;
                printf("%d [%d/%d] imported %s fail\n", index, imported_num, image_num, image_path);
                goto error;
            }
        }

        rockface_image_release(&image);

        printf("%d [%d/%d] imported %s success\n", index, imported_num, image_num, image_path);
        imported_num++;
    }
    printf("imported count: %d/%d\n", imported_num, image_num);

error:
    close_db(db);
    closedir(dir);
    return ret_code;
}

int main(int argc, char** argv) {

    rockface_ret_t ret;
    char *img_dir_path = NULL;
    char *db_path = NULL;
    char *licence_path = NULL;
    int is_mask_face_recog = 0;

    if(argc != 4 && argc != 5) {
        printf("\nUsage:\n");
        printf("\timport_face_library <image dir> <database path> <is_mask_face_recog>\n");
        printf("or\n");
        printf("\timport_face_library <image dir> <database path> <is_mask_face_recog> <licence file>\n");
        return -1;
    }

    // read image
    img_dir_path = argv[1];
    db_path = argv[2];
    is_mask_face_recog = atoi(argv[3]);

    printf("import image dir: %s\n", img_dir_path);
    printf("create database path: %s\n", db_path);
    printf("mask face recog: %d\n", is_mask_face_recog);

    if (argc == 5) {
        licence_path = argv[4];
        printf("licence path: %s\n", licence_path);
    }

    rockface_handle_t face_handle = rockface_create_handle();

    if (licence_path != NULL) {
        ret = rockface_set_licence(face_handle, licence_path);
        if (ret < 0) {
            printf("Error: authorization error %d!", ret);
            return ret;
        }
    } else {
        printf("Warning: can only try for a while without authorization");
    }

    ret = rockface_init_detector(face_handle);
    ret = rockface_init_landmark(face_handle, 5);
    ret = rockface_init_recognizer(face_handle);
    if (is_mask_face_recog == 1) {
        ret = rockface_init_mask_recognizer(face_handle);
        ret = rockface_init_mask_classifier(face_handle);
    }

    import_face_library(face_handle, img_dir_path, db_path, is_mask_face_recog);

    //release handle
    rockface_release_handle(face_handle);

    return 0;
}

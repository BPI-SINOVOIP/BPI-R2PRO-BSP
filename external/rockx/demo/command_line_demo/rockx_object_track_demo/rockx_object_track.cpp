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

using namespace std;

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t object_det_handle;
    rockx_handle_t object_track_handle;
    struct timeval tv;

    if( argc != 2 ){
        printf("\nUsage: mser_sample <video_frame_list.txt>\n");
        return -1;
    }

    const char *img_path = argv[1];

    // create a object detection handle
    ret = rockx_create(&object_det_handle, ROCKX_MODULE_OBJECT_DETECTION, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
    }

    // create a object track handle
    ret = rockx_create(&object_track_handle, ROCKX_MODULE_OBJECT_TRACK, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
    }

    FILE *fp = fopen(img_path,"r");
    while (!feof(fp))
    {
        char imagepath[64];
        while (fscanf(fp, "%s", imagepath) > 0)
        {
            // read image
            rockx_image_t input_image;
            rockx_image_read(imagepath, &input_image, 1);

            // create rockx_object_array_t for store result
            rockx_object_array_t object_array;
            memset(&object_array, 0, sizeof(rockx_object_array_t));

            // detect object
            ret = rockx_object_detect(object_det_handle, &input_image, &object_array, nullptr);
            if (ret != ROCKX_RET_SUCCESS) {
                printf("rockx_object_detect error %d\n", ret);
                return -1;
            }

            // object track
            int max_track_time = 4;
            rockx_object_array_t in_track_objects;
            rockx_object_array_t out_track_objects;

            ret = rockx_object_track(object_track_handle, input_image.width,  input_image.height, max_track_time,
                    &object_array, &out_track_objects);
            if (ret != ROCKX_RET_SUCCESS) {
                printf("rockx_object_track error %d\n", ret);
                return -1;
            }

            // process result
            for (int i = 0; i < out_track_objects.count; i++) {
                int left = out_track_objects.object[i].box.left;
                int top = out_track_objects.object[i].box.top;
                int right = out_track_objects.object[i].box.right;
                int bottom = out_track_objects.object[i].box.bottom;
                int cls_idx = out_track_objects.object[i].cls_idx;
                const char *cls_name = OBJECT_DETECTION_LABELS_91[cls_idx];
                float score = out_track_objects.object[i].score;
                int track_id = out_track_objects.object[i].id;
                printf("box=(%d %d %d %d) cls_name=%s, score=%f track_id=%d\n", left, top, right, bottom,
                        cls_name, score, track_id);
                char show_str[32];
                memset(show_str, 0, 32);
                snprintf(show_str, 32, "%d - %s", track_id, cls_name);
                // draw
                rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {255, 0, 0}, 2);
                rockx_image_draw_text(&input_image, show_str, {left, top-8}, {255, 0, 0}, 2);
            }

            // save image
            char output_name[128];
            snprintf(output_name, 128, "./out/%s", imagepath);
            rockx_image_write(output_name, &input_image);

            rockx_image_release(&input_image);

            //process every image
            printf("%s\n", output_name);
        }
    }
    fclose(fp);

    // release handle
    rockx_destroy(object_det_handle);
    rockx_destroy(object_track_handle);
}

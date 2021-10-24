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
        printf("\tface_mask_det <image file>\n");
        printf("or\n");
        printf("\tface_mask_det <image file> <licence file>\n");
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

    ret = rockface_init_mask_detector(face_handle);

    // read image
    rockface_image_t input_image;
    rockface_image_read(img_path, &input_image, 1);

    // detect face
    rockface_mask_array_t face_array;
    memset(&face_array, 0, sizeof(rockface_mask_array_t));

    ret = rockface_mask_detect(face_handle, &input_image, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("rockface_face_detect error %d\n", ret);
        return -1;
    }

    for (int i = 0; i < face_array.count; i++) {
        rockface_mask_t *det_face = &(face_array.face_masks[i]);
        printf(" %d face_box=(%d %d %d %d) score=%f has_mask=%d\n", i,
            face_array.face_masks[i].face_box.left,
            face_array.face_masks[i].face_box.top,
            face_array.face_masks[i].face_box.right,
            face_array.face_masks[i].face_box.bottom,
            face_array.face_masks[i].score,
            face_array.face_masks[i].has_mask
            );
    }

    // release image
    rockface_image_release(&input_image);

    //release handle
    rockface_release_handle(face_handle);
    return 0;
}

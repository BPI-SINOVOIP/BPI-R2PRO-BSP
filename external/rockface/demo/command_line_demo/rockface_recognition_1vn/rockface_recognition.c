#include <stdio.h>
#include <string.h>
#include "rockface_recognition.h"

static rockface_det_t *get_max_face(rockface_det_array_t *face_array) {
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

rockface_ret_t run_face_detection(rockface_handle_t face_handle, rockface_image_t *in_image, rockface_det_t *out_face) {
    rockface_ret_t ret;

    // create rockface_face_array_t for store result
    rockface_det_array_t face_array;
    memset(&face_array, 0, sizeof(rockface_det_array_t));

    // detect face
    ret = rockface_detect(face_handle, in_image, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("rockface_face_detect error %d\n", ret);
        return ret;
    }

    // Get max face
    rockface_det_t* max_face = get_max_face(&face_array);
    if (max_face == NULL) {
        printf("error no face detected\n");
        return ROCKFACE_RET_FAIL;
    }

    memcpy(out_face, max_face, sizeof(rockface_det_t));

    return ROCKFACE_RET_SUCCESS;
}

rockface_ret_t run_get_face_feature(rockface_handle_t face_handle, rockface_image_t *in_image, rockface_det_t *face_det, rockface_feature_t *out_feature) {
    rockface_ret_t ret;

    // Face Align
    rockface_image_t out_img;
    memset(&out_img, 0, sizeof(rockface_image_t));
    ret = rockface_align(face_handle, in_image, &(face_det->box), NULL, &out_img);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("error align face %d\n", ret);
        return ret;
    }

    // Face Recognition
    ret = rockface_feature_extract(face_handle, &out_img, out_feature);
    
    // Release Aligned Image
    rockface_image_release(&out_img);

    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("error, rockface_feature_extract %d\n", ret);
        return ret;
    }

    return ROCKFACE_RET_SUCCESS;
}
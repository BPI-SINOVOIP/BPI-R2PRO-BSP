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
#define True 1
#define False 0

#define COUNT_FILTER 2				// ir set to 0
#define CAPTURE_OUT_PATH "out"

typedef void (mem_destroy_h)(void *image);
struct image_wrapper {
    uint32_t nrefs;     // Number of references
	void *image;
	mem_destroy_h *dh;  // Destroy handler
};

struct image_face {
	int id;
	struct image_wrapper *wrapper;
	rockx_face_quality_result_t quality;
	int left;
	int top;
	int right;
	int bottom;
	int detected;		//to mark a face detected or leaved
	unsigned int count;
};

#define MAX_FACE_NUM 50
struct image_face g_faces[MAX_FACE_NUM] = {0};

struct image_wrapper *image_wrapper_alloc(void *image, mem_destroy_h *dh) {
    struct image_wrapper *m;

    m = (struct image_wrapper *)malloc(sizeof(*m));
    if (!m)
        return NULL;

    m->nrefs = 1;
    m->image = image;
	m->dh = dh;
    return m;
}

void image_wrapper_ref(struct image_wrapper *m) {
    ++m->nrefs;
}

void *image_wrapper_deref(struct image_wrapper *m) {
    if (!m)
        return NULL;

    if (--m->nrefs > 0)
        return NULL;

    if (m->image && m->dh)
        m->dh(m->image);

    free(m);

    return NULL;
}

//1 good, 0 bad
int face_filter(rockx_face_quality_result_t quality) {
	//filter quality
	if (quality.result != ROCKX_FACE_QUALITY_PASS)
		return 0;
	//filter size, TODO...
	//filter blur, TODO...
	return 1;
}

// 1, new > old
int face_compare(rockx_face_quality_result_t new_quality, rockx_face_quality_result_t old_quality) {
	if (new_quality.face_score * new_quality.det_score > old_quality.face_score * old_quality.det_score)
		return 1;
	else
		return 0;
}

void face_init() {
	memset(g_faces, 0, MAX_FACE_NUM * sizeof(struct image_face));
	for (int i=0; i < MAX_FACE_NUM; i++)
		g_faces[i].id = -1;
}

void face_update(int id,
		struct image_wrapper *wrapper,
		int left,
		int top,
		int right,
		int bottom,
		rockx_face_quality_result_t quality) {
	int i;
	int need_updated = 0;

	//filter bad quality face
	if (face_filter(quality) <= 0)
		return;

	// find face id
	for (i = 0; i < MAX_FACE_NUM; i++) {
		if ( g_faces[i].id == id)
			break;
	}
	// if new, find a slot
	if (i == MAX_FACE_NUM) {
		for (i = 0; i < MAX_FACE_NUM; i++) {
			if (g_faces[i].id < 0)
				break;
		}
		// if full,to do ...
		if (i == MAX_FACE_NUM)
			return;
		g_faces[i].count = 1;
		need_updated = 1;
	} else {
		g_faces[i].count++;
		// id find, compare image quality
        if (face_compare(quality, g_faces[i].quality) > 0)
		   need_updated = 1;
	}
	if (need_updated) {
		g_faces[i].id = id;
		g_faces[i].left = left;
		g_faces[i].top = top;
		g_faces[i].right = right;
		g_faces[i].bottom = bottom;
		g_faces[i].quality = quality;
		if (g_faces[i].wrapper)
            image_wrapper_deref(g_faces[i].wrapper);
		g_faces[i].wrapper = wrapper;
		image_wrapper_ref(wrapper);
	}
}

//for face leaved checking
void face_update_start() {
	for (int i = 0; i < MAX_FACE_NUM; i++) {
		g_faces[i].detected = 0;
	}
}

//for face leaved checking
void face_set_detected(int id) {
	// find face id
	for (int i = 0; i < MAX_FACE_NUM; i++) {
		if ( g_faces[i].id == id)
			g_faces[i].detected = 1;
	}
}

void face_snap(struct image_face face) {
	// prefer to send task to a new thread,TODO...add snap task to list
	rockx_image_t* image = (rockx_image_t*)face.wrapper->image;
    // draw
	char score_str[8];
	memset(score_str, 0, 8);
	snprintf(score_str, 8, "%.3f", face.quality.face_score);
	rockx_image_draw_rect(image, {face.left, face.top}, {face.right, face.bottom}, {255, 0, 0}, 3);
	rockx_image_draw_text(image, score_str, {face.left, face.top-12}, {255, 0, 0}, 3);
    // save image
	static int id = 0;
	char path_str[128];
	id++;
	memset(path_str, 0, 128);
	snprintf(path_str, 128, "%s/out-%d-%d-%.3f-%.3f.jpg", CAPTURE_OUT_PATH, id, face.count, face.quality.face_score, face.quality.det_score);
	printf("snap: id %d, score %.3f, face score %.3f, %s\n", face.id, face.quality.face_score, face.quality.det_score, path_str);
    rockx_image_write(path_str, image);
}

//for face leaved checking
void face_update_end() {
    for (int i = 0; i < MAX_FACE_NUM; i++) {
		if (g_faces[i].detected == 0 && g_faces[i].id >= 0 && g_faces[i].wrapper) {
			// capture if face exists for a while
			if (g_faces[i].count > COUNT_FILTER) {
				face_snap(g_faces[i]);
				g_faces[i].count = 0;
			}
			image_wrapper_deref(g_faces[i].wrapper);
			g_faces[i].wrapper = NULL;
			g_faces[i].id = -1;
		}
	}
}

void image_release(void *image) {
   rockx_image_release((rockx_image_t *)image); 
   free(image);
}

int main(int argc, char** argv) {

    rockx_ret_t ret;
    rockx_handle_t face_det_handle;
    struct timeval tv;

    rockx_handle_t face_landmark_handle;
    rockx_handle_t object_track_handle;

	char *image_dir = argv[1];

    // create a face detection handle
    ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION_V2_HORIZONTAL, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
    }

    ret = rockx_create(&face_landmark_handle, ROCKX_MODULE_FACE_LANDMARK_5, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("init ROCKX_MODULE_FACE_LANDMARK_5 error %d\n", ret);
    }

    ret= rockx_create(&object_track_handle, ROCKX_MODULE_OBJECT_TRACK, nullptr, 0);
    if (ret != ROCKX_RET_SUCCESS) {
        printf("%s %d init rockx module :%d error %d\n",__func__,__LINE__, ROCKX_MODULE_OBJECT_TRACK, ret);
        return -1;
    }

	face_init();
	while (1) {
		// read image
		char img_path[256] = {0};
		static int id = 0;
		id++;
		snprintf(img_path, 256, "%s/%d.png", image_dir, id);
		printf("processing %s\n", img_path);

		rockx_image_t *input_image = (rockx_image_t *)malloc(sizeof(rockx_image_t));
		ret = rockx_image_read(img_path, input_image, 1);
		if (ret < 0) {
			printf("rockx_image_read %s failed\n", img_path);
			break;
		}
		uint32_t img_width = input_image->width;
		uint32_t img_height = input_image->height;

		rockx_object_array_t face_array;
		memset(&face_array, 0, sizeof(rockx_object_array_t));

		// detect face
		ret = rockx_face_detect(face_det_handle, input_image, &face_array, nullptr);
		if (ret != ROCKX_RET_SUCCESS) {
			printf("rockx_face_detect error %d\n", ret);
			return -1;
		}

		rockx_object_array_t out_array;
		memset(&out_array, 0, sizeof(rockx_object_array_t));
		ret = rockx_object_track(object_track_handle, input_image->width, input_image->height,8,&face_array, &out_array);
		if (ret != ROCKX_RET_SUCCESS) {
			printf("%s %d rockx_object_track error %d\n",__FILE__,__LINE__,ret);
		}

		struct image_wrapper *wrapper = image_wrapper_alloc((void *)input_image, image_release);

		rockx_face_quality_config_t face_quality_config;
		rockx_face_quality_config_init(&face_quality_config);

		face_update_start();
		// process result
		for (int i = 0; i < out_array.count; i++) {
			int id = out_array.object[i].id;
			int left = out_array.object[i].box.left;
			int top = out_array.object[i].box.top;
			int right = out_array.object[i].box.right;
			int bottom = out_array.object[i].box.bottom;
			float score = out_array.object[i].score;
			if(left < 0 || top < 0 || right >= img_width || bottom >= img_height){
				printf("skip out of image, id=%d box=(%d %d %d %d) score=%f\n",id, left, top, right, bottom, score);
				continue;
			}

			rockx_face_quality_result_t quality_result;
			ret = rockx_face_quality(face_landmark_handle, input_image, &out_array.object[i], &face_quality_config, &quality_result);
			if (ret != ROCKX_RET_SUCCESS) {
				printf("rockx_face_filter error %d\n", ret);
			}
			printf("id %d: quality %d, score %.3f,face score %.3f\n", id, quality_result.result, quality_result.face_score, score);

			//update face cache buffer
			face_update(id, wrapper, left, top, right, bottom, quality_result);
			face_set_detected(id);// for face leaved checking
		}
		face_update_end(); //check face leaved

		image_wrapper_deref(wrapper);
	}
	// release
	rockx_destroy(face_det_handle);
	rockx_destroy(face_landmark_handle);
	rockx_destroy(object_track_handle);
}

#ifndef __TRACK_C_LINK_C_H
#define __TRACK_C_LINK_C_H
#include "stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct
{
    int x;
    int y;
    int width;
    int height;
}
Rect_T;

typedef struct
{
    Rect_T r;
	int obj_class;
	int id;
}
object_T;


void object_track(int maxTrackLifetime, int track_num_input, object_T* object_input, int* track_num_output, object_T* object_output, int width, int height);
#ifdef __cplusplus
}
#endif
#endif
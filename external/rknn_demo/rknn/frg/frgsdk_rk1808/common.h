#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "rknn_runtime.h"

#ifdef __linux__
#include <sys/time.h>
#endif

#define MAX_FACE_LANDMARK_NUM  10

enum PixelType {
    PIXEL_RGB       = 1,
    PIXEL_NV12      = (1 << 2),
};

typedef struct _RkImage {
  char *pixels;
  int type;
  int width;
  int height;
}RkImage;

typedef struct _RkRect {
    int x;
    int y;
    int width;
    int height;
}RkRect;

struct RkPoint {
    float x;
    float y;
};

struct RkFace {
  char name[10];
  int id; 
  float grade;
  int is_living;
  RkRect rect;
  struct RkPoint landmarks[MAX_FACE_LANDMARK_NUM];
};

struct face_group {
    int count;
    struct RkFace faces[100];
};

#ifdef __cplusplus
}
#endif



#endif

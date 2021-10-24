#ifndef _SSD_H_
#define _SSD_H_

#include <minigui/common.h>
#include "ssd.h"

#define IMG_CHANNEL         3
#define MODEL_INPUT_SIZE    300
#define NUM_RESULTS         1917
#define NUM_CLASS			91

#define Y_SCALE  10.0f
#define X_SCALE  10.0f
#define H_SCALE  5.0f
#define W_SCALE  5.0f

int loadLabelName(const char* locationFilename, char* labels[]);
int loadBoxPriors(const char* locationFilename, float (*boxPriors)[NUM_RESULTS]);
int postProcessSSD(float * predictions, float *output_classes, int width,
                   int heigh, struct ssd_group *group);

#endif

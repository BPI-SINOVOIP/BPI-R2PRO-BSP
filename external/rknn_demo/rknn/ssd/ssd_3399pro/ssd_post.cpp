/*
* @Author: lhp
* @Date:   2018-12-08 12:38:28
* @Last Modified by:   WZY
* @Last Modified time: 2019-02-15 14:25:31
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <math.h>

#include "rknn_api.h"
#include "ssd.h"
#include "ssd_post.h"
#include "rknn_msg.h"
#include "tracker/track_c_link_c++.h"

using namespace std;

float MIN_SCORE = 0.4f;
float NMS_THRESHOLD = 0.45f;

#define MAX_OUTPUT_NUM 100
object_T object_input[MAX_OUTPUT_NUM];
object_T object_output[MAX_OUTPUT_NUM];

#define BOX_PRIORS_TXT_PATH "/usr/share/rknn_demo/box_priors.txt"
#define LABEL_NALE_TXT_PATH "/usr/share/rknn_demo/coco_labels_list.txt"

int loadLabelName(string locationFilename, string* labels) {
    ifstream fin(locationFilename);
    string line;
    int lineNum = 0;
    while(getline(fin, line))
    {
        labels[lineNum] = line;
        lineNum++;
    }
    return 0;
}

int loadCoderOptions(string locationFilename, float (*boxPriors)[NUM_RESULTS])
{
    const char *d = ", ";
    ifstream fin(locationFilename);
    string line;
    int lineNum = 0;
    while(getline(fin, line))
    {
        char *line_str = const_cast<char *>(line.c_str());
        char *p;
        p = strtok(line_str, d);
        int priorIndex = 0;
        while (p) {
            float number = static_cast<float>(atof(p));
            boxPriors[lineNum][priorIndex++] = number;
            p=strtok(nullptr, d);
        }
        if (priorIndex != NUM_RESULTS) {
            return -1;
        }
        lineNum++;
    }
    return 0;

}

float CalculateOverlap(float xmin0, float ymin0, float xmax0, float ymax0, float xmin1, float ymin1, float xmax1, float ymax1) {
    float w = max(0.f, min(xmax0, xmax1) - max(xmin0, xmin1));
    float h = max(0.f, min(ymax0, ymax1) - max(ymin0, ymin1));
    float i = w * h;
    float u = (xmax0 - xmin0) * (ymax0 - ymin0) + (xmax1 - xmin1) * (ymax1 - ymin1) - i;
    return u <= 0.f ? 0.f : (i / u);
}

float expit(float x) {
    return (float) (1.0 / (1.0 + exp(-x)));
}

void decodeCenterSizeBoxes(float* predictions, float (*boxPriors)[NUM_RESULTS]) {

    for (int i = 0; i < NUM_RESULTS; ++i) {
        float ycenter = predictions[i*4+0] / Y_SCALE * boxPriors[2][i] + boxPriors[0][i];
        float xcenter = predictions[i*4+1] / X_SCALE * boxPriors[3][i] + boxPriors[1][i];
        float h = (float) exp(predictions[i*4 + 2] / H_SCALE) * boxPriors[2][i];
        float w = (float) exp(predictions[i*4 + 3] / W_SCALE) * boxPriors[3][i];

        float ymin = ycenter - h / 2.0f;
        float xmin = xcenter - w / 2.0f;
        float ymax = ycenter + h / 2.0f;
        float xmax = xcenter + w / 2.0f;

        predictions[i*4 + 0] = ymin;
        predictions[i*4 + 1] = xmin;
        predictions[i*4 + 2] = ymax;
        predictions[i*4 + 3] = xmax;
    }
}

int scaleToInputSize(float * outputClasses, int (*output)[NUM_RESULTS], int numClasses)
{
    int validCount = 0;
    // Scale them back to the input size.
    for (int i = 0; i < NUM_RESULTS; ++i) {
        float topClassScore = static_cast<float>(-1000.0);
        int topClassScoreIndex = -1;

        // Skip the first catch-all class.
        for (int j = 1; j < numClasses; ++j) {
            float score = expit(outputClasses[i*numClasses+j]);
            if (score > topClassScore) {
                topClassScoreIndex = j;
                topClassScore = score;
            }
        }

        if (topClassScore >= MIN_SCORE) {
            output[0][validCount] = i;
            output[1][validCount] = topClassScoreIndex;
            ++validCount;
        }
    }

    return validCount;
}

int nms(int validCount, float* outputLocations, int (*output)[NUM_RESULTS])
{
    for (int i=0; i < validCount; ++i) {
        if (output[0][i] == -1) {
            continue;
        }
        int n = output[0][i];
        for (int j=i + 1; j<validCount; ++j) {
            int m = output[0][j];
            if (m == -1) {
                continue;
            }
            float xmin0 = outputLocations[n*4 + 1];
            float ymin0 = outputLocations[n*4 + 0];
            float xmax0 = outputLocations[n*4 + 3];
            float ymax0 = outputLocations[n*4 + 2];

            float xmin1 = outputLocations[m*4 + 1];
            float ymin1 = outputLocations[m*4 + 0];
            float xmax1 = outputLocations[m*4 + 3];
            float ymax1 = outputLocations[m*4 + 2];

            float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

            if (iou >= NMS_THRESHOLD) {
                output[0][j] = -1;
            }
        }
    }

    return 0;
}

extern "C" int postProcessSSD(rknn_output *outputs,
                              rknn_tensor_attr *outputs_attr,
                              ssd_group *group, int ctx, int w, int h)
{
    static float boxPriors[4][NUM_RESULTS];
    static string labels[91];
    const int output_elems1 = NUM_RESULTS * 4;
    const int output_size1 = output_elems1 * sizeof(float);
    const int output_index1 = 0;    // node name "concat"

    const int output_elems2 = NUM_RESULTS * NUM_CLASSES;
    const int output_size2 = output_elems2 * sizeof(float);
    const int output_index2 = 1;    // node name "concat_1"

    /* load label and boxPriors */
    static int init_flag = -1;
    if (init_flag != 0) {
        loadLabelName(LABEL_NALE_TXT_PATH, labels);
        loadCoderOptions(BOX_PRIORS_TXT_PATH, boxPriors);
        init_flag = 0;
    }

    float *predictions = (float*)(outputs[0].buf);
    float *outputClasses = (float*)(outputs[1].buf);
    int output[2][NUM_RESULTS];
    /* transform */
    decodeCenterSizeBoxes(predictions, boxPriors);

    int validCount = scaleToInputSize(outputClasses, output, NUM_CLASSES);
    //printf("validCount: %d\n", validCount);
    if (validCount > 100) {
        printf("validCount too much !!\n");
        return -1;
    }

    /* detect nest box */
    nms(validCount, predictions, output);
    int last_count = 0;
    group->count = 0;
    int track = 1;
    int maxTrackLifetime = 3;
    if(!track) {
        /* box valid detect target */
        for (int i = 0; i < validCount; ++i) {
            if (output[0][i] == -1) {
                continue;
            }
            int n = output[0][i];
            int topClassScoreIndex = output[1][i];

            int x1 = static_cast<int>(predictions[n * 4 + 1] * w);
            int y1 = static_cast<int>(predictions[n * 4 + 0] * h);
            int x2 = static_cast<int>(predictions[n * 4 + 3] * w);
            int y2 = static_cast<int>(predictions[n * 4 + 2] * h);
            if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0)
                continue;

            group->objects[last_count].select.left   = x1;
            group->objects[last_count].select.top    = y1;
            group->objects[last_count].select.right  = x2;
            group->objects[last_count].select.bottom = y2;
            string label = labels[topClassScoreIndex];
            memcpy(group->objects[last_count].name, (const void *)label.c_str(), 10);
            //printf("%s\t@ (%d, %d, %d, %d)\n", label, x1, y1, x2, y2);
            last_count++;
        }
    }
    else
    {
        int track_num_input = 0;
        for (int i = 0; i < validCount; ++i) {
            if (output[0][i] == -1) {
                continue;
            }
            int n = output[0][i];
            int topClassScoreIndex = output[1][i];

            int x1 = static_cast<int>(predictions[n * 4 + 1] * w);
            int y1 = static_cast<int>(predictions[n * 4 + 0] * h);
            int x2 = static_cast<int>(predictions[n * 4 + 3] * w);
            int y2 = static_cast<int>(predictions[n * 4 + 2] * h);
            if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0)
                  continue;
            object_input[track_num_input].r.x = x1;
            object_input[track_num_input].r.y = y1;
            object_input[track_num_input].r.width = x2 -x1;
            object_input[track_num_input].r.height = y2 -y1;
            object_input[track_num_input].obj_class= topClassScoreIndex;
            track_num_input++;
        }

        int track_num_output = 0;
        object_track(maxTrackLifetime, track_num_input, object_input,
                     &track_num_output, object_output, w, h);
        for (int i = 0; i < track_num_output; ++i) {
            int topClassScoreIndex = object_output[i].obj_class;
            int x1 = (int)(object_output[i].r.x);
            int y1 = (int)(object_output[i].r.y);
            int x2 = (int)(object_output[i].r.x +object_output[i].r.width);
            int y2 = (int)(object_output[i].r.y +object_output[i].r.height);

            if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0)
                continue;
            group->objects[last_count].select.left   = x1;
            group->objects[last_count].select.top    = y1;
            group->objects[last_count].select.right  = x2;
            group->objects[last_count].select.bottom = y2;
            string label = labels[topClassScoreIndex];
            memcpy(group->objects[last_count].name, (const void *)label.c_str(), 10);
            //printf("%s\t@ (%d, %d, %d, %d)\n", group->objects[i].name, x1, y1, x2 -x1, y2 -y1);
            last_count++;
        }
    }
    group->count = last_count;
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <rga/RgaApi.h>
#include <linux/videodev2.h>
#include "rknn_api.h"
#include "rknn_msg.h"
#include "yuv.h"
#include "ssd.h"
#include "ssd_post.h"
#include "v4l2camera.h"

#define HAVE_RKNN_RUNTIME_H
#include "buffer.h"

#define DEV_NAME      "/dev/video10"
#define MODEL_NAME    "/usr/share/rknn_demo/mobilenet_ssd.rknn"

#define SRC_W         640
#define SRC_H         480
#define SRC_FPS       30
#define SRC_BPP       24
#define DST_W         300
#define DST_H         300
#define DST_BPP       24

#define SRC_V4L2_FMT  V4L2_PIX_FMT_YUV420
#define SRC_RKRGA_FMT RK_FORMAT_YCbCr_420_P
#define DST_RKRGA_FMT RK_FORMAT_RGB_888

float g_fps;
bo_t g_rga_buf_bo;
int g_rga_buf_fd;
char *g_mem_buf;
rknn_context ctx;
struct ssd_group g_ssd_group[2];
volatile int send_count;

extern int yuv_draw(char *src_ptr, int src_fd, int format, int src_w, int src_h);
extern void ssd_paint_object_msg();
extern void ssd_paint_fps_msg();

static int cur_group = 0;
inline struct ssd_group* ssd_get_ssd_group()
{
    return &g_ssd_group[cur_group];
}

inline float ssd_get_fps()
{
    return g_fps;
}

#define CHECK_STATUS(func) do { \
    status = func; \
    if (status < 0)  { \
        goto final; \
    }   \
} while(0)

long cal_fps(float *cal_fps)
{
    static int count = 0;
    static float fps = 0;
    static long begin_time = 0;
    static long end_time = 0;

    count++;
    if (begin_time == 0)
        begin_time = getCurrentTime();
    if (count >= 10) {
        end_time = getCurrentTime();
        fps = (float)10000 / (float)(end_time - begin_time);
        begin_time = end_time;
        count = 0;
        *cal_fps = fps;
    }
    ssd_paint_fps_msg();
}

rknn_input inputs[1];
//rknn_output outputs[2];
rknn_tensor_attr outputs_attr[2];
int postProcessSSD(rknn_output *outputs,
                   rknn_tensor_attr *outputs_attr,
                   struct ssd_group *group, int ctx, int w, int h);
int ssd_rknn_process(char* in_data, int w, int h, int c)
{
    int ret;
    int out_fd;
    const int input_index = 0;
    const int output_elems1 = NUM_RESULTS * 4;
    const uint32_t output_size1 = output_elems1 * sizeof(float);
    const int output_index1 = 0;    // node name "concat"

    const int output_elems2 = NUM_RESULTS * NUM_CLASSES;
    const uint32_t output_size2 = output_elems2 * sizeof(float);
    const int output_index2 = 1;    // node name "concat_1"
    cal_fps(&g_fps);

    inputs[0].index = input_index;
    inputs[0].buf = in_data;
    inputs[0].size = w * h * c / 8;
    inputs[0].pass_through = false;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    ret = rknn_inputs_set(ctx, 1, inputs);
    if(ret < 0) {
        printf("rknn_input_set fail! ret=%d\n", ret);
        return ret;
    }

    ret = rknn_run(ctx, NULL);
    if (ret < 0) {
        printf("run ret = %d\n", ret);
        return ret;
    }

    rknn_output * outputs = (rknn_output *)calloc(2, sizeof(rknn_output));
    outputs->want_float = true;
    outputs->is_prealloc = false;
    (outputs + 1)->want_float = true;
    (outputs + 1)->is_prealloc = false;
    ret = rknn_outputs_get(ctx, 2, outputs, NULL);
    if (ret < 0) {
        printf("output ret = %d\n", ret);
        return ret;
    }
    if(outputs->size != outputs_attr[0].n_elems*sizeof(float) || (outputs + 1)->size != outputs_attr[1].n_elems*sizeof(float))
    {
        printf("rknn_outputs_get fail! get outputs_size = [%d, %d], but expect [%d, %d]!\n",
                outputs->size, (outputs + 1)->size, outputs_attr[0].size, outputs_attr[1].size);
            return -1;
    }

    rknn_msg_send((void *)outputs, (void *)outputs_attr, w, h, &g_ssd_group[cur_group]);
    while(send_count >= 5) {
          printf("sleep now\n");
          usleep(200);
    }
}

void ssd_camera_callback(void *p, int w, int h)
{
    unsigned char* srcbuf = (unsigned char *)p;
    // Send camera data to minigui layer
    yuv_draw(srcbuf, 0, SRC_RKRGA_FMT, w, h);

    YUV420toRGB24_RGA(SRC_RKRGA_FMT, srcbuf, w, h,
                      DST_RKRGA_FMT, g_rga_buf_fd, DST_W, DST_H);

    memcpy(g_mem_buf, g_rga_buf_bo.ptr, DST_W * DST_H * DST_BPP / 8);

    // static FILE *fp = NULL;
    // if (fp == NULL) {
    //     fp = fopen("/tmp/test.rgb", "w+");
    //     fwrite(g_mem_buf, 1, DST_W * DST_H * DST_BPP / 8, fp);
    //     fclose(fp);
    // }
    ssd_rknn_process(g_mem_buf, DST_W, DST_H, DST_BPP);
}

int ssd_post(void *flag)
{
    int width;
    int heigh;
    rknn_output *outputs;
    rknn_tensor_attr *outputs_attr;
    struct ssd_group *group;

    while(*(int *)flag) {
        rknn_msg_recv((void **)&outputs, (void **)&outputs_attr, &width, &heigh, (void *)&group);
        send_count--;
        group = &g_ssd_group[!cur_group];
        postProcessSSD(outputs, outputs_attr, group, ctx, width, heigh);
        cur_group = !cur_group;
        rknn_outputs_release(ctx, 2, outputs);
        free(outputs);
        ssd_paint_object_msg();
    }
}

int ssd_run(void *flag)
{
    int len;
    int ret = 0;
    void *model= NULL;

    model = load_model(MODEL_NAME, &len);
    if (model == NULL)
        goto final;
    printf("read model:%s, len:%d\n", MODEL_NAME, len);
    ret = rknn_init(&ctx, model, len, RKNN_FLAG_PRIOR_MEDIUM);
    if (ret < 0) {
        printf("int error = %s\n", strerror(errno));
        goto final;
    }

    outputs_attr[0].index = 0;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[0]), sizeof(outputs_attr[0]));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        goto final;
    }

    outputs_attr[1].index = 1;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[1]), sizeof(outputs_attr[1]));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        goto final;
    }

    cameraRun(DEV_NAME, SRC_W, SRC_H, SRC_FPS, SRC_V4L2_FMT,
              ssd_camera_callback, (int*)flag);

final:
    rknn_destroy(ctx);
    free(model);
    return ret;
}

int ssd_init(int arg)
{
    rknn_msg_init();
    buffer_init(DST_W, DST_H, DST_BPP, &g_rga_buf_bo, &g_rga_buf_fd);
    if (!g_mem_buf)
        g_mem_buf = (char *)malloc(DST_W * DST_H * DST_BPP / 8);
}

int ssd_deinit()
{
    if (g_mem_buf)
        free(g_mem_buf);
    buffer_deinit(&g_rga_buf_bo, g_rga_buf_fd);
    rknn_msg_deinit();
}

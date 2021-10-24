#include <stdio.h>
#include <rga/RgaApi.h>
#include <linux/videodev2.h>
#include "buffer.h"
#include "rknn_msg.h"
#include "yuv.h"
#include "ssd.h"
#include "ssd_post.h"
#include "v4l2camera.h"
#include "device_name.h"

#define MODEL_NAME    "/usr/share/rknn_demo/ssd_inception_v2.rknn"

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
volatile int send_count = 0;
volatile int recv_count = 0;
char *dev_name;

extern int yuv_draw(char *src_ptr, int src_fd, int format,
		    int src_w, int src_h);
extern void ssd_paint_object_msg();
extern void ssd_paint_fps_msg();

pthread_mutex_t group_mutex;

static int cur_group = 0;
inline struct ssd_group* ssd_get_ssd_group()
{

    return &g_ssd_group[cur_group];
}

int ssd_group_mutex_init()
{
    pthread_mutex_init(&group_mutex, NULL);
}

int ssd_group_mutex_deinit()
{
    pthread_mutex_destroy(&group_mutex);
}

int ssd_group_mutex_lock()
{
    pthread_mutex_lock(&group_mutex);
}

int ssd_group_mutex_unlock()
{
    pthread_mutex_unlock(&group_mutex);
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

int ssd_rknn_process(char* in_data, int w, int h, int c)
{
    int status = 0;
    int in_size;
    int out_size0;
    int out_size1;
    float *out_data0 = NULL;
    float *out_data1 = NULL;
  //   printf("camera callback w=%d h=%d c=%d\n", w, h, c);

    long runTime1 = getCurrentTime();

    long setinputTime1 = getCurrentTime();
    // Set Input Data
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = w*h*c/8;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = in_data;

    status = rknn_inputs_set(ctx, 1, inputs);
    if(status < 0) {
        printf("rknn_input_set fail! ret=%d\n", status);
        return -1;
    }
    long setinputTime2 = getCurrentTime();

    // printf("set input time:%0.2ldms\n", setinputTime2-setinputTime1);

    status = rknn_run(ctx, NULL);
    if(status < 0) {
        printf("rknn_run fail! ret=%d\n", status);
        return -1;
    }

    // Get Output
    rknn_output outputs[2];
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    outputs[1].want_float = 1;
    status = rknn_outputs_get(ctx, 2, outputs, NULL);
    if(status < 0) {
        printf("rknn_outputs_get fail! ret=%d\n", status);
        return -1;
    }

    int out0_elem_num = NUM_RESULTS * NUM_CLASS;
    int out1_elem_num = NUM_RESULTS * 4;

    float *output0 = malloc(out0_elem_num*sizeof(float));
    float *output1 = malloc(out1_elem_num*sizeof(float));

    memcpy(output0, outputs[0].buf, out0_elem_num*sizeof(float));
    memcpy(output1, outputs[1].buf, out1_elem_num*sizeof(float));

    rknn_outputs_release(ctx, 2, outputs);

    long runTime2 = getCurrentTime();
    // printf("rknn run time:%0.2ldms\n", runTime2 - runTime1);

    // long postprocessTime1 = getCurrentTime();
    int ret = rknn_msg_send((void *)output1, (void *)output0, w, h, &g_ssd_group[!cur_group]);
    if (ret)
        return -1;
    while(send_count - recv_count >= 5) {
        printf("sleep now \n");
        usleep(2000);
    }
    send_count++;
    // long postprocessTime2 = getCurrentTime();
    // printf("post process time:%0.2ldms\n", postprocessTime2 - postprocessTime1);
}

void ssd_camera_callback(void *p, int w, int h, int *flag)
{
    unsigned char* srcbuf = (unsigned char *)p;
    // Send camera data to minigui layer
    yuv_draw(srcbuf, 0, SRC_RKRGA_FMT, w, h);
    cal_fps(&g_fps);

    if (*flag) {
	YUV420toRGB24_RGA(SRC_RKRGA_FMT, srcbuf, w, h,
			DST_RKRGA_FMT, g_rga_buf_fd, DST_W, DST_H);
	memcpy(g_mem_buf, g_rga_buf_bo.ptr, DST_W * DST_H * DST_BPP / 8);
	ssd_rknn_process(g_mem_buf, DST_W, DST_H, DST_BPP);
    }
}

int ssd_post(void *flag)
{
    int width;
    int heigh;
    float *predictions;
    float *output_classes;
    struct ssd_group *group;

    while(*(int *)flag) {
        if (!rknn_msg_recv((void **)&predictions, (void **)&output_classes, &width, &heigh, (void *)&group))
            recv_count++;
        group = &g_ssd_group[!cur_group];
        // if (group->count > 0 && group->posted > 0)
        // {
        //     if (predictions)
        //         free(predictions);
        //     if (output_classes)
        //         free(output_classes);
        //     printf("throw data\n");
        //     cur_group = !cur_group;
        //     continue;
        // }
        postProcessSSD(predictions, output_classes, width, heigh, group);
        cur_group = !cur_group;
        if (predictions)
            free(predictions);
        if (output_classes)
            free(output_classes);
        ssd_paint_object_msg();
    }
}

int ssd_run(void *flag)
{
    int status = 0;
    int model_len = 0;
    unsigned char* model;

    model = load_model(MODEL_NAME, &model_len);

    status = rknn_init(&ctx, model, model_len, 0);
    if(status < 0) {
        printf("rknn_init fail! ret=%d\n", status);
        return -1;
    }

    // Get Model Input Output Info
    rknn_input_output_num io_num;
    status = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (status != RKNN_SUCC) {
        printf("rknn_query fail! ret=%d\n", status);
        return -1;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    printf("input tensors:\n");
    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        status = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (status != RKNN_SUCC) {
            printf("rknn_query fail! ret=%d\n", status);
            return -1;
        }
       //print_rknn_tensor(&(input_attrs[i]));
    }

    printf("output tensors:\n");
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        status = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (status != RKNN_SUCC) {
            printf("rknn_query fail! ret=%d\n", status);
            return -1;
        }
        //print_rknn_tensor(&(output_attrs[i]));
    }

    // Open Camera and Run
    cameraRun(dev_name, SRC_W, SRC_H, SRC_FPS, SRC_V4L2_FMT,
              ssd_camera_callback, (int*)flag);

    printf("exit cameraRun\n");

    // Release
    if(model) {
        free(model);
    }
    if(ctx > 0) {
        rknn_destroy(ctx);
    }
    return status;
}


int ssd_init(char *name)
{
    dev_name = name;
    rknn_msg_init();
    buffer_init(DST_W, DST_H, DST_BPP, &g_rga_buf_bo,
		&g_rga_buf_fd);

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

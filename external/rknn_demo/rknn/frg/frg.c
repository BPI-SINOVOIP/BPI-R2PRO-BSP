#include <stdio.h>
#include <rga/RgaApi.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include "buffer.h"
#include "rknn_msg.h"
#include "yuv.h"
#include "frg.h"
#include "v4l2camera.h"

#include "frgsdk_rk1808/rk_face_recognize_wrapper.h"

#define DEV_NAME      "/dev/video0"

// rk only support fixed 300x300
#define MODEL_INPUT_WIDTH 300
#define MODEL_INPUT_HEIGHT 300

#define ALIGN_IMAGE_WIDTH 112
#define ALIGN_IMAGE_HEIGHT 112

#define SRC_W         640
#define SRC_H         480
#define SRC_FPS       30
#define SRC_BPP       24
#define DST_W         MODEL_INPUT_WIDTH
#define DST_H         MODEL_INPUT_HEIGHT
#define DST_BPP       24

#define SRC_V4L2_FMT  V4L2_PIX_FMT_YUV420
#define SRC_RKRGA_FMT RK_FORMAT_YCbCr_420_P
#define DST_RKRGA_FMT RK_FORMAT_RGB_888

static float g_fps;
static bo_t g_rga_buf_bo;
static int g_rga_buf_fd = -1;
static char *g_mem_buf = NULL;
char *dev_name;
// static rknn_context ctx;
// static volatile int send_count;

extern int yuv_draw(char *src_ptr, int src_fd, int format,
		    int src_w, int src_h);
extern void frg_paint_object_msg();
extern void frg_paint_fps_msg();
extern void frg_post_quit_msg();

static pthread_mutex_t group_mutex;
static struct frg_group* out_group = NULL;

static rknn_context ctx_detect;
static rknn_context ctx_align;
static rknn_context ctx_recognize;

struct frg_group data_base_group;

static int group_mutex_init()
{
    return pthread_mutex_init(&group_mutex, NULL);
}

static int group_mutex_deinit()
{
    return pthread_mutex_destroy(&group_mutex);
}

static int group_mutex_lock()
{
    return pthread_mutex_lock(&group_mutex);
}

static int group_mutex_unlock()
{
    return pthread_mutex_unlock(&group_mutex);
}

inline float frg_get_fps()
{
    return g_fps;
}

inline struct frg_group* frg_get_group()
{
    struct frg_group* group = NULL;
    group_mutex_lock();
    group = out_group;
    out_group = NULL;
    group_mutex_unlock();

    return group;
}

static inline void frg_set_group(struct frg_group* group)
{
    group_mutex_lock();
    free_frg_group(out_group, 1);
    out_group = group;
    group_mutex_unlock();
}

#define CHECK_STATUS(func) do { \
    status = func; \
    if (status < 0)  { \
        goto final; \
    }   \
} while(0)

static long cal_fps(float *cal_fps, long npu_time)
{
    static int count = 0;
    static float fps = 0;
    static long elapse_time = 0;

    count++;
    elapse_time += npu_time;
    if (count == 10) {
        fps = (elapse_time > 0) ? (float)10000 / (float)(elapse_time) : 0;
        count = 0;
        elapse_time = 0;
        *cal_fps = fps;
        frg_paint_fps_msg();
    }
}

static void free_frg_object(struct frg_object *obj) {
    if (obj->rgb) {
        free(obj->rgb);
        obj->rgb = NULL;
    }
    if (obj->identifier) {
        free(obj->identifier);
        obj->identifier = NULL;
    }
}

static int copy_frg_object(struct frg_object* dst_obj, struct frg_object* src_obj) {
    void *rgb = NULL;
    void *iden = NULL;

    if (!src_obj || !dst_obj)
        return -1;
    if (dst_obj->rgb || dst_obj->identifier)
        return -1;
    if (src_obj->rgb_len > 0) {
        rgb = malloc(src_obj->rgb_len);
        if (!rgb)
            return -1;
        memcpy(rgb, src_obj->rgb, src_obj->rgb_len);
    }
    if (src_obj->iden_len > 0) {
        iden = malloc(src_obj->iden_len);
        if (!iden) {
            if(rgb) free(rgb);
            return -1;
        }
        memcpy(iden, src_obj->identifier, src_obj->iden_len);
    }
    *dst_obj = *src_obj;
    dst_obj->rgb = rgb;
    dst_obj->identifier = iden;

    return 0;
}

void free_frg_group(struct frg_group *group, int pfree) {
    int i;
    if (!group)
        return;
    for(i = 0; i < group->count; i++) {
        free_frg_object(&group->objects[i]);
    }
    if (pfree)
        free(group);
    else
        group->count = 0;
}

static int frg_rknn_process(char* in_data)
{
    int status = 0;
    RkImage img_src,img_align;
    int img_size;
    struct face_group group;
    struct frg_group *new_group = NULL;
    struct frg_object new_object;

    long npu_consume_time = 0, cpu_consume_time = 0;
    long t1, t2;

    img_src.width = MODEL_INPUT_WIDTH;
    img_src.height = MODEL_INPUT_HEIGHT;
    img_src.type = PIXEL_RGB;
    img_size = img_src.width * img_src.height * 3;
    img_src.pixels = in_data;

    img_align.width = ALIGN_IMAGE_WIDTH;
    img_align.height = ALIGN_IMAGE_HEIGHT;
    img_align.type = PIXEL_RGB;
    img_align.pixels = (char*)malloc(img_align.width * img_align.height * 3 * sizeof(char));
    if (img_align.pixels == NULL) {
        printf("img_align.pixels malloc error\n");
        return -1;
    }
    t1 = getCurrentTime();
    //face detect
    rkFaceDetect(ctx_detect, &img_src);     // npu behavior
    memset(&group, 0, sizeof(group));
    // printf("%s: %d\n", __FUNCTION__, __LINE__);
    rkFacePostProcess(ctx_detect, &group);  // cpu behavior
    t2 = getCurrentTime();
    npu_consume_time += (t2 - t1);
    // printf("detect time : %0.2ldms\n", t2 - t1);
    if(group.count > 0) {
        int ret;
        int i, j;
        int area = 0;
        struct RkFace reg_face;
        rknn_output outputs[1];
        uint8_t buf_512x4[512 * 4];
        static long pre_record_time = 0, cur_time;

        cur_time = t2;
        memset(outputs, 0, sizeof(outputs));
        outputs[0].want_float = 1;
        outputs[0].is_prealloc = 1;
        outputs[0].index = 0;
        outputs[0].buf = buf_512x4;
        outputs[0].size = 512 * 4;

        // many people, get the closest one
        for(i = 0; i < group.count; i++) {
            struct RkFace *face = &group.faces[i];
            int wxh = face->rect.width * face->rect.height;
            if (wxh > area) {
                area = wxh;
                reg_face = *face;
            }
        }
        new_object.x = reg_face.rect.x;
        new_object.y = reg_face.rect.y;
        new_object.w = reg_face.rect.width;
        new_object.h = reg_face.rect.height;
        new_object.rgb = NULL;
        new_object.identifier = NULL;
        new_object.index = -1;
        new_object.match = 0;

        t1 = t2;
        rkFaceAlign(ctx_align,&img_src,&img_align,&reg_face);
        t2 = getCurrentTime();
        npu_consume_time += (t2 - t1);
        //printf("align time : %0.2ldms\n", t2 - t1);

        t1 = t2;
        rkFaceFeature(ctx_recognize, &img_align, &reg_face);
        //printf("%s: %d\n", __FUNCTION__, __LINE__);
        ret = rknn_outputs_get(ctx_recognize, 1, outputs, NULL);
        if(ret < 0) {
            free(img_align.pixels);
            printf("rknn_outputs_get fail! ret=%d\n", ret);
            return -1;
        }
        t2 = getCurrentTime();
        npu_consume_time += (t2 - t1);
        //printf("get face feature time : %0.2ldms\n", t2 - t1);

        new_group = (struct frg_group*)malloc(sizeof(*new_group));
        if (!new_group) {
            free(img_align.pixels);
            printf("new_group malloc fail!\n");
            return -1;
        }
        memset(new_group, 0, sizeof(*new_group));
        new_group->count++;
        new_group->rt_image_width = MODEL_INPUT_WIDTH;
        new_group->rt_image_height = MODEL_INPUT_HEIGHT;
        new_group->objects[0] = new_object;
        // get match
        for (i = 0, j = 1; i < data_base_group.count; i++) {
            struct frg_object *frg_obj = &data_base_group.objects[i];
            float match_rate = rkFeatureIdentify((float*)outputs[0].buf,
                                                 frg_obj->identifier, 512);
            // printf("index %d, match_rate %f\n", i, match_rate);
            if (copy_frg_object(&new_group->objects[j], frg_obj) < 0)
                continue;
            new_group->objects[j].index = i;
            new_group->objects[j].match = match_rate;
            j++;
            new_group->count++;
        }

        if (cur_time - pre_record_time > 1000) {
            static int database_index = 0;
            void *identifier = malloc(512 * 4);
            if (identifier) {
                struct frg_object *frg_obj = &data_base_group.objects[database_index];
                free_frg_object(frg_obj);
                memset(frg_obj, 0, sizeof(*frg_obj));
                frg_obj->x = 0;
                frg_obj->y = 0;
                frg_obj->w = img_align.width;
                frg_obj->h = img_align.height;
                frg_obj->rgb = img_align.pixels;
                frg_obj->rgb_len = img_align.width * img_align.height * 3;
                img_align.pixels = NULL;
                memcpy(identifier, outputs[0].buf, 512 * 4);
                frg_obj->identifier = identifier;
                frg_obj->iden_len = 512 * 4;
                frg_obj->index = database_index;
                frg_obj->match = 0.0f;
                data_base_group.count++;
                if (data_base_group.count > MAX_DATABASE_FACE_NUM)
                    data_base_group.count = MAX_DATABASE_FACE_NUM;
                // printf("detect new face, count: %d, i: %d; %p\n",
                //         data_base_group.count, database_index, frg_obj->identifier);
                database_index++;
                database_index = (database_index % MAX_DATABASE_FACE_NUM);
                pre_record_time = cur_time;
            }
        }
        frg_set_group(new_group);
        cal_fps(&g_fps, npu_consume_time);
        // printf("npu process time:%0.2ldms\n", npu_consume_time);
    }

    rknn_msg_send(NULL, NULL, 0, 0, new_group);
    if (img_align.pixels)
        free(img_align.pixels);
    return 0;
}

static void frg_camera_callback(void *p, int w, int h)
{
    unsigned char* srcbuf = (unsigned char *)p;
    // Send camera data to minigui layer
    yuv_draw(srcbuf, 0, SRC_RKRGA_FMT, w, h);
    // convert to rgb24 300x300
    YUV420toRGB24_RGA(SRC_RKRGA_FMT, srcbuf, w, h,
                      DST_RKRGA_FMT, g_rga_buf_fd, DST_W, DST_H);
    memcpy(g_mem_buf, g_rga_buf_bo.ptr, DST_W * DST_H * DST_BPP / 8);
    frg_rknn_process(g_mem_buf);
}

int frg_post(void *flag)
{
    int width;
    int height;
    void *predictions = NULL;
    void *output_classes = NULL;
    struct frg_group *group = NULL;

    while(*((int *)flag)) {
        rknn_msg_recv(&predictions, &output_classes, &width, &height, (void *)&group);
        frg_paint_object_msg();
    }
    printf("exit rknn_msg_recv\n");
    frg_post_quit_msg();
    return 0;
}

int frg_run(void *flag)
{
    int status = 0;

    // Init
    int model_len = 0;
    unsigned char* model_detect = NULL;
    unsigned char* model_align = NULL;
    unsigned char* model_recognize = NULL;

    memset(&data_base_group, 0, sizeof(data_base_group));

    model_detect = load_model(MODEL_DETECT, &model_len);
    status = rknn_init(&ctx_detect, model_detect, model_len, 0);
    if(status < 0) {
        printf("rknn_init MODEL_DETECT fail! ret=%d\n", status);
        return -1;
    }

    model_len = 0;
    model_align = load_model(MODEL_ALIGN, &model_len);
    status = rknn_init(&ctx_align, model_align, model_len, 0);
    if(status < 0) {
        printf("rknn_init MODEL_ALIGN fail! ret=%d\n", status);
        return -1;
    }

    model_len = 0;
    model_recognize = load_model(MODEL_RECOGNIZE, &model_len);
    status = rknn_init(&ctx_recognize, model_recognize, model_len, 0);
    if(status < 0) {
        printf("rknn_init MODEL_RECOGNIZE fail! ret=%d\n", status);
        return -1;
    }

    // Open Camera and Run
    cameraRun(dev_name, SRC_W, SRC_H, SRC_FPS, SRC_V4L2_FMT,
              frg_camera_callback, (int*)flag);
    rknn_msg_send(NULL, NULL, 0, 0, NULL);
    usleep(1000 * 1000);
    printf("exit cameraRun\n");

out:
    // Release
    if (model_detect) {
        free(model_detect);
    }
    if (model_align) {
        free(model_align);
    }
    if (model_recognize) {
        free(model_recognize);
    }

    if (ctx_detect)
        rknn_destroy(ctx_detect);
    if (ctx_align)
        rknn_destroy(ctx_align);
    if (ctx_recognize)
        rknn_destroy(ctx_recognize);

    free_frg_group(&data_base_group, 0);

    return status;
}


int frg_init(char *name)
{
    dev_name = name;
    rknn_msg_init();
    buffer_init(DST_W, DST_H, DST_BPP, &g_rga_buf_bo,
		&g_rga_buf_fd);

    if (!g_mem_buf)
        g_mem_buf = (char *)malloc(DST_W * DST_H * DST_BPP / 8);
}

int frg_deinit()
{
    if (g_mem_buf)
        free(g_mem_buf);
    buffer_deinit(&g_rga_buf_bo, g_rga_buf_fd);
    rknn_msg_deinit();
}

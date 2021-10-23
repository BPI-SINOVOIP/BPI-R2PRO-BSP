#if ENABLE_RGA

#include <pthread.h>
#include <fcntl.h>

#include "shadow_rga.h"
#include "../drmcon/drm_display.h"

#define USER_BO_SEQ 2
#define BO_NUM 3 // 0,1: for switch buffer, 2: for user rga process
#define VIDEO_BPP 16
static bo_t g_bo;
static int g_src_fd;

typedef struct {
    pthread_mutex_t mtx;
    int front_fb;
    int width;
    int height;
    bo_t bo[BO_NUM];
    int fb[BO_NUM];
} video_offscreen_fb;

static video_offscreen_fb video_fb;

extern int c_RkRgaFree(bo_t *bo_info);

extern int get_g_rcScr_right(void);
extern int get_g_rcScr_bottom(void);

void *shadow_rga_g_bo_ptr(void)
{
    return g_bo.ptr;
}

#define DIVIDE_SUM(a, b) (((a)/(b)) + ((a)%(b)))

int shadow_rga_init(int size)
{
    int ret;
    struct drm_bo bo;
    int width, height, bpp;
    int i;

    pthread_mutex_init(&video_fb.mtx, NULL);
    video_fb.front_fb = 0;
    video_fb.fb[0] = video_fb.fb[1] = -1;

    getdrmdispinfo(&bo, &width, &height);
    getdrmdispbpp(&bpp);

    ret = c_RkRgaInit();
    if (ret) {
        printf("c_RkRgaInit error : %s\n", strerror(errno));
        return ret;
    }
    height = DIVIDE_SUM(DIVIDE_SUM(size, bpp), width) * 8;
    ret = c_RkRgaGetAllocBuffer(&g_bo, width, height, bpp);
    if (ret) {
        printf("c_RkRgaGetAllocBuffer error : %s\n", strerror(errno));
        return ret;
    }
    printf("size = %d, g_bo.size = %d\n", size, g_bo.size);
    ret = c_RkRgaGetMmap(&g_bo);
    if (ret) {
        printf("c_RkRgaGetMmap error : %s\n", strerror(errno));
        return ret;
    }
    ret = c_RkRgaGetBufferFd(&g_bo, &g_src_fd);
    if (ret) {
        printf("c_RkRgaGetBufferFd error : %s\n", strerror(errno));
        return ret;
    }

    i = BO_NUM;
    while (i-- > 0) {
        bo_t *cur_bo = &video_fb.bo[i];
        ret = c_RkRgaGetAllocBuffer(cur_bo, width, height, VIDEO_BPP);
        if (ret) {
            printf("c_RkRgaGetAllocBuffer error : %s\n", strerror(errno));
            return ret;
        }
        printf("size = %d, cur_bo->size = %d\n", size, cur_bo->size);
        ret = c_RkRgaGetMmap(cur_bo);
        if (ret) {
            printf("c_RkRgaGetMmap error : %s\n", strerror(errno));
            return ret;
        }
        ret = c_RkRgaGetBufferFd(cur_bo, &video_fb.fb[i]);
        if (ret) {
            printf("c_RkRgaGetBufferFd error : %s\n", strerror(errno));
            return ret;
        }
        video_fb.width = width;
        video_fb.height = height;
        memset(cur_bo->ptr, 16, width * height);
        memset((char*)cur_bo->ptr + width * height, 128, cur_bo->size - width * height);
    }

    return 0;
}

void shadow_rga_exit(void)
{
    int ret;
    int i;
    close(g_src_fd);
    ret = c_RkRgaUnmap(&g_bo);
    if (ret)
        printf("c_RkRgaUnmap error : %s\n", strerror(errno));
    ret = c_RkRgaFree(&g_bo);
    if (ret)
        printf("c_RkRgaFree error : %s\n", strerror(errno));

    i = BO_NUM;
    while (i-- > 0) {
        if (video_fb.fb[i] < 0)
            continue;
        close(video_fb.fb[i]);
        video_fb.fb[i] = -1;
        ret = c_RkRgaUnmap(&video_fb.bo[i]);
        if (ret)
            printf("c_RkRgaUnmap error : %s\n", strerror(errno));
        ret = c_RkRgaFree(&video_fb.bo[i]);
        if (ret)
            printf("c_RkRgaFree error : %s\n", strerror(errno));
    }
    pthread_mutex_destroy(&video_fb.mtx);
}

void shadow_rga_refresh(int fd, int offset, int src_w, int src_h,
                        int dst_w, int dst_h, int rotate)
{
    int ret;
    int bpp;
    rga_info_t src;
    rga_info_t src1;
    rga_info_t dst;
    int srcFormat;
    int src1Format;
    int dstFormat;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1; //g_src_fd;
    // The head exist minigui info, need make a offset.
    src.virAddr = g_bo.ptr + offset;
    src.mmuFlag = 1;

    memset(&src1, 0, sizeof(rga_info_t));
    src1.fd = -1;
    src1.mmuFlag = 1;

    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = fd;
    dst.mmuFlag = 1;

    getdrmdispbpp(&bpp);
    if (bpp == 32) {
        srcFormat = RK_FORMAT_RGBA_8888;
        dstFormat = RK_FORMAT_RGBA_8888;
    } else {
        srcFormat = RK_FORMAT_RGB_565;
        dstFormat = RK_FORMAT_RGB_565;
    }
    src1Format = RK_FORMAT_YCbCr_420_P;

    rga_set_rect(&src1.rect, 0, 0, video_fb.width, video_fb.height,
                 video_fb.width, video_fb.height, src1Format);
    src1.rotation = rotate;
    rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dstFormat);

    pthread_mutex_lock(&video_fb.mtx);
    src1.fd = video_fb.fb[video_fb.front_fb];
    pthread_mutex_unlock(&video_fb.mtx);
    ret = c_RkRgaBlit(&src1, &dst, NULL);
    if (ret)
        printf("c_RkRgaBlit1 error : %s\n", strerror(errno));

    rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, srcFormat);
    src.rotation = rotate;
    src.blend = 0xff0105;
    rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dstFormat);
    ret = c_RkRgaBlit(&src, &dst, NULL);
    if (ret)
        printf("c_RkRgaBlit2 error : %s\n", strerror(errno));
}

void shadow_rga_refresh90(int fd, int offset, int src_w, int src_h,
                          int dst_w, int dst_h)
{
    shadow_rga_refresh(fd, offset, src_w, src_h, dst_w, dst_h, HAL_TRANSFORM_ROT_90);
}

void shadow_rga_refresh270(int fd, int offset, int src_w, int src_h,
                           int dst_w, int dst_h)
{
    shadow_rga_refresh(fd, offset, src_w, src_h, dst_w, dst_h, HAL_TRANSFORM_ROT_270);
}

int yuv_draw(char *src_ptr, int src_fd, int format, int src_w, int src_h) {
    int ret;
    rga_info_t src;

    rga_info_t dst;
    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1; //rga_src_fd;
    src.virAddr = src_ptr;
    src.mmuFlag = 1;

    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = video_fb.fb[video_fb.front_fb ^ 1];
    dst.mmuFlag = 1;

    rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, format);

    // resize as a rect with src_w and src_h
    video_fb.width = src_w;
    video_fb.height = src_h;
    rga_set_rect(&dst.rect, 0, 0, video_fb.width, video_fb.height, video_fb.width, video_fb.height, RK_FORMAT_YCbCr_420_P);
    // do copy
    ret = c_RkRgaBlit(&src, &dst, NULL);
    if (ret)
        printf("c_RkRgaBlit0 error : %s\n", strerror(errno));
    else {
        pthread_mutex_lock(&video_fb.mtx);
        video_fb.front_fb ^= 1;
        pthread_mutex_unlock(&video_fb.mtx);
    }
    return ret;
}

void shadow_rga_get_user_fd(int *fd, int *screen_w, int *screen_h)
{
    *fd = video_fb.fb[USER_BO_SEQ];
    *screen_w = get_g_rcScr_right();
    *screen_h = get_g_rcScr_bottom();
}

void shadow_rga_switch(void *src_ptr, int src_fd, int src_fmt, int src_w, int src_h)
{
    rga_info_t src;
    rga_info_t dst;
    int dst_w, dst_h;

    memset(&src, 0, sizeof(rga_info_t));
    if (src_ptr) {
        src.fd = -1;
        src.virAddr = src_ptr;
    } else {
        src.fd = src_fd;
    }
    src.mmuFlag = 1;
    rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, src_fmt);

    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = video_fb.fb[video_fb.front_fb ^ 1];
    dst_w = get_g_rcScr_right();
    dst_h = get_g_rcScr_bottom();
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, src_w, src_h, dst_w, dst_h, RK_FORMAT_YCbCr_420_P);

    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return;
    }

    pthread_mutex_lock(&video_fb.mtx);
    video_fb.width = dst_w;
    video_fb.height = dst_h;
    video_fb.front_fb ^= 1;
    pthread_mutex_unlock(&video_fb.mtx);
}


void shadow_rga_get_buffer_fd(void *bo, int *fd)
{
    c_RkRgaGetBufferFd((bo_t *)bo, fd);
}

#endif

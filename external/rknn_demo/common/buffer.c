#include "buffer.h"

#include <stdio.h>
#include <sys/time.h>

extern int c_RkRgaFree(bo_t *bo_info);

unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");

    if(fp == NULL) {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char*)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if(fp) {
        fclose(fp);
    }
    return model;
}

long getCurrentTime(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// void print_rknn_tensor(rknn_tensor_attr *attr)
// {
//     printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
//             attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0],
//             attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, attr->fl, attr->zp, attr->scale);
// }

int buffer_init(int width, int height, int bpp, bo_t *g_rga_buf_bo,
		int *g_rga_buf_fd)
{
    int ret = -1;

    ret = c_RkRgaInit();
    if (ret) {
        printf("c_RkRgaInit error : %s\n", strerror(errno));
        return ret;
    }
    ret = c_RkRgaGetAllocBuffer(g_rga_buf_bo, width, height, bpp);
    if (ret) {
        printf("c_RkRgaGetAllocBuffer error : %s\n", strerror(errno));
        return ret;
    }
    ret = c_RkRgaGetMmap(g_rga_buf_bo);
    if (ret) {
        printf("c_RkRgaGetMmap error : %s\n", strerror(errno));
        return ret;
    }
    ret = c_RkRgaGetBufferFd(g_rga_buf_bo, g_rga_buf_fd);
    if (ret)
        printf("c_RkRgaGetBufferFd error : %s\n", strerror(errno));

    return ret;
}

int buffer_deinit(bo_t *g_rga_buf_bo, int g_rga_buf_fd)
{
    int ret = -1;
    close(g_rga_buf_fd);
    ret = c_RkRgaUnmap(g_rga_buf_bo);
    if (ret)
        printf("c_RkRgaUnmap error : %s\n", strerror(errno));
    ret = c_RkRgaFree(g_rga_buf_bo);
}


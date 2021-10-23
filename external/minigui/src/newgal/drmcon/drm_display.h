#ifndef __DRM_DISPLAY_H__
#define __DRM_DISPLAY_H__

struct drm_bo {
    int fd;
    void *ptr;
    size_t size;
    size_t offset;
    size_t pitch;
    unsigned handle;
    int fb_id;
};

int drm_init(int bpp);
int drm_deinit(void);
char * getdrmdispbuff(void);
int getdrmdispinfo(struct drm_bo *bo, int *w, int *h);
struct drm_bo *getdrmdisp(void);
void setdrmdisp(struct drm_bo *bo);
int drm_setmode(int num, int bpp);
int drm_invalide(void);
void getdrmdispbpp(int *bpp);

#endif

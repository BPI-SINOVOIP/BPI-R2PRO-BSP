#ifndef __SSD_H__
#define __SSD_H__

typedef struct _SSDRECT
{
    /**
     * The x coordinate of the upper-left corner of the rectangle.
     */
    int left;
    /**
     * The y coordinate of the upper-left corner of the rectangle.
     */
    int top;
    /**
     * The x coordinate of the lower-right corner of the rectangle.
     */
    int right;
    /**
     * The y coordinate of the lower-right corner of the rectangle.
     */
    int bottom;
} SSDRECT;

struct ssd_object {
  char name[10];
  SSDRECT select;
};

struct ssd_group {
    int count;
    struct ssd_object objects[100];
};

int ssd_run(void *flag);
int ssd_post(void *flag);
int ssd_init(int arg);
int ssd_deinit();
float ssd_get_fps();
struct ssd_group* ssd_get_ssd_group();

#endif
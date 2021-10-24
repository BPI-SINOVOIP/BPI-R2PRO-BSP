#ifndef __FRG_H__
#define __FRG_H__

#define MODEL_DETECT      "/usr/share/rknn_demo/detect.rknn"
#define MODEL_ALIGN       "/usr/share/rknn_demo/align.rknn"
#define MODEL_RECOGNIZE   "/usr/share/rknn_demo/recognize.rknn"

// for demo usage, limit to 4 is enough
#define MAX_DATABASE_FACE_NUM 4

struct frg_object {
  int x, y, w, h;
  void* rgb;
  int rgb_len;
  float* identifier;
  int iden_len;
  int index; // index: >= 0 in database, if -1 means the front face of realtime
  float match;
};

struct frg_group {
  int count;
  int rt_image_width, rt_image_height;
  struct frg_object objects[MAX_DATABASE_FACE_NUM + 1];
};

int frg_run(void *flag);
int frg_post(void *flag);
int frg_init(char *name);
int frg_deinit();
float frg_get_fps();
struct frg_group* frg_get_group();
// void free_frg_object(struct frg_object *obj);
void free_frg_group(struct frg_group *group, int pfree);

#endif

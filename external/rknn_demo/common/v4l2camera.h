#ifndef _V4L2CAMERA_H_
#define _V4L2CAMERA_H_

typedef void (*camera_callback_t)(void *in_data, int w, int h, int *flag);
typedef void (*callback_for_uvc)(void *buffer, size_t size);

int cameraRun(char *dev_name, unsigned int input_w, unsigned int input_h,
              unsigned int cam_fps, unsigned int cam_format,
              camera_callback_t callback, int *flag);
void register_callback_for_uvc(callback_for_uvc cb);

#endif

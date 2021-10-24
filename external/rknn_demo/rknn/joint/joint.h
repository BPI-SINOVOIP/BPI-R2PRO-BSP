#ifndef __JOIOT_H__
#define __JOIOT_H__

int joint_run(void *flag);
int joint_post(void *flag);
int joint_init(char *name);
int joint_deinit();
float* joint_get_joint_group();

#endif

#ifndef __UEVENT_MONITOR_H__
#define __UEVENT_MONITOR_H__

int uevent_monitor_init(void);
int checkdev(char *path, char **dev, char **type, char **attributes);

#endif
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <glib.h>

#include <sys/inotify.h>
#include "json-c/json.h"
#include "msg_process.h"
#include "manage.h"

#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "uevent_monitor.c"

static char* substring(char* ch, int pos, int length)
{
    char* pch = ch;
    char* subch = (char*)calloc(sizeof(char), length + 1);
    int i;

    pch = pch + pos;

    for (i = 0; i < length; i++) {
        subch[i] = *(pch++);
    }
    subch[length] = '\0';
    return subch;
}

static char *checkmountpath(char *dev)
{
    char *ret = 0;
    char filename[] = "/proc/mounts";
    FILE *fp;
    char StrLine[1024];
    if ((fp = fopen(filename, "r")) == NULL) {
        LOG_INFO("error!");
        return ret;
    }

    while (!feof(fp)) {
        char *tmp;
        fgets(StrLine, 1024, fp);
        tmp = strstr(StrLine, dev);
        if (tmp) {
            char *s = strstr(StrLine, " ") + 1;
            char *e = strstr(s, " ");
            ret = substring(s, 0, e - s);
            fclose(fp);

            return ret;
        }
    }

    fclose(fp);
    return ret;
}

int checkdev(char *path, char **dev, char **type, char **attributes)
{
    char filename[] = "/proc/mounts";
    FILE *fp;
    char StrLine[1024];
    if ((fp = fopen(filename, "r")) == NULL) {
        LOG_INFO("error!");
        return -1;
    }

    while (!feof(fp)) {
        char *tmp;
        fgets(StrLine, 1024, fp);
        tmp = strstr(StrLine, path);
        if (tmp) {
            char a[32];
            char b[32];
            char c[32];
            char d[256];
            sscanf(StrLine, "%s %s %s %s", a, b, c, d);
            *dev = g_strdup(a);
            *type = g_strdup(c);
            *attributes = g_strdup(d);
            fclose(fp);

            return 0;
        }

    }

    fclose(fp);
    return -1;
}

static char *search(char *buf, int len, char *str)
{
    char *ret = 0;
    int i = 0;
    ret = strstr(buf, str);

    if (ret)
        return ret;
    for (i = 1; i < len; i++) {
        if (buf[i - 1] == 0) {
            ret = strstr(&buf[i], str);
            if (ret)
                return ret;
        }
    }
    return ret;
}

static char *getparameters(char *buf, int len, char *str)
{
    char *ret = search(buf, len, str);
    if (ret) {
        ret += strlen(str) + 1;
    }
    return ret;
}

static void *uevent_monitor_thread(void *arg)
{
    int sockfd;
    int i, j, len;
    char *buf;
    int buflen = 2000;
    struct iovec iov;
    struct msghdr msg;
    struct sockaddr_nl sa;

    prctl(PR_SET_NAME, "event_monitor", 0, 0, 0);

    buf = (char*)calloc(sizeof(char), buflen);
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = (void *)buf;
    iov.iov_len = buflen;
    msg.msg_name = (void *)&sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sockfd == -1) {
        LOG_INFO("socket creating failed:%s\n", strerror(errno));
        goto err_event_monitor;
    }

    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        LOG_INFO("bind error:%s\n", strerror(errno));
        goto err_event_monitor;
    }

    while (1) {
        len = recvmsg(sockfd, &msg, 0);
        if (len < 0) {
            //LOG_INFO("receive error\n");
        } else if (len < 32 || len > buflen) {
            LOG_INFO("invalid message");
        } else {
            char *p = strstr(buf, "libudev");

            if (p == buf) {
                if (search(buf, len, "DEVTYPE=partition") || search(buf, len, "DEVTYPE=disk") ) {
                    char *dev = getparameters(buf, len, "DEVNAME");
                    if (search(buf, len, "ACTION=add")) {
                        char *path = checkmountpath(dev);
                        if (path) {
                            LOG_INFO("%s %s mount to %s\n", __func__, dev, path);
                            dev_add(dev, path);
                            if (path)
                                free(path);
                        }
                    } else if (search(buf, len, "ACTION=remove")) {
                        LOG_INFO("%s %s unmount\n", __func__, dev);
                        dev_remove(dev);
                    } else if (search(buf, len, "ACTION=change")) {
                        LOG_INFO("%s %s change\n", __func__, dev);
                        dev_changed(dev);
                    }
                }
            }
        }
    }

err_event_monitor:
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

int uevent_monitor_init(void)
{
    pthread_t tid;

    return pthread_create(&tid, NULL, uevent_monitor_thread, NULL);
}

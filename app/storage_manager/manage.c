#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <time.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <glib.h>
#include <gdbus.h>
#include <sys/mount.h>

#include <sys/inotify.h>
#include <sys/vfs.h>
#include "json-c/json.h"
#include "msg_process.h"
#include "rkdb.h"
#include "uevent_monitor.h"
#include "db_monitor.h"
#include "dbserver.h"

#include "dbus_helpers.h"
#include "mkfs.h"

#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "manage.c"

#define MSG_CMD_ADD                        1
#define MSG_CMD_REMOVE                     2
#define MSG_CMD_ADD_DB_STORAGE_CONFIG      3
#define MSG_CMD_ADD_DB_MEDIA_FOLDER        4
#define MSG_CMD_ADD_DB_DISK_PATH           5
#define MSG_CMD_DB_DATA_CHANGED            6
#define MSG_CMD_DEV_CHANGE                 7

#define EVENT_NUM 12
static tmsg_buffer* msg_buff = NULL;

static GHashTable *db_file_hash;
static DBusConnection *dbus_conn;
static int low_free_status = 0;
static char *mkfs_path = NULL;

#define MEDIA_WAIT_NUM 1
static int media_wait_cnt = 0;
static pthread_mutex_t media_disk_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t media_disk_cond = PTHREAD_COND_INITIALIZER;

#define STORAGE_MANAGER   "rockchip.StorageManager"
#define STORAGE_MANAGER_PATH      "/"
#define STORAGE_MANAGER_FILE_INTERFACE    STORAGE_MANAGER ".file"

struct OldFile {
    char *media_file;
    char *thumb_file;
    char *media_file_name;
};

struct StorageConfig {
    int id;
    int freesize;
    int freesizenotice;
    char *mountpath;
};

struct DbMediaFolder {
    int pathid;
    char *media_folder;
    char *thumb_folder;
    char *format;
    int type;
    int camid;
    int duty;
    int maxnum;
    struct DbMediaFolder *next;
};

struct ScanPath {
    char *media_path;
    char *thumb_path;
    int totalsize;
    int count;
    int wd;
    int num;
    unsigned long long totalsize_byte;
    struct DbMediaFolder *db_media_folder;
    struct ScanPath *next;
};

struct FileStatus {
    int id;
    char *name;
    char *dev;
    char *mountpath;
    char *type;
    char *attributes;
    struct ScanPath *scanpath;
    int mount;
    int scan_status;
    int run;
    int mount_status;
    size_t media_size;
    size_t total_size;
    size_t free_size;
    int format_status;
    int format_prog;
    char *format_err;
    char *db;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t scan_tid;
    pthread_t scan_monitor_tid;
    pthread_t delete_file_tid;
    pthread_t format_tid;
};

struct StorageConfig storage_config = {0, 0, 0};
static struct DbMediaFolder *db_media_folder_list = NULL;
static pthread_mutex_t scanfile_mutex;

static void file_scan_run(char *path);
static int db_delete_file(struct FileStatus *status, struct ScanPath *scanpath, char *name);
static json_object *cmd_get_disks_status(char *path);
static json_object *cmd_get_media_path(void);

char *event_str[EVENT_NUM] = {
    "IN_ACCESS",
    "IN_MODIFY",
    "IN_ATTRIB",
    "IN_CLOSE_WRITE",
    "IN_CLOSE_NOWRITE",
    "IN_OPEN",
    "IN_MOVED_FROM",
    "IN_MOVED_TO",
    "IN_CREATE",
    "IN_DELETE",
    "IN_DELETE_SELF",
    "IN_MOVE_SELF"
};

long long str2llu(char* s)
{
    unsigned long long n = 0;
    while ( *s ) {
        n *= 10;
        n += *s - '0';
        ++s;
    }

    return n;
}

void judge_emmc_mount(char *dt_realy)
{
    if (NULL == mkfs_path) {
        g_free(dt_realy);
        return;
    }
    char buffer[512];
    FILE *read_fp;
    int chars_read;
    char cmd[512];

    char *short_dev = dt_realy;
    while(strstr(short_dev, "/")) {
        LOG_INFO("short_dev is %s,", short_dev);
        short_dev = strstr(short_dev, "/");
        if (strlen(short_dev) > 1)
            short_dev += 1;
        else
            break;
    }

    memset(buffer, 0, sizeof(buffer));
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "ls -l %s", mkfs_path);
    read_fp = popen(cmd, "r");
    if (read_fp != NULL) {
        chars_read = fread(buffer, sizeof(char), sizeof(buffer) - 1,
                           read_fp);
        if (chars_read > 0) {
            LOG_INFO("buffer is %s, dt is %s\n", buffer, short_dev);
            if (NULL != strstr(buffer, short_dev)) {
                g_free(mkfs_path);
                mkfs_path = NULL;
            }
        }
        pclose(read_fp);
    }

    g_free(dt_realy);
}


int runapp_wait(char *cmd)
{
    int ret = -1;
    char buffer[512];
    FILE *read_fp;
    int chars_read;

    memset(buffer, 0, sizeof(buffer));
    read_fp = popen(cmd, "r");
    if (read_fp != NULL) {
        chars_read = fread(buffer, sizeof(char), sizeof(buffer) - 1,
                           read_fp);
        if (chars_read > 0)
            ret = 0;
        int cnt = 0;
        while (mkfs_path) {
            LOG_INFO("wait for mkfs %d\n", cnt);
            cnt += 1;
            sleep(1);
            if (cnt > 15) {
                LOG_INFO("wait mkfs cnt out\n");
                if (mkfs_path)
                    g_free(mkfs_path);
                break;
            }
        }
        pclose(read_fp);
    }

    return ret;
}

int runapp(char *cmd)
{
    int ret = -1;
    char buffer[512];
    FILE *read_fp;
    int chars_read;

    memset(buffer, 0, sizeof(buffer));
    read_fp = popen(cmd, "r");
    if (read_fp != NULL) {
        chars_read = fread(buffer, sizeof(char), sizeof(buffer) - 1,
                           read_fp);
        if (chars_read > 0)
            ret = 0;
        pclose(read_fp);
    }

    return ret;
}

int create_dir(const char *sPathName)
{
    char DirName[256];
    int i;
    strcpy(DirName, sPathName);
    int len = strlen(DirName);

    for (i = 1; i < len; i++) {
        if (DirName[i] == '/') {
            DirName[i] = 0;
            if (access(DirName, R_OK) != 0) {
                if(mkdir(DirName, 0755) == -1) {
                    LOG_INFO("mkdir error\n");
                    return -1;
                }
            }
            DirName[i] = '/';
        }
    }

    if (access(DirName, R_OK) != 0) {
        if(mkdir(DirName, 0755) == -1) {
            LOG_INFO("mkdir error\n");
            return -1;
        }
    }

    return 0;
}

void msg_init(void)
{
    msg_buff = msg_buffer_init();
}

static void msg_send(int msg, char *data, int len, char *data1, int len1)
{
    msg_buff->sendmsg(msg_buff, msg, 0, data, len, data1, len1);
}

static tmsg_element* msg_get(void)
{
    tmsg_element* event = NULL;
    event = msg_buff->get_timeout(msg_buff, 1000);
    return event;
}

static int signal_update_media_path(void)
{
    DBusMessage *signal;
    DBusMessageIter iter;
    char *json_str;
    json_object *j_array;

    signal = dbus_message_new_signal(STORAGE_MANAGER_PATH,
                                     STORAGE_MANAGER_FILE_INTERFACE, "UpdateMediaPath");
    if (!signal)
        return -1;

    j_array = cmd_get_media_path();

    json_str = (char *)json_object_to_json_string(j_array);
    LOG_INFO("storage_manager %s, %s\n", __func__, json_str);
    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(dbus_conn, signal, NULL);
    dbus_message_unref(signal);

    json_object_put(j_array);

    return 0;
}

static int signal_free_size_notice(void)
{
    DBusMessage *signal;
    DBusMessageIter iter;
    char *json_str;
    json_object *j_cfg = json_object_new_object();

    signal = dbus_message_new_signal(STORAGE_MANAGER_PATH,
                                     STORAGE_MANAGER_FILE_INTERFACE, "FreeSizeNotice");
    if (!signal)
        return -1;

    json_object_object_add(j_cfg, "iLowFreeStatus", json_object_new_int(low_free_status));

    json_str = (char *)json_object_to_json_string(j_cfg);
    LOG_INFO("storage_manager %s, %s\n", __func__, json_str);
    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(dbus_conn, signal, NULL);
    dbus_message_unref(signal);

    json_object_put(j_cfg);

    return 0;
}

static int signal_formating_notice(int is_formating)
{
    DBusMessage *signal;
    DBusMessageIter iter;
    char *json_str;
    json_object *j_cfg = json_object_new_object();

    signal = dbus_message_new_signal(STORAGE_MANAGER_PATH,
                                     STORAGE_MANAGER_FILE_INTERFACE, "FormatNotice");
    if (!signal)
        return -1;

    json_object_object_add(j_cfg, "iFormating", json_object_new_int(is_formating));

    json_str = (char *)json_object_to_json_string(j_cfg);
    LOG_INFO("storage_manager %s, %s\n", __func__, json_str);
    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(dbus_conn, signal, NULL);
    dbus_message_unref(signal);

    json_object_put(j_cfg);

    return 0;
}

static int signal_update_disks_status(char *path)
{
    DBusMessage *signal;
    DBusMessageIter iter;
    char *json_str;
    json_object *j_array;

    signal = dbus_message_new_signal(STORAGE_MANAGER_PATH,
                                     STORAGE_MANAGER_FILE_INTERFACE, "UpdateDisksStatus");
    if (!signal)
        return -1;

    j_array = cmd_get_disks_status(path);

    json_str = (char *)json_object_to_json_string(j_array);
    LOG_INFO("storage_manager %s, %s\n", __func__, json_str);
    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(dbus_conn, signal, NULL);
    dbus_message_unref(signal);

    json_object_put(j_array);

    return 0;
}

static void file_add(char *db, int pathid, char *path, char *format, char *name, int *count, unsigned long long size)
{
    char *data;
    int offset;
    struct tm tm;
    int ctime = 0;

    if (format) {
        char *tmp = NULL;//strstr(format, "%Y");
        if (tmp) {
            //"%Y-%m-%d %H:%M:%S
            strptime(name, format, &tm);
        } else {
            struct stat statbuf;
            char *filename = g_strdup_printf("%s/%s", path, name);
            lstat(filename, &statbuf);
            ctime = statbuf.st_ctime;
            struct tm *ptm = localtime(&statbuf.st_ctime);
            memcpy(&tm, ptm, sizeof(struct tm));
            g_free(filename);
        }

        tmp = strstr(format, "CCC");
        if (tmp) {
            int cnt = 0;
            char *scanfpar;
            offset = tmp - format;
            while (tmp[cnt] == 'C') {
                cnt++;
            }
            scanfpar = g_strdup_printf("%c0%dd", '%', cnt);
            sscanf(name + offset, scanfpar, count);
            g_free(scanfpar);
        }
    }

    data = g_strdup_printf("%d,'%s','%llu',%d", pathid,
                           name, size, ctime);
    //LOG_INFO("%s,format=%s, data=\"%s\"\n", __func__, format, data);
    g_free(rkdb_insert(db, "iPATHID,sNAME,sSIZE,iTIME", data));
    g_free(data);
}

static void checkdisksize(char *path, int *totalsize, int *freesize)
{
    struct statfs diskInfo;
    statfs(path, &diskInfo);
    unsigned long long totalBlocks = diskInfo.f_bsize;
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;
    *totalsize = totalSize >> 10;
    unsigned long long freeDisk = diskInfo.f_bfree * totalBlocks;
    *freesize = freeDisk >> 10;
}

static void free_old_file(struct OldFile *oldfile)
{
    if (oldfile) {
        if (oldfile->media_file)
            g_free(oldfile->media_file);
        if (oldfile->thumb_file)
            g_free(oldfile->thumb_file);
        if (oldfile->media_file_name)
            g_free(oldfile->media_file_name);
        free(oldfile);
    }
}

static struct OldFile *get_old_file(struct FileStatus *status, struct ScanPath *scanpath)
{
    struct OldFile *oldfile = (struct OldFile *)malloc(sizeof(struct OldFile));
    char *where = g_strdup_printf("iPATHID=%d", scanpath->db_media_folder->pathid);
    char *j_str = rkdb_select(status->db, NULL, where, "iTIME asc", "0,1");
    json_object *j_ret = json_tokener_parse(j_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");
    int num = json_object_array_length(j_array);
    if (num > 0) {
        json_object *j_obj = json_object_array_get_idx(j_array, 0);
        oldfile->media_file = g_strdup_printf("%s/%s", scanpath->media_path, (char *)json_object_get_string(json_object_object_get(j_obj, "sNAME")));
        if (scanpath->thumb_path) {
            oldfile->thumb_file = g_strdup_printf("%s/%s", scanpath->thumb_path, (char *)json_object_get_string(json_object_object_get(j_obj, "sNAME")));
            int len = strlen(oldfile->thumb_file);
            oldfile->thumb_file[len - 3] = 'j';
            oldfile->thumb_file[len - 2] = 'p';
            oldfile->thumb_file[len - 1] = 'g';
        } else {
            oldfile->thumb_file = NULL;
        }
        oldfile->media_file_name = g_strdup_printf("%s", (char *)json_object_get_string(json_object_object_get(j_obj, "sNAME")));
    } else {
        oldfile->media_file = NULL;
        oldfile->thumb_file = NULL;
        oldfile->media_file_name = NULL;
    }

    json_object_put(j_ret);
    g_free(where);
    g_free(j_str);

    return oldfile;
}

//TIP: ext2 design for emmc
static void *format_thread(void *arg)
{
    struct FileStatus *status = (struct FileStatus *)arg;

    bool is_format_media_path = false;
    if (status->run) {
        int type = 0;
        if (g_str_equal(status->type, "vfat"))
            type = 1;
        else if (g_str_equal(status->type, "ext2"))
            type = 2;

        if (type) {
            status->run = 0;
            status->format_status = 1;
            status->format_prog = 0;
            pthread_mutex_lock(&status->mutex);
            pthread_cond_signal(&status->cond);
            pthread_mutex_unlock(&status->mutex);
            pthread_join(status->scan_tid, NULL);
#ifdef WAIT_MEDIA_STOP
            // notify to stop storage in media floder
            if (storage_config.mountpath && g_str_equal(storage_config.mountpath, status->mountpath)) {
                signal_formating_notice(1);
                is_format_media_path = true;
                pthread_mutex_lock(&media_disk_mutex);
                LOG_INFO("wait media storage stop\n");
                media_wait_cnt = MEDIA_WAIT_NUM;
                while (media_wait_cnt)
                    pthread_cond_wait(&media_disk_cond, &media_disk_mutex);
                LOG_INFO("wait media storage stop end\n");
                sleep(2);
            }
#endif
            int ret = umount2(status->mountpath, MNT_DETACH);
            if (ret == 0) {
                status->mount_status = 0;
                LOG_INFO("format %s begin\n", status->dev);
                if (type == 1) {
                    char *err = mkfs_vfat(status->dev, &status->format_prog);
                    if (err) {
                        LOG_INFO("%s err = %s\n", __func__, err);
                        status->format_status = -1;
                        if (status->format_err)
                            g_free(status->format_err);
                        status->format_err = err;
                        ret = -1;
                    } else {
                        ret = 0;
                    }
                } else if (type == 2) {
                    int cnt = 0;
                    if (NULL != strstr(status->dev, "media")) {
                        char *cmd = g_strdup_printf("mkfs.ext2 -F -L media %s", status->dev);
                        if (mkfs_path)
                            g_free(mkfs_path);
                        mkfs_path = g_strdup(status->dev);
                        LOG_DEBUG("cmd: %s\n", cmd);
                        ret = runapp_wait(cmd);
                        g_free(cmd);
                        cmd = g_strdup_printf("tune2fs -c 0 -i 0 %s", status->dev);
                        ret = runapp(cmd);
                        g_free(cmd);
                        cmd = g_strdup_printf("fsck.ext2 -y %s", status->dev);
                        ret = runapp(cmd);
                        g_free(cmd);
                    } else {
                        char *cmd = g_strdup_printf("mkfs.ext2 %s", status->dev);
                        LOG_DEBUG("cmd: %s\n", cmd);
                        ret = runapp(cmd);
                        g_free(cmd);
                        cmd = g_strdup_printf("fsck.ext2 -y %s", status->dev);
                        ret = runapp(cmd);
                        g_free(cmd);
                    }
                }
                if (ret == 0)
                    LOG_INFO("format %s finish\n", status->dev);
                else
                    LOG_INFO("format %s err\n", status->dev);

                for (int i = 0; i < 5; i++) {
                    LOG_DEBUG("status->dev %s, status->mountpath %s, status->type %s\n", status->dev, status->mountpath, status->type);
                    ret = mount(status->dev, status->mountpath, status->type, MS_NOATIME | MS_NOSUID, NULL);
                    if (ret == 0) {
                        status->mount_status = 1;
                        break;
                    } else {
                        LOG_INFO("mount %s err, retry %d\n", status->mountpath, i);
                    }
                    sleep(1);
                }
                if (ret == 0) {
                    file_scan_run(status->mountpath);
                    status->format_status = 2;
                }
            } else {
                LOG_INFO("umount %s err\n", status->mountpath);
                status->format_status = -1;
                if (status->format_err)
                    g_free(status->format_err);
                status->format_err = g_strdup("umount fail");
                file_scan_run(status->mountpath);
            }
        }
        if (is_format_media_path) {
            signal_formating_notice(0);
            pthread_mutex_unlock(&media_disk_mutex);
        }
    }

err_format:
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

static void *delete_file_thread(void *arg)
{
	  struct FileStatus *status = (struct FileStatus *)arg;

    while (status->run) {
        int i;
        long total = 0;
        checkdisksize(status->mountpath, &status->total_size, &status->free_size);

        total = status->free_size;
        struct ScanPath *scanpath = status->scanpath;
        size_t media_size = 0;
        while (scanpath) {
            while ((scanpath->db_media_folder->maxnum >= 0) && (scanpath->num > scanpath->db_media_folder->maxnum)) {
                struct OldFile *oldfile = get_old_file(status, scanpath);
                db_delete_file(status, scanpath, oldfile->media_file_name);
                LOG_INFO("Delete %s file. num = %d, maxnum = %d\n", oldfile->media_file, scanpath->num, scanpath->db_media_folder->maxnum);
                if (remove(oldfile->media_file) != 0)
                    LOG_INFO("Delete %s file err.\n", oldfile->media_file);

                if (oldfile->thumb_file) {
                    LOG_INFO("Delete %s file.\n", oldfile->thumb_file);
                    remove(oldfile->thumb_file);
                }
                free_old_file(oldfile);
            }
            scanpath->totalsize = scanpath->totalsize_byte / 1024;

            total += scanpath->totalsize;
            media_size += scanpath->totalsize;
            scanpath = scanpath->next;
        }
        status->media_size = media_size;
        total = total - storage_config.freesize * 1024;
        //LOG_INFO("total = %dKB, free = %dKB, media size = %dKB, Threshold = %dKB\n",
        //        status->total_size, status->free_size, status->media_size, storage_config.freesize * 1024);
        if (total <= 0) {
            LOG_INFO("Error, invalid threshold! Unable to delete file.\n");
            LOG_INFO("total = %dKB, free = %dKB, media size = %dKB, Threshold = %dKB\n",
                    status->total_size, status->free_size, status->media_size, storage_config.freesize * 1024);
        }

        if ((status->free_size < (storage_config.freesize * 1024)) && (total > 0)) {
            if ((status->free_size < (storage_config.freesizenotice * 1024)) && (low_free_status == 0)) {
                low_free_status = 1;
                signal_free_size_notice();
            }
            scanpath = status->scanpath;
            while (scanpath) {
                float duty = (float)scanpath->totalsize * 100 / total;
                LOG_INFO("media_path = %s, totalsize = %d, num = %d, current duty = %f, conf duty = %d\n", scanpath->media_path,
                       scanpath->totalsize, scanpath->num, duty, scanpath->db_media_folder->duty);

                if (duty > scanpath->db_media_folder->duty && scanpath->db_media_folder->duty >= 0) {
                    struct OldFile *oldfile = get_old_file(status, scanpath);
                    db_delete_file(status, scanpath, oldfile->media_file_name);
                    LOG_INFO("Delete %s file.\n", oldfile->media_file);
                    if (remove(oldfile->media_file) != 0)
                        LOG_INFO("Delete %s file err.\n", oldfile->media_file);

                    if (oldfile->thumb_file) {
                        LOG_INFO("Delete %s file.\n", oldfile->thumb_file);
                        remove(oldfile->thumb_file);
                    }
                    free_old_file(oldfile);
                }

                scanpath = scanpath->next;
            }
        } else {
            if (low_free_status == 1) {
                low_free_status = 0;
                signal_free_size_notice();
            }
            usleep(2000000);
        }
    }
err_event_monitor:
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}


static unsigned long long get_file_size_from_disk(char *path, char *name)
{
    struct stat statbuf;
    char *filename = g_strdup_printf("%s/%s", path, name);
    lstat(filename, &statbuf);

    g_free(filename);

    return statbuf.st_size;
}

static long long get_file_size_from_db(char *table, int pathid, char *name)
{
    long long size = -1;
    char *col = "sSize";
    char *where = g_strdup_printf("iPATHID=%d and sNAME='%s'", pathid, name);
    char *j_str = rkdb_select(table, col, where, NULL, NULL);

    json_object *j_ret = json_tokener_parse(j_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");

    int len = json_object_array_length(j_array);

    for (int i = 0; i < len; i++) {
        json_object *j_obj = json_object_array_get_idx(j_array, i);
        size = str2llu((char *)json_object_get_string(json_object_object_get(j_obj, "sSIZE")));
    }
    json_object_put(j_ret);
    g_free(j_str);
    g_free(where);

    return size;
}

static int db_delete_file(struct FileStatus *status, struct ScanPath *scanpath, char *name)
{
    long long size = get_file_size_from_db(status->db, scanpath->db_media_folder->pathid, name);

    if (size >= 0) {
        scanpath->totalsize_byte -= size;
        char *data = g_strdup_printf("iPATHID=%d and sNAME='%s'", scanpath->db_media_folder->pathid, name);
        scanpath->num --;
        g_free(rkdb_delete(status->db, data));
        g_free(data);
    }
}

static void *file_monitor_thread(void *arg)
{
    int fd;
    int len;
    int nread;
    char buf[BUFSIZ];
    struct inotify_event *event;
    int i;
    struct FileStatus *status;
    status = (struct FileStatus *)arg;

    prctl(PR_SET_NAME, "file_monitor_thread", 0, 0, 0);

    fd = inotify_init();
    if (fd < 0) {
        LOG_INFO("inotify_init failed\n");
        return NULL;
    }

    struct ScanPath *scanpath = status->scanpath;
    while (scanpath) {
        scanpath->wd = inotify_add_watch(fd, scanpath->media_path, IN_CREATE | IN_MOVED_TO | IN_DELETE | IN_MOVED_FROM | IN_CLOSE_WRITE);
        scanpath = scanpath->next;
    }

    buf[sizeof(buf) - 1] = 0;
    while ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
        nread = 0;
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];

            for (i = 0; i < EVENT_NUM; i++) {
                if ((event->mask >> i) & 1) {
                    if (event->len > 0) {
                        scanpath = status->scanpath;
                        while (scanpath) {
                            if (event->wd == scanpath->wd) {
                                if (g_str_equal(event_str[i], "IN_CREATE") || g_str_equal(event_str[i], "IN_MOVED_TO")) {
                                    int cnt = -1;
                                    scanpath->num ++;
                                    file_add(status->db, scanpath->db_media_folder->pathid, scanpath->media_path, scanpath->db_media_folder->format, event->name, &cnt, 0);
                                    if (cnt >= scanpath->count)
                                        scanpath->count = cnt + 1;
                                } else if (g_str_equal(event_str[i], "IN_DELETE") || g_str_equal(event_str[i], "IN_MOVED_FROM")) {
                                    db_delete_file(status, scanpath, event->name);
                                } else if (g_str_equal(event_str[i], "IN_CLOSE_WRITE")) {
                                    unsigned long long size = get_file_size_from_disk(scanpath->media_path, event->name);
                                    scanpath->totalsize_byte += size;

                                    char *set = g_strdup_printf("sSIZE='%llu'", size);

                                    char *where = g_strdup_printf("iPATHID=%d and sNAME='%s'", scanpath->db_media_folder->pathid, event->name);
                                    g_free(rkdb_update(status->db, set, where));
                                    g_free(where);
                                    g_free(set);
                                 }
                                break;
                            }
                            scanpath = scanpath->next;
                        }
                    } else
                        LOG_INFO("%d,%s --- %s\n", event->wd, " ", event_str[i]);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

static void scanfile(struct FileStatus *status)
{
    size_t media_size = 0;
    struct ScanPath *scanpath = status->scanpath;

    pthread_mutex_lock(&scanfile_mutex);
    while (scanpath) {
        DIR *dp;
        struct dirent *entry;
        struct stat statbuf;
        int len;
        char *dir = scanpath->media_path;

        if (status->run == 0)
            break;

        create_dir(scanpath->media_path);

        if (status->run == 0)
            break;

        if (scanpath->thumb_path)
            create_dir(scanpath->thumb_path);

        if (status->run == 0)
            break;

        if ((dp = opendir(dir)) == NULL) {
            LOG_INFO("open %s error\n", dir);
            break;
        }

        chdir(dir);
        while (((entry = readdir(dp)) != NULL) && (status->run)) {
            lstat(entry->d_name, &statbuf);
            if (!S_ISDIR(statbuf.st_mode)) {
                int cnt = -1;
                scanpath->num++;
                scanpath->totalsize_byte += statbuf.st_size;
                file_add(status->db, scanpath->db_media_folder->pathid, dir, scanpath->db_media_folder->format, entry->d_name, &cnt, statbuf.st_size);
                if (cnt >= scanpath->count)
                    scanpath->count = cnt + 1;
            }
#ifdef FILE_LOAD_DELAY
            if (FILE_LOAD_DELAY > 0)
                usleep(FILE_LOAD_DELAY);
#endif
        }
        scanpath->totalsize = scanpath->totalsize_byte / 1024;
        media_size += scanpath->totalsize;
        chdir("/");
        closedir(dp);
        scanpath = scanpath->next;
    }
    status->media_size = media_size;
    pthread_mutex_unlock(&scanfile_mutex);
}

static char *pathtodb(char *path)
{
    char *db;
    int len;
    int i;
    db = g_strdup_printf("%s", path);
    len = strlen(db);
    for (i = 0; i < len; i++) {
        if (db[i] == '/')
            db[i] = '_';
    }
    return db;
}

static void *file_scan_thread(void *arg)
{
    char *db;

    char *col_para = "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                     "iPATHID INT DEFAULT 0," \
                     "sNAME TEXT," \
                     "sSIZE TEXT," \
                     "iTIME INT";

    struct FileStatus *status;
    prctl(PR_SET_NAME, "file_scan_thread", 0, 0, 0);
    status = (struct FileStatus *)arg;

    db = pathtodb(status->mountpath);

    g_free(rkdb_create(db, col_para));
    g_free(rkdb_delete(db, NULL));
    if (status->db)
        g_free(status->db);
    status->db = db;
    checkdisksize(status->mountpath, &status->total_size, &status->free_size);
    signal_update_disks_status(status->mountpath);
    LOG_INFO("%s: mount path:%s\n", __FUNCTION__, status->mountpath);
    scanfile(status);

    if (status->run) {
        pthread_create(&status->scan_monitor_tid, NULL, file_monitor_thread, status);
        pthread_create(&status->delete_file_tid, NULL, delete_file_thread, status);
    }

    pthread_mutex_lock(&status->mutex);
    if (status->run)
        pthread_cond_wait(&status->cond, &status->mutex);
    pthread_mutex_unlock(&status->mutex);

    pthread_cancel(status->scan_monitor_tid);
    pthread_join(status->scan_monitor_tid, NULL);
    pthread_cancel(status->delete_file_tid);
    pthread_join(status->delete_file_tid, NULL);

    status->media_size = 0;
    status->total_size = 0;
    status->free_size = 0;

    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

static void file_scan_run(char *path)
{
    struct FileStatus *status;

    status = g_hash_table_lookup(db_file_hash, path);
    LOG_INFO("%s: path: %s\n", __FUNCTION__, path);
    if (status) {
        LOG_DEBUG("%s: path: %s, status->run: %d\n", __FUNCTION__, path, status->run);
    } else {
        LOG_DEBUG("%s: path: %s, status is null\n", __FUNCTION__, path);
    }
    if (status != NULL && status->run == 0) {
        char *dev = NULL;
        char *type = NULL;
        char *attributes = NULL;

        if (checkdev(path, &dev, &type, &attributes) == 0) {
            if (status->dev)
                g_free(status->dev);
            if (status->type)
                g_free(status->type);
            if (status->attributes)
                g_free(status->attributes);
            status->dev = dev;
            status->type = type;
            status->attributes = attributes;
            status->run = 1;
            status->mount_status = 1;
            LOG_DEBUG("befpre pthread: path: %s\n", path);
            pthread_create(&status->scan_tid, NULL, file_scan_thread, status);
        }
    }
}

static int getidbypath(struct FileStatus *status, char *path)
{
    struct ScanPath *scanpath = status->scanpath;
    while (scanpath) {
        if (g_str_equal(scanpath->media_path, path))
            return scanpath->db_media_folder->pathid;

        scanpath = scanpath->next;
    }
    return -1;
}

static char *getpathbyid(struct FileStatus *status, int pathid)
{
    char *path = NULL;
    struct ScanPath *scanpath = status->scanpath;
    while (scanpath) {
        if (scanpath->db_media_folder->pathid == pathid) {
            path = scanpath->media_path;
            break;
        }

        scanpath = scanpath->next;
    }

    return path;
}

static void add_media_folder(char *json_str)
{
    json_object *j_ret = json_tokener_parse(json_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");

    int len = json_object_array_length(j_array);

    for (int i = 0; i < len; i++) {
        struct DbMediaFolder *mediafolder = malloc(sizeof(struct DbMediaFolder));
        if (db_media_folder_list) {
            struct DbMediaFolder *tmp = db_media_folder_list;
            while (tmp->next) {
                tmp = tmp->next;
            }
            tmp->next = mediafolder;
        } else {
            db_media_folder_list = mediafolder;
        }
        json_object *j_obj = json_object_array_get_idx(j_array, i);
        mediafolder->media_folder = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sMediaFolder")));
        mediafolder->thumb_folder = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sThumbFolder")));
        mediafolder->pathid = (int)json_object_get_int(json_object_object_get(j_obj, "id"));
        mediafolder->format = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sFormat")));
        mediafolder->camid = (int)json_object_get_int(json_object_object_get(j_obj, "iCamId"));
        mediafolder->type = (int)json_object_get_int(json_object_object_get(j_obj, "iType"));
        mediafolder->duty = (int)json_object_get_int(json_object_object_get(j_obj, "iDuty"));
        mediafolder->maxnum = (int)json_object_get_int(json_object_object_get(j_obj, "iMaxNum"));
        mediafolder->next = NULL;
    }
    json_object_put(j_ret);
}

static void add_disk_path(char *json_str)
{
    json_object *j_ret = json_tokener_parse(json_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);

    for (int i = 0; i < len; i++) {
        json_object *j_obj = json_object_array_get_idx(j_array, i);

        char *path = (char *)json_object_get_string(json_object_object_get(j_obj, "sPath"));
        struct FileStatus *status = g_hash_table_lookup(db_file_hash, path);

        if (status == NULL) {
            status = malloc(sizeof(struct FileStatus));
            memset(status, 0, sizeof(struct FileStatus));
            pthread_mutex_init(&(status->mutex), NULL);
            pthread_cond_init(&(status->cond), NULL);
            g_hash_table_replace(db_file_hash, g_strdup(path), (gpointer)status);
        }
        status->mountpath = g_strdup(path);
        status->id = (int)json_object_get_int(json_object_object_get(j_obj, "id"));
        status->name = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sName")));
        status->mount = (int)json_object_get_int(json_object_object_get(j_obj, "iMount"));

        struct DbMediaFolder *media_folder = db_media_folder_list;
        struct ScanPath *scanpath_last = NULL;
        while (media_folder) {
            struct ScanPath *scanpath = (struct ScanPath *)calloc(1, sizeof(struct ScanPath));
            if (scanpath_last == NULL) {
                status->scanpath = scanpath;
            } else {
                scanpath_last->next = scanpath;
            }
            scanpath_last = scanpath;
            scanpath->media_path = g_strdup_printf("%s/%s", status->mountpath, media_folder->media_folder);
            if (media_folder->thumb_folder && (!g_str_equal(media_folder->thumb_folder, "")))
                scanpath->thumb_path = g_strdup_printf("%s/%s", status->mountpath, media_folder->thumb_folder);
            else
                scanpath->thumb_path = NULL;
            scanpath->db_media_folder = media_folder;
            media_folder = media_folder->next;
        }
        file_scan_run(status->mountpath);
    }
    json_object_put(j_ret);
}

static void add_storage_config(char *json_str)
{
    json_object *j_ret = json_tokener_parse(json_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);

    for (int i = 0; i < len; i++) {
        json_object *j_obj = json_object_array_get_idx(j_array, i);
        char *path = (char *)json_object_get_string(json_object_object_get(j_obj, "sMountPath"));
        if (storage_config.mountpath)
            g_free(storage_config.mountpath);
        storage_config.mountpath = g_strdup_printf("%s", path);
        storage_config.id = (int)json_object_get_int(json_object_object_get(j_obj, "id"));
        storage_config.freesize = (int)json_object_get_int(json_object_object_get(j_obj, "iFreeSize"));
        storage_config.freesizenotice = (int)json_object_get_int(json_object_object_get(j_obj, "iFreeSizeNotice"));
    }
    json_object_put(j_ret);
    signal_update_media_path();
}

static void update_storage_config(json_object *j_cfg)
{
    json_object *j_data = json_object_object_get(j_cfg, "data");

    json_object_object_foreach(j_data, key, val) {
        if (g_str_equal(key, "sMountPath")) {
            if (storage_config.mountpath)
                g_free(storage_config.mountpath);
            storage_config.mountpath = g_strdup((char *)json_object_get_string(val));
        } else if (g_str_equal(key, "iFreeSize")) {
            storage_config.freesize = (int)json_object_get_int(val);
        } else if (g_str_equal(key, "iFreeSizeNotice")) {
            storage_config.freesizenotice = (int)json_object_get_int(val);
        }
    }
    signal_update_media_path();
}

static void update_storage_media_folder(json_object *j_cfg)
{
    int id = -1;
    int camid = -1;
    int type = -1;
    json_object *j_data = json_object_object_get(j_cfg, "data");
    json_object *j_key = json_object_object_get(j_cfg, "key");

    json_object_object_foreach(j_key, key, val) {
        if (g_str_equal(key, "id"))
            id = (int)json_object_get_int(val);
        else if (g_str_equal(key, "iCamId"))
            camid = (int)json_object_get_int(val);
        else if (g_str_equal(key, "iType"))
            type = (int)json_object_get_int(val);
    }

    if ((id == -1) && (camid == -1) && (type == -1))
        return;

    struct DbMediaFolder *db_media_folder = db_media_folder_list;

    while (db_media_folder) {
        if ((id != -1) && (db_media_folder->pathid != id))
            goto next;
        if ((camid != -1) && (db_media_folder->camid != camid))
            goto next;
        if ((type != -1) && (db_media_folder->type != type))
            goto next;

        break;
next:
        db_media_folder = db_media_folder->next;
    }

    if (db_media_folder) {
        json_object_object_foreach(j_data, key, val) {
            if (g_str_equal(key, "sFormat")) {
                if (db_media_folder->format)
                    g_free(db_media_folder->format);
                db_media_folder->format = g_strdup((char *)json_object_get_string(val));
            } else if (g_str_equal(key, "iDuty")) {
                db_media_folder->duty = (int)json_object_get_int(val);
            } else if (g_str_equal(key, "iMaxNum")) {
                db_media_folder->maxnum = (int)json_object_get_int(val);
            }
        }
    }

    signal_update_media_path();
}

static void update_storage(char *json_str)
{
    json_object *j_cfg;
    char *table = 0;

    j_cfg = json_tokener_parse(json_str);

    table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));
    if (g_str_equal(cmd, "Delete")) {

    } else if (g_str_equal(cmd, "Update")) {
        if (g_str_equal(table, TABLE_STORAGE_CONFIG)) {
            update_storage_config(j_cfg);
        } else if (g_str_equal(table, TABLE_STORAGE_MEDIA_FOLDER))
            update_storage_media_folder(j_cfg);
    }

    json_object_put(j_cfg);
}

void hash_dev_remove(gpointer key, gpointer value, gpointer data)
{
    struct FileStatus *status = (struct FileStatus *)value;

    if (status->dev) {
        if (g_str_equal(status->dev, data)) {
            status->run = 0;
            status->mount_status = 0;
            LOG_INFO("remove: %s\n", status->mountpath);
            signal_update_disks_status(status->mountpath);
            pthread_mutex_lock(&status->mutex);
            pthread_cond_signal(&status->cond);
            pthread_mutex_unlock(&status->mutex);
        }
    }
}

void hash_dev_add(gpointer key, gpointer value, gpointer data)
{
    struct FileStatus *status = (struct FileStatus *)value;
    if (status)
        LOG_INFO("get path: %s\n", status->mountpath);
    if (status)
        file_scan_run(status->mountpath);
}

static void *file_manage_thread(void *arg)
{
    prctl(PR_SET_NAME, "file_manage_thread", 0, 0, 0);
    while (1) {
        tmsg_element* event = msg_get();
        if (event) {
            switch (event->msg) {
            case MSG_CMD_ADD_DB_DISK_PATH:
                if (event->dt)
                    add_disk_path(event->dt);
                break;
            case MSG_CMD_ADD_DB_MEDIA_FOLDER:
                if (event->dt)
                    add_media_folder(event->dt);
                break;
            case MSG_CMD_ADD_DB_STORAGE_CONFIG:
                if (event->dt)
                    add_storage_config(event->dt);
                break;
            case MSG_CMD_DB_DATA_CHANGED:
                if (event->dt)
                    update_storage(event->dt);
                break;
            case MSG_CMD_ADD:
                LOG_INFO("MSG_CMD_ADD %s,%s\n", event->dt, event->dt1);
                if (event->dt && event->dt1)
                    file_scan_run(event->dt1);
                break;
            case MSG_CMD_REMOVE:
                LOG_INFO("MSG_CMD_REMOVE %s\n", event->dt);
                g_hash_table_foreach(db_file_hash, hash_dev_remove, event->dt);
                break;
            case MSG_CMD_DEV_CHANGE:
                usleep(500000);
                LOG_INFO("MSG_CMD_DEV_CHANGE %s\n", event->dt);
                g_hash_table_foreach(db_file_hash, hash_dev_add, event->dt);
                if (NULL != mkfs_path) {
                    judge_emmc_mount(g_strdup(event->dt));
                }
                break;
            }

            free_tmsg_element(event);
        }
    }
}

void dev_add(char *dev, char *path)
{
    msg_send(MSG_CMD_ADD, dev, strlen(dev), path, strlen(path));
}

void dev_remove(char *dev)
{
    msg_send(MSG_CMD_REMOVE, dev, strlen(dev), NULL, 0);
}

void add_db_media_folder(char *data)
{
    msg_send(MSG_CMD_ADD_DB_MEDIA_FOLDER, data, strlen(data), NULL, 0);
}

void add_db_disk_path(char *data)
{
    msg_send(MSG_CMD_ADD_DB_DISK_PATH, data, strlen(data), NULL, 0);
}

void add_db_storage_config(char *data)
{
    msg_send(MSG_CMD_ADD_DB_STORAGE_CONFIG, data, strlen(data), NULL, 0);
}

void db_data_changed(char *json_str)
{
    msg_send(MSG_CMD_DB_DATA_CHANGED, json_str, strlen(json_str), NULL, 0);
}

void dev_changed(char *dev)
{
    msg_send(MSG_CMD_DEV_CHANGE, dev, strlen(dev), NULL, 0);
}

json_object *disks_to_json(struct FileStatus *status)
{
    json_object *j_cfg = json_object_new_object();
    json_object_object_add(j_cfg, "id", json_object_new_int(status->id));
    json_object_object_add(j_cfg, "sMountPath", json_object_new_string(status->mountpath));
    json_object_object_add(j_cfg, "sName", json_object_new_string(status->name));
    if (status->run) {
        json_object_object_add(j_cfg, "sDev", json_object_new_string(status->dev));
        json_object_object_add(j_cfg, "sAttributes", json_object_new_string(status->attributes));
        json_object_object_add(j_cfg, "sType", json_object_new_string(status->type));
    } else {
        json_object_object_add(j_cfg, "sDev", json_object_new_string(""));
        json_object_object_add(j_cfg, "sAttributes", json_object_new_string(""));
        json_object_object_add(j_cfg, "sType", json_object_new_string(""));
    }
    json_object_object_add(j_cfg, "iStatus", json_object_new_int(status->mount_status));
    json_object_object_add(j_cfg, "iMediaSize", json_object_new_int(status->media_size));
    json_object_object_add(j_cfg, "iTotalSize", json_object_new_int(status->total_size));
    json_object_object_add(j_cfg, "iFreeSize", json_object_new_int(status->free_size));
    json_object_object_add(j_cfg, "iFormatStatus", json_object_new_int(status->format_status));
    json_object_object_add(j_cfg, "iFormatProg", json_object_new_int(status->format_prog));
    if (status->format_err)
        json_object_object_add(j_cfg, "sFormatErr", json_object_new_string(status->format_err));
    else
        json_object_object_add(j_cfg, "sFormatErr", json_object_new_string(""));

    if (status->format_status == 2 || status->format_status == -1) {
        status->format_status = 0;
        status->format_prog = 0;
        if (status->format_err)
            g_free(status->format_err);
        status->format_err = NULL;
    }

    return j_cfg;
}

static json_object *cmd_get_disks_status(char *path)
{
    json_object *j_array = json_object_new_array();

    if (g_str_equal(path, "")) {
        GList *list, *values;
        list = values = g_hash_table_get_values(db_file_hash);
        while (values) {
            struct FileStatus *status = (struct FileStatus *)values->data;
            if (status != NULL) {
                json_object *j_ret = disks_to_json(status);
                json_object_array_add(j_array, j_ret);
            }
            values = values->next;
        }
        g_list_free(list);
    } else {
        struct FileStatus *status = g_hash_table_lookup(db_file_hash, path);
        if (status != NULL) {
            json_object *j_ret = disks_to_json(status);
            json_object_array_add(j_array, j_ret);
        }
    }

    return j_array;
}

DBusMessage *method_get_disks_status(DBusConnection *conn,
                                  DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array;
    const char *str;
    char *json_str;
    json_object *j_cfg;
    json_object *j_array;
    char *path;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    j_cfg = json_tokener_parse(json_str);

    path = (char *)json_object_get_string(json_object_object_get(j_cfg, "sMountPath"));

    j_array = cmd_get_disks_status(path);

    str = json_object_to_json_string(j_array);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    json_object_put(j_array);
    json_object_put(j_cfg);

    return reply;
}

static json_object *cmd_format(json_object *j_cfg)
{
    char *path = NULL;
    char *type = NULL;
    int ret = -1;

    json_object *j_ret = json_object_new_object();

    path = (char *)json_object_get_string(json_object_object_get(j_cfg, "sMountPath"));
    type = (char *)json_object_get_string(json_object_object_get(j_cfg, "sType"));

    struct FileStatus *status = g_hash_table_lookup(db_file_hash, path);
    LOG_INFO("format path = %s, type = %s\n", path, type);
    if (status) {
        ret = 0;
        LOG_INFO("path = %s, run = %d, format_status = %d\n", status->mountpath, status->run, status->format_status);
        if (!status->format_status)
            pthread_create(&status->format_tid, NULL, format_thread, status);
    }

    json_object_object_add(j_ret, "iReturn", json_object_new_int(ret));
    json_object_object_add(j_ret, "sErrMsg", json_object_new_string(""));

    return j_ret;
}

DBusMessage *method_format(DBusConnection *conn,
                                  DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array;
    const char *str;
    char *json_str;
    json_object *j_cfg;
    json_object *j_array;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    j_cfg = json_tokener_parse(json_str);

    j_array = cmd_format(j_cfg);

    str = json_object_to_json_string(j_array);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    json_object_put(j_array);
    json_object_put(j_cfg);

    return reply;
}

DBusMessage *method_adddisk(DBusConnection *conn,
                                  DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array;
    const char *str;
    char *json_str;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    if (json_str)
        dev_add("", json_str);

    str = "";

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    return reply;
}

static json_object *cmd_media_storage_stop_notify(char *json_str) {
#ifdef WAIT_MEDIA_STOP
    LOG_INFO("json_str: %s\n", json_str);
    pthread_mutex_lock(&media_disk_mutex);
    if (media_wait_cnt)
        media_wait_cnt--;
    pthread_cond_signal(&media_disk_cond);
    pthread_mutex_unlock(&media_disk_mutex);
#else
    LOG_INFO("%s is disabled\n", __FUNCTION__);
#endif
    return NULL;
}

DBusMessage *method_media_storage_stop_notify(DBusConnection *conn,
                                  DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array;
    const char *str;
    char *json_str;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    if (json_str)
        cmd_media_storage_stop_notify(json_str);

    str = "";

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    return reply;
}

static json_object *cmd_getfilelist(json_object *j_cfg)
{
    char *col = 0;
    char *where = 0;
    char *table = 0;
    char *limit = 0;
    char *order = 0;
    int id = -1;
    int pathid = -1;
    char *path = 0;
    int istatus = -1;
    json_object *j_array;
    int starttime = -1;
    int endtime = -1;

    json_object *j_ret = json_object_new_object();

    json_object_object_foreach(j_cfg, key, val) {
        if (g_str_equal(key, "path")) {
            path = (char *)json_object_get_string(val);
        } else if (g_str_equal(key, "id")) {
            id = (int)json_object_get_int(val);
        } else if (g_str_equal(key, "pathid")) {
            pathid = (int)json_object_get_int(val);
        } else if (g_str_equal(key, "order")) {
            if ((int)json_object_get_int(val) == 0)
                order = "iTIME desc";
            else
                order = "iTIME asc";
        } else if (g_str_equal(key, "limit")) {
            limit = (char *)json_object_get_string(val);
        } else if (g_str_equal(key, "starttime")) {
            starttime = (int)json_object_get_int(val);
        } else if (g_str_equal(key, "endtime")) {
            endtime = (int)json_object_get_int(val);
        }
    }

    if (path) {
        json_object_object_add(j_ret, "PATH", json_object_new_string(path));
        GList *list, *values;
        list = values = g_hash_table_get_values(db_file_hash);
        while (values) {
            struct FileStatus *status = (struct FileStatus *)values->data;
            char *tmp = strstr(path, status->mountpath);
            if (tmp) {
                table = pathtodb(status->mountpath);
                id = status->id;
                pathid = getidbypath(status, path);
                istatus = status->mount_status;
                break;
            }
            values = values->next;
        }
        g_list_free(list);
    } else {
        GList *list, *values;
        list = values = g_hash_table_get_values(db_file_hash);
        while (values) {
            struct FileStatus *status = (struct FileStatus *)values->data;
            if (id == status->id) {
                table = pathtodb(status->mountpath);
                path = getpathbyid(status, pathid);
                istatus = status->mount_status;
                break;
            }

            values = values->next;
        }
        g_list_free(list);
        if (path) {
            json_object_object_add(j_ret, "PATH", json_object_new_string(path));
        } else {
            json_object_object_add(j_ret, "PATH", json_object_new_string(""));
        }
    }

    col = "sNAME,sSIZE,iTIME";
    if (starttime != -1) {
        if (endtime != -1) {
            where = g_strdup_printf("iPATHID=%d and iTIME>=%d and iTIME <=%d", pathid, starttime, endtime);
        } else {
            where = g_strdup_printf("iPATHID=%d and iTIME>=%d", pathid, starttime);
        }
    } else {
        if (endtime != -1) {
            where = g_strdup_printf("iPATHID=%d and iTIME <=%d", pathid, endtime);
        } else {
            where = g_strdup_printf("iPATHID=%d", pathid);
        }
    }

    json_object_object_add(j_ret, "iStatus", json_object_new_int(istatus));
    if (istatus == 1) {
        char *j_str = rkdb_select(table, col, where, order, limit);
        json_object *j_sel = json_tokener_parse(j_str);
        json_object *j_array = json_object_object_get(j_sel, "jData");
        json_object_object_add(j_ret, "FILES", json_tokener_parse(json_object_to_json_string(j_array)));
        json_object_put(j_sel);
        g_free(j_str);
    }

    g_free(where);
    g_free(table);

    return j_ret;
}

DBusMessage *method_getfilelist(DBusConnection *conn,
                                DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array;
    const char *str;
    char *json_str;
    json_object *j_cfg;
    json_object *j_array;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    j_cfg = json_tokener_parse(json_str);

    j_array = cmd_getfilelist(j_cfg);

    str = json_object_to_json_string(j_array);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    json_object_put(j_array);
    json_object_put(j_cfg);

    return reply;
}

static json_object *scanpathtojson(struct FileStatus *status)
{
    json_object *j_array = json_object_new_array();

    struct ScanPath *scanpath = status->scanpath;
    while (scanpath) {
        json_object *j_cfg = json_object_new_object();
        json_object_object_add(j_cfg, "sMediaPath", json_object_new_string(scanpath->media_path));
        if (scanpath->thumb_path)
            json_object_object_add(j_cfg, "sThumbPath", json_object_new_string(scanpath->thumb_path));
        else
            json_object_object_add(j_cfg, "sThumbPath", json_object_new_string(""));
        json_object_object_add(j_cfg, "sFormat", json_object_new_string(scanpath->db_media_folder->format));
        json_object_object_add(j_cfg, "iType", json_object_new_int(scanpath->db_media_folder->type));
        json_object_object_add(j_cfg, "iCount", json_object_new_int(scanpath->count));
        json_object_object_add(j_cfg, "iDuty", json_object_new_int(scanpath->db_media_folder->duty));
        json_object_object_add(j_cfg, "iUseSize", json_object_new_int(scanpath->totalsize));
        json_object_object_add(j_cfg, "iNum", json_object_new_int(scanpath->num));
        json_object_object_add(j_cfg, "iCamId", json_object_new_int(scanpath->db_media_folder->camid));
        json_object_array_add(j_array, j_cfg);
        scanpath = scanpath->next;
    }

    return j_array;
}

static json_object *cmd_get_media_path(void)
{
    json_object *j_cfg = json_object_new_object();
    GList *list, *values;

    list = values = g_hash_table_get_values(db_file_hash);
    while (values) {
        struct FileStatus *status = (struct FileStatus *)values->data;
        if (status && status->mountpath && storage_config.mountpath) {
            if (g_str_equal(status->mountpath, storage_config.mountpath)) {
                json_object_object_add(j_cfg, "sMountPath", json_object_new_string(status->mountpath));
                json_object_object_add(j_cfg, "sScanPath", scanpathtojson(status));
                json_object_object_add(j_cfg, "iStatus", json_object_new_int(status->mount_status));
                json_object_object_add(j_cfg, "iFreeSize", json_object_new_int(storage_config.freesize));
                json_object_object_add(j_cfg, "iFreeSizeNotice", json_object_new_int(storage_config.freesizenotice));
                json_object_object_add(j_cfg, "iLowFreeStatus", json_object_new_int(low_free_status));
            }
        }

        values = values->next;
    }
    if (list)
        g_list_free(list);

    return j_cfg;
}

DBusMessage *method_get_media_path(DBusConnection *conn,
                            DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array;
    const char *str;
    char *json_str;
    json_object *j_array;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    j_array = cmd_get_media_path();

    str = json_object_to_json_string(j_array);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    json_object_put(j_array);

    return reply;
}

static const GDBusMethodTable methods[] = {
    {
        GDBUS_METHOD("GetDisksStatus",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_get_disks_status)
    },
    {
        GDBUS_METHOD("GetFileList",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_getfilelist)
    },
    {
        GDBUS_METHOD("GetMediaPath",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_get_media_path)
    },
    {
        GDBUS_METHOD("DiskFormat",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_format)
    },
    {
        GDBUS_METHOD("AddDisk",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_adddisk)
    },
    {
        GDBUS_METHOD("MediaStorageStopNotify",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_media_storage_stop_notify)
    },
    { },
};

static const GDBusSignalTable signals[] = {
    {
        GDBUS_SIGNAL("UpdateDisksStatus",
        GDBUS_ARGS({ "json", "s" }))
    },
    {
        GDBUS_SIGNAL("UpdateMediaPath",
        GDBUS_ARGS({ "json", "s" }))
    },
    {
        GDBUS_SIGNAL("FreeSizeNotice",
        GDBUS_ARGS({ "json", "s" }))
    },
    {
        GDBUS_SIGNAL("FormatNotice",
        GDBUS_ARGS({ "json", "s" }))
    },
    { },
};

static int dbus_manager_init(DBusConnection *dbus_conn)
{
    g_dbus_register_interface(dbus_conn, "/",
                              STORAGE_MANAGER_FILE_INTERFACE,
                              methods,
                              signals, NULL, STORAGE_MANAGER_FILE_INTERFACE, NULL);

    return 0;
}

void manage_init(void)
{
    DBusError dbus_err;
    pthread_t tid;

    dbus_error_init(&dbus_err);
    dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, STORAGE_MANAGER, &dbus_err);

    db_file_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                         g_free, NULL);
    pthread_mutex_init(&scanfile_mutex, NULL);
    pthread_create(&tid, NULL, file_manage_thread, NULL);
    dbus_manager_init(dbus_conn);
}

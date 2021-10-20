#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "json-c/json.h"
#include "db_monitor.h"
#include "manage.h"
#include "dbserver.h"
#include "dbus_signal.h"

#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "db_monitor.c"

void signal_storage_datachanged(void *user_data)
{
    db_data_changed(user_data);
}

#ifdef AUTO_ADJUST_MEDIAPATH
static int set_emmc_path(char *emmc_path)
{
    char *json_str = dbserver_storage_disk_path_get_by_name("Emmc");
    if (json_str) {
        json_object *data = json_tokener_parse(json_str);
        for(;;) {
            printf("%s: json_object: %s\n", __FUNCTION__, json_str);
            json_object *j_data_array = json_object_object_get(data, "jData");
            if (!j_data_array) {
                LOG_INFO("j_data_array is null\n");
                break;
            }
            json_object *j_data = json_object_array_get_idx(j_data_array, 0);
            json_object *emmc_path_obj = json_object_object_get(j_data, "sPath");
            if (!emmc_path_obj) {
                LOG_INFO("emmc_path_obj is null\n");
                break;
            }
            char *db_emmc_path = (char *)json_object_get_string(emmc_path_obj);
            printf("db_emmc_path is %s\n", db_emmc_path);
            if (g_str_equal(emmc_path, db_emmc_path)) {
                LOG_INFO("emmc_path: %s is same to db_emmc_path", emmc_path);
                break;
            }
            dbserver_storage_disk_path_set_by_name("Emmc", emmc_path);
            char *mount_json_str = dbserver_get_storage_config();
            printf("mount_json_str: %s\n", mount_json_str);
            json_object *mount_data = json_tokener_parse(mount_json_str);
            for(;;) {
                json_object *j_mount_array = json_object_object_get(mount_data, "jData");
                if (!j_mount_array) {
                    LOG_INFO("j_mount_array is null\n");
                    break;
                }
                json_object *j_mount_data = json_object_array_get_idx(j_mount_array, 0);
                json_object *mount_path_obj = json_object_object_get(j_mount_data, "sMountPath");
                if (!mount_path_obj) {
                    LOG_INFO("mount_path_obj is null\n");
                    break;
                }
                char *db_mount_path = (char *)json_object_get_string(mount_path_obj);
                printf("db_mount_path: %s\n", db_mount_path);
                if (g_str_equal(db_emmc_path, db_mount_path)) {
                    LOG_INFO("mountpath is same to db_emmc_path, update it\n");
                    dbserver_update_storage_config_mountpath(emmc_path);
                }
                LOG_INFO("mountpath update end\n");
                break;
            }
            if (mount_data)
                json_object_put(mount_data);
            if (mount_json_str)
                g_free(mount_json_str);
            LOG_INFO("emmc path update end\n");
            break;
        }
        if (data)
            json_object_put(data);
        if (json_str)
            g_free(json_str);
        return 0;
    }

    return -1;
}
#endif

static int get_storage_config(void)
{
    char *json_str = dbserver_get_storage_config();

    if (json_str) {
        add_db_storage_config(json_str);
        g_free(json_str);
        return 0;
    }

    return -1;
}

static int get_meida_folder(void)
{
    char *json_str = dbserver_get_storage_media_folder();

    if (json_str) {
        add_db_media_folder(json_str);
        g_free(json_str);
        return 0;
    }

    return -1;
}

static int get_disk_path(void)
{
    char *json_str = dbserver_get_storage_disk_path(NULL);

    if (json_str) {
        add_db_disk_path(json_str);
        g_free(json_str);
        return 0;
    }

    return -1;
}

void db_monitor_init(void)
{
    disable_loop();
#ifdef AUTO_ADJUST_MEDIAPATH
    if (access("/dev/block/by-name/media", F_OK)) {
        LOG_INFO("/dev/block/by-name/media folder does not exist, using /userdata/ as emmc path\n");
        while (set_emmc_path("/userdata") != 0) {
            LOG_INFO("set_emmc_path, wait dbserver.\n");
            usleep(50000);
        }
    } else {
        LOG_INFO("/dev/block/by-name/media folder exist\n");
    }
#endif
    while (get_storage_config() != 0) {
        LOG_INFO("dbserver_get_storage_config, wait dbserver.\n");
        usleep(50000);
    }
    while (get_meida_folder() != 0) {
        LOG_INFO("dbserver_get_meida_folder, wait dbserver.\n");
        usleep(50000);
    }

    while (get_disk_path() != 0) {
        LOG_INFO("dbserver_get_disk_path, wait dbserver.\n");
        usleep(50000);
    }
    dbus_monitor_signal_registered(DBSERVER_STORAGE_INTERFACE, DS_SIGNAL_DATACHANGED, &signal_storage_datachanged);
}
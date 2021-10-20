#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <libgen.h>

#include <glib.h>
#include <gdbus.h>

#include <pthread.h>

#include "json-c/json.h"
#include "rkdb.h"
#include "uevent_monitor.h"
#include "db_monitor.h"
#include "manage.h"

#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "main.c"

enum {
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
};

int enable_minilog = 0;
int storage_manager_log_level = LOG_INFO;

char *db_file = "/tmp/file.db";

static void *main_init(void *arg)
{
    LOG_INFO("storage_manager init\n");
    msg_init();
    remove(db_file);
    rkdb_init(db_file);
    db_monitor_init();
    manage_init();
    uevent_monitor_init();
    LOG_INFO("storage_manager init finish\n");
}

int main( int argc , char ** argv)
{
    pthread_t thread_id;
    GMainLoop *main_loop;
    char *db_path;

#ifdef ENABLE_MINILOGGER
    enable_minilog = 1;
    __minilog_log_init(argv[0], NULL, false, false, "storage_manager","1.0");
#endif

    if (argc == 2)
        db_file = argv[1];

    db_path = g_strdup(db_file);
    db_path = dirname(db_path);

    if (access(db_path, 0)) {
        LOG_INFO("storage_manager: %s folder does not exist\n", db_path);
        return 0;
    }

    if (db_path)
        g_free(db_path);

    main_loop = g_main_loop_new(NULL, FALSE);

    pthread_create(&thread_id, NULL, (void*)main_init, NULL);

    g_main_loop_run(main_loop);

    if (main_loop)
        g_main_loop_unref(main_loop);

    return 0;
}

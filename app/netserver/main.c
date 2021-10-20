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
#include <gdbus.h>
#include <getopt.h>

#include <pthread.h>
#include "netctl.h"
#include "db_monitor.h"
#include "manage.h"
#include "udp_broadcast.h"
#include "port.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "main.c"

#define DEFAULT_NGINX_CONF_PATH "/etc/nginx/nginx.conf"

enum {
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
};

int enable_minilog = 0;
int netserver_log_level = LOG_WARN;
char *nginx_conf_actual_path = NULL;

static void *main_init(void *arg)
{
    LOG_INFO("netserver init\n");
    netctl_init();
    database_init();
    port_init(nginx_conf_actual_path);
    manage_init();
    netctl_run();
    udp_broadcast_init();
    LOG_INFO("netserver finish\n");
}

static void usage_tip(FILE *fp, int argc, char **argv) {
  fprintf(fp,
        "Usage: %s [options]\n"
        "Version %s\n"
        "Options:\n"
        "-c | --config      set nginx conf file \n"
        "-h | --help        for help \n\n", argv[0], "V1.0");
}

static const char short_options[] = "c:h";
static const struct option long_options[] = {
    {"config", required_argument, NULL, 'c'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}};

int main( int argc , char ** argv)
{
#ifdef ENABLE_MINILOGGER
    enable_minilog = 1;
    __minilog_log_init(argv[0], NULL, false, false, "netserver","1.0");
#endif
    pthread_t thread_id;
    GMainLoop *main_loop;

    for (;;) {
        int idx;
        int c;
        c = getopt_long(argc, argv, short_options, long_options, &idx);
        if (-1 == c)
            break;
        switch (c) {
        case 'c':
            nginx_conf_actual_path = (char *)optarg;
            break;
        case 'h':
            usage_tip(stdout, argc, argv);
            exit(EXIT_SUCCESS);
        default:
            usage_tip(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }
    if (!nginx_conf_actual_path) {
        LOG_WARN("The path of nginx.conf is not specified, use the default path\n");
        nginx_conf_actual_path = DEFAULT_NGINX_CONF_PATH;
    }

    main_loop = g_main_loop_new(NULL, FALSE);

    database_hash_init();
    netctl_hash_init();
    pthread_create(&thread_id, NULL, (void*)main_init, NULL);

    g_main_loop_run(main_loop);
    netctl_deinit();
    if (main_loop)
        g_main_loop_unref(main_loop);

    return 0;
}

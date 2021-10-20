// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <gdbus.h>
#include <glib.h>
#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "daemon_service.h"
#include "db_manager.h"
#include "system_manager.h"

static unsigned int g_service_check_period_ms = 0;
static unsigned int g_no_mediaserver = 0;

static void usage_tip(FILE *fp, int argc, char **argv) {
	fprintf(fp, "Usage: %s [options]\n"
			"Version %s\n"
			"Options:\n"
			"-d | --debug    Debug option\n"
			"-n | --no-mediaserver without mediaserver\n"
			"-h | --help     For help \n"
			"\n",
			argv[0], "V1.0");
}

static const char short_options[] = "dhn";
static const struct option long_options[] = {
	{"debug", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"no-mediaserver", no_argument, NULL, 'n'},
	{0, 0, 0, 0}};

static void parse_args(int argc, char **argv) {
	for (;;) {
		int idx;
		int c;
		c = getopt_long(argc, argv, short_options, long_options, &idx);
		if (-1 == c)
			break;
		switch (c) {
			case 0: /* getopt_long() flag */
				break;
			case 'n':
				g_no_mediaserver = 1;
				break;
			case 'd':
				g_service_check_period_ms = 0;
				break;
			case 'h':
			default:
				usage_tip(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char **argv) {
  GMainLoop *main_loop;

  g_service_check_period_ms = SERVICE_CHECK_PERIOD_MS;

  parse_args(argc, argv);

  /* 1. check db file */
  db_manager_init_db();

  /* 2. start services and check services status periodically*/
  daemon_services_init(g_no_mediaserver);
  daemon_services_start(g_service_check_period_ms);

  /* 3. start system services, process system command */
  system_manager_init();

  main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(main_loop);
  g_main_loop_unref(main_loop);

  return 0;
}

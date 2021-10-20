// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "db_manager.h"

int db_manager_init_db(void) {
  if (!access(SYSCONFIG_DB_RUNNING_PATH, F_OK)) {
    return 0;
  }

  /* copy defaut db to runtime db */
  return copy(SYSCONFIG_DB_DEFAULT_PATH, SYSCONFIG_DB_RUNNING_PATH);
}

int db_manager_reset_db(void) {
  /* copy defaut db to runtime db */
  return copy(SYSCONFIG_DB_DEFAULT_PATH, SYSCONFIG_DB_RUNNING_PATH);
}

int db_manager_import_db(char *db_path) {
  /* TODO: check db*/

  /* copy new db to runtime db */
  return copy(db_path, SYSCONFIG_DB_RUNNING_PATH);
}

int db_manager_export_db(char *db_path) {
  /* copy runtime db to export db */
  return copy(SYSCONFIG_DB_RUNNING_PATH, db_path);
}

// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int system_cmd(const char *cmd) {
  pid_t status;

  if (!cmd)
    return -1;

  LOG("%s: %s\n", __func__, cmd);

  status = system(cmd);

  if (-1 == status) {
    ERROR("system error!");
  } else {
    if (WIFEXITED(status)) {
      if (0 == WEXITSTATUS(status)) {
        return 0;
      } else {
        ERROR("run shell script fail, script exit code: %d\n",
              WEXITSTATUS(status));
      }
    } else {
      ERROR("exit status = [%d]\n", WEXITSTATUS(status));
    }
  }
  return status;
}

int copy(const char *src, const char *dst) {
  if (!src || !dst)
    return -1;

  LOG("%s: %s -> %s\n", __func__, src, dst);

  if (!access(src, F_OK)) {
    char cmd[128] = {'\0'};
    snprintf(cmd, 127, "cp %s %s\n", src, dst);
    return system_cmd(cmd);
  } else {
    ERROR("File %s does not exist\n", src);
    return -1;
  }
}

// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __COMMON_H__
#define __COMMON_H__

#include <minilogger/log.h>

#define LOG minilog_info
#define LOGD minilog_debug
#define ERROR minilog_error

#ifdef __cplusplus
extern "C" {
#endif

int copy(const char *src, const char *dst);

#ifdef __cplusplus
}
#endif

#endif

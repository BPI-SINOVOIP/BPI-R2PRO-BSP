// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_MANAGER "rockchip.system"
#define SYSTEM_MANAGER_PATH "/"
#define SYSTEM_MANAGER_INTERFACE SYSTEM_MANAGER ".server"
#define ERROR_INTERFACE SYSTEM_MANAGER ".Error"

void system_manager_init(void);

#ifdef __cplusplus
}
#endif

#endif

// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __DAEMON_SEVICE_H__
#define __DAEMON_SEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SERVICE_CHECK_PERIOD_MS 3000
int daemon_services_init(int no_mediaserver);
int daemon_services_start(unsigned int timer_ms);
int daemon_services_stop(void);

#ifdef __cplusplus
}
#endif

#endif

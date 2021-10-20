// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef SHM_CONTROL_H_
#define SHM_CONTROL_H_

#include "flow_common.h"
#include <mutex>
#include <shmc/shm_queue.h>
#include <stdint.h>
#include <thread>
#include <vector>

extern shmc::ShmQueue<shmc::SVIPC> queue_w_;

namespace {
constexpr const char *kShmKey = "0x10007";
constexpr size_t kQueueBufSize = 1024 * 1024 * 1;
} // namespace

namespace ShmControl {

void PushUserHandler(void *handler, int type, void *buffer, int size);

} // namespace rndis_server

#endif // SHM_CONTROL_H_

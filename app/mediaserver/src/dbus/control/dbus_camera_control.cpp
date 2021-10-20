// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_camera_control.h"
#include "flow_export.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_camera_control.cpp"

namespace rockchip {
namespace mediaserver {

DBusCameraControl::DBusCameraControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_CAMERA_PATH) {}

DBusCameraControl::~DBusCameraControl() {}

int32_t DBusCameraControl::StartCamera(const int32_t &id) {
  LOG_INFO("DBusCameraControl::StartCamera\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::StopCamera(const int32_t &id) {
  LOG_INFO("DBusCameraControl::StopCamera\n");
  // TODO
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

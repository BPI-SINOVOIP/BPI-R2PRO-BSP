// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_CAMERA_CONTROL_H_
#define _RK_DBUS_CAMERA_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <memory>

#include "easymedia/buffer.h"
#include "easymedia/encoder.h"
#include "easymedia/flow.h"
#include "easymedia/key_string.h"
#include "easymedia/media_config.h"
#include "easymedia/media_type.h"
#include "easymedia/utils.h"

#include "dbus_dispatcher.h"
#include "flow_export.h"

namespace rockchip {
namespace mediaserver {

class FlowUnit;
class DBusCameraControl : public control::camera_adaptor,
                          public DBus::IntrospectableAdaptor,
                          public DBus::ObjectAdaptor {
public:
  DBusCameraControl() = delete;
  DBusCameraControl(DBus::Connection &connection);
  virtual ~DBusCameraControl();

  friend std::shared_ptr<CameraControl> GetCameraControl(int id);

  int32_t StartCamera(const int32_t &id);
  int32_t StopCamera(const int32_t &id);

private:
  std::shared_ptr<CameraControl> camera_control_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_DBUS_CAMERA_CONTROL_H_
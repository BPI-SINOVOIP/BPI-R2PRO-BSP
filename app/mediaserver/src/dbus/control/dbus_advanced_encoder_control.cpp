// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_advanced_encoder_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_advanced_encoder_control.cpp"

namespace rockchip {
namespace mediaserver {

DBusAdvancedEncoderControl::DBusAdvancedEncoderControl(
    DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_ADVANCED_ENCODER_PATH) {}

DBusAdvancedEncoderControl::~DBusAdvancedEncoderControl() {}

int32_t DBusAdvancedEncoderControl::SSetQp(const int32_t &id,
                                           const std::vector<int32_t> &param) {
  LOG_INFO("DBusAdvancedEncoderControl::SetGOP id %d\n", id);
  VideoEncoderQp qp;
  qp.qp_init = param[0];
  qp.qp_step = param[1];
  qp.qp_min = param[2];
  qp.qp_max = param[3];
  qp.qp_min_i = param[4];
  qp.qp_max_i = param[5];
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_)
    mpp_encoder_control_->SetImageQuality(qp);
  return 0;
}

int32_t
DBusAdvancedEncoderControl::SSetSplit(const int32_t &id,
                                      const std::vector<int32_t> &param) {
  LOG_INFO("DBusAdvancedEncoderControl::SSetSplit id %d\n", id);
  mpp_encoder_control_ = GetEncoderControl(id);
  int mode = param[0];
  int size = param[1];
  if (mpp_encoder_control_)
    mpp_encoder_control_->SetSplit(mode, size);
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

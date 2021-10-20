// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_ADVANCED_ENCODER_CONTROL_H_
#define _RK_DBUS_ADVANCED_ENCODER_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
class DBusAdvancedEncoderControl : public control::advancedencoder_adaptor,
                                   public DBus::IntrospectableAdaptor,
                                   public DBus::ObjectAdaptor {
public:
  DBusAdvancedEncoderControl() = delete;
  DBusAdvancedEncoderControl(DBus::Connection &connection);
  virtual ~DBusAdvancedEncoderControl();

  friend std::shared_ptr<EncoderControl> GetEncodersControl(int id);

  int32_t SSetQp(const int32_t &id, const std::vector<int32_t> &param);
  int32_t SSetSplit(const int32_t &id, const std::vector<int32_t> &param);

private:
  std::shared_ptr<EncoderControl> mpp_encoder_control_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MPP_ENCODER_H_

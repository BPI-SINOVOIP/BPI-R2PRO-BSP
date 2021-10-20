// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_ENCODER_CONTROL_H_
#define _RK_DBUS_ENCODER_CONTROL_H_

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
class DBusEncoderControl : public control::encoder_adaptor,
                           public DBus::IntrospectableAdaptor,
                           public DBus::ObjectAdaptor {
public:
  DBusEncoderControl() = delete;
  DBusEncoderControl(DBus::Connection &connection);
  virtual ~DBusEncoderControl();

  friend std::shared_ptr<EncoderControl> GetEncodersControl(int id);

  int32_t SetGOP(const int32_t &id, const int32_t &param);
  int32_t SetMaxRate(const int32_t &id, const int32_t &param);
  int32_t SetBitRate(const int32_t &id, const std::string &param);
  int32_t SetStreamSmooth(const int32_t &id, const int32_t &param);
  int32_t SetResolution(const int32_t &id, const std::string &param);
  int32_t SetFrameRate(const int32_t &id, const std::string &param);
  int32_t SetRCQuality(const int32_t &id, const std::string &param);
  int32_t SetOutputDataType(const int32_t &id, const std::string &param);
  int32_t SetRCMode(const int32_t &id, const std::string &param);
  int32_t SetH264Profile(const int32_t &id, const std::string &param);
  int32_t SetSmart(const int32_t &id, const std::string &param);
  int32_t SetSVC(const int32_t &id, const std::string &param);
  int32_t SetVideoType(const int32_t &id, const std::string &param);
  int32_t SetForceIdrFrame(const int32_t &id);
  int32_t SetFullRange();
  int32_t StopFlow(const int32_t &id);
  int32_t RestartFlow(const int32_t &id);
private:
  std::shared_ptr<EncoderControl> mpp_encoder_control_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MPP_ENCODER_H_

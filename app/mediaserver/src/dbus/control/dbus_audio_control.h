// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_AUDIO_CONTROL_H_
#define _RK_DBUS_AUDIO_CONTROL_H_

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
class DBusAudioControl : public control::audio_adaptor,
                         public DBus::IntrospectableAdaptor,
                         public DBus::ObjectAdaptor {
public:
  DBusAudioControl() = delete;
  DBusAudioControl(DBus::Connection &connection);
  virtual ~DBusAudioControl();

  friend std::shared_ptr<AudioControl> GetAudioControl(int id);

  int32_t SetSampleRate(const int32_t &id, const int32_t &param);
  int32_t SetVolume(const int32_t &id, const int32_t &param);
  int32_t SetBitRate(const int32_t &id, const int32_t &param);
  int32_t SetInput(const int32_t &id, const std::string &param);
  int32_t SetEncodeType(const int32_t &id, const std::string &param);
  int32_t SetANS(const int32_t &id, const std::string &param);

private:
  std::shared_ptr<AudioControl> audio_control_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_DBUS_CAMERA_CONTROL_H_
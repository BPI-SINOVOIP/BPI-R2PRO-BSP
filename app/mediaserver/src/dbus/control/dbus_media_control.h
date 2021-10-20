// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_MEDIA_CONTROL_H_
#define _RK_DBUS_MEDIA_CONTROL_H_

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

#include "dbus_advanced_encoder_control.h"
#include "dbus_audio_control.h"
#include "dbus_camera_control.h"
#include "dbus_dispatcher.h"
#include "dbus_encoder_control.h"
#ifdef USE_ROCKFACE
#include "dbus_face_recognize_control.h"
#endif
#include "dbus_feature_control.h"

namespace rockchip {
namespace mediaserver {

class FlowUnit;
class DBusMediaControl {
public:
  DBusMediaControl(){};
  virtual ~DBusMediaControl() {}
  int ConnectDBusServer(DBus::Connection &connection);

private:
  std::shared_ptr<DBusEncoderControl> encoder_controls_;
  std::shared_ptr<DBusAdvancedEncoderControl> adencoder_controls_;
  std::shared_ptr<DBusCameraControl> camera_controls_;
  std::shared_ptr<DBusAudioControl> audio_controls_;
  std::shared_ptr<DBusFeatureControl> feature_controls_;
#ifdef USE_ROCKFACE
  std::shared_ptr<DBusFaceRecognizeControl> face_recongnize_controls_;
#endif
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MEDIA_CONTROL_H_
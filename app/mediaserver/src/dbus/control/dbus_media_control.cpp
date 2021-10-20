// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_media_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_media_control.cpp"

namespace rockchip {
namespace mediaserver {

int DBusMediaControl::ConnectDBusServer(DBus::Connection &connection) {
  camera_controls_ = std::make_shared<DBusCameraControl>(connection);
  audio_controls_ = std::make_shared<DBusAudioControl>(connection);
  encoder_controls_ = std::make_shared<DBusEncoderControl>(connection);
  adencoder_controls_ =
      std::make_shared<DBusAdvancedEncoderControl>(connection);
  feature_controls_ = std::make_shared<DBusFeatureControl>(connection);
#ifdef USE_ROCKFACE
  face_recongnize_controls_ =
      std::make_shared<DBusFaceRecognizeControl>(connection);
#endif
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

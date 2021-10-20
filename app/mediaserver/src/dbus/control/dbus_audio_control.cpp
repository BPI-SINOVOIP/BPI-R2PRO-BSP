// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_audio_control.h"
#include "flow_export.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_audio_control.cpp"

namespace rockchip {
namespace mediaserver {

DBusAudioControl::DBusAudioControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_AUDIO_PATH) {}

DBusAudioControl::~DBusAudioControl() {}

int32_t DBusAudioControl::SetSampleRate(const int32_t &id,
                                        const int32_t &param) {
  LOG_INFO("DBusAudioControl::SetSampleRate id %d param %d\n", id, param);
  // TODO
  return 0;
}

int32_t DBusAudioControl::SetVolume(const int32_t &id, const int32_t &param) {
  LOG_INFO("DBusAudioControl::SetVolume id %d param %d\n", id, param);
  audio_control_ = GetAudioControl(id);
  if (audio_control_ == nullptr) {
    LOG_ERROR("DBusAudioControl::SetVolume id %d is no exist\n", id);
    return -1;
  }
  audio_control_->SetVolume(param);
  return 0;
}

int32_t DBusAudioControl::SetBitRate(const int32_t &id, const int32_t &param) {
  LOG_INFO("DBusAudioControl::SetBitRate id %d param %d\n", id, param);
  // TODO
  return 0;
}

int32_t DBusAudioControl::SetInput(const int32_t &id,
                                   const std::string &param) {
  LOG_INFO("DBusAudioControl::SetInput id %d param %s\n", id, param.c_str());
  // TODO
  return 0;
}

int32_t DBusAudioControl::SetEncodeType(const int32_t &id,
                                        const std::string &param) {
  LOG_INFO("DBusAudioControl::SetEncodeType id %d param %s\n", id,
           param.c_str());
  // TODO
  return 0;
}

int32_t DBusAudioControl::SetANS(const int32_t &id, const std::string &param) {
  LOG_INFO("DBusAudioControl::SetANS id %d param %s\n", id, param.c_str());
  // TODO
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_FACE_RECOGNIZE_CONTROL_H_
#define _RK_DBUS_FACE_RECOGNIZE_CONTROL_H_

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
class DBusFaceRecognizeControl : public control::facerecognize_adaptor,
                                 public DBus::IntrospectableAdaptor,
                                 public DBus::ObjectAdaptor {
public:
  DBusFaceRecognizeControl() = delete;
  DBusFaceRecognizeControl(DBus::Connection &connection);
  virtual ~DBusFaceRecognizeControl();

  int32_t SetImageToRegn(const int32_t &id, const std::string &path);
  int32_t SetPromptVolume(const int32_t &id, const int32_t &param);
  int32_t SetLiveDetectThreshold(const int32_t &id, const int32_t &param);
  int32_t SetFaceDetectionThreshold(const int32_t &id, const int32_t &param);
  int32_t SetFaceRecognitionThreshold(const int32_t &id, const int32_t &param);
  int32_t SetFaceMinPixel(const int32_t &id, const int32_t &param);
  int32_t SetLeftCornerX(const int32_t &id, const int32_t &param);
  int32_t SetLeftCornerY(const int32_t &id, const int32_t &param);
  int32_t SetDetectWidth(const int32_t &id, const int32_t &param);
  int32_t SetDetectHeight(const int32_t &id, const int32_t &param);
  int32_t SetLiveDetectEn(const int32_t &id, const std::string &param);
  int32_t SetLiveDetectBeginTime(const int32_t &id, const std::string &param);
  int32_t SetLiveDetectEndTime(const int32_t &id, const std::string &param);
  int32_t DeleteFaceInfo(const int32_t &id, const int32_t &faceId);
  int32_t ClearFaceDB(const int32_t &id, const int32_t &faceId);

private:
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_DBUS_FACE_RECOGNIZE_CONTROL_H_
// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_FEATURE_CONTROL_H_
#define _RK_DBUS_FEATURE_CONTROL_H_

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
class DBusFeatureControl : public control::feature_adaptor,
                           public DBus::IntrospectableAdaptor,
                           public DBus::ObjectAdaptor {
public:
  DBusFeatureControl() = delete;
  DBusFeatureControl(DBus::Connection &connection);
  virtual ~DBusFeatureControl();

  friend std::shared_ptr<AudioControl> GetAudioControl(int id);
  friend std::shared_ptr<CameraControl> GetCameraControl(int id);
  friend std::shared_ptr<EncoderControl> GetEncoderControl(int id);

  int32_t TakePicture(const int32_t &id, const int32_t &count);
  int32_t StartRecord(const int32_t &id);
  int32_t StopRecord(const int32_t &id);
  int32_t GetRecordStatus(const int32_t &id);
  int32_t SyncSchedules(const int32_t &id);
  int32_t SetOsd(const int32_t &db_region_id,
                 const std::map<std::string, std::string> &db_region_data);
  int32_t SetRoi(const int32_t &id, const std::string &roi_data);
  int32_t SetMoveDetect(const int32_t &id,
                        const std::map<std::string, std::string> &param);
  int32_t SetRegionInvade(const int32_t &id,
                          const std::map<std::string, std::string> &param);
  int32_t SetRockxStatus(const std::string &model_status);
  int32_t SetFaceDetectEn(const int32_t &id, const int32_t &enable);
  int32_t SetFaceRegEn(const int32_t &id, const int32_t &enable);
  int32_t SetFaceCaptureEn(const int32_t &id, const int32_t &enable);
  int32_t SetDrawFaceEn(const int32_t &id, const int32_t &enable);

private:
  std::shared_ptr<AudioControl> audio_control_;
  std::shared_ptr<CameraControl> camera_control_;
  std::shared_ptr<EncoderControl> encoder_control_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_DBUS_FEATURE_CONTROL_H_

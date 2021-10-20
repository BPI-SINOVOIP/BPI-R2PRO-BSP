// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_feature_control.h"
#include "flow_db_protocol.h"
#include "flow_export.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_feature_control.cpp"

namespace rockchip {
namespace mediaserver {

DBusFeatureControl::DBusFeatureControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_FEATURE_PATH) {}

DBusFeatureControl::~DBusFeatureControl() {}

int32_t DBusFeatureControl::TakePicture(const int32_t &id,
                                        const int32_t &count) {
  LOG_INFO("DBusFeatureControl::TakePicture id %d count %d\n", id, count);
  return rockchip::mediaserver::TakePhoto(id, count);
}

int32_t DBusFeatureControl::StartRecord(const int32_t &id) {
  LOG_INFO("DBusFeatureControl::StartRecord %id\n", id);
  return rockchip::mediaserver::StartRecord(id);
}

int32_t DBusFeatureControl::StopRecord(const int32_t &id) {
  LOG_INFO("DBusFeatureControl::StopRecord %id\n", id);
  return rockchip::mediaserver::StopRecord(id);
}

int32_t DBusFeatureControl::GetRecordStatus(const int32_t &id) {
  LOG_INFO("DBusFeatureControl::GetRecordStatus %id\n", id);
  return rockchip::mediaserver::GetRecordStatus(id);
}

int32_t DBusFeatureControl::SyncSchedules(const int32_t &id) {
  LOG_INFO("DBusFeatureControl::SyncSchedules\n");
#ifdef ENABLE_SCHEDULES_SERVER
  return rockchip::mediaserver::SyncSchedules();
#else
  return 0;
#endif
}

int32_t DBusFeatureControl::SetOsd(
    const int32_t &db_region_id,
    const std::map<std::string, std::string> &db_region_data) {
  LOG_INFO("DBusFeatureControl::SetOsd id %d\n", db_region_id);
  for (auto &iter : db_region_data) {
    LOG_DEBUG("key %s value %s\n", iter.first.c_str(), iter.second.c_str());
  }

#if (defined(ENABLE_OSD_SERVER) && defined(USE_ROCKFACE))
  SetOsdRegion(db_region_id, db_region_data);
#endif
  return 0;
}

int32_t DBusFeatureControl::SetRoi(const int32_t &id,
                                   const std::string &roi_data) {
  LOG_INFO("DBusFeatureControl::SetRoi id %d roi_data %s\n", id,
           roi_data.c_str());
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  auto roi_regions = db_protocol->GetRoiRegions(id);
  if (!roi_regions.empty())
    SetRoiRegion(id, roi_regions);
  return 0;
}

int32_t DBusFeatureControl::SetMoveDetect(
    const int32_t &id, const std::map<std::string, std::string> &param) {
  LOG_INFO("DBusFeatureControl::SetMoveDetect id %d\n", id);
  std::string value;
  value = param.find(DB_MOVE_DETECT_ENABLED)->second;
  int move_detect_enable = atoi(value.c_str());
  value = param.find(DB_MOVE_DETECT_SENSITIVITY)->second;
  int sensitivity_level = atoi(value.c_str());
  std::string grid_map = param.find(DB_MOVE_DETECT_GRID_MAP)->second;
  SetMDEnabled(move_detect_enable);
  SetMDSensitivity(sensitivity_level);
  SetMDRect();
  return 0;
}

int32_t DBusFeatureControl::SetRegionInvade(
    const int32_t &id, const std::map<std::string, std::string> &param) {
  LOG_INFO("DBusFeatureControl::SetRegionInvade id %d\n", id);
  for (auto &iter : param) {
    LOG_DEBUG("key %s value %s\n", iter.first.c_str(), iter.second.c_str());
  }
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER) &&                    \
     defined(ENABLE_DBUS))
  std::string value;
  region_invade_s region_invade;
  value = param.find(DB_REGION_INVADE_ENABLED)->second;
  region_invade.enable = atoi(value.c_str());
  value = param.find(DB_REGION_INVADE_WIDTH)->second;
  region_invade.width = atoi(value.c_str());
  value = param.find(DB_REGION_INVADE_HEIGHT)->second;
  region_invade.height = atoi(value.c_str());
  value = param.find(DB_REGION_INVADE_POSITION_X)->second;
  region_invade.position_x = atof(value.c_str());
  value = param.find(DB_REGION_INVADE_POSITION_Y)->second;
  region_invade.position_y = atof(value.c_str());
  value = param.find(DB_REGION_INVADE_PROPORTION)->second;
  region_invade.proportion = atoi(value.c_str());
  value = param.find(DB_REGION_INVADE_SENSITIVITY_LEVEL)->second;
  region_invade.sensitivity_level = atoi(value.c_str());
  value = param.find(DB_REGION_INVADE_TIME_THRESHOLD)->second;
  region_invade.time_threshold = atoi(value.c_str());
  rockchip::mediaserver::SetRegionInvade(id, region_invade);
#endif
  return 0;
}

int32_t DBusFeatureControl::SetRockxStatus(const std::string &model_status) {
#if (defined(USE_ROCKX))
  int pos = model_status.find(":");
  std::string model_name = model_status.substr(0, pos);
  int status =
      atoi(model_status.substr(pos + 1, model_status.size() - 1).c_str());
  LOG_INFO("DBusFeatureControl::SetRockxStatus model %s , status %d \n",
           model_name.c_str(), status);
  rockchip::mediaserver::SetRockxStatus(model_name, status);
#endif
  return 0;
}

int32_t DBusFeatureControl::SetFaceDetectEn(const int32_t &id,
                                            const int32_t &enable) {
  LOG_INFO("DBusFeatureControl::SetFaceDetectEn id %d enable %d\n", id, enable);
#if (defined(USE_ROCKFACE))
  int ret = 0;
  easymedia::FaceDetectArg fda;
  ret = GetFaceDetectArg(id, fda);
  if (ret < 0)
    return ret;
  fda.enable = enable;
  SetFaceDetectArg(id, fda);

  easymedia::NNinputArg nia;
  ret = GetNNInputArg(id, nia);
  if (ret < 0)
    return ret;
  nia.enable = enable;
  SetNNInputArg(id, nia);
#endif
  return 0;
}

int32_t DBusFeatureControl::SetFaceRegEn(const int32_t &id,
                                         const int32_t &enable) {
  LOG_INFO("DBusFeatureControl::SetFaceRegEn id %d enable %d\n", id, enable);
#if (defined(USE_ROCKFACE))
  int ret = 0;
  easymedia::FaceRegArg fra;
  fra.type = easymedia::USER_ENABLE;
  ret = GetFaceRegArg(id, fra);
  if (ret < 0)
    return ret;
  fra.enable = enable;
  SetFaceRegArg(id, fra);
#endif
  return 0;
}

int32_t DBusFeatureControl::SetFaceCaptureEn(const int32_t &id,
                                             const int32_t &enable) {
  LOG_INFO("DBusFeatureControl::SetFaceCaptureEn id %d enable %d\n", id,
           enable);
#if (defined(USE_ROCKFACE))
  int ret = 0;
  easymedia::FaceCaptureArg fca;
  ret = GetFaceCaptureArg(id, fca);
  if (ret < 0)
    return ret;
  fca.enable = enable;
  SetFaceCaptureArg(id, fca);
#endif
  return 0;
}

int32_t DBusFeatureControl::SetDrawFaceEn(const int32_t &id,
                                          const int32_t &enable) {
  LOG_INFO("DBusFeatureControl::SetDrawFaceEn id %d enable %d\n", id, enable);
#if (defined(USE_ROCKFACE))
  int ret = 0;
  easymedia::DrawFilterArg dfa;
  ret = GetDrawFilterArg(id, dfa);
  if (ret < 0)
    return ret;
  dfa.enable = enable;
  SetDrawFilterArg(id, dfa);
#endif
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

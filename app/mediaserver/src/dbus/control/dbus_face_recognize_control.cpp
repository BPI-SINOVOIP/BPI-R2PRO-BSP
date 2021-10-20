// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_face_recognize_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_face_recognize_control.cpp"

namespace rockchip {
namespace mediaserver {

DBusFaceRecognizeControl::DBusFaceRecognizeControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_FACE_RECOGNIZE_PATH) {}

DBusFaceRecognizeControl::~DBusFaceRecognizeControl() {}

int32_t DBusFaceRecognizeControl::SetImageToRegn(const int32_t &id,
                                                 const std::string &path) {
  LOG_INFO("DBusFaceRecognizeControl::SetImageToRegn id %d path %s\n", id,
           path.c_str());
#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
  LOG_INFO("DBusFaceRecognizeControl::SetImageToRegn id %d path %s\n", id,
           path.c_str());
  SetImageToRecognize(id, path);
#else
  LOG_INFO("DBusFaceRecognizeControl::SetImageToRegn id %d path %s, but rockface is disabled\n", id,
           path.c_str());
#endif
  return 0;
}

int32_t DBusFaceRecognizeControl::DeleteFaceInfo(const int32_t &id,
                                                 const int32_t &faceId) {
  LOG_INFO("DBusFaceRecognizeControl::DeleteFaceInfo id %d face id %d\n", id,
           faceId);
#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
  DeleteFaceInfoInDB(id, faceId);
#endif
  return 0;
}

int32_t DBusFaceRecognizeControl::ClearFaceDB(const int32_t &id,
                                                 const int32_t &faceId) {
  LOG_INFO("DBusFaceRecognizeControl::ClearFaceDB\n");
#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
  ClearFaceDBInfo();
#endif
  return 0;
}

int32_t DBusFaceRecognizeControl::SetPromptVolume(const int32_t &id,
                                                  const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetPromptVolume id %d path %d\n", id,
           param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetLiveDetectThreshold(const int32_t &id,
                                                         const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetLiveDetectThreshold id %d path %d\n",
           id, param);
  return 0;
}

int32_t
DBusFaceRecognizeControl::SetFaceDetectionThreshold(const int32_t &id,
                                                    const int32_t &param) {
  LOG_INFO(
      "DBusFaceRecognizeControl::SetFaceDetectionThreshold id %d path %d\n", id,
      param);
  return 0;
}

int32_t
DBusFaceRecognizeControl::SetFaceRecognitionThreshold(const int32_t &id,
                                                      const int32_t &param) {
  LOG_INFO(
      "DBusFaceRecognizeControl::SetFaceRecognitionThreshold id %d path %d\n",
      id, param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetFaceMinPixel(const int32_t &id,
                                                  const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetPromptVolume id %d path %d\n", id,
           param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetLeftCornerX(const int32_t &id,
                                                 const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetLeftCornerX id %d path %d\n", id,
           param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetLeftCornerY(const int32_t &id,
                                                 const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetLeftCornerY id %d path %d\n", id,
           param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetDetectWidth(const int32_t &id,
                                                 const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetDetectWidth id %d path %d\n", id,
           param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetDetectHeight(const int32_t &id,
                                                  const int32_t &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetDetectHeight id %d path %d\n", id,
           param);
  return 0;
}

int32_t DBusFaceRecognizeControl::SetLiveDetectEn(const int32_t &id,
                                                  const std::string &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetLiveDetectEn id %d param %s\n", id,
           param.c_str());
  return 0;
}

int32_t
DBusFaceRecognizeControl::SetLiveDetectBeginTime(const int32_t &id,
                                                 const std::string &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetLiveDetectBeginTime id %d param %s\n",
           id, param.c_str());
  return 0;
}

int32_t
DBusFaceRecognizeControl::SetLiveDetectEndTime(const int32_t &id,
                                               const std::string &param) {
  LOG_INFO("DBusFaceRecognizeControl::SetLiveDetectEndTime id %d param %s\n",
           id, param.c_str());
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

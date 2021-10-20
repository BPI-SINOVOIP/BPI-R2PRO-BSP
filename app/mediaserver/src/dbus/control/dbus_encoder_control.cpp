// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_encoder_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_encoder_control.cpp"

namespace rockchip {
namespace mediaserver {

DBusEncoderControl::DBusEncoderControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_ENCODER_PATH) {}

DBusEncoderControl::~DBusEncoderControl() {}

int32_t DBusEncoderControl::SetGOP(const int32_t &id, const int32_t &param) {
  LOG_INFO("DBusEncoderControl::SetGOP id %d param %d\n", id, param);
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetGOP id %d is no exist\n", id);
    return -1;
  }
  mpp_encoder_control_->SetGop(param);
  return 0;
}

int32_t DBusEncoderControl::SetMaxRate(const int32_t &id,
                                       const int32_t &param) {
  LOG_INFO("DBusEncoderControl::SetMaxRate id %d param %d\n", id, param);
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetMaxRate id %d is no exist\n", id);
    return -1;
  }
  int bps = param * KB_UNITS;
  mpp_encoder_control_->SetBitRate(bps);
  return 0;
}

int32_t DBusEncoderControl::SetBitRate(const int32_t &id,
                                       const std::string &param) {
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetBitRate id %d is no exist\n", id);
    return -1;
  }
  nlohmann::json para_json = nlohmann::json::parse(param);
  int minBps = 0;
  int maxBps = 0;
  int targetBps = 0;
  if (param.find("iMinRate") != std::string::npos)
    minBps = atoi(para_json.at("iMinRate").dump().c_str());
  if (param.find("iMaxRate") != std::string::npos)
    maxBps =  atoi(para_json.at("iMaxRate").dump().c_str());
  if (param.find("iTargetRate") != std::string::npos)
    targetBps = atoi(para_json.at("iTargetRate").dump().c_str());
  LOG_INFO("DBusEncoderControl::SetBitRate id %d, min %d, max: %d, target: %d\n",
            id, minBps, maxBps, targetBps);
  if (minBps || maxBps || targetBps) {
    minBps *= KB_UNITS;
    maxBps *= KB_UNITS;
    targetBps *= KB_UNITS;
    mpp_encoder_control_->SetBitRate2(minBps, maxBps, targetBps);
  }
  return 0;
}

int32_t DBusEncoderControl::SetStreamSmooth(const int32_t &id,
                                            const int32_t &param) {
  LOG_INFO("DBusEncoderControl::SetStreamSmooth id %d param %d\n", id, param);
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetMaxRate id %d is no exist\n", id);
    return -1;
  }
  mpp_encoder_control_->SetSreamSmooth(param);
  return 0;
}
int32_t DBusEncoderControl::SetResolution(const int32_t &id,
                                          const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetResolution id %d param %s\n", id,
           param.c_str());
  // NEED TODO，Just Reset Flow
  ResetPipes();
  return 0;
}

int32_t DBusEncoderControl::SetFrameRate(const int32_t &id,
                                         const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetFrameRate id %d param %s\n", id,
           param.c_str());
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_INFO("DBusEncoderControl::SetFrameRate id %d is no exist\n", id);
    return -1;
  }
  if (param.find("/") != std::string::npos) {
    int pos = param.find("/");
    int numerator = atoi(param.substr(0, pos).c_str());
    int denominator = atoi(param.substr(pos + 1, param.size() - 1).c_str());
    mpp_encoder_control_->SetFrameRate(numerator, denominator);
  } else {
    int numerator = atoi(param.c_str());
    int denominator = 1;
    mpp_encoder_control_->SetFrameRate(numerator, denominator);
  }
  return 0;
}

int32_t DBusEncoderControl::SetRCQuality(const int32_t &id,
                                         const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetRCQuality id %d param %s\n", id,
           param.c_str());
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetRCQuality id %d is no exist\n", id);
    return -1;
  }
  mpp_encoder_control_->SetRCQuality(param.c_str());
  return 0;
}

int32_t DBusEncoderControl::SetOutputDataType(const int32_t &id,
                                              const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetOutputDataType id %d param %s\n", id,
           param.c_str());
  // NEED TODO，Just Reset Flow
  ResetPipes();
  return 0;
}

int32_t DBusEncoderControl::SetRCMode(const int32_t &id,
                                      const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetRCMode id %d param %s\n", id, param.c_str());
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetRCMode id %d is no exist\n", id);
    return -1;
  }
  std::string value = param;
  for (int i = 0; i < value.size(); i++)
    value[i] = tolower(value[i]);
  const char *rc_mode = value.c_str();
  LOG_INFO("DBusEncoderControl::SetRCMode rc_mode %s %s\n", rc_mode, KEY_VBR);
  mpp_encoder_control_->SetRCMode(rc_mode);
  return 0;
}

int32_t DBusEncoderControl::SetH264Profile(const int32_t &id,
                                           const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetH264Profile id %d param %s\n", id,
           param.c_str());
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetH264Profile id %d is no exist\n", id);
    return -1;
  }
  mpp_encoder_control_->SetH264Profile(param.c_str());
  return 0;
}

int32_t DBusEncoderControl::SetSmart(const int32_t &id,
                                     const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetSmart id %d param %s\n", id, param.c_str());
  // NEED TODO，Just Reset Flow
  ResetPipes();
  return 0;
}

int32_t DBusEncoderControl::SetSVC(const int32_t &id,
                                   const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetSVC id %d param %s\n", id, param.c_str());
  // TODO
  return 0;
}

int32_t DBusEncoderControl::SetVideoType(const int32_t &id,
                                         const std::string &param) {
  LOG_INFO("DBusEncoderControl::SetVideoType id %d param %s\n", id,
           param.c_str());
  // TODO
  return 0;
}

int32_t DBusEncoderControl::SetForceIdrFrame(const int32_t &id) {
  LOG_INFO("DBusEncoderControl::SetForceIdrFrame id %d\n", id);
  mpp_encoder_control_ = GetEncoderControl(id);
  if (mpp_encoder_control_ == nullptr) {
    LOG_ERROR("DBusEncoderControl::SetForceIdrFrame id %d is no exist\n", id);
    return -1;
  }
  mpp_encoder_control_->SetForceIdrFrame();
  return 0;
}

int32_t DBusEncoderControl::SetFullRange() {
  LOG_INFO("DBusEncoderControl::SetFullRange\n");
  // Just Reset Flow
  ResetPipes();
  return 0;
}

int32_t DBusEncoderControl::StopFlow(const int32_t &id) {
  LOG_INFO("DBusEncoderControl::StopFlow\n");
  StopPipes();
  return 0;
}

int32_t DBusEncoderControl::RestartFlow(const int32_t &id) {
  LOG_INFO("DBusEncoderControl::RestartFlow\n");
  RestartPipes();
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

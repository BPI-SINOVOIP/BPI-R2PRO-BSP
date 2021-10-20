// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_control.h"
#include "flow_db_protocol.h"
#include "flow_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_control.cpp"

namespace rockchip {
namespace mediaserver {

int EncoderControl::SetFrameRate(const int num, const int den) {
  easymedia::video_encoder_set_fps(flow_, num, den);
  return 0;
}

int EncoderControl::SetBitRate(const int val) {
  // adaptive by rkmedia
  easymedia::video_encoder_set_bps(flow_, 0, 0, val);
  return 0;
}

int EncoderControl::SetBitRate2(const int minRate, const int maxRate, const int targetRate) {
  // adaptive by rkmedia
  easymedia::video_encoder_set_bps(flow_, targetRate, minRate, maxRate);
  return 0;
}

int EncoderControl::SetMaxRate(const int val) {
  easymedia::video_encoder_set_bps(flow_, 0, 0, val);
  return 0;
}

int EncoderControl::SetForceIdrFrame() {
  easymedia::video_encoder_force_idr(flow_);
  return 0;
}

int EncoderControl::SetMaxBitRate(const int val) {
  easymedia::video_encoder_set_bps(flow_, 0, 0, val);
  return 0;
}

int EncoderControl::SetGop(const int val) {
  auto value = std::make_shared<easymedia::ParameterBuffer>(0);
  value->SetValue(val);
  easymedia::video_encoder_set_gop_size(flow_, val);
  return 0;
}

int EncoderControl::SetSreamSmooth(const int val) {
  auto value = std::make_shared<easymedia::ParameterBuffer>(0);
  value->SetValue(val);
  // TODO
  return 0;
}

int EncoderControl::SetRCQuality(const char *val) {
  LOG_INFO("EncoderControl::SetRCQuality %s\n", val);
  std::string level;
  if (!strcmp(val, DB_VALUE_LEVEL_HIGHEST))
    level = KEY_HIGHEST;
  else if (!strcmp(val, DB_VALUE_LEVEL_HIGHER))
    level = KEY_HIGHER;
  else if (!strcmp(val, DB_VALUE_LEVEL_HIGH))
    level = KEY_HIGH;
  else if (!strcmp(val, DB_VALUE_LEVEL_MEDIUM))
    level = KEY_MEDIUM;
  else if (!strcmp(val, DB_VALUE_LEVEL_LOW))
    level = KEY_LOW;
  else if (!strcmp(val, DB_VALUE_LEVEL_LOWER))
    level = KEY_LOWER;
  else
    level = KEY_LOWEST;
  easymedia::video_encoder_set_rc_quality(flow_, level.c_str());
  return 0;
}

int EncoderControl::SetImageQuality(VideoEncoderQp qp) {
  easymedia::video_encoder_set_qp(flow_, qp);
  return 0;
}

int EncoderControl::SetRCMode(const char *val) {
  easymedia::video_encoder_set_rc_mode(flow_, val);
  return 0;
}

int EncoderControl::SetH264Profile(const char *val) {
  int profile_idc;
  if (!strcmp(val, DB_VALUE_H264_BASELINE))
    profile_idc = 66;
  else if (!strcmp(val, DB_VALUE_H264_MAIN))
    profile_idc = 77;
  else
    profile_idc = 100;
  LOG_INFO("EncoderControl::SetH264Profile profile_idc %d\n", profile_idc);
  easymedia::video_encoder_set_avc_profile(flow_, profile_idc);
  return 0;
}

int EncoderControl::SetH264Profile2(int val) {
  LOG_INFO("EncoderControl::SetH264Profile profile_idc %d\n", val);
  easymedia::video_encoder_set_avc_profile(flow_, val);
  return 0;
}

int EncoderControl::SetSVC(const int val) {
  LOG_INFO("TODO::EncoderControl::SetSVC %d\n", val);
  auto value = std::make_shared<easymedia::ParameterBuffer>(0);
  value->SetValue(val);
  // TODO
  return 0;
}

int EncoderControl::SetSmart264(const int val) {
  LOG_INFO("TODO::EncoderControl::SetSmart264 %d\n", val);
  auto value = std::make_shared<easymedia::ParameterBuffer>(0);
  value->SetValue(val);
  // TODO
  return 0;
}

int EncoderControl::SetRoiRegion(std::string roi_regions) {
  LOG_INFO("EncoderControl::SetRoiRegion %s\n", roi_regions.c_str());
  easymedia::video_encoder_set_roi_regions(flow_, roi_regions);
  return 0;
}

int EncoderControl::SetRoiRegion2(EncROIRegion *regions, int region_cnt) {
  LOG_INFO("EncoderControl::SetRoiRegion region_cnt is %d\n", region_cnt);
  easymedia::video_encoder_set_roi_regions(flow_, regions, region_cnt);
  return 0;
}

int EncoderControl::SetMultiSlice(const int width, const int height) {
  int split_mode = 1; // TODO: Configurable
  int split_size = width * height / 2;
  LOG_INFO("Split frame to slice with size = %d...\n", split_size);
  easymedia::video_encoder_set_split(flow_, split_mode, split_size);

  return 0;
}

int EncoderControl::SetSplit(const int mode, const int size) {
  easymedia::video_encoder_set_split(flow_, mode, size);
  return 0;
}

int EncoderControl::SetUserdata(void *data, int len, int all_frames) {
  easymedia::video_encoder_set_userdata(flow_, data, len, all_frames);
  return 0;
}

int AudioControl::SetSampleRate(const int val) {
  LOG_INFO("TODO::AudioControl::SetSampleRate %d\n", val);
  // TODO
  return 0;
}

int AudioControl::SetBitRate(const int val) {
  LOG_INFO("TODO::AudioControl::SetBitRate %d\n", val);
  // TODO
  return 0;
}

int AudioControl::SetVolume(const int val) {
  LOG_INFO("Set capture val %d\n", val);
  flow_->Control(easymedia::S_ALSA_VOLUME, &val);
  return 0;
}

int AudioControl::SetANS(const char *param) {
  LOG_INFO("TODO::AudioControl::SetANS %s\n", param);
  // TODO
  return 0;
}

FlowControl::FlowControl(std::shared_ptr<easymedia::Flow> &flow,
                         StreamType type)
    : stream_type_(type) {
  if (type == StreamType::CAMERA) {
    camera_control_ = std::make_shared<CameraControl>(flow);
  } else if (type == StreamType::VIDEO_ENCODER) {
    encoder_control_ = std::make_shared<EncoderControl>(flow);
  } else if (type == StreamType::AUDIO) {
    audio_control_ = std::make_shared<AudioControl>(flow);
  }
}

std::shared_ptr<CameraControl> FlowControl::GetCameraControl() {
  return camera_control_;
}

std::shared_ptr<EncoderControl> FlowControl::GetEncoderControl() {
  return encoder_control_;
}

std::shared_ptr<AudioControl> FlowControl::GetAudioControl() {
  return audio_control_;
}

} // namespace mediaserver
} // namespace rockchip

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_CONTROL_H_
#define _RK_FLOW_CONTROL_H_

#include <memory>

#include "buffer.h"
#include "encoder.h"
#include "flow.h"
#include "stream.h"

#include "flow_common.h"
#include "flow_unit.h"

namespace rockchip {
namespace mediaserver {

class CameraControl {
public:
  CameraControl() = delete;
  CameraControl(std::shared_ptr<easymedia::Flow> &flow) { flow_ = flow; }
  int SetFrameRate(const int val);

private:
  std::shared_ptr<easymedia::Flow> flow_;
};

class EncoderControl {
public:
  EncoderControl() = delete;
  EncoderControl(std::shared_ptr<easymedia::Flow> &flow) { flow_ = flow; }
  int SetFrameRate(const int num, const int den);
  int SetBitRate(const int val);
  int SetBitRate2(const int minRate, const int maxRate, const int targetRate);
  int SetMaxRate(const int val);
  int SetForceIdrFrame();
  int SetMaxBitRate(const int val);
  int SetGop(const int val);
  int SetSreamSmooth(const int val);
  int SetRCQuality(const char *val);
  int SetImageQuality(VideoEncoderQp qp);
  int SetRCMode(const char *val);
  int SetH264Profile(const char *val);
  int SetH264Profile2(int val);
  int SetSVC(const int val);
  int SetSmart264(const int val);
  int SetRoiRegion(std::string roi_regions);
  int SetRoiRegion2(EncROIRegion *regions, int region_cnt);
  int SetMultiSlice(const int width, const int height);
  int SetSplit(const int mode, const int size);
  int SetUserdata(void *data, int len, int all_frames);

private:
  std::shared_ptr<easymedia::Flow> flow_;
};

class AudioControl {
public:
  AudioControl() = delete;
  AudioControl(std::shared_ptr<easymedia::Flow> &flow) { flow_ = flow; }
  int SetSampleRate(const int val);
  int SetBitRate(const int val);
  int SetVolume(const int val);
  int SetANS(const char *param);

private:
  std::shared_ptr<easymedia::Flow> flow_;
};

class FlowControl {
public:
  FlowControl() = delete;
  FlowControl(std::shared_ptr<easymedia::Flow> &flow, StreamType type);
  virtual ~FlowControl() = default;

  std::shared_ptr<CameraControl> GetCameraControl();
  std::shared_ptr<EncoderControl> GetEncoderControl();
  std::shared_ptr<AudioControl> GetAudioControl();

private:
  StreamType stream_type_;
  std::shared_ptr<CameraControl> camera_control_;
  std::shared_ptr<EncoderControl> encoder_control_;
  std::shared_ptr<AudioControl> audio_control_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_FLOW_CONTROL_H_

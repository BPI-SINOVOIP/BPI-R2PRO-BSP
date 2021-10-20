// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_UNIT_H_
#define _RK_FLOW_UNIT_H_

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "flow.h"
#include "stream.h"

#include "flow_common.h"
#include "flow_control.h"

#ifdef ENABLE_OSD_SERVER
#include "osd/osd_server.h"
#endif

namespace rockchip {
namespace mediaserver {

class FlowControl;
class FlowUnit {
public:
  FlowUnit() {}
  virtual ~FlowUnit() {}
  friend class FlowParser;

  void DumpProps();
  props_v &GetFlowParamProp() { return flow_param_props_; }
  void SetFlowParamProp(props_v props) { flow_param_props_ = props; }
  props_v &GetStreamParamProp() { return stream_param_props_; }
  void SetStreamParamProp(props_v props) { stream_param_props_ = props; }

  std::string GetPropByName(props_v props, std::string name);
  int SetPropByName(props_v &props, std::string name, std::string value);
  FlowType GetFlowTypeByString(std::string type);
  std::string GetStringByFlowType(FlowType type);
  StreamType GetStreamTypeByString(std::string type);
  std::string GetStringByStreamType(StreamType type);

  std::string FlowParamMapToStr();
  std::string StreamParamMapToStr();

  std::shared_ptr<easymedia::Flow> &GetFlow() { return flow_; }
  void SetFlow(std::shared_ptr<easymedia::Flow> flow) { flow_ = flow; }

  // flow_index
  FlowType GetFlowType();
  int GetStreamId();
  StreamType GetStreamType();
  std::string GetFlowIndexName();
  std::string GetUpFlowIndexName();
  std::string GetFlowInputDataType();
  int GetOutSlotIndex();
  int GetInSlotIndexOfDown();
  int GetOpenPipeId();
  int GetOpenFlowId();
  int GetFixResolution();
  int GetAjustResolution();
  std::string GetProductKey();
  std::string GetProductSecret();
  std::string GetDeviceName();
  std::string GetDeviceSecret();
  std::string GetPID();
  std::string GetUUID();
  std::string GetAuthkey();

  // flow name
  std::string GetFlowName();
  std::string GetStreamName();

  // flow & stream param
  std::string GetDevice();
  std::string GetOutputDataType();
  void SetOutputDataType(std::string type);
  std::string GetInputDataType();
  void SetInputDataType(std::string type);
  std::string GetNeedHwDraw();
  std::string GetStreamRockxModel();
  std::string GetKeyEventPath();
  int GetKeyEventCode();

  // flow & stream video param
  std::string GetGop();
  std::string GetMaxRate();
  std::string GetStreamSmooth();
  std::string GetFrameRate();
  std::string GetResolution();
  std::string GetImageQuality();
  std::string GetRCMode();
  std::string GetRCQuality();
  std::string GetSmart();
  std::string GetSVC();
  std::string GetVideoType();
  std::string GetRoiRegions();
  std::string GetRIRegions();

  void SetGop(std::string value);
  void SetBitRate(std::string value);
  void SetMinRate(std::string value);
  void SetMaxRate(std::string value);
  void SetStreamSmooth(std::string value);
  void SetFrameRate(std::string value);
  void SetFrameRateIn(std::string value);
  void SetResolution(std::string width, std::string height);
  void GetResolution(std::string &width, std::string &height);
  void SetResolution2(std::string width, std::string height);
  void SetImageQuality(std::string value);
  void SetRCMode(std::string value);
  void SetRCQuality(std::string value);
  void SetH264Profile(std::string value);
  void SetH264Profile2(std::string value);
  void SetSmart(std::string value);
  void SetSVC(std::string value);
  void SetVideoType(std::string value);
  void SetRoiRegions(std::string value);
  void SetRIRegions(std::string value);
  void SetMoveDetecResolution(std::string width, std::string height);
  void SetMoveDetectRoiCnt(std::string value);
  void SetMoveDetectRoiRect(std::string value);
  void SetEnable(std::string value);
  void SetRegEnable(std::string value);
  void SetInterval(std::string value);
  void SetDuration(std::string value);
  void SetPercentage(std::string value);
  void SetSensitivity(std::string value);
  void SetPortNum(std::string value);
  void SetFullRange(std::string value);

  // flow & stream audio param
  std::string GetChannel();
  std::string GetSampleFormat();
  std::string GetSampleRate();

  void SetChannel(std::string value);
  void SetSampleRate(std::string value);
  void SetSampleFormat(std::string value);
  void SetVolume(std::string value);
  void SetAudioSource(std::string value);
  void SetEncodeType(std::string value);
  void SetANS(std::string value);

  void SetFilePath(std::string value);
  void SetFilePathInStreamParam(std::string value);
  std::string GetFilePath();
  void SetFilePreFix(std::string value);
  std::string GetFilePreFix();

  std::string GetRect();
  void SetRect(std::string value);

  std::string GetDetectRect();
  void SetDetectRect(std::string value);

  void BindControl(std::shared_ptr<easymedia::Flow> &flow, StreamType type) {
    if (!control_)
      control_ = std::make_shared<FlowControl>(flow, type);
  }

  void UnBindControl() {
    if (control_)
      control_.reset();
    control_ = nullptr;
  }

  std::shared_ptr<FlowControl> GetControl() {
    if (control_)
      return control_;
    return nullptr;
  }

#ifdef ENABLE_OSD_SERVER
  void RegisterOsdServer(std::shared_ptr<easymedia::Flow> &flow) {
    std::string width, height;
    GetResolution(width, height);
    int w = atoi(width.c_str());
    int h = atoi(height.c_str());
    osd_server_.reset(new OSDServer(flow, w, h));
    if (osd_server_) {
      osd_server_->start();
    }
  }

  void UnRegisterOsdServer() {
    if (osd_server_)
      osd_server_->stop();
    osd_server_.reset();
    osd_server_ = nullptr;
  }

  void SetOsdRegion(int region_id,
                    const std::map<std::string, std::string> &map) {
    if (osd_server_)
      osd_server_->SetOsdRegion(region_id, map);
  }

  void SetOsdTip(int x, int y, std::string tip) {
    if (osd_server_)
      osd_server_->SetOsdTip(x, y, tip);
  }

  void SetRegionInvade(region_invade_s region_invade) {
    if (osd_server_)
      osd_server_->SetRegionInvade(region_invade);
  }

  void SetRegionInvadeBlink(int blink_cnt, int interval_ms) {
    if (osd_server_)
      osd_server_->SetRegionInvadeBlink(blink_cnt, interval_ms);
  }
#endif

private:
  std::string flow_name_;
  props_v flow_index_props_;
  props_v flow_param_props_;
  props_v stream_param_props_;
  props_v extra_props_;

  std::shared_ptr<easymedia::Flow> flow_;
  std::shared_ptr<FlowControl> control_;

#ifdef ENABLE_OSD_SERVER
  std::unique_ptr<OSDServer> osd_server_;
#endif
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_FLOW_UNIT_H_

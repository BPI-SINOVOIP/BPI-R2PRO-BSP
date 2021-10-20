// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_PARSER_H_
#define _RK_FLOW_PARSER_H_

#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <vector>

#include "flow.h"
#include "flow_common.h"
#include "flow_unit.h"

namespace rockchip {
namespace mediaserver {

typedef std::vector<std::shared_ptr<FlowUnit>> flow_unit_v;
typedef std::vector<flow_unit_v> flow_pipe_v;

class FlowPipe;
class FlowParser {
public:
  FlowParser() = delete;
  FlowParser(const char *path);
  virtual ~FlowParser() {}

  void DumpPipes();

#if (defined(ENABLE_DBUS) || !defined(ENABLE_MEDIASERVER_BIN))
  void SyncCameraDBData(int id, std::string key, std::string value);
  void SyncEncoderDBData(int id, std::string key, std::string value);
  void SyncMoveDetecDBData(int id, std::string key, std::string value);
  void SyncMoveDetecDBData(int id, int md_roi_cnt, std::string md_roi_rect);
  void SyncSmartCoverDBData(int id, std::string key, std::string value);
  void SyncVideoDBData(int id, std::string key, std::string value);
  void SyncAudioDBData(int id, std::string key, std::string value);
  void SyncRoiDBData(int id, std::string value);
  void SyncRIDBData(int id, std::string value);
  void SyncRTSPDBData(int id, std::string key, std::string value);
  void SyncRTMPDBData(int id, std::string key, std::string value);
#endif

#ifdef ENABLE_DBUS
#if (defined(ENABLE_OSD_SERVER) && defined(USE_ROCKFACE))
  void SyncBodyDetectDBData(int id, std::string value);
#endif
#endif

  void SyncMuxerPath(int id, std::string value);
  void SyncFilePath(int id, std::string value);
  void SyncSnapPath(int id, std::string value);

  void SyncReOpenFlow();
  void SyncRgafiter();
  void SyncThrougfiter();
  void SyncRtsp();
  void SyncSmartEncorde();

  int JsConfigRead(const char *path);
  int JsConfigWrite(const char *path);
  int JsConfigParse();
  int JsConfigReBuild(std::vector<std::shared_ptr<FlowPipe>> pipes);

  flow_unit_v &GetFlowUnits(int pipe_index);

  int GetStreamById(int id, StreamType type);
  int GetStreamById(int id, StreamType type, std::string name);
  int GetPipeIndex(int id, StreamType type);
  int GetPipeIndex(int id, StreamType type, std::string name);
  int GetPipeNum() { return flow_pipes_.size(); }

  int GetFlowCount(int pipe_index, StreamType type);
  int GetFlowIndex(int pipe_index, StreamType type, int index = 0);
  int GetFlowIndex(int pipe_index, StreamType type, std::string name);
  int GetMuxerFlowIndex(int pipe_index, bool is_rtmp);
  int GetFaceCaptureFlowIndex(int pipe_index);
  int GetNextFlowIndex(int pipe_index, int index, StreamType type);
  int GetFlowIndexByFlowIndexName(int pipe_index, std::string name);
  int GetFlowIndexByUpFlowIndexName(int pipe_index, std::string name);

private:
  nlohmann::json config_js;
  flow_pipe_v flow_pipes_;
};

} // namespace mediaserver
} // namespace rockchip

#endif

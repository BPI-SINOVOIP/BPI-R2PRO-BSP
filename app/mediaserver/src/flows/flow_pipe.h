// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_PIPE_H_
#define _RK_FLOW_PIPE_H_

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iterator>
#include <list>
#include <vector>

#include "buffer.h"
#include "flow.h"
#include "stream.h"

#include "flow_common.h"
#include "flow_parser.h"
#include "flow_unit.h"

namespace rockchip {
namespace mediaserver {

class FlowPipe {
public:
  FlowPipe() {}
  virtual ~FlowPipe() = default;

  int GetFlowIndex(std::string flow_index_name);
  int GetFlowIndex(StreamType type);
  int GetFlowIndex(StreamType type, std::string name);
  int GetFlowIndexByInput(StreamType type, std::string data_type);

  std::shared_ptr<easymedia::Flow> GetFlow(int index);
  std::shared_ptr<easymedia::Flow> GetFlow(StreamType type);
  std::shared_ptr<easymedia::Flow> GetFlow(StreamType type, std::string name);
  std::shared_ptr<easymedia::Flow> GetFlow(StreamType type, std::string name,
                                           std::string model);
  std::shared_ptr<easymedia::Flow> GetFlowByInput(StreamType type,
                                                  std::string data_type);
  int SetFlow(std::shared_ptr<easymedia::Flow> flow, StreamType type);

  std::shared_ptr<FlowUnit> GetFlowunit(int index);
  std::shared_ptr<FlowUnit> GetFlowunit(StreamType type);
  std::shared_ptr<FlowUnit> GetFlowunit(StreamType type, std::string name);
  std::shared_ptr<FlowUnit> GetFlowunitByInput(StreamType type,
                                               std::string data_type);

  std::string GetFlowName(StreamType type);
  std::string GetFlowIndexName(StreamType type);
  std::string GetFlowParam(StreamType type);
  std::string GetStreamParam(StreamType type);

  friend int FlowEventProc(int pipe_index, int flow_index, bool &loop);

  int RegisterOsdServer();
  int UnRegisterOsdServer();

  int BindControler();
  int UnBindControler();

  int RegisterCallBack();
  int UnRegisterCallBack();

  int CreateFlow(std::shared_ptr<FlowUnit> flow_unit);
  void CreateFlows(flow_unit_v &flows);
  int InitFlow(int flow_index);
  int InitFlows();
  int InitMultiSlice();
  int DeinitFlowFirst(int flow_index);
  int DeinitFlowSecond(int flow_index);
  int DeinitFlows();
  int DeinitFlow(int flow_index);
  void DestoryFlows();

  std::shared_ptr<FlowControl> GetControler(StreamType type) {
    auto flow_unit = GetFlowunit(type);
    if (flow_unit)
      return flow_unit->GetControl();
    return nullptr;
  }

  int GetPipeSize() { return flow_units_.size(); }
  flow_unit_v &GetFlowUnits() { return flow_units_; }

private:
  flow_unit_v flow_units_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_FLOW_PIPE_H_

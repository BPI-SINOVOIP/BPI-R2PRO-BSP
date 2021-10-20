// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _FLOW_DB_PROTOCOL_H_
#define _FLOW_DB_PROTOCOL_H_

#include <nlohmann/json.hpp>
#include <string>

#if (defined(ENABLE_DBUS) || !defined(ENABLE_MEDIASERVER_BIN))
#include "dbus_dbserver_key.h"
#endif
#include "flow_export.h"

namespace rockchip {
namespace mediaserver {

extern int h264_qp_table[6][6];
extern int h265_qp_table[6][6];

#ifdef ENABLE_DBUS

class FlowDbProtocol {
public:
  FlowDbProtocol() {}
  virtual ~FlowDbProtocol() {}

  void DbDataDispatch(std::string db_data);
  void DbDataToMap(std::string db_data,
                   std::map<std::string, std::string> &map);
  std::string GetValueByKey(std::string key,
                            std::map<std::string, std::string> &map);
  std::string GetValueByKey(std::string db_data, std::string key);
  std::string GetRoiRegions(int stream_id);
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))
  void GetRegionInvade(std::map<std::string, std::string> map,
                       region_invade_s &region_invade);
  void GetRegionInvade(std::string db_data, region_invade_s &region_invade);
  std::string GetRegionInvadeRect(std::string db_data,
                                  std::string img_rect_str);
#endif
  std::string GetMoveDetectRegions(int id, std::string move_detect_db,
                                   int &md_roi_cnt);

private:
};

#endif

} // namespace mediaserver
} // namespace rockchip

#endif // _FLOW_DB_PROTOCOL_H_

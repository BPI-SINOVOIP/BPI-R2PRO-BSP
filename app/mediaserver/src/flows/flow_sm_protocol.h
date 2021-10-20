// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _FLOW_SM_PROTOCOL_H_
#define _FLOW_SM_PROTOCOL_H_

#include <nlohmann/json.hpp>
#include <string>

#ifdef ENABLE_DBUS
#include "dbus_storage_manager_key.h"
#endif
#include "flow_export.h"

namespace rockchip {
namespace mediaserver {

#ifdef ENABLE_DBUS

class FlowSMProtocol {
public:
  FlowSMProtocol() {}
  virtual ~FlowSMProtocol() {}

  void DisksStatusChanged(std::string dbus_data);
  void MediaPathChanged(std::string dbus_data);

  std::string GetStoragePath(std::string dbus_data);
  std::string GetMediaPath(std::string dbus_data, int type, int id);
  int GetDiskStatus(std::string dbus_data); // 0, invalid;  1: valid (mounted)
  std::string FreeSizeNotice(std::string dbus_data);
  std::string FormatNotice(std::string dbus_data);
  static std::string storage_path_;

private:
};

#endif

} // namespace mediaserver
} // namespace rockchip

#endif // _FLOW_DB_PROTOCOL_H_

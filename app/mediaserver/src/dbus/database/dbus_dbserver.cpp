// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_dbserver.h"
#include "flow_db_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_dbserver.cpp"

namespace rockchip {
namespace mediaserver {

DBusDbServer::DBusDbServer(DBus::Connection &connection,
                           const char *adaptor_path, const char *adaptor_name)
    : DBus::ObjectProxy(connection, adaptor_path, adaptor_name) {}

DBusDbServer::~DBusDbServer() {}

void DBusDbServer::DataChanged(const std::string &param) {
  // NO NEED TODO
}

std::string DBusDbServer::ProctectCmd(char *tmp_cmd, int try_times) {
  std::string rst;
  while (try_times) {
    try {
      rst = Cmd(tmp_cmd);
      break;
    } catch (DBus::Error err) {
      LOG_ERROR("%s:try cmd: %s, time:%d, fail\n", __FUNCTION__, tmp_cmd, try_times);
    }
    try_times--;
  }
  return rst;
}

std::string DBusDbServer::SelectVideoDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_VIDEO_CMD, id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbServer::SelectAudioDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_AUDIO_CMD, id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbServer::SelectOsdDb(int region_id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_OSD_REGION_CMD, region_id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbServer::SelectRoiDb(int region_id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_ROI_REGION_CMD, region_id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbServer::SelectPortDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_PORT_CMD, id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbServer::SelectImageScenarioDb() {
  char tmp[128];
  sprintf(tmp, DB_SELECT_IMAGE_SCENARIO_CMD);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbServer::SelectImageEnhanceDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_IMAGE_ENHANCEMENT_CMD, id);
  return ProctectCmd(tmp, -1);
}

DBusDbListen::DBusDbListen(DBus::Connection &connection)
    : DBus::InterfaceProxy(DBSERVE_DBCHANGE_INTERFACE),
      DBus::ObjectProxy(connection, DBSERVE_PATH, DBSERVE_BUS_NAME) {
  connect_signal(DBusDbListen, DataChanged, DataChangedCb);
}

DBusDbListen::~DBusDbListen() {}

void DBusDbListen::DataChangedCb(const DBus::SignalMessage &sig) {
  return;
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  LOG_INFO("DBusDbListen DataChanged :\n[%s]\n", db_data.c_str());
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  db_protocol->DbDataDispatch(db_data);
}

DBusDbEvent::DBusDbEvent(DBus::Connection &connection, const char *adaptor_path,
                         const char *adaptor_name)
    : DBus::ObjectProxy(connection, adaptor_path, adaptor_name) {}

DBusDbEvent::~DBusDbEvent() {}

void DBusDbEvent::DataChanged(const std::string &param) {
  // NO NEED TODO
}

std::string DBusDbEvent::ProctectCmd(char *tmp_cmd, int try_times) {
  std::string rst;
  while (try_times) {
    try {
      rst = Cmd(tmp_cmd);
      break;
    } catch (DBus::Error err) {
      LOG_ERROR("%s:try cmd: %s, time:%d, fail\n", __FUNCTION__, tmp_cmd, try_times);
    }
    try_times--;
  }
  return rst;
}

std::string DBusDbEvent::SelectRegionInvade(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_REGION_INVADE_CMD, id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbEvent::SelectMoveDetectDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_MOVE_DETECTION_CMD, id);
  return ProctectCmd(tmp, -1);
}

std::string DBusDbEvent::SelectSmartCoverDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_SMART_COVER_CMD, id);
  return ProctectCmd(tmp, -1);
}

DBusDbEventListen::DBusDbEventListen(DBus::Connection &connection)
    : DBus::InterfaceProxy(DBSERVE_DBEVENT_CHANGE_INTERFACE),
      DBus::ObjectProxy(connection, DBSERVE_PATH, DBSERVE_BUS_NAME) {
  connect_signal(DBusDbEventListen, DataChanged, DataChangedCb);
}

DBusDbEventListen::~DBusDbEventListen() {}

void DBusDbEventListen::DataChangedCb(const DBus::SignalMessage &sig) {
  return;
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  LOG_INFO("DBusDbEventListen DataChanged :\n[%s]\n", db_data.c_str());
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  db_protocol->DbDataDispatch(db_data);
}

} // namespace mediaserver
} // namespace rockchip

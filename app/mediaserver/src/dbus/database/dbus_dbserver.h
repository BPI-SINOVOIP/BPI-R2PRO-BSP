// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_DBSERVER_PROXY_H_
#define _RK_DBUS_DBSERVER_PROXY_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_dbserver_proxy.h"
#include "dbus_dispatcher.h"
#include <dbus-c++/dbus.h>

namespace rockchip {
namespace mediaserver {

#define DB_UPDATE_CMD                                                          \
  "{ \"table\": \"media\", "                                                   \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": { \"sResolution\": \"1280*720\", "                                \
  "\"iMaxRate\": 500, "                                                        \
  "\"cmd\": \"Update\" }"

#define DB_SELECT_VIDEO_CMD                                                    \
  "{ \"table\": \"video\", "                                                   \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_AUDIO_CMD                                                    \
  "{ \"table\": \"audio\", "                                                   \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_OSD_REGION_CMD                                               \
  "{ \"table\": \"osd\", "                                                     \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_ROI_REGION_CMD                                               \
  "{ \"table\": \"roi\", "                                                     \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_PORT_CMD                                                     \
  "{ \"table\": \"port\", "                                                    \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_IMAGE_SCENARIO_CMD                                           \
  "{ \"table\": \"image_scenario\", "                                          \
  "\"key\": { \"id\": 0 }, "                                                   \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_IMAGE_ENHANCEMENT_CMD                                        \
  "{ \"table\": \"image_enhancement\", "                                       \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

class DBusDbServer : public rockchip::dbserver::media_proxy,
                     public DBus::IntrospectableProxy,
                     public DBus::ObjectProxy {
public:
  DBusDbServer(DBus::Connection &connection, const char *adaptor_path,
               const char *adaptor_name);
  virtual ~DBusDbServer();

  std::string SelectVideoDb(int id);
  std::string SelectAudioDb(int id);
  std::string SelectOsdDb(int region_id);
  std::string SelectRoiDb(int region_id);
  std::string SelectPortDb(int id);
  std::string SelectImageScenarioDb();
  std::string SelectImageEnhanceDb(int id);
  void UpdateMediaDb(std::string table) { Cmd(table); }
  void DataChanged(const std::string &param);

private:
  std::string ProctectCmd(char *tmp_cmd, int try_times);
  DBus::Pipe *pipe_;
};

class DBusDbListen : public DBus::InterfaceProxy, public DBus::ObjectProxy {
public:
  DBusDbListen(DBus::Connection &connection);
  virtual ~DBusDbListen();
  void DataChangedCb(const DBus::SignalMessage &sig);
};

#define DB_SELECT_REGION_INVADE_CMD                                            \
  "{ \"table\": \"RegionalInvasion\", "                                        \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_MOVE_DETECTION_CMD                                           \
  "{ \"table\": \"MoveDetection\", "                                           \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

#define DB_SELECT_SMART_COVER_CMD                                              \
  "{ \"table\": \"SmartCover\", "                                              \
  "\"key\": { \"id\": %d }, "                                                  \
  "\"data\": \"*\", "                                                          \
  "\"cmd\": \"Select\" }"

class DBusDbEvent : public rockchip::dbserver::event_proxy,
                    public DBus::IntrospectableProxy,
                    public DBus::ObjectProxy {
public:
  DBusDbEvent(DBus::Connection &connection, const char *adaptor_path,
              const char *adaptor_name);
  virtual ~DBusDbEvent();
  void DataChanged(const std::string &param);
  std::string SelectRegionInvade(int id);
  std::string SelectMoveDetectDb(int id);
  std::string SelectSmartCoverDb(int id);

private:
  std::string ProctectCmd(char *tmp_cmd, int try_times);
  DBus::Pipe *pipe_;
};

class DBusDbEventListen : public DBus::InterfaceProxy,
                          public DBus::ObjectProxy {
public:
  DBusDbEventListen(DBus::Connection &connection);
  virtual ~DBusDbEventListen();
  void DataChangedCb(const DBus::SignalMessage &sig);
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MEDIA_CONTROL_H_

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_MANAGER_H_
#define _RK_FLOW_MANAGER_H_

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <iterator>
#include <list>
#include <vector>

#include "buffer.h"
#include "control.h"
#include "encoder.h"
#include "flow.h"
#include "key_string.h"
#include "media_config.h"
#include "media_type.h"
#include "message.h"
#include "message.h"
#include "stream.h"
#include "utils.h"

#include "flow_nn_handler.h"
#include "flow_parser.h"
#include "flow_pipe.h"

#ifdef ENABLE_DBUS
#include "dbus_dbserver.h"
#include "dbus_storage_manager.h"
#include "dbus_ispserver.h"
#endif

#ifdef ENABLE_OSD_SERVER
#include "osd_common.h"
#endif

#ifdef ENABLE_SCHEDULES_SERVER
#include "schedules_manager.h"
#endif

#ifdef ENABLE_ZBAR
#include "zbar/scan_image.h"
#endif

#ifndef ENABLE_MEDIASERVER_BIN
typedef void (*ExternMDCallbackPtr)(MoveDetectEvent *);
#endif

namespace rockchip {
namespace mediaserver {

class FlowManager;
typedef std::shared_ptr<FlowManager> FlowManagerPtr;

class FlowManager {
public:
  FlowManager() {}
  virtual ~FlowManager() {}

  int SyncFlowParser();
  int SyncConfig();
  int ConfigParse(std::string conf);
  int SaveConfig(std::string conf);

  int BindReuseFlow();

  int RegisterRtspHandler();
  int UnRegisterRtspHandler();
  int RegisterRtspHandler(int pipe_index);
  int UnRegisterRtspHandler(int pipe_index);

#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))
  int CreateNNHandler();
  int DestoryNNHandler();
  std::shared_ptr<NNHandler> &GetNNHander() { return nn_handler_; }
#endif

  int CreatePipes();
  int DestoryPipes();
  int ResetPipes();
  int StopPipes();
  int RestartPipes();
  int CreatePipe(int id, StreamType type);
  int DestoryPipe(int id, StreamType type);
  int ResetPipe(int id, StreamType type);
  int ResetFlow(int id, int flow_index);
  int ResetFlowByType(int id, StreamType type);
  int ResetFlowByInput(int id, StreamType type, std::string input_data_type);

#ifdef ENABLE_DBUS
  int SyncDBConfig();
  int SyncSMConfig();
  int GetDiskStatus(); /* 0, invalid;	1: valid (mounted) */

  int RegisterDBserverProxy(std::shared_ptr<DBusDbServer> db_server) {
    db_server_ = db_server;
    return 0;
  }

  int RegisterDBEventProxy(std::shared_ptr<DBusDbEvent> db_event) {
    db_event_ = db_event;
    return 0;
  }

  int RegisterStorageManagerProxy(
      std::shared_ptr<DBusStorageManager> storage_manager) {
    storage_manager_ = storage_manager;
    return 0;
  }

  int RegisterIspserverProxy(std::shared_ptr<DBusIspserver> ispserver) {
    ispserver_ = ispserver;
    return 0;
  }

  std::string SelectVideoDb(int id) {
    std::string empty;
    if (db_server_)
      return db_server_->SelectVideoDb(id);
    return empty;
  }

  std::string SelectAudioDb(int id) {
    std::string empty;
    if (db_server_)
      return db_server_->SelectAudioDb(id);
    return empty;
  }

  std::string SelectOsdDb(int region_id) {
    std::string empty;
    if (db_server_)
      return db_server_->SelectOsdDb(region_id);
    return empty;
  }

  std::string SelectRoiDb(int region_id) {
    std::string empty;
    if (db_server_)
      return db_server_->SelectRoiDb(region_id);
    return empty;
  }

  std::string SelectPortDb(int id) {
    std::string empty;
    if (db_server_)
      return db_server_->SelectPortDb(id);
    return empty;
  }

  std::string SelectImageScenarioDb() {
    std::string empty;
    if (db_server_)
      return db_server_->SelectImageScenarioDb();
    return empty;
  }

  std::string SelectImageEnhanceDb(int id) {
    std::string empty;
    if (db_server_)
      return db_server_->SelectImageEnhanceDb(id);
    return empty;
  }

  std::string SelectRegionInvadeDb(int region_id) {
    std::string empty;
    if (db_event_)
      return db_event_->SelectRegionInvade(region_id);
    return empty;
  }

  std::string SelectMoveDetectDb(int id) {
    std::string empty;
    if (db_event_)
      return db_event_->SelectMoveDetectDb(id);
    return empty;
  }

  std::string SelectSmartCoverDb(int id) {
    std::string empty;
    if (db_event_)
      return db_event_->SelectSmartCoverDb(id);
    return empty;
  }

  void UpdateMediaDb(std::string table) {
    if (db_server_)
      db_server_->UpdateMediaDb(table);
  }

  std::string GetStorageStatus() {
    std::string empty;
    if (storage_manager_)
      return storage_manager_->GetStorageStatus();
    return empty;
  }

  std::string GetStoragePath() {
    std::string empty;
    if (storage_manager_)
      return storage_manager_->GetStoragePath();
    return empty;
  }

  std::string SendMediaStorageStopMsg() {
    std::string empty;
    if (storage_manager_)
      return storage_manager_->SendMediaStorageStopMsg();
    return empty;
  }

  std::string GetIspserverExposureDump() {
    std::string empty;
    if (ispserver_)
      return ispserver_->GetIspserverExposureDump();
    return empty;
  }

  void SendIspserverTurnoffSignal() {
    if (ispserver_)
      return ispserver_->SendIspserverTurnoffSignal();
  }

  friend class DBusServer;

#endif // ENABLE_DBUS

#ifdef ENABLE_OSD_SERVER
  std::map<std::string, std::string> &GetOsdParams(int region_id) {
    return osd_params_v_[region_id];
  }

  bool OsdParamsEmpty() { return osd_params_v_.size() == 0 ? true : false; }
#endif

  static FlowManagerPtr &GetInstance();

  std::vector<std::shared_ptr<FlowPipe>> &GetPipes() { return flow_pipes_; }
  std::shared_ptr<FlowPipe> &GetPipeByIndex(int index) {
    return flow_pipes_[index];
  }
  int GetPipesSize() { return flow_pipes_.size(); }
  int GetStreamById(int id, StreamType type);
  int GetStreamById(int id, StreamType type, std::string name);
  int GetPipeIndexById(int id, StreamType type);
  int GetPipeIndexById(int id, StreamType type, std::string name);

#ifndef ENABLE_MEDIASERVER_BIN
  int SyncVideoDBData(int id, std::string key, std::string value);
  int SyncAudioDBData(int id, std::string key, std::string value);
  void SetMDCallback(ExternMDCallbackPtr callback) {
    extern_md_callback_ = callback;
  }
  void MDCallback(MoveDetectEvent *event_info) {
    extern_md_callback_(event_info);
  }
#endif

#ifdef ENABLE_SCHEDULES_SERVER
  void InitScheduleMutex();
  void CreateSchedules();
  void DestorySchedules();
  void SyncSchedulesConfig();
  void StopSchedules();
  int SchedulesManagerIsRunning();
  void EventSnap() {
    if (schedules_manager_)
      schedules_manager_->EventSnap();
  }
  int MDTriggerEnabled() {
    if (schedules_manager_)
      return schedules_manager_->MDTriggerEnabled();
    else
      return 0;
  }
  void MDEventCountAdd() {
    if (schedules_manager_)
      schedules_manager_->MDEventCountAdd();
  }
#endif

#ifdef ENABLE_ZBAR
  void CreateScanImage();
  void DestoryScanImage();
#endif

  friend class MediaServer;

private:
  static FlowManagerPtr instance_;
  static bool restart_pipes_enable_;
  std::unique_ptr<FlowParser> flow_parser_;
  std::vector<std::shared_ptr<FlowPipe>> flow_pipes_;

#ifdef ENABLE_DBUS
  std::shared_ptr<DBusDbServer> db_server_;
  std::shared_ptr<DBusDbEvent> db_event_;
  std::shared_ptr<DBusStorageManager> storage_manager_;
  std::shared_ptr<DBusIspserver> ispserver_;
#endif

#ifdef ENABLE_OSD_SERVER
  std::vector<std::map<std::string, std::string>> osd_params_v_;
  std::vector<region_invade_s> region_invade_v_;
#endif

#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))
  std::shared_ptr<NNHandler> nn_handler_;
#endif

#ifdef ENABLE_SCHEDULES_SERVER
  std::unique_ptr<SchedulesManager> schedules_manager_;
  pthread_mutex_t schedule_mutex;
#endif

#ifdef ENABLE_ZBAR
  std::unique_ptr<ScanImage> scan_image_;
#endif

#ifndef ENABLE_MEDIASERVER_BIN
  ExternMDCallbackPtr extern_md_callback_;
#endif
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_FLOW_MANAGER_H_

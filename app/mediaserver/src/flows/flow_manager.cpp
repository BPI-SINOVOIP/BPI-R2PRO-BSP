// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_manager.h"
#include "flow_db_protocol.h"
#include "flow_export.h"
#include "flow_sm_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_manager.cpp"

namespace rockchip {
namespace mediaserver {

#define LOCAL_FILE_AS_INPUT 0

FlowManagerPtr FlowManager::instance_ = nullptr;
bool FlowManager::restart_pipes_enable_ = false;

FlowManagerPtr &FlowManager::GetInstance() {
  if (instance_ == nullptr) {
    LOG_DEBUG("flow manager create instance\n");
    instance_ = std::make_shared<FlowManager>();
  }
  return std::ref(instance_);
}

#ifdef ENABLE_DBUS
int FlowManager::SyncDBConfig() {
  int smart_enabled = 0;
  std::map<std::string, std::string> config_map;
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string db_config = SelectVideoDb(id);
    if (db_config.empty()) {
      LOG_INFO("select video database empty\n");
      break;
    }
    int pipe_index = GetPipeIndexById(id, StreamType::CAMERA);
    if (pipe_index >= 0) {
      int link_index = flow_parser_->GetFlowIndex(pipe_index, StreamType::LINK);
      if (link_index >= 0) {
        LOG_INFO("link flow pipe only use video config by conf file\n");
        break;
      }
      db_protocol->DbDataToMap(db_config, config_map);
      for (auto it : config_map) {
        flow_parser_->SyncVideoDBData(id, it.first.c_str(), it.second.c_str());
      }
      config_map.clear();
    }
  }

  for (int id = 0; id < MAX_SOUND_NUM; id++) {
    std::string db_config = SelectAudioDb(0);
    if (db_config.empty()) {
      LOG_INFO("select audio database empty\n");
      break;
    }
    int pipe_index = GetPipeIndexById(id, StreamType::AUDIO);
    if (pipe_index >= 0) {
      int link_index = flow_parser_->GetFlowIndex(pipe_index, StreamType::LINK);
      if (link_index >= 0) {
        LOG_INFO("link flow pipe only use audio config by conf file\n");
        break;
      }
      db_protocol->DbDataToMap(db_config, config_map);
      for (auto it : config_map) {
        flow_parser_->SyncAudioDBData(id, it.first.c_str(), it.second.c_str());
      }
      config_map.clear();
    }
  }

#if (defined(ENABLE_OSD_SERVER))
  osd_params_v_.clear();
  for (int id = 0; id < MAX_OSD_REGION_NUM; id++) {
    std::string db_config = SelectOsdDb(id);
    if (db_config.empty() ||
        db_config.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_INFO("select osd database empty\n");
      break;
    }
    db_protocol->DbDataToMap(db_config, config_map);
    osd_params_v_.emplace_back(config_map);
    config_map.clear();
  }
#endif

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string db_config = SelectVideoDb(id);
    if (db_config.empty()) {
      LOG_INFO("select video database empty\n");
      break;
    }
    if ((id == 0) &&
        (db_config.find(DB_VALUE_SMART_CLOSE) != std::string::npos)) {
      LOG_INFO("main stream smart is open, set main stream roi to null\n");
      flow_parser_->SyncRoiDBData(id, "");
    } else {
      std::string roi_regions = db_protocol->GetRoiRegions(id);
      if (!roi_regions.empty()) {
        LOG_INFO("roi_regions is %s\n", roi_regions.c_str());
        flow_parser_->SyncRoiDBData(id, roi_regions);
      }
    }
  }

#if (defined(ENABLE_OSD_SERVER) && defined(USE_ROCKFACE))

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string ri_config = SelectRegionInvadeDb(id);
    if (ri_config.empty() ||
        ri_config.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_DEBUG("select region invade database id %d empty\n", id);
    } else {
      LOG_ERROR("select region invade database id %d\n", id);
      flow_parser_->SyncRIDBData(id, ri_config);
      flow_parser_->SyncBodyDetectDBData(id, ri_config);
    }
  }
#endif

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    // Only use one stream for motion detection
    std::string md_db = SelectMoveDetectDb(0);
    int md_roi_cnt = 0;
    std::string md_roi_rect;
    if (!md_db.empty())
      md_roi_rect = db_protocol->GetMoveDetectRegions(id, md_db, md_roi_cnt);
    if (!md_roi_rect.empty()) {
      LOG_INFO("md_roi_cnt is %d, md_roi_rect is %s\n", md_roi_cnt, md_roi_rect.c_str());
      flow_parser_->SyncMoveDetecDBData(id, md_roi_cnt, md_roi_rect);
    } else {
      LOG_INFO("md_roi_rect is null\n");
    }
  }

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string smart_cover_db = SelectSmartCoverDb(id);
    LOG_ERROR("select smart_cover_db %s\n", smart_cover_db.c_str());
    if (smart_cover_db.empty() ||
        smart_cover_db.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_DEBUG("select smart_cover_db id %d empty\n", id);
    } else {
      db_protocol->DbDataToMap(smart_cover_db, config_map);
      for (auto it : config_map) {
        flow_parser_->SyncSmartCoverDBData(id, it.first.c_str(),
                                           it.second.c_str());
      }
      config_map.clear();
    }
  }

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string rtsp_db = SelectPortDb(3);
    LOG_ERROR("select rtsp_db %s\n", rtsp_db.c_str());
    if (rtsp_db.empty() ||
        rtsp_db.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_INFO("select rtsp_db database empty\n");
      break;
    }
    db_protocol->DbDataToMap(rtsp_db, config_map);
    for (auto it : config_map) {
      flow_parser_->SyncRTSPDBData(id, it.first.c_str(), it.second.c_str());
    }
    config_map.clear();
  }

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string rtmp_db = SelectPortDb(4);
    LOG_ERROR("select rtmp_db %s\n", rtmp_db.c_str());
    if (rtmp_db.empty() ||
        rtmp_db.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_INFO("select rtsp_db database empty\n");
      break;
    }
    db_protocol->DbDataToMap(rtmp_db, config_map);
    for (auto it : config_map) {
      flow_parser_->SyncRTMPDBData(id, it.first.c_str(), it.second.c_str());
    }
    config_map.clear();
  }

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    int scenario_id = 0;
    std::string scenario_db = SelectImageScenarioDb();
    LOG_ERROR("select scenario_db %s\n", scenario_db.c_str());
    if (scenario_db.empty() ||
        scenario_db.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_INFO("select scenario_db database empty\n");
      break;
    }
    if (scenario_db.find(DB_IMAGE_SCENARIO_NORMAL))
      scenario_id = 0;
    else if (scenario_db.find(DB_IMAGE_SCENARIO_BACKLIGHT))
      scenario_id = 1;
    else if (scenario_db.find(DB_IMAGE_SCENARIO_FRONTLIGHT))
      scenario_id = 2;
    else if (scenario_db.find(DB_IMAGE_SCENARIO_LOW_ILLUMINATION))
      scenario_id = 3;
    else if (scenario_db.find(DB_IMAGE_SCENARIO_CUSTOM1))
      scenario_id = 4;
    else if (scenario_db.find(DB_IMAGE_SCENARIO_CUSTOM2))
      scenario_id = 5;

    std::string enhance_db = SelectImageEnhanceDb(scenario_id);
    LOG_ERROR("select enhance_db %s\n", enhance_db.c_str());
    if (enhance_db.empty() ||
        enhance_db.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
      LOG_INFO("select enhance_db database empty\n");
      break;
    }
    db_protocol->DbDataToMap(enhance_db, config_map);
    for (auto it : config_map) {
      flow_parser_->SyncEncoderDBData(id, it.first.c_str(), it.second.c_str());
    }
    config_map.clear();
  }

  return 0;
}

int FlowManager::SyncSMConfig() {
  std::string sm_path = GetStoragePath();
  if (sm_path.empty())
    return -1;
  LOG_DEBUG("sm_path is %s\n", sm_path.c_str());

  std::unique_ptr<FlowSMProtocol> sm_protocol;
  sm_protocol.reset(new FlowSMProtocol);

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    std::string video_path =
        sm_protocol->GetMediaPath(sm_path, MEDIA_PATH_VIDEO, id);
    LOG_DEBUG("video_path id %d is %s\n", id, video_path.c_str());
    if (!video_path.empty())
      flow_parser_->SyncMuxerPath(id, video_path);

    std::string photo_path =
        sm_protocol->GetMediaPath(sm_path, MEDIA_PATH_PHOTO, id);
    LOG_DEBUG("photo_path id %d is %s\n", id, photo_path.c_str());
    if (!photo_path.empty())
      flow_parser_->SyncFilePath(id, photo_path);

    std::string snap_path =
        sm_protocol->GetMediaPath(sm_path, MEDIA_PATH_SNAP, id);
    LOG_DEBUG("snap_path id %d is %s\n", id, snap_path.c_str());
    if (!snap_path.empty())
      flow_parser_->SyncSnapPath(id, snap_path);
  }
  return 0;
}

int FlowManager::GetDiskStatus() {
  std::string sm_path = GetStoragePath();
  if (sm_path.empty())
    return -1;
  std::unique_ptr<FlowSMProtocol> sm_protocol;
  sm_protocol.reset(new FlowSMProtocol);
  return sm_protocol->GetDiskStatus(sm_path);
}

#endif

int FlowManager::SyncFlowParser() {
  flow_parser_->SyncRgafiter();
  flow_parser_->SyncThrougfiter();
  flow_parser_->SyncRtsp();
  flow_parser_->SyncSmartEncorde();
  return 0;
}

int FlowManager::SyncConfig() {
#ifdef ENABLE_DBUS
  SyncDBConfig();
  SyncSMConfig();
#endif
  SyncFlowParser();
  return 0;
}

int FlowManager::ConfigParse(std::string conf) {
  LOG_INFO("flow manager parse config\n");
  flow_parser_.reset(new FlowParser(conf.c_str()));
  SyncConfig();
  return 0;
}

int FlowManager::SaveConfig(std::string conf) {
  flow_parser_->JsConfigReBuild(flow_pipes_);
  flow_parser_->JsConfigWrite(conf.c_str());
  return 0;
}

int FlowManager::BindReuseFlow() {
  int pipe_num = flow_parser_->GetPipeNum();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto &flows = flow_parser_->GetFlowUnits(pipe_index);
    for (auto &iter : flows) {
      auto &flow_unit = iter;
      int opened_pipe_index = flow_unit->GetOpenPipeId();
      int opened_flow_index = flow_unit->GetOpenFlowId();
      if (opened_pipe_index < 0 || opened_flow_index < 0)
        continue;
      LOG_INFO("pipe %d flow %s was opened before by pipe id %d flow id %d\n",
               pipe_index, flow_unit->GetFlowIndexName().c_str(),
               opened_pipe_index, opened_flow_index);
      auto &opened_flow_units = flow_parser_->GetFlowUnits(opened_pipe_index);
      auto &opened_flow_unit = opened_flow_units[opened_flow_index];
      StreamType stream_type = opened_flow_unit->GetStreamType();
      auto &flow_pipe = flow_pipes_[pipe_index];
      auto &opened_flow_pipe = flow_pipes_[opened_pipe_index];
      auto opened_flow = opened_flow_pipe->GetFlow(stream_type);
      flow_pipe->SetFlow(opened_flow, stream_type);
    }
  }
  return 0;
}

void RtspCallback(easymedia::Flow *flow) {
  SetForceIdrFrame(flow, StreamType::RTSP);
}

int FlowManager::RegisterRtspHandler() {
  for (int i = 0; i < flow_pipes_.size(); i++) {
    auto rtsp_flow = flow_pipes_[i]->GetFlow(StreamType::RTSP);
    auto encoder_flow = flow_pipes_[i]->GetFlow(StreamType::VIDEO_ENCODER);
    if (!encoder_flow || !rtsp_flow)
      continue;
    rtsp_flow->SetPlayVideoHandler(RtspCallback);
  }
  return 0;
}

int FlowManager::UnRegisterRtspHandler() {
  for (int i = 0; i < flow_pipes_.size(); i++) {
    auto rtsp_flow = flow_pipes_[i]->GetFlow(StreamType::RTSP);
    auto encoder_flow = flow_pipes_[i]->GetFlow(StreamType::VIDEO_ENCODER);
    if (!encoder_flow || !rtsp_flow)
      continue;
    rtsp_flow->SetPlayVideoHandler(nullptr);
  }
  return 0;
}

int FlowManager::RegisterRtspHandler(int pipe_index) {
  for (int i = 0; i < flow_pipes_.size(); i++) {
    auto rtsp_flow = flow_pipes_[pipe_index]->GetFlow(StreamType::RTSP);
    auto encoder_flow =
        flow_pipes_[pipe_index]->GetFlow(StreamType::VIDEO_ENCODER);
    if (encoder_flow && rtsp_flow)
      rtsp_flow->SetPlayVideoHandler(RtspCallback);
  }
  return 0;
}

int FlowManager::UnRegisterRtspHandler(int pipe_index) {
  auto rtsp_flow = flow_pipes_[pipe_index]->GetFlow(StreamType::RTSP);
  auto encoder_flow =
      flow_pipes_[pipe_index]->GetFlow(StreamType::VIDEO_ENCODER);
  if (encoder_flow && rtsp_flow)
    rtsp_flow->SetPlayVideoHandler(nullptr);
  return 0;
}

#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))

int FlowManager::CreateNNHandler() {
  nn_handler_ = std::make_shared<NNHandler>();
  for (int i = 0; i < flow_pipes_.size(); i++) {
    auto flow_unit = flow_pipes_[i]->GetFlowunit(StreamType::FILTER,
                                                 RKMEDIA_FILTER_NN_INPUT);
    if (!flow_unit)
      continue;
    auto &flow = flow_unit->GetFlow();
    if (!flow)
      continue;
    auto reference_flow_unit =
        flow_pipes_[i]->GetFlowunit(StreamType::VIDEO_ENCODER);
    if (!reference_flow_unit)
      reference_flow_unit = flow_pipes_[i]->GetFlowunit(StreamType::CAMERA);
    if (!reference_flow_unit)
      continue;
    std::string width, height;
    reference_flow_unit->GetResolution(width, height);
    int w = atoi(width.c_str());
    int h = atoi(height.c_str());
    auto &draw_handlers = nn_handler_->GetDrawHandler();
    draw_handlers.emplace_back(new DrawHandler(flow, w, h));
  }
  return 0;
}

int FlowManager::DestoryNNHandler() {
  if (nn_handler_) {
    auto &draw_handlers = nn_handler_->GetDrawHandler();
    draw_handlers.clear();
    nn_handler_.reset();
  }
  nn_handler_ = nullptr;
  return 0;
}

#endif

int FlowManager::CreatePipes() {
  LOG_INFO("flow manager create flow pipe\n");
  for (int index = 0; index < flow_parser_->GetPipeNum(); index++) {
    auto flow_pipe = std::make_shared<FlowPipe>();
    auto &flow_units = flow_parser_->GetFlowUnits(index);
    flow_pipe->CreateFlows(flow_units);
    flow_pipes_.emplace_back(flow_pipe);
  }
  BindReuseFlow();
  for (int index = 0; index < flow_pipes_.size(); index++) {
    auto &flow_pipe = flow_pipes_[index];
    flow_pipe->InitFlows();
// flow_pipe->InitMultiSlice();
#ifdef ENABLE_OSD_SERVER
    flow_pipe->RegisterOsdServer();
#endif
    flow_pipe->BindControler();
    flow_pipe->RegisterCallBack();
  }
  RegisterRtspHandler();
#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))
  CreateNNHandler();
#endif
  return 0;
}

int FlowManager::DestoryPipes() {
#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))
  DestoryNNHandler();
#endif
  UnRegisterRtspHandler();
  for (int index = 0; index < flow_pipes_.size(); index++) {
    auto &flow_pipe = flow_pipes_[index];
    flow_pipe->UnRegisterCallBack();
    flow_pipe->UnBindControler();
#ifdef ENABLE_OSD_SERVER
    flow_pipe->UnRegisterOsdServer();
#endif
    flow_pipe->DeinitFlows();
    flow_pipe->DestoryFlows();
    flow_pipe.reset();
  }
  flow_pipes_.clear();
  return 0;
}

int FlowManager::ResetPipes() {
#ifdef ENABLE_DBUS
  SendIspserverTurnoffSignal();
#endif
  SyncConfig();
  DestoryPipes();
  CreatePipes();
  return 0;
}

int FlowManager::StopPipes() {
  SyncConfig();
  DestoryPipes();
  restart_pipes_enable_ = true;
  return 0;
}

int FlowManager::RestartPipes() {
  if (!restart_pipes_enable_)
    return -1;
  restart_pipes_enable_ = false;
  CreatePipes();
  return 0;
}

int FlowManager::ResetFlow(int id, int flow_index) {
  LOG_INFO("pipe_index is %d, flow_index is %d\n", id, flow_index);
  auto &flow_pipe = flow_pipes_[id];
  auto flow_unit = flow_pipe->GetFlowunit(flow_index);
  SyncConfig();
// Deinit
#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))
  DestoryNNHandler();
#endif
  UnRegisterRtspHandler(id);
  flow_pipe->UnRegisterCallBack();
  flow_pipe->UnBindControler();
#ifdef ENABLE_OSD_SERVER
  flow_pipe->UnRegisterOsdServer();
#endif
  flow_pipe->DeinitFlow(flow_index);
  // Create flow
  flow_pipe->CreateFlow(flow_unit);
  BindReuseFlow();
  flow_pipe->InitFlow(flow_index);
#ifdef ENABLE_OSD_SERVER
  flow_pipe->RegisterOsdServer();
#endif
  flow_pipe->BindControler();
  flow_pipe->RegisterCallBack();
  RegisterRtspHandler(id);
#if (defined(USE_ROCKFACE) || defined(USE_ROCKX))
  CreateNNHandler();
#endif
  return 0;
}

int FlowManager::ResetFlowByType(int id, StreamType type) {
  auto &flow_pipe = flow_pipes_[id];
  int flow_index = flow_pipe->GetFlowIndex(type);
  ResetFlow(id, flow_index);
  return 0;
}

int FlowManager::ResetFlowByInput(int id, StreamType type,
                                  std::string input_data_type) {
  auto &flow_pipe = flow_pipes_[id];
  int flow_index = flow_pipe->GetFlowIndexByInput(type, input_data_type);
  ResetFlow(id, flow_index);
  return 0;
}

int FlowManager::CreatePipe(int id, StreamType type) {
  LOG_INFO("flow manager create flow pipe %d type %d\n", id, type);
  int pipe_index = GetPipeIndexById(id, type);
  auto &flow_pipe = flow_pipes_[pipe_index];
  auto &flow_units = flow_parser_->GetFlowUnits(pipe_index);
  flow_pipe->CreateFlows(flow_units);
  flow_pipe->InitFlows();
// flow_pipe->InitMultiSlice();
#ifdef ENABLE_OSD_SERVER
  flow_pipe->RegisterOsdServer();
#endif
  flow_pipe->BindControler();
  flow_pipe->RegisterCallBack();
  return 0;
}

int FlowManager::DestoryPipe(int id, StreamType type) {
  int pipe_index = GetPipeIndexById(id, type);
  auto &flow_pipe = flow_pipes_[pipe_index];
  flow_pipe->UnRegisterCallBack();
  flow_pipe->UnBindControler();
#ifdef ENABLE_OSD_SERVER
  flow_pipe->UnRegisterOsdServer();
#endif
  flow_pipe->DeinitFlows();
  flow_pipe->DestoryFlows();
  return 0;
}

int FlowManager::ResetPipe(int id, StreamType type) {
#ifdef ENABLE_DBUS
  SyncDBConfig();
#endif
  DestoryPipe(id, type);
  CreatePipe(id, type);
  return 0;
}

int FlowManager::GetStreamById(int id, StreamType type) {
  int pipe_num = flow_parser_->GetPipeNum();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = flow_parser_->GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto &flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      if (stream_type == type) {
        if (id == flow_unit->GetStreamId()) {
          return pipe_index;
        }
      }
    }
  }
  return -1;
}

int FlowManager::GetStreamById(int id, StreamType type, std::string name) {
  int pipe_num = flow_parser_->GetPipeNum();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = flow_parser_->GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto &flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      std::string stream_name = flow_unit->GetStreamName();
      if (stream_type == type && stream_name == name) {
        if (id == flow_unit->GetStreamId()) {
          return pipe_index;
        }
      }
    }
  }
  return -1;
}

int FlowManager::GetPipeIndexById(int id, StreamType type) {
  int cnt = 0;
  int pipe_index = GetStreamById(id, type);
  if (pipe_index >= 0) {
    return pipe_index;
  }
  int pipe_num = flow_parser_->GetPipeNum();
  for (pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = flow_parser_->GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto &flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      if (stream_type == type) {
        if (id == cnt) {
          return pipe_index;
        } else {
          cnt++;
          break;
        }
      }
    }
  }
  return -1;
}

int FlowManager::GetPipeIndexById(int id, StreamType type, std::string name) {
  int cnt = 0;
  int pipe_index = GetStreamById(id, type, name);
  if (pipe_index >= 0) {
    return pipe_index;
  }
  int pipe_num = flow_parser_->GetPipeNum();
  for (pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = flow_parser_->GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto &flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      std::string stream_name = flow_unit->GetStreamName();
      if (stream_type == type && stream_name == name) {
        if (id == cnt) {
          return pipe_index;
        } else {
          cnt++;
          break;
        }
      }
    }
  }
  return -1;
}

#ifndef ENABLE_MEDIASERVER_BIN

int FlowManager::SyncVideoDBData(int id, std::string key, std::string value) {
  flow_parser_->SyncVideoDBData(id, key, value);
  return 0;
}

int FlowManager::SyncAudioDBData(int id, std::string key, std::string value) {
  flow_parser_->SyncAudioDBData(id, key, value);
  return 0;
}

#endif // DISABLE_MEDIASERVER_BIN

#ifdef ENABLE_SCHEDULES_SERVER

void FlowManager::InitScheduleMutex() {
  pthread_mutex_init(&schedule_mutex, NULL);
}

void FlowManager::CreateSchedules() {
  LOG_INFO("Create Schedules Manager\n");
  pthread_mutex_lock(&schedule_mutex);
  schedules_manager_.reset(new SchedulesManager());
  if (schedules_manager_) {
    schedules_manager_->start();
  }
  pthread_mutex_unlock(&schedule_mutex);
}

void FlowManager::DestorySchedules() {
  LOG_INFO("Destory Schedules Manager\n");
  pthread_mutex_lock(&schedule_mutex);
  if (schedules_manager_) {
    if (schedules_manager_->running_status())
      schedules_manager_->stop();
    schedules_manager_.reset();
    schedules_manager_ = nullptr;
  }
  pthread_mutex_unlock(&schedule_mutex);
}

int FlowManager::SchedulesManagerIsRunning() {
  int ret = 0;
  pthread_mutex_lock(&schedule_mutex);
  if (schedules_manager_)
    ret = schedules_manager_->running_status();
  pthread_mutex_unlock(&schedule_mutex);
  return ret;
}

void FlowManager::SyncSchedulesConfig() {
  LOG_INFO("Sync Schedules Manager\n");
  pthread_mutex_lock(&schedule_mutex);
  LOG_DEBUG("running_status: %d, is_running:%d\n", schedules_manager_->running_status(), schedules_manager_->running_flag_);
  if (schedules_manager_ && schedules_manager_->running_status()) {
    schedules_manager_->SyncSchedulesConfig();
  } else {
    schedules_manager_.reset(new SchedulesManager());
    if (schedules_manager_) {
      schedules_manager_->start();
    }
  }
  pthread_mutex_unlock(&schedule_mutex);
}

void FlowManager::StopSchedules() {
  LOG_INFO("Stop Schedules Manager\n");
  pthread_mutex_lock(&schedule_mutex);
  LOG_DEBUG("running_status: %d, is_running:%d\n", schedules_manager_->running_status(), schedules_manager_->running_flag_);
  if (schedules_manager_ && schedules_manager_->running_status()) {
    schedules_manager_->stop();
  }
  pthread_mutex_unlock(&schedule_mutex);
}

#endif // ENABLE_SCHEDULES_SERVER

#ifdef ENABLE_ZBAR

void FlowManager::CreateScanImage() {
  LOG_INFO("Create Scan Image\n");
  scan_image_.reset(new ScanImage());
  if (scan_image_) {
    scan_image_->start();
  }
}

void FlowManager::DestoryScanImage() {
  LOG_INFO("Destory Scan Image\n");
  if (scan_image_)
    scan_image_->stop();
  scan_image_.reset();
  scan_image_ = nullptr;
}

#endif // ENABLE_ZBAR

} // namespace mediaserver
} // namespace rockchip

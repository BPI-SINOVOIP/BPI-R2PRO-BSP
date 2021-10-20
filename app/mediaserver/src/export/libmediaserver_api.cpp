// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dbus_dbserver_key.h"
#include "flow_export.h"
#include "flow_manager.h"
#include <libmediaserver_api.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "libmediaserver_api.cpp"

using namespace rockchip::mediaserver;

#ifdef __cplusplus
extern "C" {
#endif

std::shared_ptr<easymedia::Flow> GetLinkFlow(int id, StreamType type,
                                             std::string input_data_type) {
  auto pipe = GetFlowPipe(id, type);
  if (pipe) {
    auto link_flow = pipe->GetFlowByInput(StreamType::LINK, input_data_type);
    if (link_flow) {
      return link_flow;
    } else {
      LOG_ERROR("StreamType %d id %d linkflow no found\n", type, id);
    }
  } else {
    LOG_ERROR("StreamType %d id %d pipe no found\n", type, id);
  }
  return nullptr;
}

std::shared_ptr<FlowUnit> GetFlowUnit(int id, StreamType type) {
  auto audio_pipe = GetFlowPipe(id, type);
  if (audio_pipe) {
    auto flow_unit = audio_pipe->GetFlowunit(type);
    if (flow_unit) {
      return flow_unit;
    } else {
      LOG_ERROR("id %d flow_unit no found\n", id);
    }
  } else {
    LOG_ERROR("id %d pipe no found\n", id);
  }
  return nullptr;
}

void libMediaDbDataToMap(char *json, std::map<std::string, std::string> &map) {
  nlohmann::json data = nlohmann::json::parse(json);
  std::string key, value;
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it.value().is_string())
      value = it.value().get<std::string>();
    else
      value = std::to_string(it.value().get<int>());
    map[it.key()] = value;
  }
}

int libMediaInit(const char *json_file, RESOURCE_S env) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  assert(flow_manager);
#ifdef ENABLE_OSD_SERVER
  OSDServer::bmp_path_ = env.cBMPPath;
  OSDServer::ttc_path_ = env.cTTCPath;
#endif
  flow_manager->ConfigParse(json_file);
  return 0;
}

int libMediaStart() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  assert(flow_manager);
  flow_manager->CreatePipes();
  return 0;
}

int libMediaSaveConfig(const char *json_file) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->SaveConfig(json_file);
}

int libMediaCleanup() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->DestoryPipes();
  flow_manager.reset();
  return 0;
}

int libMediaSyncVideoData(int id, char *json) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  assert(flow_manager);
  std::map<std::string, std::string> config_map;
  libMediaDbDataToMap(json, config_map);
  for (auto it : config_map) {
    flow_manager->SyncVideoDBData(id, it.first.c_str(), it.second.c_str());
  }
  return 0;
}

int libMediaSyncAudioData(char *json) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  assert(flow_manager);
  std::map<std::string, std::string> config_map;
  libMediaDbDataToMap(json, config_map);
  for (auto it : config_map) {
    flow_manager->SyncAudioDBData(0, it.first.c_str(), it.second.c_str());
  }
  return 0;
}

int libMediaAudioStart(AudioHandler handler) {
  int audio_stream_id = 0;
  auto link_flow = GetLinkFlow(audio_stream_id, StreamType::AUDIO, "audio:");
  if (link_flow) {
    link_flow->SetAudioHandler(handler);
    return 0;
  }
  return -1;
}

int libMediaAudioStop() {
  int audio_stream_id = 0;
  auto link_flow = GetLinkFlow(audio_stream_id, StreamType::AUDIO, "audio:");
  if (link_flow) {
    link_flow->SetAudioHandler(nullptr);
    return 0;
  }
  return -1;
}

int libMediaVideoStart(int id, VideoHandler handler) {
  auto link_flow = GetLinkFlow(id, StreamType::CAMERA, "video:");
  if (link_flow) {
    link_flow->SetVideoHandler(handler);
    return 0;
  }
  return -1;
}

int libMediaVideoStop(int id) {
  auto link_flow = GetLinkFlow(id, StreamType::CAMERA, "video:");
  if (link_flow) {
    link_flow->SetVideoHandler(nullptr);
    return 0;
  }
  return -1;
}

int libMediaCaptureStart(int id, CaptureHandler handler) {
  auto link_flow = GetLinkFlow(id, StreamType::CAMERA, "image:");
  if (link_flow) {
    link_flow->SetCaptureHandler(handler);
    return 0;
  }
  return -1;
}

int libMediaCaptureStop(int id) {
  auto link_flow = GetLinkFlow(id, StreamType::CAMERA, "image:");
  if (link_flow) {
    link_flow->SetCaptureHandler(nullptr);
    return 0;
  }
  return -1;
}

int libMediaGetSnapshot(int id) { return TakePhoto(id, 1); }

int libMediaAudioChangeEncodeType(char *type) {
  auto enc_flow_unit = GetFlowUnit(0, StreamType::AUDIO_ENCODER);
  auto audio_flow_unit = GetFlowUnit(0, StreamType::AUDIO);
  enc_flow_unit->SetEncodeType(type);
  audio_flow_unit->SetSampleFormat(type);

  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  AudioHandler link_handler = nullptr;
  auto link_flow = GetLinkFlow(0, StreamType::AUDIO, "audio:");
  if (link_flow)
    link_handler = link_flow->GetAudioHandler();
  flow_manager->ResetFlowByType(0, StreamType::AUDIO);
  flow_manager->ResetFlowByType(0, StreamType::AUDIO_ENCODER);
  if (link_flow) {
    flow_manager->ResetFlowByInput(0, StreamType::LINK, "audio:");
    link_flow = GetLinkFlow(0, StreamType::AUDIO, "audio:");
    link_flow->SetAudioHandler(link_handler);
  }
  return 0;
}

int libMediaVideoChangeOutputDataType(int id, char *type) {
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  enc_flow_unit->SetOutputDataType(type);

  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  VideoHandler link_handler = nullptr;
  auto link_flow = GetLinkFlow(id, StreamType::CAMERA, "video:");
  if (link_flow)
    link_handler = link_flow->GetVideoHandler();
  flow_manager->ResetFlowByType(id, StreamType::VIDEO_ENCODER);
  if (link_flow) {
    flow_manager->ResetFlowByInput(id, StreamType::LINK, "video:");
    link_flow = GetLinkFlow(id, StreamType::CAMERA, "video:");
    link_flow->SetVideoHandler(link_handler);
  }
  return 0;
}

int libMediaVideoChangeResolution(int id, int w, int h) {
  auto cam_flow_unit = GetFlowUnit(id, StreamType::CAMERA);
  cam_flow_unit->SetResolution2(std::to_string(w), std::to_string(h));
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  enc_flow_unit->SetResolution2(std::to_string(w), std::to_string(h));

  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  VideoHandler link_handler = nullptr;
  auto link_flow = GetLinkFlow(id, StreamType::CAMERA, "video:");
  if (link_flow)
    link_handler = link_flow->GetVideoHandler();
  flow_manager->ResetFlowByType(id, StreamType::CAMERA);
  flow_manager->ResetFlowByType(id, StreamType::VIDEO_ENCODER);
  if (link_flow) {
    flow_manager->ResetFlowByInput(id, StreamType::LINK, "video:");
    link_flow = GetLinkFlow(id, StreamType::CAMERA, "video:");
    link_flow->SetVideoHandler(link_handler);
  }
  return 0;
}

int libMediaVideoChangeBitrate(int id, int bitrate) {
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetMaxRate(std::to_string(bitrate));
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetBitRate(bitrate);
  return 0;
}

int libMediaVideoChangeFramerate(int id, int num, int den) {
  std::string fps;
  fps.append(std::to_string(num).c_str());
  fps.append("/");
  fps.append(std::to_string(den).c_str());
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetFrameRate(fps);
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetFrameRate(num, den);
  return 0;
}

int libMediaVideoChangeIFrameInterval(int id, int value) {
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetGop(std::to_string(value));
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetGop(value);
  return 0;
}

int libMediaVideoForceIFrame(int id) {
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetForceIdrFrame();
  return 0;
}

int libMediaVideoSetSmartPFrame(int id, int value) {
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetSmart(std::to_string(value));
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->ResetPipes();
  return 0;
}

int libMediaVideoChangeProfile(int id, int value) {
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetH264Profile2(std::to_string(value));
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetH264Profile2(value);
  return 0;
}

int libMediaVideoRateControl(int id, char *mode) {
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetRCMode(mode);
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetRCMode(mode);
  return 0;
}

int libMediaVideoSetROI(int id, ROI_REGION_S *regions, int region_cnt) {
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetRoiRegion2((EncROIRegion *)regions, region_cnt);
  return 0;
}

int libMediaVideoSetUserdata(int id, void *data, int len, int all_frames) {
  auto encoder_control = GetEncoderControl(id);
  if (!encoder_control) {
    return -1;
  }
  encoder_control->SetUserdata(data, len, all_frames);
  return 0;
}

int libMediaVideoSetTextOSD(int id, OSD_CFG_S osd) {
#ifdef ENABLE_OSD_SERVER
  std::map<std::string, std::string> osd_map;
  osd_map[DB_OSD_ENABLED] = std::to_string(osd.u16Status);
  osd_map[DB_OSD_POSITION_X] = std::to_string(osd.stOsdPos.u16PosX);
  osd_map[DB_OSD_POSITION_Y] = std::to_string(osd.stOsdPos.u16PosY);
  osd_map[DB_OSD_DISPLAY_TEXT] = osd.cOsdTitle[0];
  char color[20];
  snprintf(color, sizeof(color), "%x", osd.stOsdColor.u32FgColor);
  osd_map[DB_OSD_FRONT_COLOR] = color;
  osd_map[DB_OSD_FONT_SIZE] = std::to_string(osd.u16FontPixelSize);

  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetOsdRegion(osd.s32OsdRegHdl, osd_map);
#endif
  return 0;
}

int libMediaVideoSetBMPOSD(int id, OSD_CFG_S osd) {
#ifdef ENABLE_OSD_SERVER
  std::map<std::string, std::string> osd_map;
  osd_map[DB_OSD_ENABLED] = std::to_string(osd.u16Status);
  osd_map[DB_OSD_POSITION_X] = std::to_string(osd.stOsdPos.u16PosX);
  osd_map[DB_OSD_POSITION_Y] = std::to_string(osd.stOsdPos.u16PosY);

  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetOsdRegion(14, osd_map);
#endif
  return 0;
}

int libMediaVideoSetDateOSD(int id, OSD_CFG_S osd) {
#ifdef ENABLE_OSD_SERVER
  std::map<std::string, std::string> osd_map;
  osd_map[DB_OSD_ENABLED] = std::to_string(osd.u16Status);
  osd_map[DB_OSD_FONT_SIZE] = std::to_string(osd.u16FontPixelSize);
  char color[20];
  snprintf(color, sizeof(color), "%x", osd.stOsdColor.u32FgColor);
  osd_map[DB_OSD_FRONT_COLOR] = color;
  osd_map[DB_OSD_POSITION_X] = std::to_string(osd.stOsdPos.u16PosX);
  osd_map[DB_OSD_POSITION_Y] = std::to_string(osd.stOsdPos.u16PosY);

  osd_map[DB_OSD_TIME_STYLE] = osd.cTimeStyle;
  osd_map[DB_OSD_DISPLAY_WEEK_ENABLED] = std::to_string(osd.u16DisplayWeek);
  osd_map[DB_OSD_DATE_STYLE] = osd.cDateStyle;
  osd_map[DB_OSD_FRONT_COLOR_MODE] = osd.cOsdColorMode;

  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetOsdRegion(1, osd_map);
#endif
  return 0;
}

int libMediaVideoSetPrivacyMask(int id, PRIVACYMASK_DATA_S mask) {
#ifdef ENABLE_OSD_SERVER
  std::map<std::string, std::string> osd_map;
  osd_map[DB_OSD_ENABLED] = "1";
  osd_map[DB_OSD_POSITION_X] = std::to_string(mask.s32X[0]);
  osd_map[DB_OSD_POSITION_Y] = std::to_string(mask.s32Y[0]);
  osd_map[DB_OSD_WIDTH] = std::to_string(mask.s32W[0]);
  osd_map[DB_OSD_HTIGHT] = std::to_string(mask.s32H[0]);

  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetOsdRegion(10, osd_map);
#endif
  return 0;
}

int libMediaVideoRmPrivacyMask(int id) {
#ifdef ENABLE_OSD_SERVER
  std::map<std::string, std::string> osd_map;
  osd_map[DB_OSD_ENABLED] = "0";
  auto enc_flow_unit = GetFlowUnit(id, StreamType::VIDEO_ENCODER);
  if (!enc_flow_unit) {
    return -1;
  }
  enc_flow_unit->SetOsdRegion(10, osd_map);
#endif
  return 0;
}

int libMediaSetMDCallback(MDCallbackPtr callback) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  assert(flow_manager);
  flow_manager->SetMDCallback((ExternMDCallbackPtr)callback);
  return 0;
}

int libMediaVideoSetMDEnabled(int enabled) {
  SetMDEnabled(enabled);
  return 0;
}

int libMediaVideoSetMDSensitivity(int sensitivity) {
  SetMDSensitivity(sensitivity);
  return 0;
}

int libMediaVideoSetMDRect(IMAGE_RECT_S *rects, int rect_cnt) {
  SetMDRect2((ImageRect *)rects, rect_cnt);
  return 0;
}

#ifdef __cplusplus
};
#endif

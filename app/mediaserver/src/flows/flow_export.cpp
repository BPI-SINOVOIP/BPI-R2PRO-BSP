// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_export.h"

#ifdef ENABLE_DBUS
#include "dbserver.h"
#include "flow_db_protocol.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_export.cpp"

namespace rockchip {
namespace mediaserver {

static bool takephoto_en = true;

static int GetFlowPipeNum() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  return flow_manager->GetPipesSize();
}

std::shared_ptr<FlowPipe> GetFlowPipe(int id, StreamType type) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  if (id < 0 || id >= flow_manager->GetPipesSize())
    return nullptr;
  int index = flow_manager->GetPipeIndexById(id, type);
  if (index < 0)
    return nullptr;
  auto flow_pipe = flow_manager->GetPipeByIndex(index);
  return flow_pipe;
}

std::shared_ptr<FlowPipe> GetFlowPipe(int id, StreamType type,
                                      std::string name) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  if (id < 0 || id >= flow_manager->GetPipesSize())
    return nullptr;
  int index = flow_manager->GetPipeIndexById(id, type, name);
  if (index < 0)
    return nullptr;
  auto flow_pipe = flow_manager->GetPipeByIndex(index);
  return flow_pipe;
}

std::shared_ptr<CameraControl> GetCameraControl(int id) {
  StreamType type = StreamType::CAMERA;
  auto flow_pipe = GetFlowPipe(id, type);
  if (!flow_pipe)
    return nullptr;
  auto flow_control = flow_pipe->GetControler(type);
  if (flow_control) {
    auto camera_control = flow_control->GetCameraControl();
    return camera_control;
  }
  return nullptr;
}

std::shared_ptr<EncoderControl> GetEncoderControl(int id) {
  StreamType type = StreamType::VIDEO_ENCODER;
  auto flow_pipe = GetFlowPipe(id, type);
  if (!flow_pipe)
    return nullptr;
  auto flow_control = flow_pipe->GetControler(type);
  if (flow_control) {
    auto encoder_control = flow_control->GetEncoderControl();
    return encoder_control;
  }
  return nullptr;
}

std::shared_ptr<AudioControl> GetAudioControl(int id) {
  StreamType type = StreamType::AUDIO;
  auto flow_pipe = GetFlowPipe(id, type);
  if (!flow_pipe)
    return nullptr;
  auto flow_control = flow_pipe->GetControler(type);
  if (flow_control) {
    auto audio_control = flow_control->GetAudioControl();
    return audio_control;
  }
  return nullptr;
}

static std::shared_ptr<easymedia::Flow> GetRecoderFlow(int id) {
  auto flow_pipe = GetFlowPipe(id, StreamType::CAMERA);
  if (!flow_pipe)
    return nullptr;
  auto flow_units = flow_pipe->GetFlowUnits();
  for (int index = 0; index < flow_units.size(); index++) {
    auto &flow_unit = flow_units[index];
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == StreamType::MUXER) {
      std::string flow_param = flow_unit->FlowParamMapToStr();
      if ((flow_param.find(NODE_FLOW_PARAM_FILE_PREFIX) != std::string::npos)) {
        return flow_unit->GetFlow();
      }
    }
  }
  return nullptr;
}

void ResetPipes() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->ResetPipes();
}

void StopPipes() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->StopPipes();
}

void RestartPipes() {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->RestartPipes();
}

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

std::shared_ptr<FlowUnit> GetLinkFlowUnit(int id, StreamType type,
                                          std::string input_data_type) {
  auto pipe = GetFlowPipe(id, type);
  if (pipe) {
    auto link_flow_unit =
        pipe->GetFlowunitByInput(StreamType::LINK, input_data_type);
    if (link_flow_unit) {
      return link_flow_unit;
    } else {
      LOG_ERROR("StreamType %d id %d link flow unit no found\n", type, id);
    }
  } else {
    LOG_ERROR("StreamType %d id %d pipe no found\n", type, id);
  }
  return nullptr;
}

#ifdef ENABLE_DBUS
std::string GetVideoParmas(int id) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  if (id < 0 || id >= MAX_CAM_NUM)
    return nullptr;
  return flow_manager->SelectVideoDb(id);
}
#endif

/* audio functions */
void SetSampleRate(int val) { LOG_INFO("sample_rate is %d\n", val); }

void SetVolume(int val) {
  LOG_INFO("volume is %d\n", val);
  auto audio_control = GetAudioControl(0);
  if (audio_control)
    audio_control->SetVolume(val);
}

void SetBitRate(int val) { LOG_INFO("bit_rate is %d\n", val); }

void SetAudioSource(const char *param) {
  LOG_INFO("audio_source is %s\n", param);
}

void SetAudioEncodeType(const char *param) {
  LOG_INFO("encode_type is %s\n", param);
}

void SetANS(const char *param) { LOG_INFO("ANS is %s\n", param); }

/* video functions */
void SetGop(int id, int val) {
  LOG_INFO("id is %d, gop is %d\n", id, val);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetGop(val);
}

void SetMaxRate(int id, int val) {
  LOG_INFO("id is %d, maxrate is %d kbps\n", id, val);
  int bps = val * KB_UNITS;
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetMaxRate(bps);
}

void SetStreamSmooth(int id, int val) {
  LOG_INFO("id is %d, stream_smooth is %d\n", id, val);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetSreamSmooth(val);
}

void SetForceIdrFrame(int id) {
  LOG_INFO("id is %d, SetForceIdr\n", id);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetForceIdrFrame();
}

void SetForceIdrFrame(easymedia::Flow *search_flow, StreamType search_type) {
  LOG_INFO("SetForceIdrFrame search_flow %p\n", search_flow);
  for (int i = 0; i < GetFlowPipeNum(); i++) {
    auto flow_pipe = GetFlowPipe(i, search_type);
    if (!flow_pipe)
      continue;
    auto flow_control = flow_pipe->GetControler(StreamType::VIDEO_ENCODER);
    if (!flow_control)
      continue;
    auto flow_unit = flow_pipe->GetFlowunit(search_type);
    if (!flow_unit)
      continue;
    if (flow_unit->GetFlow().get() == search_flow) {
      auto encoder_control = flow_control->GetEncoderControl();
      if (encoder_control) {
        encoder_control->SetForceIdrFrame();
      }
    }
  }
}

void SetFrameRate(int id, int val) {
  LOG_INFO("id is %d, frame_rate is %d\n", id, val);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetFrameRate(val, 1);
}

void SetFrameRate(int id, int num, int den) {
  LOG_INFO("id is %d, numerator is %d, denominator is %d\n", id, num, den);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetFrameRate(num, den);
}

void SetResolution(int id, const char *param) {
  std::string str = param;
  int pos = str.find("*");
  int width = atoi(str.substr(0, pos).c_str());
  int hight = atoi(str.substr(pos + 1, str.size() - 1).c_str());
  LOG_INFO("id is %d, width is %d, hight is %d\n", id, width, hight);
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->ResetPipes();
  // flow_manager->ResetPipe(id, StreamType::CAMERA);
}

void SetRCQuality(int id, std::string level) {
  LOG_INFO("id is %d, rc_quality level %s\n", id, level.c_str());
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetRCQuality(level.c_str());
}

void SetVideoEncodeType(int id, const char *param) {
  LOG_INFO("id is %d, encode_type is %s\n", id, param);
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->ResetPipes();
}

std::string GetVideoEncoderType(int id) {
  std::string enc_type;
#ifdef ENABLE_DBUS
  std::map<std::string, std::string> map;
  auto parmas = GetVideoParmas(id);
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  enc_type = db_protocol->GetValueByKey(parmas, DB_VIDEO_OUTPUT_DATA_TYPE);
  LOG_INFO("id is %d, get encode_type is %s\n", id, enc_type.c_str());
#endif
  return enc_type;
}

void SetRCMode(int id, const char *param) {
  LOG_INFO("id is %d, rc_mode is %s\n", id, param);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetRCMode(param);
}

void SetH264Profile(int id, const char *param) {
  LOG_INFO("id is %d, h264 profile %s\n", id, param);
  auto encoder_control = GetEncoderControl(id);
  if (encoder_control)
    encoder_control->SetH264Profile(param);
}

void SetSmart(int id, const char *param) {
  LOG_INFO("id is %d, smart is %s\n", id, param);
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  flow_manager->ResetPipes();
}

void SetSVC(int id, const char *param) {
  LOG_INFO("id is %d, SVC is %s\n", id, param);
}

void SetVideoType(int id, const char *param) {
  LOG_INFO("id is %d, video_type is %s\n", id, param);
}

int StartRecord(int id) {
  LOG_INFO("StartRecord id %d\n", id);
#ifdef ENABLE_DBUS
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  if (flow_manager->GetDiskStatus() != 1) {
    LOG_ERROR("DistStatus !=1,  StartRecord id %d, failed\n", id);
    return -1;
  }
#endif

  auto flow = GetRecoderFlow(id);
  if (flow) {
    LOG_INFO("StartRecord...\n");
    flow->Control(easymedia::S_START_SRTEAM);
  }
  SetForceIdrFrame(id);
  return 0;
}

int StopRecord(int id) {
  LOG_INFO("StopRecord id %d\n", id);
  auto flow = GetRecoderFlow(id);
  if (flow) {
    LOG_INFO("StopRecord...\n");
    flow->Control(easymedia::S_STOP_SRTEAM);
  }
  return 0;
}

int GetRecordStatus(int id) {
  LOG_DEBUG("GetRecordStatus id %d\n", id);
  int status = 0;
  auto flow = GetRecoderFlow(id);
  if (flow) {
    flow->Control(easymedia::G_MUXER_GET_STATUS, &status);
    LOG_DEBUG("GetRecordStatus, status is %d\n", status);
  }
  return status;
}

int StartRecord() {
  for (int i = 0; i < GetFlowPipeNum(); i++)
    StartRecord(i);
  return 0;
}

int StopRecord() {
  for (int i = 0; i < GetFlowPipeNum(); i++)
    StopRecord(i);
  return 0;
}

int TakePhotoEnableSet(bool en) {
  takephoto_en = en;
  return 0;
}

int TakePhoto(int id, int count) {
  if (!takephoto_en) {
    LOG_WARN("take photo is forbid\n");
    return -1;
  }
  StreamType type = StreamType::JPEG_THROUGH_GUARD;
  auto flow_pipe = GetFlowPipe(id, type);
  if (!flow_pipe) {
    LOG_ERROR("flow_pipe is null\n");
    return -1;
  }
  auto guard_flow = flow_pipe->GetFlow(type);
  if (!guard_flow) {
    LOG_ERROR("guard_flow is null\n");
    return -1;
  }
  guard_flow->Control(easymedia::S_ALLOW_THROUGH_COUNT, &count);
  return 0;
}

#ifdef ENABLE_SCHEDULES_SERVER

int SyncSchedules() {
  LOG_INFO("SyncSchedules\n");
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  if (flow_manager)
    flow_manager->SyncSchedulesConfig();
  LOG_DEBUG("SyncSchedules end\n");
  return 0;
}

int StopSchedules() {
  LOG_INFO("%s\n", __FUNCTION__);
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  if (flow_manager) {
    flow_manager->StopSchedules();
  }
  LOG_DEBUG("%s end\n", __FUNCTION__);
  return 0;
}

int SetFileDuration(int id, int duration) {
  LOG_INFO("SetFileDuration id %d\n", id);
  int status = 0;
  auto flow = GetRecoderFlow(id);
  if (flow)
    flow->Control(easymedia::S_MUXER_FILE_DURATION, duration);
  return 0;
}

int SetFilePath(int id, std::string path) {
  LOG_INFO("SetFileDuration id %d\n", id);
  int status = 0;
  auto flow = GetRecoderFlow(id);
  if (flow)
    flow->Control(easymedia::S_MUXER_FILE_PATH, path);
  return 0;
}

int SetFilePrefix(int id, std::string prefix) {
  LOG_INFO("SetFilePrefix id %d\n", id);
  int status = 0;
  auto flow = GetRecoderFlow(id);
  if (flow)
    flow->Control(easymedia::S_MUXER_FILE_PREFIX, prefix);
  return 0;
}
#endif

#ifdef ENABLE_OSD_SERVER
/* osd functions */
void SetOsdRegion(int id, const std::map<std::string, std::string> &map) {
  for (int i = 0; i < GetFlowPipeNum(); i++) {
    StreamType type = StreamType::VIDEO_ENCODER;
    auto flow_pipe = GetFlowPipe(i, type);
    if (!flow_pipe)
      continue;
    auto flow_unit = flow_pipe->GetFlowunit(type);
    if (!flow_unit)
      continue;
    flow_unit->SetOsdRegion(id, map);
  }
}
#endif

/* roi functions */
void SetRoiRegion(int stream_id, std::string roi_regions) {
  LOG_INFO("id is %d, SetRoiRegion\n", stream_id);
  auto encoder_control = GetEncoderControl(stream_id);
  if (encoder_control)
    encoder_control->SetRoiRegion(roi_regions);
}

#ifdef USE_ROCKFACE

int GetFaceDetectArg(int id, easymedia::FaceDetectArg &fda) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_DETECT);
  if (flow_pipe) {
    auto flow =
        flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_DETECT);
    flow->Control(easymedia::G_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

int SetFaceDetectArg(int id, easymedia::FaceDetectArg fda) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_DETECT);
  if (flow_pipe) {
    auto flow =
        flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_DETECT);
    flow->Control(easymedia::S_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

int GetNNInputArg(int id, easymedia::NNinputArg &fda) {
  auto flow_pipe = GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_NN_INPUT);
  if (flow_pipe) {
    auto flow = flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_NN_INPUT);
    flow->Control(easymedia::G_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

int SetNNInputArg(int id, easymedia::NNinputArg fda) {
  auto flow_pipe = GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_NN_INPUT);
  if (flow_pipe) {
    auto flow = flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_NN_INPUT);
    flow->Control(easymedia::S_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

int GetFaceCaptureArg(int id, easymedia::FaceCaptureArg &cfa) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_FACE_CAPTURE);
  if (flow_pipe) {
    auto flow =
        flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_FACE_CAPTURE);
    flow->Control(easymedia::G_NN_INFO, &cfa);
    return 0;
  }
  return -1;
}

int SetFaceCaptureArg(int id, easymedia::FaceCaptureArg fda) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_FACE_CAPTURE);
  if (flow_pipe) {
    auto flow =
        flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_FACE_CAPTURE);
    flow->Control(easymedia::S_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

int GetDrawFilterArg(int id, easymedia::DrawFilterArg &cfa) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_DRAW_FILTER);
  if (flow_pipe) {
    auto flow =
        flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_DRAW_FILTER);
    flow->Control(easymedia::G_NN_INFO, &cfa);
    return 0;
  }
  return -1;
}

int SetDrawFilterArg(int id, easymedia::DrawFilterArg fda) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_DRAW_FILTER);
  if (flow_pipe) {
    auto flow =
        flow_pipe->GetFlow(StreamType::FILTER, RKMEDIA_FILTER_DRAW_FILTER);
    flow->Control(easymedia::S_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

int GetFaceRegArg(int id, easymedia::FaceRegArg &cfa) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_RECOGNIZE);
  if (flow_pipe) {
    auto flow = flow_pipe->GetFlow(StreamType::FILTER,
                                   RKMEDIA_FILTER_ROCKFACE_RECOGNIZE);
    flow->Control(easymedia::G_NN_INFO, &cfa);
    return 0;
  }
  return -1;
}

int SetFaceRegArg(int id, easymedia::FaceRegArg fda) {
  auto flow_pipe =
      GetFlowPipe(id, StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_RECOGNIZE);
  if (flow_pipe) {
    auto flow = flow_pipe->GetFlow(StreamType::FILTER,
                                   RKMEDIA_FILTER_ROCKFACE_RECOGNIZE);
    flow->Control(easymedia::S_NN_INFO, &fda);
    return 0;
  }
  return -1;
}

#endif

void SetMDEnabled(int enabled) {
  LOG_INFO("SetMDEnabled\n");
  for (int id = 0; id < GetFlowPipeNum(); id++) {
    auto flow_pipe = GetFlowPipe(id, StreamType::MOVE_DETEC);
    if (flow_pipe) {
      auto md_flow = flow_pipe->GetFlow(StreamType::MOVE_DETEC);
      md_flow->Control(easymedia::S_MD_ROI_ENABLE, enabled);
      break;
    }
  }
}

void SetMDSensitivity(int sensitivity) {
  LOG_INFO("SetMDSensitivity\n");
  for (int id = 0; id < GetFlowPipeNum(); id++) {
    auto flow_pipe = GetFlowPipe(id, StreamType::MOVE_DETEC);
    if (flow_pipe) {
      auto md_flow = flow_pipe->GetFlow(StreamType::MOVE_DETEC);
      md_flow->Control(easymedia::S_MD_SENSITIVITY, sensitivity);
      break;
    }
  }
}

void SetMDRect() {
  LOG_INFO("SetMDRect\n");
#ifdef ENABLE_DBUS
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  std::string md_db = flow_manager->SelectMoveDetectDb(0);
  std::string md_roi_rect;
  int md_roi_cnt = 0;

  for (int id = 0; id < MAX_CAM_NUM; id++) {
    if (!md_db.empty())
      md_roi_rect = db_protocol->GetMoveDetectRegions(id, md_db, md_roi_cnt);
    if (!md_roi_rect.empty()) {
      LOG_INFO("md_roi_cnt is %d\n", md_roi_cnt);
      LOG_INFO("md_roi_rect is %s\n", md_roi_rect.c_str());
      auto flow_pipe = GetFlowPipe(id, StreamType::MOVE_DETEC);
      if (flow_pipe) {
        auto md_flow = flow_pipe->GetFlow(StreamType::MOVE_DETEC);
        easymedia::video_move_detect_set_rects(md_flow, md_roi_rect);
        break;
      }
    }
  }
#endif
}

void SetMDRect2(ImageRect *rects, int rect_cnt) {
  printf("SetMDRect rect_cnt is %d\n", rect_cnt);
  for (int id = 0; id < MAX_CAM_NUM; id++) {
    auto flow_pipe = GetFlowPipe(id, StreamType::MOVE_DETEC);
    if (flow_pipe) {
      auto md_flow = flow_pipe->GetFlow(StreamType::MOVE_DETEC);
      easymedia::video_move_detect_set_rects(md_flow, rects, rect_cnt);
      break;
    }
  }
}

void SetQP(int stream_id, VideoEncoderQp qp) {
  LOG_INFO("SetQP, stream_id is %d, qp_init is %d, qp_step is %d, qp_min is "
           "%d, qp_max is %d, min_i_qp is %d, max_i_qp is %d\n",
           stream_id, qp.qp_init, qp.qp_step, qp.qp_min, qp.qp_max, qp.qp_min_i,
           qp.qp_max_i);
  auto encoder_control = GetEncoderControl(stream_id);
  if (encoder_control)
    encoder_control->SetImageQuality(qp);
}

void SetSplit(int stream_id, int mode, int size) {
  LOG_INFO("SetSplit, stream_id is %d, mode is %d, size is %d\n", stream_id,
           mode, size);
  auto encoder_control = GetEncoderControl(stream_id);
  if (encoder_control)
    encoder_control->SetSplit(mode, size);
}

#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER) &&                    \
     defined(ENABLE_DBUS))
/* region invade functions */
int GetRegionInvade(int id, region_invade_s &region_invade) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  std::string db_config = flow_manager->SelectRegionInvadeDb(id);
  if (db_config.empty() ||
      db_config.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
    LOG_INFO("SelectRegionInvadeDb empty\n");
    return -1;
  }
  db_protocol->GetRegionInvade(db_config, region_invade);
  return 0;
}
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))

void SetRegionInvade(int stream_id, region_invade_s region_invade) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  for (int id = 0; id < GetFlowPipeNum(); id++) {
    auto flow_pipe = GetFlowPipe(id, StreamType::VIDEO_ENCODER);
    if (flow_pipe) {
      auto enc_flow_unit = flow_pipe->GetFlowunit(StreamType::VIDEO_ENCODER);
      enc_flow_unit->SetRegionInvade(region_invade);
      break;
    }
  }

  for (int id = 0; id < GetFlowPipeNum(); id++) {
    std::string stream_name = RKMEDIA_FILTER_ROCKFACE_BODYDETECT;
    auto flow_pipe = GetFlowPipe(id, StreamType::FILTER, stream_name);
    if (flow_pipe) {
      auto flow_unit = flow_pipe->GetFlowunit(StreamType::FILTER, stream_name);
      auto flow = flow_pipe->GetFlow(StreamType::FILTER, stream_name);
      if (flow && flow_unit) {
        ImageRect rect;
        auto str_rect = flow_unit->GetRect();
        if (str_rect.empty())
          continue;
        sscanf(str_rect.c_str(), "(%d,%d,%d,%d)", &rect.x, &rect.y, &rect.w,
               &rect.h);
        float gradient_x_ = (float)rect.w / WEB_VIEW_RECT_W;
        float gradient_y_ = (float)rect.h / WEB_VIEW_RECT_H;
        easymedia::BodyDetectArg bda;
        bda.enable = region_invade.enable;
        bda.interval = region_invade.time_threshold;
        bda.duration = region_invade.time_threshold;
        bda.percentage = region_invade.proportion;
        bda.rect.x = UPALIGNTO16((int)(gradient_x_ * region_invade.position_x));
        bda.rect.y = UPALIGNTO16((int)(gradient_y_ * region_invade.position_y));
        bda.rect.w = UPALIGNTO16((int)(gradient_x_ * region_invade.width));
        bda.rect.h = UPALIGNTO16((int)(gradient_y_ * region_invade.height));
        flow->Control(easymedia::S_NN_INFO, &bda);
      }
      break;
    }
  }
}
#endif

#ifdef USE_ROCKFACE
void SetNNResultInput(RknnResult *result, int size) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  auto nn_handler = flow_manager->GetNNHander();
  if (nn_handler)
    nn_handler->NNResultInput(result, size);
}
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void SetFacePicUpload(char *path, int size) {
  dbserver_snapshot_record_set(path);
}
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))
void SetOsdNNResult(RknnResult *result, int type, int size) {
  if (NNRESULT_TYPE_AUTHORIZED_STATUS == type) {
    for (int i = 0; i < GetFlowPipeNum(); i++) {
      StreamType type = StreamType::VIDEO_ENCODER;
      auto flow_pipe = GetFlowPipe(i, type);
      if (!flow_pipe)
        continue;
      auto nn_flow_unit = flow_pipe->GetFlowunit(StreamType::FILTER,
                                                 RKMEDIA_FILTER_DRAW_FILTER);
      if (!nn_flow_unit)
        continue;
      auto encoder_flow_unit = flow_pipe->GetFlowunit(type);
      if (!encoder_flow_unit)
        continue;
      encoder_flow_unit->SetOsdTip(200, 1000, OSD_NN_TIP_AUTHORIZED);
    }
  } else if (NNRESULT_TYPE_BODY == type) {
    for (int i = 0; i < GetFlowPipeNum(); i++) {
      StreamType type = StreamType::VIDEO_ENCODER;
      auto flow_pipe = GetFlowPipe(i, type);
      if (!flow_pipe)
        continue;
      auto nn_flow_unit = flow_pipe->GetFlowunit(StreamType::FILTER,
                                                 RKMEDIA_FILTER_DRAW_FILTER);
      if (!nn_flow_unit)
        continue;
      auto encoder_flow_unit = flow_pipe->GetFlowunit(type);
      if (!encoder_flow_unit)
        continue;
      encoder_flow_unit->SetRegionInvadeBlink(2, 100);
    }
  }
}
#endif

#if (defined(USE_ROCKX))
void SetRockXNNResultInput(RknnResult *result, int size) {
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  auto nn_handler = flow_manager->GetNNHander();
  if (nn_handler)
    nn_handler->RockXNNResultInput(result, size);
  else
    LOG_DEBUG("nn_handler is null\n");
}

void SetRockxStatus(std::string model_name, int status) {
  for (int i = 0; i < GetFlowPipeNum(); i++) {
    StreamType type = StreamType::CAMERA;
    auto flow_pipe = GetFlowPipe(i, type);
    if (!flow_pipe)
      continue;
    auto nn_flow_unit = flow_pipe->GetFlow(
        StreamType::FILTER, RKMEDIA_FILTER_ROCKX_FILTER, model_name);
    if (!nn_flow_unit)
      continue;
    LOG_INFO("%s %d SetRockxStatus %s %d successful\n", __FUNCTION__, __LINE__,
             model_name.c_str(), status);
    easymedia::RockxFilterArg arg;
    arg.enable = status;
    arg.interval = 0;
    nn_flow_unit->Control(easymedia::S_NN_INFO, &arg);
  }
}
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void SetImageToRecognize(const int32_t &id, const std::string &path) {
  std::string stream_name = RKMEDIA_FILTER_ROCKFACE_RECOGNIZE;
  for (int i = 0; i < GetFlowPipeNum(); i++) {
    auto flow_pipe = GetFlowPipe(i, StreamType::FILTER, stream_name);
    if (flow_pipe) {
      auto flow_unit = flow_pipe->GetFlowunit(StreamType::FILTER, stream_name);
      auto flow = flow_pipe->GetFlow(StreamType::FILTER, stream_name);
      if (flow && flow_unit) {
        easymedia::FaceRegArg arg;
        arg.type = easymedia::USER_ADD_PIC;
        snprintf(arg.pic_path, sizeof(arg.pic_path), "%s", path.c_str());
        flow->Control(easymedia::S_NN_INFO, &arg);
      }
      break;
    }
  }
}
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void DeleteFaceInfoInDB(const int32_t &id, const int32_t &faceId) {
  std::string stream_name = RKMEDIA_FILTER_ROCKFACE_RECOGNIZE;
  for (int i = 0; i < GetFlowPipeNum(); i++) {
    auto flow_pipe = GetFlowPipe(i, StreamType::FILTER, stream_name);
    if (flow_pipe) {
      auto flow_unit = flow_pipe->GetFlowunit(StreamType::FILTER, stream_name);
      auto flow = flow_pipe->GetFlow(StreamType::FILTER, stream_name);
      if (flow && flow_unit) {
        easymedia::FaceRegArg arg;
        arg.type = easymedia::USER_DEL;
        arg.user_id = faceId;
        flow->Control(easymedia::S_NN_INFO, &arg);
      }
      break;
    }
  }
}
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void ClearFaceDBInfo() {
  std::string stream_name = RKMEDIA_FILTER_ROCKFACE_RECOGNIZE;
  for (int i = 0; i < GetFlowPipeNum(); i++) {
    auto flow_pipe = GetFlowPipe(i, StreamType::FILTER, stream_name);
    if (flow_pipe) {
      auto flow_unit = flow_pipe->GetFlowunit(StreamType::FILTER, stream_name);
      auto flow = flow_pipe->GetFlow(StreamType::FILTER, stream_name);
      if (flow && flow_unit) {
        easymedia::FaceRegArg arg;
        arg.type = easymedia::USER_CLR;
        flow->Control(easymedia::S_NN_INFO, &arg);
      }
      break;
    }
  }
}
#endif

#ifdef ENABLE_DBUS
std::string SendMediaStorageStopMsg() {
  LOG_INFO("%s\n", __FUNCTION__);
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();
  return flow_manager->SendMediaStorageStopMsg();
}
#endif

} // namespace mediaserver
} // namespace rockchip

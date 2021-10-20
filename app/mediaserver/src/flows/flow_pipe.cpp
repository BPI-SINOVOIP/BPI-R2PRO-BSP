// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/prctl.h>

#include "flow_db_protocol.h"
#include "flow_export.h"
#include "flow_pipe.h"
#ifdef ENABLE_DBUS
#include "dbserver.h"
#endif
#ifdef LINK_API_ENABLE
#include "link_manager.h"
#endif

#include "flow_export.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_pipe.cpp"

namespace rockchip {
namespace mediaserver {

int FlowPipe::GetFlowIndex(std::string flow_index_name) {
  for (int flow_index = flow_units_.size() - 1; flow_index >= 0; flow_index--) {
    auto &flow_unit = flow_units_[flow_index];
    auto result = flow_index_name.find(flow_unit->GetFlowIndexName());
    if (result != std::string::npos)
      return flow_index;
  }
  return -1;
}

int FlowPipe::GetFlowIndex(StreamType type) {
  for (int flow_index = flow_units_.size() - 1; flow_index >= 0; flow_index--) {
    auto &flow_unit = flow_units_[flow_index];
    if (flow_unit->GetStreamType() == type)
      return flow_index;
  }
  return -1;
}

int FlowPipe::GetFlowIndex(StreamType type, std::string name) {
  for (int flow_index = flow_units_.size() - 1; flow_index >= 0; flow_index--) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    if (stream_type == type && name == stream_name)
      return flow_index;
  }
  return -1;
}

int FlowPipe::GetFlowIndexByInput(StreamType type, std::string data_type) {
  for (int flow_index = flow_units_.size() - 1; flow_index >= 0; flow_index--) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto input_data_type = flow_unit->GetFlowInputDataType();
    if (stream_type == type && input_data_type == data_type)
      return flow_index;
  }
  return -1;
}

std::shared_ptr<easymedia::Flow> FlowPipe::GetFlow(int index) {
  if (index < flow_units_.size() && index >= 0)
    return flow_units_[index]->GetFlow();
  return nullptr;
}

std::shared_ptr<easymedia::Flow> FlowPipe::GetFlow(StreamType type) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      return flow_units_[flow_index]->GetFlow();
    }
  }
  return nullptr;
}

std::shared_ptr<easymedia::Flow> FlowPipe::GetFlow(StreamType type,
                                                   std::string name) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    if (stream_type == type && stream_name == name) {
      return flow_units_[flow_index]->GetFlow();
    }
  }
  return nullptr;
}

std::shared_ptr<easymedia::Flow>
FlowPipe::GetFlow(StreamType type, std::string name, std::string model) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    auto stream_model = flow_unit->GetStreamRockxModel();
    if (stream_type == type && stream_name == name && stream_model == model) {
      return flow_units_[flow_index]->GetFlow();
    }
  }
  return nullptr;
}

std::shared_ptr<easymedia::Flow>
FlowPipe::GetFlowByInput(StreamType type, std::string data_type) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    std::string input_data_type;
    input_data_type = flow_unit->GetFlowInputDataType();
    if (input_data_type.empty())
      input_data_type = flow_unit->GetInputDataType();
    if (stream_type == type &&
        (input_data_type.find(data_type) != std::string::npos)) {
      return flow_units_[flow_index]->GetFlow();
    }
  }
  return nullptr;
}

int FlowPipe::SetFlow(std::shared_ptr<easymedia::Flow> flow, StreamType type) {
  int index = FlowPipe::GetFlowIndex(type);
  if (index < flow_units_.size() && index >= 0) {
    flow_units_[index]->SetFlow(flow);
  }
  return -1;
}

std::shared_ptr<FlowUnit> FlowPipe::GetFlowunit(int index) {
  if (index < flow_units_.size())
    return flow_units_[index];
  return nullptr;
}

std::shared_ptr<FlowUnit> FlowPipe::GetFlowunit(StreamType type) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      return flow_units_[flow_index];
    }
  }
  return nullptr;
}

std::shared_ptr<FlowUnit> FlowPipe::GetFlowunit(StreamType type,
                                                std::string name) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    if (stream_type == type && stream_name == name) {
      return flow_units_[flow_index];
    }
  }
  return nullptr;
}

std::shared_ptr<FlowUnit> FlowPipe::GetFlowunitByInput(StreamType type,
                                                       std::string data_type) {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    std::string input_data_type;
    input_data_type = flow_unit->GetFlowInputDataType();
    if (input_data_type.empty())
      input_data_type = flow_unit->GetInputDataType();
    if (stream_type == type &&
        (input_data_type.find(data_type) != std::string::npos)) {
      return flow_units_[flow_index];
    }
  }
  return nullptr;
}

std::string FlowPipe::GetFlowName(StreamType type) {
  std::string flow_name;
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      flow_name = flow_unit->GetFlowName();
    }
  }
  return flow_name;
}

std::string FlowPipe::GetFlowIndexName(StreamType type) {
  std::string flow_index_name;
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      flow_index_name = flow_unit->GetFlowIndexName();
    }
  }
  return flow_index_name;
}

std::string FlowPipe::GetFlowParam(StreamType type) {
  std::string param;
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      param = flow_unit->FlowParamMapToStr();
    }
  }
  return param;
}

std::string FlowPipe::GetStreamParam(StreamType type) {
  std::string param;
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    std::string flow_name = flow_unit->GetFlowName();
    auto stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      param = flow_unit->StreamParamMapToStr();
    }
  }
  return param;
}

int FlowEventProc(std::shared_ptr<easymedia::Flow> flow, bool &loop) {
  char thread_name[40];
  snprintf(thread_name, sizeof(thread_name), "FlowEventProc");
  prctl(PR_SET_NAME, thread_name);

  while (loop) {
    flow->EventHookWait();
    auto msg = flow->GetEventMessage();
    auto event = flow->GetEventParam(msg);
    if (event == nullptr && msg == nullptr)
      continue;
    LOG_DEBUG("FlowEventProc flow %x msg id %d param %d\n", (int)flow.get(),
             event->GetId(), event->GetParam());
    // TODO
    FlowManagerPtr &flow_manager = FlowManager::GetInstance();
    switch (event->GetId()) {
    case MSG_FLOW_EVENT_INFO_EOS:
      LOG_INFO("FlowEventProc MSG_FLOW_EVENT_INFO_EOS\n");
      break;
    case MSG_FLOW_EVENT_INFO_MOVEDETECTION: {
#ifdef ENABLE_SCHEDULES_SERVER
      if (!flow_manager->MDTriggerEnabled()) // need enable move detect trigger
        break;
#endif
      MoveDetectEvent *mdevent = (MoveDetectEvent *)event->GetParams();
      if (mdevent) {
#ifndef ENABLE_MEDIASERVER_BIN
        flow_manager->MDCallback(mdevent);
        break;
#endif
        LOG_DEBUG("@@@ MD: Get movement info[%d]: ORI:%dx%d, DS:%dx%d\n",
                 mdevent->info_cnt, mdevent->ori_width, mdevent->ori_height,
                 mdevent->ds_width, mdevent->ds_height);
        MoveDetecInfo *mdinfo = mdevent->data;
        for (int i = 0; i < mdevent->info_cnt; i++) {
          LOG_DEBUG("--> %d rect:(%d, %d, %d, %d)\n", i, mdinfo->x, mdinfo->y,
                   mdinfo->w, mdinfo->h);
          mdinfo++;
        }
#ifdef ENABLE_SCHEDULES_SERVER
        flow_manager->EventSnap();
        flow_manager->MDEventCountAdd();
#endif
      }
    } break;
#if 0 // def LINK_API_ENABLE
    case MSG_FLOW_EVENT_INFO_FACE_CAPTURE: {
      auto &link_manager = LinkManager::GetInstance();
      LOG_INFO("FlowEventProc FacePictureUpload %s\n",
               (const char *)event->GetParams());
      link_manager->FacePictureUpload((const char *)event->GetParams());
    } break;
#endif
    default:
      easymedia::msleep(33);
      break;
    }
  }
  return 0;
}

void FlowCallBack(void *handler, int type, void *ptr, int size) {
  switch (type) {
#ifdef USE_ROCKX
  case NNRESULT_TYPE_OBJECT_DETECT: {
    SetRockXNNResultInput((RknnResult *)ptr, size);
  } break;
#endif
#ifdef USE_ROCKFACE
  case NNRESULT_TYPE_FACE: {
    SetNNResultInput((RknnResult *)ptr, size);
  } break;
  case NNRESULT_TYPE_FACE_REG: {
    for (int i = 0; i < size; i++) {
      RknnResult *nn_array = (RknnResult *)ptr;
      RknnResult *nn = &nn_array[i];

      FaceReg *face_reg = &nn->face_info.face_reg;
      if (face_reg->type == FACE_REG_RECOGNIZE) {
        if (face_reg->user_id >= 0) {
#ifdef ENABLE_DBUS
          char *similarity_buf = new char[8];
          snprintf(similarity_buf, sizeof(similarity_buf), "%f ",
                   face_reg->similarity);
          dbserver_control_record_set(face_reg->user_id, (char *)"", (char *)"Processed",
                                      similarity_buf);
          delete[] similarity_buf;
#endif
          LOG_INFO("recognize user_%d,similarity = %f, pic_path = %s\n",
              face_reg->user_id, face_reg->similarity, face_reg->pic_path);
        } else {
          LOG_DEBUG("recognize Unknow, similarity = %f, pic_path = %s\n",
              face_reg->similarity, face_reg->pic_path);
        }
      } else if (face_reg->type == FACE_REG_REGISTER) {
        if (face_reg->user_id >= 0) {
#ifdef ENABLE_DBUS
          dbserver_face_load_complete_by_path(face_reg->pic_path, 1,
                                              face_reg->user_id);
#endif
          LOG_INFO("register user_%d succeful, pic_path = %s\n", face_reg->user_id,
              face_reg->pic_path);
        } else if (face_reg->user_id == -1) {
#ifdef ENABLE_DBUS
          dbserver_face_load_complete_by_path(face_reg->pic_path, 2, -1);
#endif
          LOG_INFO("face register repeated\n");
        } else if (face_reg->user_id == -99) {
#ifdef ENABLE_DBUS
          dbserver_face_load_complete_by_path(face_reg->pic_path, -1, -1);
#endif
          LOG_INFO("register user_%d fail, pic_path = %s\n", face_reg->user_id,
              face_reg->pic_path);
        }
      } else {
        assert(0);
      }
    }
  } break;
#endif
#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
  case NNRESULT_TYPE_FACE_PICTURE_UPLOAD: {
    if (size)
      SetFacePicUpload((char *)ptr, size);
  } break;
#endif
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))
  case NNRESULT_TYPE_AUTHORIZED_STATUS: {
    SetOsdNNResult((RknnResult *)ptr, type, size);
  } break;
  case NNRESULT_TYPE_BODY: {
    SetOsdNNResult((RknnResult *)ptr, type, size);
  } break;
#endif
  default:
    break;
  }
}

std::vector<std::string> SplitStringToVector(std::string params) {
  std::vector<std::string> v;
  std::string str2 = params;
  while (str2.find(" ") != std::string::npos) {
    int found = str2.find(" ");
    v.push_back(str2.substr(0, found));
    str2 = str2.substr(found + 1);
  }
  v.push_back(str2);
  return v;
}

#ifdef ENABLE_OSD_SERVER
int FlowPipe::RegisterOsdServer() {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto &flow = flow_unit->GetFlow();
    if (flow_unit->GetStreamType() == StreamType::VIDEO_ENCODER) {
      std::string output_data_type = flow_unit->GetOutputDataType();
      if (!output_data_type.compare(IMAGE_JPEG))
        continue;
      flow_unit->RegisterOsdServer(flow);
#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
      auto ri_config = flow_unit->GetRIRegions();
      if (!ri_config.empty()) {
        region_invade_s region_invade;
        std::unique_ptr<FlowDbProtocol> db_protocol;
        db_protocol.reset(new FlowDbProtocol);
        memset(&region_invade, 0, sizeof(region_invade_s));
        db_protocol->GetRegionInvade(ri_config, region_invade);
        flow_unit->SetRegionInvade(region_invade);
      } else {
        LOG_ERROR("1 select ri_config flow_index %d empty\n", flow_index);
      }
#endif
    }
  }
  return 0;
}

int FlowPipe::UnRegisterOsdServer() {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    if (flow_unit->GetStreamType() == StreamType::VIDEO_ENCODER) {
      flow_unit->UnRegisterOsdServer();
    }
  }
  return 0;
}
#endif

int FlowPipe::BindControler() {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    auto &flow = flow_unit->GetFlow();
    flow_unit->BindControl(flow, flow_unit->GetStreamType());
  }
  return 0;
}

int FlowPipe::UnBindControler() {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    auto &flow_unit = flow_units_[flow_index];
    flow_unit->UnBindControl();
  }
  return 0;
}

int FlowPipe::RegisterCallBack() {
  auto encoder_flow =
      GetFlow(StreamType::VIDEO_ENCODER, RKMEDIA_STREAM_NAME_RKMPP);
  auto draw_flow = GetFlow(StreamType::FILTER, RKMEDIA_FILTER_DRAW_FILTER);
  if (encoder_flow && draw_flow)
    draw_flow->Control(easymedia::S_NN_DRAW_HANDLER, encoder_flow.get());
  auto detect_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_DETECT);
  if (detect_flow)
    detect_flow->Control(easymedia::S_NN_CALLBACK, FlowCallBack);
  auto body_detect_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_BODYDETECT);
  if (body_detect_flow)
    body_detect_flow->Control(easymedia::S_NN_CALLBACK, FlowCallBack);
  auto face_capture_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_FACE_CAPTURE);
  if (face_capture_flow)
    face_capture_flow->Control(easymedia::S_NN_CALLBACK, FlowCallBack);
  auto face_recognize_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_RECOGNIZE);
  if (face_recognize_flow)
    face_recognize_flow->Control(easymedia::S_NN_CALLBACK, FlowCallBack);
  auto rockx_flow = GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKX_FILTER);
  if (rockx_flow)
    rockx_flow->Control(easymedia::S_NN_CALLBACK, FlowCallBack);
  return 0;
}

int FlowPipe::UnRegisterCallBack() {
  auto face_capture_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_FACE_CAPTURE);
  if (face_capture_flow)
    face_capture_flow->Control(easymedia::S_NN_CALLBACK, nullptr);
  auto body_detect_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_BODYDETECT);
  if (body_detect_flow)
    body_detect_flow->Control(easymedia::S_NN_CALLBACK, nullptr);
  auto detect_flow =
      GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKFACE_DETECT);
  if (detect_flow)
    detect_flow->Control(easymedia::S_NN_CALLBACK, nullptr);
  auto encoder_flow =
      GetFlow(StreamType::VIDEO_ENCODER, RKMEDIA_STREAM_NAME_RKMPP);
  auto draw_flow = GetFlow(StreamType::FILTER, RKMEDIA_FILTER_DRAW_FILTER);
  if (encoder_flow && draw_flow)
    draw_flow->Control(easymedia::S_NN_DRAW_HANDLER, nullptr);
  auto rockx_flow = GetFlow(StreamType::FILTER, RKMEDIA_FILTER_ROCKX_FILTER);
  if (rockx_flow)
    rockx_flow->Control(easymedia::S_NN_CALLBACK, nullptr);
  return 0;
}

int FlowPipe::CreateFlow(std::shared_ptr<FlowUnit> flow_unit) {
  std::string param;
  std::string flow_name = flow_unit->GetFlowName();
  std::string flow_param = flow_unit->FlowParamMapToStr();
  std::string stream_param = flow_unit->StreamParamMapToStr();
  auto type = flow_unit->GetStreamType();

#ifndef USE_ROCKFACE_REG
  if (flow_param.find("rockface_recognize") != std::string::npos) {
    LOG_INFO("skip face recognize flow!\n");
    return -1;
  }
#else
  if (flow_param.find("rockface_recognize") != std::string::npos) {
    LOG_ERROR("set rockface_recognize, when not use rockface reg\n");
  }
#endif

  if (type == StreamType::MUXER) {
    std::string vid_param = "";
    std::string aud_param = "";
    std::string flow_index_name = flow_unit->GetFlowIndexName();
    std::string upflow_index_name = flow_unit->GetUpFlowIndexName();
    auto v = SplitStringToVector(upflow_index_name);
    for (auto name : v) {
      int upflow_index = GetFlowIndex(name);
      auto flow_unit = GetFlowunit(upflow_index);
      StreamType type = flow_unit->GetStreamType();
      if (type == StreamType::VIDEO_ENCODER)
        vid_param = GetStreamParam(StreamType::VIDEO_ENCODER);
      else if (type == StreamType::AUDIO_ENCODER)
        aud_param = GetStreamParam(StreamType::AUDIO_ENCODER);
    }
    LOG_DEBUG("vid_param: %s, aud_param: %s\n", (char *)vid_param.c_str(), (char *)aud_param.c_str());
    param = easymedia::JoinFlowParam(flow_param, 2, aud_param, vid_param);
  } else {
    param = easymedia::JoinFlowParam(flow_param, 1, stream_param);
  }

  LOG_DEBUG("flow_name :%s\n", flow_name.c_str());
  LOG_DEBUG("flow_param :\n%s\n", param.c_str());
  int reopen_pipe_index = flow_unit->GetOpenPipeId();
  int reopen_flow_index = flow_unit->GetOpenFlowId();
  if (reopen_pipe_index >= 0 && reopen_flow_index >= 0) {
    flow_unit->SetFlow(nullptr);
    return 0;
  }
  auto flow = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
      flow_name.c_str(), param.c_str());
  if (!flow) {
    LOG_ERROR("Create flow %s failed\n", flow_name.c_str());
    LOG_ERROR("flow param :\n%s\n", param.c_str());
    exit(EXIT_FAILURE);
  }

  if (type == StreamType::VIDEO_ENCODER && enable_encoder_debug)
    easymedia::video_encoder_enable_statistics(flow, 1);

  flow->RegisterEventHandler(flow, FlowEventProc);
  flow_unit->SetFlow(flow);
  return 0;
}

void FlowPipe::CreateFlows(flow_unit_v &flows) {
  std::string param;
  for (auto &iter : flows) {
    int ret = CreateFlow(iter);
    if (!ret)
      flow_units_.emplace_back(iter);
  }
}

int FlowPipe::InitFlow(int flow_index) {
  auto &flow_unit = flow_units_[flow_index];
  auto &flow = flow_unit->GetFlow();
  auto tsream_type = flow_unit->GetStreamType();
  auto flow_type = flow_unit->GetFlowType();
  if (FlowType::SOURCE == flow_type)
    return 0;

  if (StreamType::MOVE_DETEC == tsream_type) {
    FlowManagerPtr &flow_manager = FlowManager::GetInstance();
    auto flow_pipes_ = flow_manager->GetPipes();
    std::shared_ptr<easymedia::Flow> main_enc_flow;
    std::string smart_status;
    for (int i = 0; i < flow_manager->GetPipesSize(); i++) {
      auto flow_unit = flow_pipes_[i]->GetFlowunit(StreamType::CAMERA);
      int stream_id = flow_unit->GetStreamId();
      if (stream_id == 0) {
        main_enc_flow = flow_pipes_[i]->GetFlow(StreamType::VIDEO_ENCODER);
        auto main_enc_flow_unit =
            flow_pipes_[i]->GetFlowunit(StreamType::VIDEO_ENCODER);
        smart_status = main_enc_flow_unit->GetSmart();
        break;
      }
    }
    if (!smart_status.compare(DB_SMART_VALUE_OPEN)) {
      LOG_INFO("InitSmart\n");
      LOG_DEBUG("\n# main_enc_flow is %p!\n", main_enc_flow.get());
      LOG_DEBUG("\n# sub_md_flow is %p!\n", flow.get());
      easymedia::video_encoder_set_move_detection(main_enc_flow, flow);
    } else {
      LOG_INFO("smart is close\n");
    }
  }

  std::string flow_index_name = flow_unit->GetFlowIndexName();
  auto upflow_index_name = flow_unit->GetUpFlowIndexName();
  int out_slot_index = flow_unit->GetOutSlotIndex();
  int in_slot_index_of_down = flow_unit->GetInSlotIndexOfDown();
  auto v = SplitStringToVector(upflow_index_name);
  for (auto name : v) {
    int upflow_index = GetFlowIndex(name);
    auto &upflow = flow_units_[upflow_index]->GetFlow();
    upflow->AddDownFlow(flow, in_slot_index_of_down, out_slot_index);
    LOG_DEBUG("flow_name %s upflow_name %s in_slot %d out_slot %d\n",
              flow_index_name.c_str(), name.c_str(), in_slot_index_of_down,
              out_slot_index);
    out_slot_index++;
  }
  return 0;
}

int FlowPipe::InitFlows() {
  for (int flow_index = flow_units_.size() - 1; flow_index >= 0; flow_index--) {
    InitFlow(flow_index);
  }
  return 0;
}

int FlowPipe::InitMultiSlice() {
  for (int i = 0; i < MAX_CAM_NUM; i++) {
    auto encoder_control = GetEncoderControl(i);
    if (!encoder_control)
      return -1;

    FlowManagerPtr &flow_manager = FlowManager::GetInstance();
    int pipe_index =
        flow_manager->GetPipeIndexById((int)i, StreamType::VIDEO_ENCODER);
    if (pipe_index < 0) {
      LOG_ERROR("pipe_index is %d\n", pipe_index);
      return -1;
    }
    auto &pipe = flow_manager->GetPipeByIndex(pipe_index);
    int flow_index = pipe->GetFlowIndex(StreamType::VIDEO_ENCODER);
    if (flow_index < 0) {
      LOG_ERROR("flow_index is %d\n", flow_index);
      return -1;
    }
    auto &flows = pipe->GetFlowUnits();
    auto &encoder_flow_unit = flows[flow_index];

    std::string width_s;
    std::string height_s;
    encoder_flow_unit->GetResolution(width_s, height_s);
    int width = stoi(width_s);
    int height = stoi(height_s);
    encoder_control->SetMultiSlice(width, height);
  }
  return 0;
}

int FlowPipe::DeinitFlowFirst(int flow_index) {
  auto &flow_unit = flow_units_[flow_index];
  auto &flow = flow_unit->GetFlow();
  auto flow_type = flow_unit->GetFlowType();
  auto stream_type = flow_unit->GetStreamType();
  if (FlowType::SOURCE == flow_type)
    return 0;

  auto flow_index_name = flow_unit->GetFlowIndexName();
  auto upflow_index_name = flow_unit->GetUpFlowIndexName();
  LOG_DEBUG("#### upflow %s remove flow %s\n", upflow_index_name.c_str(),
            flow_index_name.c_str());

  auto v = SplitStringToVector(upflow_index_name);
  for (auto name : v) {
    int upflow_index = GetFlowIndex(name);
    auto &upflow = flow_units_[upflow_index]->GetFlow();
    upflow->RemoveDownFlow(flow);
  }
  return 0;
}

int FlowPipe::DeinitFlowSecond(int flow_index) {
  auto &flow_unit = flow_units_[flow_index];
  auto &flow = flow_unit->GetFlow();
  flow->UnRegisterEventHandler();
  LOG_DEBUG("#### flow %s reset count %d\n",
            flow_unit->GetFlowIndexName().c_str(), flow.use_count());
  flow.reset();
  flow = nullptr;
  return 0;
}

int FlowPipe::DeinitFlows() {
  for (int flow_index = 0; flow_index < flow_units_.size(); flow_index++) {
    DeinitFlowFirst(flow_index);
  }

  for (int flow_index = (flow_units_.size() - 1); flow_index >= 0; flow_index--) {
    DeinitFlowSecond(flow_index);
  }
  return 0;
}

int FlowPipe::DeinitFlow(int flow_index) {
  DeinitFlowFirst(flow_index);
  DeinitFlowSecond(flow_index);
  return 0;
}

void FlowPipe::DestoryFlows() { flow_units_.clear(); }

} // namespace mediaserver
} // namespace rockchip

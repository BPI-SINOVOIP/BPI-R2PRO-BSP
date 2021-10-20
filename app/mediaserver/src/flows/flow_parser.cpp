// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_parser.h"
#include "flow_pipe.h"
#include <assert.h>
#include <string.h>

#include "flow_db_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_parser.cpp"

namespace rockchip {
namespace mediaserver {

FlowParser::FlowParser(const char *path) {
  JsConfigRead(path);
  JsConfigParse();
  SyncReOpenFlow();
}

void FlowParser::DumpPipes() {
  int flow_index = 0;
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    LOG_INFO("Pipe id: %d \n", pipe_index);
    auto &flows = GetFlowUnits(pipe_index);
    for (auto &iter : flows) {
      iter->DumpProps();
    }
  }
}

#if (defined(ENABLE_DBUS) || !defined(ENABLE_MEDIASERVER_BIN))

void FlowParser::SyncCameraDBData(int id, std::string key, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int camera_index = GetFlowIndex(pipe_index, StreamType::CAMERA);
  if (camera_index < 0)
    return;
  auto &camera_flow_unit = flow_units[camera_index];
  if (key == DB_VIDEO_RESOLUTION) {
    if (!camera_flow_unit->GetFixResolution()) {
      std::string str = value;
      int pos = str.find("*");
      std::string width = str.substr(0, pos);
      std::string height = str.substr(pos + 1, str.size() - 1);
      camera_flow_unit->SetResolution(width, height);
    }
  }
}

void FlowParser::SyncEncoderDBData(int id, std::string key, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int encoder_index = GetFlowIndex(pipe_index, StreamType::VIDEO_ENCODER);
  if (encoder_index < 0)
    return;
  auto &encoder_flow_unit = flow_units[encoder_index];
  if (key == DB_VIDEO_GOP) {
    encoder_flow_unit->SetGop(value);
  } else if (key == DB_VIDEO_MAX_RATE) {
    int bps = atoi(value.c_str()) * KB_UNITS;
    encoder_flow_unit->SetMaxRate(std::to_string(bps));
  } else if (key == DB_VIDEO_BITRATE) {
    int bps = atoi(value.c_str()) * KB_UNITS;
    encoder_flow_unit->SetBitRate(std::to_string(bps));
  } else if (key == DB_VIDEO_MIN_RATE) {
    int bps = atoi(value.c_str()) * KB_UNITS;
    encoder_flow_unit->SetMinRate(std::to_string(bps));
  } else if (key == DB_VIDEO_STREAM_SMOOTH) {
    encoder_flow_unit->SetStreamSmooth(value);
  } else if (key == DB_VIDEO_FRAME_RATE) {
    if (value.find("/") != std::string::npos) {
      encoder_flow_unit->SetFrameRate(value);
    } else {
      value.append("/1");
      encoder_flow_unit->SetFrameRate(value);
    }
  } else if (key == DB_VIDEO_FRAME_RATE_IN) {
    if (value.find("/") != std::string::npos) {
      encoder_flow_unit->SetFrameRateIn(value);
    } else {
      value.append("/1");
      encoder_flow_unit->SetFrameRateIn(value);
    }
  } else if (key == DB_VIDEO_RESOLUTION) {
    std::string str = value;
    int pos = str.find("*");
    std::string width = str.substr(0, pos);
    std::string height = str.substr(pos + 1, str.size() - 1);
    encoder_flow_unit->SetResolution(width, height);
  } else if (key == DB_VIDEO_RC_QUALITY) {
    encoder_flow_unit->SetRCQuality(value);
  } else if (key == DB_VIDEO_OUTPUT_DATA_TYPE) {
    encoder_flow_unit->SetOutputDataType(value);
  } else if (key == DB_VIDEO_RC_MODE) {
    encoder_flow_unit->SetRCMode(value);
  } else if (key == DB_VIDEO_H264_PROFILE) {
    encoder_flow_unit->SetH264Profile(value);
  } else if (key == DB_VIDEO_SMART) {
    encoder_flow_unit->SetSmart(value);
  } else if (key == DB_VIDEO_SVC) {
    encoder_flow_unit->SetSVC(value);
  } else if (key == DB_VIDEO_TYPE) {
    encoder_flow_unit->SetVideoType(value);
  } else if (key == DB_IMAGE_GRAY_SCALE_MODE) {
    if (!value.compare(DB_IMAGE_GRAY_SCALE_0_255))
      encoder_flow_unit->SetFullRange(DB_VALUE_ENABLE);
    else
      encoder_flow_unit->SetFullRange(DB_VALUE_DISABLE);
  }
}

void FlowParser::SyncSmartCoverDBData(int id, std::string key,
                                      std::string value) {
  if (key == DB_MEDIA_SMART_COVER_FACE_DETECT_ENABLE) {
    StreamType stream_type = StreamType::FILTER;
    std::string stream_name = RKMEDIA_FILTER_ROCKFACE_DETECT;
    int pipe_index = GetPipeIndex(id, stream_type, stream_name);
    if (pipe_index < 0)
      return;
    auto &flow_units = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, stream_type, stream_name);
    if (flow_index < 0)
      return;
    auto &flow_unit = flow_units[flow_index];
    flow_unit->SetEnable(value);

    stream_type = StreamType::FILTER;
    stream_name = RKMEDIA_FILTER_NN_INPUT;
    pipe_index = GetPipeIndex(id, stream_type, stream_name);
    if (pipe_index < 0)
      return;
    auto &nn_input_flow_units = GetFlowUnits(pipe_index);
    flow_index = GetFlowIndex(pipe_index, stream_type, stream_name);
    if (flow_index < 0)
      return;
    auto &nn_input_flow_unit = nn_input_flow_units[flow_index];
    nn_input_flow_unit->SetEnable(value);
  } else if (key == DB_MEDIA_SMART_COVER_FACE_CAPTURE_ENABLE) {
    StreamType stream_type = StreamType::FILTER;
    std::string stream_name = RKMEDIA_FILTER_FACE_CAPTURE;
    int pipe_index = GetPipeIndex(id, stream_type, stream_name);
    if (pipe_index < 0)
      return;
    auto &flow_units = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, stream_type, stream_name);
    if (flow_index < 0)
      return;
    auto &flow_unit = flow_units[flow_index];
    flow_unit->SetEnable(value);
  } else if (key == DB_MEDIA_SMART_COVER_DRAW_FACE_ENABLE) {
    StreamType stream_type = StreamType::FILTER;
    std::string stream_name = RKMEDIA_FILTER_DRAW_FILTER;
    int pipe_index = GetPipeIndex(id, stream_type, stream_name);
    if (pipe_index < 0)
      return;
    auto &flow_units = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, stream_type, stream_name);
    if (flow_index < 0)
      return;
    auto &flow_unit = flow_units[flow_index];
    flow_unit->SetEnable(value);
  } else if (key == DB_MEDIA_SMART_COVER_FACE_REG_ENABLE) {
    StreamType stream_type = StreamType::FILTER;
    std::string stream_name = RKMEDIA_FILTER_ROCKFACE_RECOGNIZE;
    int pipe_index = GetPipeIndex(id, stream_type, stream_name);
    if (pipe_index < 0)
      return;
    auto &flow_units = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, stream_type, stream_name);
    if (flow_index < 0)
      return;
    auto &flow_unit = flow_units[flow_index];
    flow_unit->SetRegEnable(value);
  }
}

void FlowParser::SyncMoveDetecDBData(int id, std::string key,
                                     std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::MOVE_DETEC);
  if ((pipe_index < 0) || (pipe_index != id))
    return;
  int move_index = GetFlowIndex(pipe_index, StreamType::MOVE_DETEC);
  if (move_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  auto &move_flow_unit = flow_units[move_index];
  if (key == DB_VIDEO_RESOLUTION) {
    if (!move_flow_unit->GetFixResolution()) {
      std::string str = value;
      int pos = str.find("*");
      std::string width = str.substr(0, pos);
      std::string height = str.substr(pos + 1, str.size() - 1);
      move_flow_unit->SetMoveDetecResolution(width, height);
    }
  }
}

void FlowParser::SyncMoveDetecDBData(int id, int md_roi_cnt,
                                     std::string md_roi_rect) {
  int pipe_index = GetPipeIndex(id, StreamType::MOVE_DETEC);
  if (pipe_index < 0)
    return;
  int move_index = GetFlowIndex(pipe_index, StreamType::MOVE_DETEC);
  if (move_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  auto &move_flow_unit = flow_units[move_index];
  std::string md_roi_cnt_s = std::to_string(md_roi_cnt);
  move_flow_unit->SetMoveDetectRoiCnt(md_roi_cnt_s);
  move_flow_unit->SetMoveDetectRoiRect(md_roi_rect);
}

void FlowParser::SyncVideoDBData(int id, std::string key, std::string value) {
  SyncCameraDBData(id, key, value);
  SyncEncoderDBData(id, key, value);
  SyncMoveDetecDBData(id, key, value);
}

void FlowParser::SyncAudioDBData(int id, std::string key, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::AUDIO);
  if (pipe_index < 0)
    return;
  int audio_index = GetFlowIndex(pipe_index, StreamType::AUDIO);
  if (audio_index < 0)
    return;

  auto &flow_units = GetFlowUnits(pipe_index);
  auto &audio_flow_unit = flow_units[audio_index];
  if (key == DB_AUDIO_SAMPLE_RATE) {
    int audio_enc_index = GetFlowIndex(pipe_index, StreamType::AUDIO_ENCODER);
    int audio_rtsp_index = GetFlowIndex(pipe_index, StreamType::RTSP);
    audio_flow_unit->SetSampleRate(value);
    if (audio_enc_index >= 0) {
      auto &audio_enc_flow_unit = flow_units[audio_enc_index];
      audio_enc_flow_unit->SetSampleRate(value);
    }
    if (audio_rtsp_index >= 0) {
      auto &audio_rtsp_flow_unit = flow_units[audio_rtsp_index];
      audio_rtsp_flow_unit->SetSampleRate(value);
    }
  } else if (key == DB_AUDIO_VOLUME) {
    audio_flow_unit->SetVolume(value);
  } else if (key == DB_AUDIO_BIT_RATE) {
    audio_flow_unit->SetBitRate(value);
  } else if (key == DB_AUDIO_SOURCE) {
    audio_flow_unit->SetAudioSource(value);
  } else if (key == DB_AUDIO_ENCODE_TYPE) {
    int audio_enc_index = GetFlowIndex(pipe_index, StreamType::AUDIO_ENCODER);
    if (audio_enc_index >= 0) {
      auto &audio_enc_flow_unit = flow_units[audio_enc_index];
      audio_enc_flow_unit->SetEncodeType(value);
    }
    audio_flow_unit->SetSampleFormat(value);
  } else if (key == DB_AUDIO_ANS) {
    audio_flow_unit->SetANS(value);
  } else if (key == DB_VIDEO_SMART) {
    audio_flow_unit->SetSmart(value);
  }
}

void FlowParser::SyncRoiDBData(int id, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int encoder_index = GetFlowIndex(pipe_index, StreamType::VIDEO_ENCODER);
  if (encoder_index < 0)
    return;
  auto &encoder_flow_unit = flow_units[encoder_index];
  encoder_flow_unit->SetRoiRegions(value);
}

void FlowParser::SyncRIDBData(int id, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int encoder_index = GetFlowIndex(pipe_index, StreamType::VIDEO_ENCODER);
  if (encoder_index < 0)
    return;
  auto &encoder_flow_unit = flow_units[encoder_index];
  encoder_flow_unit->SetRIRegions(value);
}

void FlowParser::SyncRTSPDBData(int id, std::string key, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::RTSP);
  if (pipe_index < 0)
    return;
  int rtsp_index = GetFlowIndex(pipe_index, StreamType::RTSP);
  if (rtsp_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  auto &rtsp_flow_unit = flow_units[rtsp_index];
  if (key == DB_PORT_NUM) {
    rtsp_flow_unit->SetPortNum(value);
  }
}

void FlowParser::SyncRTMPDBData(int id, std::string key, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::MUXER);
  if (pipe_index < 0)
    return;
  int muxer_index = GetFlowIndex(pipe_index, StreamType::MUXER);
  if (muxer_index < 0)
    return;
  int skip_index = 0;
  while (muxer_index >= 0) {
    skip_index++;
    do {
      auto &flow_units = GetFlowUnits(pipe_index);
      auto &muxer_flow_unit = flow_units[muxer_index];
      std::string path = muxer_flow_unit->GetFilePath();
      if (path.find(DB_RTMP) == std::string::npos)
        break;
      if (key == DB_PORT_NUM) {
        LOG_INFO("rtmp old path is %s\n", path.c_str());
        path.replace(path.rfind(":") + 1, path.rfind("live") - path.rfind(":") - 2,
                    value);
        LOG_INFO("rtmp new path is %s\n", path.c_str());
        muxer_flow_unit->SetFilePath(path);
      }
    } while (0);
    muxer_index = GetFlowIndex(pipe_index, StreamType::MUXER, skip_index);
  }
}

#endif // (defined(ENABLE_DBUS) || !defined(ENABLE_MEDIASERVER_BIN))

#ifdef ENABLE_DBUS

#if (defined(ENABLE_OSD_SERVER) && defined(USE_ROCKFACE))

void FlowParser::SyncBodyDetectDBData(int id, std::string value) {
  std::string stream_name = RKMEDIA_FILTER_ROCKFACE_BODYDETECT;
  int pipe_index = GetPipeIndex(id, StreamType::FILTER, stream_name);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int bd_index = GetFlowIndex(pipe_index, StreamType::FILTER, stream_name);
  if (bd_index < 0)
    return;
  auto &bd_flow_unit = flow_units[bd_index];
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  auto enable = db_protocol->GetValueByKey(value, DB_REGION_INVADE_ENABLED);
  if (!enable.empty())
    bd_flow_unit->SetEnable(enable);
  auto time_threshold =
      db_protocol->GetValueByKey(value, DB_REGION_INVADE_TIME_THRESHOLD);
  if (!time_threshold.empty())
    bd_flow_unit->SetDuration(time_threshold);
  auto proportion =
      db_protocol->GetValueByKey(value, DB_REGION_INVADE_PROPORTION);
  if (!proportion.empty())
    bd_flow_unit->SetPercentage(proportion);
  auto img_rect_str = bd_flow_unit->GetRect();
  if (!img_rect_str.empty()) {
    auto det_rect_str = db_protocol->GetRegionInvadeRect(value, img_rect_str);
    if (!det_rect_str.empty() && !img_rect_str.empty())
      bd_flow_unit->SetDetectRect(det_rect_str);
  }
}

#endif

#endif

void FlowParser::SyncMuxerPath(int id, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int muxer_index = GetMuxerFlowIndex(pipe_index, 0);
  if (muxer_index < 0)
    return;
  auto &muxer_flow_unit = flow_units[muxer_index];
  if (!muxer_flow_unit->GetFilePreFix().empty()) {
    muxer_flow_unit->SetFilePath(value);
  }
}

void FlowParser::SyncFilePath(int id, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int file_index = GetFlowIndex(pipe_index, StreamType::FILE);
  if (file_index < 0)
    return;
  auto &file_flow_unit = flow_units[file_index];
  if (!file_flow_unit->GetFilePreFix().empty()) {
    file_flow_unit->SetFilePath(value);
  }
}

void FlowParser::SyncSnapPath(int id, std::string value) {
  int pipe_index = GetPipeIndex(id, StreamType::CAMERA);
  if (pipe_index < 0)
    return;
  auto &flow_units = GetFlowUnits(pipe_index);
  int face_capture_index = GetFaceCaptureFlowIndex(pipe_index);
  if (face_capture_index < 0)
    return;
  auto &face_capture_flow_unit = flow_units[face_capture_index];
  if (!face_capture_flow_unit->GetFilePreFix().empty()) {
    face_capture_flow_unit->SetFilePathInStreamParam(value);
  }
}

void FlowParser::SyncReOpenFlow() {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto &flows = GetFlowUnits(pipe_index);
    for (auto &iter : flows) {
      auto &flow_unit = iter;
      int reopen_pipe_index = flow_unit->GetOpenPipeId();
      int reopen_flow_index = flow_unit->GetOpenFlowId();
      if (reopen_pipe_index < 0 || reopen_flow_index < 0)
        continue;
      LOG_INFO("flowunit: %s copy pipe id %d, flow id %d\n",
               flow_unit->GetFlowIndexName().c_str(), reopen_pipe_index,
               reopen_pipe_index);
      auto &reopen_flow_units = GetFlowUnits(reopen_pipe_index);
      auto &reopen_flow_unit = reopen_flow_units[reopen_flow_index];
      StreamType stream_type = reopen_flow_unit->GetStreamType();
      auto flow_param_prop = reopen_flow_unit->GetFlowParamProp();
      auto stream_param_prop = reopen_flow_unit->GetStreamParamProp();
      flow_unit->SetFlowParamProp(flow_param_prop);
      flow_unit->SetStreamParamProp(stream_param_prop);
    }
  }
}

void FlowParser::SyncRgafiter() {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto &flows = GetFlowUnits(pipe_index);
    int rga_flow_index =
        GetFlowIndex(pipe_index, StreamType::FILTER, RKMEDIA_FILTER_NAME_RGA);
    if (rga_flow_index < 0)
      continue;
    auto &rga_flow_unit = flows[rga_flow_index];
    int need_ajust_reso = rga_flow_unit->GetAjustResolution();
    if (!need_ajust_reso)
      continue;
    auto rga_upflow_index_name = rga_flow_unit->GetUpFlowIndexName();
    auto rga_flow_index_name = rga_flow_unit->GetFlowIndexName();
    int rga_upflow_index =
        GetFlowIndexByFlowIndexName(pipe_index, rga_upflow_index_name);
    if (rga_upflow_index < 0)
      continue;
    int rga_downflow_index =
        GetNextFlowIndex(pipe_index, rga_flow_index, StreamType::VIDEO_ENCODER);
    if (rga_downflow_index < 0)
      continue;
    auto &rga_upflow_unit = flows[rga_upflow_index];
    auto &rga_downflow_unit = flows[rga_downflow_index];
    std::string source_width, source_height;
    std::string target_width, target_height;
    rga_upflow_unit->GetResolution(source_width, source_height);
    rga_downflow_unit->GetResolution(target_width, target_height);
    rga_flow_unit->SetResolution(target_width, target_height);
    char r[128] = {0};
    snprintf(r, sizeof(r), "(0,0,%s,%s)->(0,0,%s,%s)", source_width.c_str(),
             source_height.c_str(), target_width.c_str(),
             target_height.c_str());
    rga_flow_unit->SetRect(r);
  }
}

void FlowParser::SyncThrougfiter() {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto &flows = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, StreamType::JPEG_THROUGH_GUARD);
    if (flow_index < 0)
      continue;
    auto &through_unit = flows[flow_index];
    int source_index = GetFlowIndex(pipe_index, StreamType::CAMERA);
    if (source_index < 0) {
      source_index = GetFlowIndex(pipe_index, StreamType::FILE);
      if (source_index < 0) {
        continue;
      }
    }
    int encoder_index = GetFlowIndex(pipe_index, StreamType::JPEG_ENCODER);
    if (encoder_index < 0)
      continue;
    auto &source_unit = flows[source_index];
    auto &encoder_unit = flows[encoder_index];
    std::string target_width, target_height;
    source_unit->GetResolution(target_width, target_height);
    // through_unit->SetResolution(target_width, target_height);
    encoder_unit->SetResolution(target_width, target_height);
  }
}

void FlowParser::SyncRtsp() {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto &flows = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, StreamType::RTSP);
    if (flow_index < 0)
      break;
    auto &flow_unit = flows[flow_index];
    std::string rstp_data_type = flow_unit->GetFlowInputDataType();
    LOG_INFO("rstp input_data_type: %s\n", (char *)rstp_data_type.c_str());
    std::string input_data_type;
    if (rstp_data_type == RTSP_DATA_TYPE_MPEGTS) {
      input_data_type.append(rstp_data_type);
    } else {
      int video_enc_index = GetFlowIndex(pipe_index, StreamType::VIDEO_ENCODER);
      int audio_enc_index = GetFlowIndex(pipe_index, StreamType::AUDIO_ENCODER);
      if (audio_enc_index >= 0) {
        auto &audio_enc_unit = flows[audio_enc_index];
        std::string audio_enc_type = audio_enc_unit->GetOutputDataType();
        input_data_type.append(audio_enc_type);
        input_data_type.append(",");
      }
      if (video_enc_index >= 0) {
        auto &video_enc_unit = flows[video_enc_index];
        std::string video_enc_type = video_enc_unit->GetOutputDataType();
        input_data_type.append(video_enc_type);
      }
    }
    if (!input_data_type.empty())
      flow_unit->SetInputDataType(input_data_type);
  }
}

void FlowParser::SyncSmartEncorde() {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto &flows = GetFlowUnits(pipe_index);
    int flow_index = GetFlowIndex(pipe_index, StreamType::VIDEO_ENCODER);
    if (flow_index < 0)
      break;
    auto &flow_unit = flows[flow_index];
    std::string smart = flow_unit->GetSmart();
    if (!smart.compare(DB_SMART_VALUE_CLOSE)) {
      LOG_INFO("smart is close, disable target and min bps\n");
      flow_unit->SetBitRate(DB_SMART_DISABLE);
      flow_unit->SetMinRate(DB_SMART_DISABLE);
    }
  }
}

int FlowParser::JsConfigRead(const char *path) {
  std::ifstream config_file(path);
  if (!config_file) {
    LOG_ERROR("JsConfigRead %s does not exist.\n", path);
    return -1;
  }
  config_file >> config_js;
  config_file.close();
  return 0;
}

int FlowParser::JsConfigParse() {
  for (int pipe_id = 0; pipe_id < config_js.size(); pipe_id++) {
    flow_unit_v flow_units;
    // Found Pipe
    std::string node_str = NODE_PIPE_ID;
    node_str.append(std::to_string(pipe_id));
    if (config_js.dump().find(node_str) == std::string::npos) {
      LOG_WARN("config_js no found %s\n", node_str.c_str());
      continue;
    }
    nlohmann::json pipe_js = config_js.at(node_str);
    std::string pipe_str = pipe_js.dump();

    for (int flow_id = 0; flow_id < pipe_js.size(); flow_id++) {
      std::string value;
      auto flow_unit = std::make_shared<FlowUnit>();
      // Found Flow
      node_str = NODE_FLOW_ID;
      node_str.append(std::to_string(flow_id));
      if (pipe_js.dump().find(node_str) == std::string::npos) {
        LOG_WARN("pipe_js no found %s\n", node_str.c_str());
        continue;
      }
      nlohmann::json flow_js = pipe_js.at(node_str);
      std::string flow_str = flow_js.dump();
      // Found Flow Index
      if (flow_str.find(NODE_FLOW_INDEX) != std::string::npos) {
        nlohmann::json flow_index_js = flow_js.at(NODE_FLOW_INDEX);
        for (auto it = flow_index_js.begin(); it != flow_index_js.end(); ++it) {
          if (it.value().is_string())
            value = it.value().get<std::string>();
          else
            value = std::to_string(it.value().get<int>());
          flow_unit->flow_index_props_.emplace_back(it.key(), value);
        }
      }
      // Found Flow Name
      if (flow_str.find(NODE_FLOW_NAME) != std::string::npos) {
        nlohmann::json flow_name_js = flow_js.at(NODE_FLOW_NAME);
        flow_unit->flow_name_ = flow_name_js.get<std::string>();
        if (flow_name_js == "muxer_flow") {
          LOG_ERROR("muxer_flow is temporarily not supported\n");
          continue;
        }
      }
      // Found Flow Param
      if (flow_str.find(NODE_FLOW_PARAM) != std::string::npos) {
        nlohmann::json flow_parma_js = flow_js.at(NODE_FLOW_PARAM);
        for (auto it = flow_parma_js.begin(); it != flow_parma_js.end(); ++it) {
          if (it.value().is_string())
            value = it.value().get<std::string>();
          else
            value = std::to_string(it.value().get<int>());
          flow_unit->flow_param_props_.emplace_back(it.key(), value);
        }
      }
      // Found Stream Param
      if (flow_str.find(NODE_STREAM_PARAM) != std::string::npos) {
        nlohmann::json flow_stream_js = flow_js.at(NODE_STREAM_PARAM);
        for (auto it = flow_stream_js.begin(); it != flow_stream_js.end();
             ++it) {
          if (it.value().is_string())
            value = it.value().get<std::string>();
          else
            value = std::to_string(it.value().get<int>());
          flow_unit->stream_param_props_.emplace_back(it.key(), value);
        }
      }
      flow_units.emplace_back(flow_unit);
    }
    flow_pipes_.emplace_back(flow_units);
  }
  return 0;
}

int FlowParser::JsConfigWrite(const char *path) {
  std::ofstream Save_file(path);
  if (!Save_file) {
    LOG_ERROR("JsConfigWrite %s create failed.\n", path);
    return -1;
  }
  Save_file << std::setw(4) << config_js << std::endl;
  Save_file.close();
  Save_file.flush();
  return 0;
}

int FlowParser::JsConfigReBuild(std::vector<std::shared_ptr<FlowPipe>> pipes) {
  config_js.clear();
  for (int pipe_id = 0; pipe_id < pipes.size(); pipe_id++) {
    nlohmann::json pipe_js;
    std::string pipe_node_str = NODE_PIPE_ID;
    pipe_node_str.append(std::to_string(pipe_id));
    auto flow_units = pipes[pipe_id]->GetFlowUnits();
    for (int flow_id = 0; flow_id < flow_units.size(); flow_id++) {
      nlohmann::json array_js;
      nlohmann::json flow_js;
      std::string flow_node_str = NODE_FLOW_ID;
      flow_node_str.append(std::to_string(flow_id));
      auto flow_unit = flow_units[flow_id];

      // Get Flow Name
      flow_js[NODE_FLOW_NAME] = flow_unit->GetFlowName();
      // Get Flow Index
      array_js.clear();
      for (int index = 0; index < flow_unit->flow_index_props_.size();
           index++) {
        std::string name = flow_unit->flow_index_props_[index].first;
        std::string value = flow_unit->flow_index_props_[index].second;
        array_js[name.c_str()] = value;
      }
      flow_js[NODE_FLOW_INDEX] = array_js;
      // Get Flow Param
      array_js.clear();
      for (int index = 0; index < flow_unit->flow_param_props_.size();
           index++) {
        std::string name = flow_unit->flow_param_props_[index].first;
        std::string value = flow_unit->flow_param_props_[index].second;
        array_js[name.c_str()] = value;
      }
      flow_js[NODE_FLOW_PARAM] = array_js;
      // Get Stream Param
      array_js.clear();
      for (int index = 0; index < flow_unit->stream_param_props_.size();
           index++) {
        std::string name = flow_unit->stream_param_props_[index].first;
        std::string value = flow_unit->stream_param_props_[index].second;
        array_js[name.c_str()] = value;
      }
      flow_js[NODE_STREAM_PARAM] = array_js;
      pipe_js[flow_node_str] = flow_js;
    }
    config_js[pipe_node_str] = pipe_js;
  }
  return 0;
}

flow_unit_v &FlowParser::GetFlowUnits(int pipe_index) {
  if (pipe_index >= flow_pipes_.size()) {
    LOG_ERROR("pipe index %d over the actual size\n", pipe_index);
  }
  return flow_pipes_[pipe_index];
}

int FlowParser::GetStreamById(int id, StreamType type) {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = GetFlowUnits(pipe_index);
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

int FlowParser::GetStreamById(int id, StreamType type, std::string name) {
  int pipe_num = flow_pipes_.size();
  for (int pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto &flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      auto stream_name = flow_unit->GetStreamName();
      if (stream_type == type && stream_name == name) {
        if (id == flow_unit->GetStreamId()) {
          return pipe_index;
        }
      }
    }
  }
  return -1;
}

int FlowParser::GetPipeIndex(int id, StreamType type) {
  int cnt = 0;
  int pipe_index = GetStreamById(id, type);
  if (pipe_index >= 0) {
    LOG_INFO("find type %d in pipe_index %d\n", type, pipe_index);
    return pipe_index;
  }
  int pipe_num = flow_pipes_.size();
  for (pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      if (stream_type == type) {
        if (id == cnt)
          return pipe_index;
        else
          cnt++;
      }
    }
  }
  return -1;
}

int FlowParser::GetPipeIndex(int id, StreamType type, std::string name) {
  int cnt = 0;
  int pipe_index = GetStreamById(id, type, name);
  if (pipe_index >= 0) {
    LOG_INFO("find type %d in pipe_index %d\n", type, pipe_index);
    return pipe_index;
  }
  int pipe_num = flow_pipes_.size();
  for (pipe_index = 0; pipe_index < pipe_num; pipe_index++) {
    auto flows = GetFlowUnits(pipe_index);
    for (auto iter : flows) {
      auto flow_unit = iter;
      StreamType stream_type = flow_unit->GetStreamType();
      auto stream_name = flow_unit->GetStreamName();
      if (stream_type == type && stream_name == name) {
        if (id == cnt)
          return pipe_index;
        else
          cnt++;
      }
    }
  }
  return -1;
}

int FlowParser::GetFlowIndex(int pipe_index, StreamType type, int index) {
  int flow_index = 0;
  auto flows = GetFlowUnits(pipe_index);
  for (auto iter : flows) {
    auto flow_unit = iter;
    StreamType stream_type = flow_unit->GetStreamType();
    if (stream_type == type) {
      if (index == 0)
        return flow_index;
      else
        index--;
    } else {
      flow_index++;
    }
  }
  return -1;
}

int FlowParser::GetFlowIndex(int pipe_index, StreamType type,
                             std::string name) {
  int pipe_num = flow_pipes_.size();
  int flow_index = -1;
  auto &flows = GetFlowUnits(pipe_index);
  for (auto &iter : flows) {
    flow_index++;
    auto &flow_unit = iter;
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    if (name == stream_name && stream_type == type) {
      return flow_index;
    }
  }
  return -1;
}

int FlowParser::GetMuxerFlowIndex(int pipe_index, bool is_rtmp) {
  int pipe_num = flow_pipes_.size();
  int flow_index = -1;
  auto &flows = GetFlowUnits(pipe_index);
  for (auto &iter : flows) {
    flow_index++;
    auto &flow_unit = iter;
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    auto file_prefix = flow_unit->GetFilePreFix();
    if (file_prefix.empty() && !is_rtmp)
      continue;
    if (stream_type == StreamType::MUXER) {
      return flow_index;
    }
  }
  return -1;
}

int FlowParser::GetFaceCaptureFlowIndex(int pipe_index) {
  int pipe_num = flow_pipes_.size();
  int flow_index = -1;
  auto &flows = GetFlowUnits(pipe_index);
  for (auto &iter : flows) {
    flow_index++;
    auto &flow_unit = iter;
    auto stream_type = flow_unit->GetStreamType();
    auto file_prefix = flow_unit->GetFilePreFix();
    if (file_prefix.empty())
      continue;
    if (stream_type == StreamType::FILTER) {
      return flow_index;
    }
  }
  return -1;
}

int FlowParser::GetNextFlowIndex(int pipe_index, int index, StreamType type) {
  int pipe_num = flow_pipes_.size();
  int flow_index = -1;
  auto &flows = GetFlowUnits(pipe_index);
  for (auto &iter : flows) {
    flow_index++;
    auto &flow_unit = iter;
    auto stream_type = flow_unit->GetStreamType();
    auto stream_name = flow_unit->GetStreamName();
    if (stream_type == type && flow_index > index) {
      return flow_index;
    }
  }
  return -1;
}

int FlowParser::GetFlowIndexByFlowIndexName(int pipe_index, std::string name) {
  int pipe_num = flow_pipes_.size();
  int flow_index = -1;
  auto &flows = GetFlowUnits(pipe_index);
  for (auto &iter : flows) {
    flow_index++;
    auto &flow_unit = iter;
    auto flow_index_name = flow_unit->GetFlowIndexName();
    if (flow_index_name == name) {
      return flow_index;
    }
  }
  return -1;
}

int FlowParser::GetFlowIndexByUpFlowIndexName(int pipe_index,
                                              std::string name) {
  int pipe_num = flow_pipes_.size();
  int flow_index = -1;
  auto &flows = GetFlowUnits(pipe_index);
  for (auto &iter : flows) {
    flow_index++;
    auto &flow_unit = iter;
    auto upflow_index_name = flow_unit->GetUpFlowIndexName();
    if (upflow_index_name == name) {
      return flow_index;
    }
  }
  return -1;
}

} // namespace mediaserver
} // namespace rockchip

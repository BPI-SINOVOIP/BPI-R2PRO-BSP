// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_unit.h"
#include "flow_db_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_unit.cpp"

namespace rockchip {
namespace mediaserver {

void FlowUnit::DumpProps() {
  LOG_INFO("DumpProps:\n");
  LOG_INFO("\t flow index name: %s \n", flow_name_.c_str());
  LOG_INFO("\t flow_index_props_:\n");
  for (auto it : flow_index_props_)
    LOG_INFO("\t\t prop name %s, value %s\n", it.first.c_str(),
             it.second.c_str());
  LOG_INFO("\t flow_param_props_:\n");
  for (auto it : flow_param_props_)
    LOG_INFO("\t\t prop name %s, value %s\n", it.first.c_str(),
             it.second.c_str());
  LOG_INFO("\t stream_param_props_:\n");
  for (auto it : stream_param_props_)
    LOG_INFO("\t\t prop name %s, value %s\n", it.first.c_str(),
             it.second.c_str());
}

std::string FlowUnit::GetPropByName(props_v props, std::string name) {
  for (auto &it : props) {
    if (name == it.first)
      return it.second.c_str();
  }
  return "";
}

int FlowUnit::SetPropByName(props_v &props, std::string name,
                            std::string value) {
  for (auto &it : props) {
    if (name == it.first) {
      it.second = value;
      return 0;
    }
  }
  props.emplace_back(name, value);
  return 0;
}

FlowType FlowUnit::GetFlowTypeByString(std::string type) {
  std::map<std::string, FlowType> type_map = {{"source", FlowType::SOURCE},
                                              {"io", FlowType::IO},
                                              {"sink", FlowType::SINK}};
  auto it = type_map.find(type);
  if (it != type_map.end())
    return it->second;
  return FlowType::UNKNOW;
}

std::string FlowUnit::GetStringByFlowType(FlowType type) {
  std::map<FlowType, std::string> type_map = {{FlowType::SOURCE, "source"},
                                              {FlowType::IO, "io"},
                                              {FlowType::SINK, "sink"}};
  auto it = type_map.find(type);
  if (it != type_map.end())
    return it->second;
  return "unknow";
}

StreamType FlowUnit::GetStreamTypeByString(std::string type) {
  std::map<std::string, StreamType> type_map = {
      {"file", StreamType::FILE},
      {"camera", StreamType::CAMERA},
      {"audio", StreamType::AUDIO},
      {"video_enc", StreamType::VIDEO_ENCODER},
      {"video_dec", StreamType::VIDEO_DECODER},
      {"jpeg_through_guard", StreamType::JPEG_THROUGH_GUARD},
      {"jpeg_enc", StreamType::JPEG_ENCODER},
      {"jpeg_dec", StreamType::JPEG_DECODER},
      {"audio_enc", StreamType::AUDIO_ENCODER},
      {"rtsp", StreamType::RTSP},
      {"link", StreamType::LINK},
      {"filter", StreamType::FILTER},
      {"muxer", StreamType::MUXER},
      {"move_detec", StreamType::MOVE_DETEC},
      {"general", StreamType::GENERAL}};
  auto it = type_map.find(type);
  if (it != type_map.end())
    return it->second;
  return StreamType::UNKNOW;
}

std::string FlowUnit::GetStringByStreamType(StreamType type) {
  std::map<StreamType, std::string> type_map = {
      {StreamType::FILE, "file"},
      {StreamType::CAMERA, "camera"},
      {StreamType::AUDIO, "audio"},
      {StreamType::VIDEO_ENCODER, "video_enc"},
      {StreamType::VIDEO_DECODER, "video_dec"},
      {StreamType::JPEG_THROUGH_GUARD, "jpeg_through_guard"},
      {StreamType::JPEG_ENCODER, "jpeg_enc"},
      {StreamType::JPEG_DECODER, "jpeg_dec"},
      {StreamType::AUDIO_ENCODER, "audio_enc"},
      {StreamType::RTSP, "rtsp"},
      {StreamType::LINK, "link"},
      {StreamType::FILTER, "filter"},
      {StreamType::MUXER, "muxer"},
      {StreamType::MOVE_DETEC, "move_detec"},
      {StreamType::GENERAL, "general"}};
  auto it = type_map.find(type);
  if (it != type_map.end())
    return it->second;
  return "unknow";
}

std::string FlowUnit::FlowParamMapToStr() {
  std::string params = "";
  for (auto it = flow_param_props_.begin(); it != flow_param_props_.end(); ++it)
    PARAM_STRING_APPEND2(params, it->first, it->second);
  return params;
}

std::string FlowUnit::StreamParamMapToStr() {
  std::string params = "";
  for (auto it = stream_param_props_.begin(); it != stream_param_props_.end();
       ++it)
    PARAM_STRING_APPEND2(params, it->first, it->second);
  return params;
}

FlowType FlowUnit::GetFlowType() {
  std::string type =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_FLOW_TYPE);
  return GetFlowTypeByString(type);
}

int FlowUnit::GetStreamId() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_STREAM_ID);
  if (indexs.empty()) {
    return -1;
  }
  int inIndexs = atoi(indexs.c_str());
  return inIndexs;
}

StreamType FlowUnit::GetStreamType() {
  std::string type =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_STREAM_TYPE);
  if (type.empty()) {
    LOG_WARN("flow %s no found node flow stream type\n", flow_name_.c_str());
    return StreamType::UNKNOW;
  }
  return GetStreamTypeByString(type);
}

std::string FlowUnit::GetFlowIndexName() {
  return GetPropByName(flow_index_props_, NODE_FLOW_INDEX_FLOW_INDEX_NAME);
}

std::string FlowUnit::GetUpFlowIndexName() {
  return GetPropByName(flow_index_props_, NODE_FLOW_INDEX_UPFLOW_INDEX_NAME);
}

int FlowUnit::GetOutSlotIndex() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_OUT_SLOT_INDEX);
  if (indexs.empty()) {
    LOG_WARN("flow %s no found node flow out slot index\n", flow_name_.c_str());
    return -1;
  }
  int outIndexs = atoi(indexs.c_str());
  return outIndexs;
}
int FlowUnit::GetInSlotIndexOfDown() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_IN_SLOT_INDEX);
  if (indexs.empty()) {
    LOG_WARN("flow %s no found node flow out slot index\n", flow_name_.c_str());
    return -1;
  }
  int inIndexs = atoi(indexs.c_str());
  return inIndexs;
}

int FlowUnit::GetOpenPipeId() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_OPEN_PIEP_ID);
  if (indexs.empty()) {
    return -1;
  }
  int inIndexs = atoi(indexs.c_str());
  return inIndexs;
}

int FlowUnit::GetOpenFlowId() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_OPEN_FLOW_ID);
  if (indexs.empty()) {
    return -1;
  }
  int inIndexs = atoi(indexs.c_str());
  return inIndexs;
}

int FlowUnit::GetFixResolution() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_FIX_RESO);
  if (indexs.empty()) {
    return 0;
  }
  return atoi(indexs.c_str());
}

int FlowUnit::GetAjustResolution() {
  std::string indexs =
      GetPropByName(flow_index_props_, NODE_FLOW_INDEX_AJUST_RESO);
  if (indexs.empty()) {
    return 0;
  }
  return atoi(indexs.c_str());
}

std::string FlowUnit::GetProductKey() {
  return GetPropByName(flow_index_props_, "product_key");
}

std::string FlowUnit::GetProductSecret() {
  return GetPropByName(flow_index_props_, "product_secret");
}

std::string FlowUnit::GetDeviceName() {
  return GetPropByName(flow_index_props_, "device_name");
}

std::string FlowUnit::GetDeviceSecret() {
  return GetPropByName(flow_index_props_, "device_secret");
}

std::string FlowUnit::GetPID() {
  return GetPropByName(flow_index_props_, "pid");
}

std::string FlowUnit::GetUUID() {
  return GetPropByName(flow_index_props_, "uuid");
}

std::string FlowUnit::GetAuthkey() {
  return GetPropByName(flow_index_props_, "authkey");
}

std::string FlowUnit::GetFlowName() { return flow_name_; }

std::string FlowUnit::GetStreamName() {
  std::string type =
      GetPropByName(flow_param_props_, NODE_FLOW_PARAM_STREAM_NAME);
  if (type.empty()) {
    // to avoid frequent calls
    // LOG_WARN("flow %s no found node stream name\n", flow_name_.c_str());
  }
  return type;
}

std::string FlowUnit::GetFlowInputDataType() {
  std::string type =
      GetPropByName(flow_param_props_, NODE_STREAM_PARAM_INPUT_DATA_TYPE);
  if (type.empty()) {
    LOG_WARN("flow %s no found node input data type\n", flow_name_.c_str());
  }
  return type;
}

std::string FlowUnit::GetStreamRockxModel() {
  std::string type =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_ROCKX_MODEL);
  if (type.empty()) {
    LOG_WARN("flow %s no found node stream rockx model name\n",
             flow_name_.c_str());
  }
  return type;
}

std::string FlowUnit::GetKeyEventPath() {
  std::string path =
      GetPropByName(flow_param_props_, NODE_FLOW_PARAM_KEY_EVENT_PATH);
  if (path.empty()) {
    LOG_WARN("flow %s no found node key event path\n", flow_name_.c_str());
  }
  return path;
}

int FlowUnit::GetKeyEventCode() {
  std::string code =
      GetPropByName(flow_param_props_, NODE_FLOW_PARAM_KEY_EVENT_CODE);
  if (code.empty()) {
    LOG_WARN("flow %s no found node key event code\n", flow_name_.c_str());
  }
  return atoi(code.c_str());
}

std::string FlowUnit::GetDevice() {
  std::string type =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_DEVICE);
  if (type.empty()) {
    LOG_WARN("flow %s no found node output data type\n", flow_name_.c_str());
  }
  return type;
}

std::string FlowUnit::GetOutputDataType() {
  std::string type =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE);
  if (type.empty()) {
    LOG_WARN("flow %s no found node output data type\n", flow_name_.c_str());
  }
  return type;
}

std::string FlowUnit::GetInputDataType() {
  std::string type =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_INPUT_DATA_TYPE);
  if (type.empty()) {
    LOG_WARN("flow %s no found node input data type\n", flow_name_.c_str());
  }
  return type;
}

std::string FlowUnit::GetNeedHwDraw() {
  std::string type =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_NEED_HW_DRAW);
  if (type.empty()) {
    return "0";
  }
  return type;
}

std::string FlowUnit::GetGop() {
  std::string value = GetPropByName(stream_param_props_, NODE_STREAM_PARAM_GOP);
  if (value.empty()) {
    LOG_WARN("flow %s no found node gop\n", flow_name_.c_str());
  }
  return value;
}

std::string FlowUnit::GetFrameRate() {
  std::string value =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_FRAMERATE);
  if (value.empty()) {
    LOG_WARN("flow %s no found node framerate\n", flow_name_.c_str());
  }
  return value;
}

std::string FlowUnit::GetRoiRegions() {
  std::string value =
      GetPropByName(stream_param_props_, NODE_STREAM_ROI_REGIONS);
  if (value.empty()) {
    LOG_WARN("flow %s no found node roi region\n", flow_name_.c_str());
  }
  return value;
}

std::string FlowUnit::GetRIRegions() {
  std::string value = GetPropByName(extra_props_, NODE_STREAM_RI_REGIONS);
  if (value.empty()) {
    LOG_WARN("flow %s no found node ri region\n", flow_name_.c_str());
  }
  return value;
}

std::string FlowUnit::GetSmart() {
  std::string value =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_SMART);
  if (value.empty()) {
    LOG_WARN("flow %s no found node smart\n", flow_name_.c_str());
  }
  return value;
}

void FlowUnit::SetGop(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_GOP, value);
}

void FlowUnit::SetBitRate(std::string value) {
  // TODO
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_BITRATE, value);
}

void FlowUnit::SetMinRate(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_BITRATE_MIN, value);
}

void FlowUnit::SetMaxRate(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_BITRATE_MAX, value);
}

void FlowUnit::SetStreamSmooth(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_STREAMSMOOTH, value);
}

void FlowUnit::SetFrameRate(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_FRAMERATE, value);
}

void FlowUnit::SetFrameRateIn(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_FRAMERATE_IN, value);
}

void FlowUnit::SetResolution(std::string width, std::string height) {
  int w = atoi(width.c_str());
  int h = atoi(height.c_str());
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_WIDTH, width);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_HEIGHT, height);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_VIR_WIDTH, width);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_VIR_HEIGHT, height);
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_WIDTH, width);
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_HEIGHT, height);
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_VIR_WIDTH, width);
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_VIR_HEIGHT, height);
}

void FlowUnit::SetMoveDetecResolution(std::string width, std::string height) {
  int w = atoi(width.c_str());
  int h = atoi(height.c_str());
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_MD_ORI_WIDTH, width);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_MD_ORI_HEIGHT, height);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_MD_DS_WIDTH, width);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_MD_DS_HEIGHT, height);
}

void FlowUnit::GetResolution(std::string &width, std::string &height) {
  width = GetPropByName(stream_param_props_, NODE_STREAM_PARAM_WIDTH);
  height = GetPropByName(stream_param_props_, NODE_STREAM_PARAM_HEIGHT);
}

void FlowUnit::SetResolution2(std::string width, std::string height) {
  int w = atoi(width.c_str());
  int h = atoi(height.c_str());
  int vir_w = UPALIGNTO16(w);
  int vir_h = UPALIGNTO16(h);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_WIDTH, width);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_HEIGHT, height);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_VIR_WIDTH,
                std::to_string(vir_w));
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_VIR_HEIGHT,
                std::to_string(vir_h));
}

void FlowUnit::SetImageQuality(std::string value) {
  bool enc_h265 = false;
  int level = 0;
  int *qp_table;

  if (value == DB_VALUE_LEVEL_HIGHEST)
    level = 0;
  else if (value == DB_VALUE_LEVEL_HIGHER)
    level = 1;
  else if (value == DB_VALUE_LEVEL_MEDIUM)
    level = 2;
  else if (value == DB_VALUE_LEVEL_LOWER)
    level = 3;
  else
    level = 4;

  std::string output_type =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE);
  if (output_type.find("h265") != std::string::npos) {
    qp_table = h265_qp_table[level];
  } else {
    qp_table = h264_qp_table[level];
  }

  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_IMAGE_QUALITY_INIT,
                std::to_string(qp_table[0]));
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_IMAGE_QUALITY_MIN,
                std::to_string(qp_table[1]));
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_IMAGE_QUALITY_MAX,
                std::to_string(qp_table[2]));
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_IMAGE_QUALITY_STEP,
                std::to_string(qp_table[3]));
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_IMAGE_QUALITY_MIN_I_QP,
                std::to_string(qp_table[4]));
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_IMAGE_QUALITY_MAX_I_QP,
                std::to_string(qp_table[5]));
}

void FlowUnit::SetOutputDataType(std::string value) {
  if (value == DB_ENCORDE_TYPE_H264) {
    SetPropByName(flow_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                  VIDEO_H264);
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                  VIDEO_H264);
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_CODECTYPE,
                  std::to_string(CODEC_TYPE_H264));
  } else if (value == DB_ENCORDE_TYPE_H265) {
    SetPropByName(flow_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                  VIDEO_H265);
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                  VIDEO_H265);
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_CODECTYPE,
                  std::to_string(CODEC_TYPE_H265));
  } else if (value == DB_ENCORDE_TYPE_MJPEG) {
    SetPropByName(flow_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                  IMAGE_JPEG);
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                  IMAGE_JPEG);
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_CODECTYPE,
                  std::to_string(CODEC_TYPE_JPEG));
  } else if (value == DB_ENCORDE_TYPE_MPEG4) {
    // TODO
  }
}

void FlowUnit::SetInputDataType(std::string value) {
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_INPUT_DATA_TYPE, value);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_INPUT_DATA_TYPE, value);
}

void FlowUnit::SetRCMode(std::string value) {
  if (value == DB_RC_MODE_CBR)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_MODE, KEY_CBR);
  else if (value == DB_RC_MODE_VBR)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_MODE, KEY_VBR);
  else
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_MODE, KEY_CBR);
}

void FlowUnit::SetRCQuality(std::string value) {
  if (value == DB_VALUE_LEVEL_HIGHEST)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY,
                  KEY_HIGHEST);
  else if (value == DB_VALUE_LEVEL_HIGHER)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY,
                  KEY_HIGHER);
  else if (value == DB_VALUE_LEVEL_HIGH)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY, KEY_HIGH);
  else if (value == DB_VALUE_LEVEL_MEDIUM)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY,
                  KEY_MEDIUM);
  else if (value == DB_VALUE_LEVEL_LOW)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY, KEY_LOW);
  else if (value == DB_VALUE_LEVEL_LOWER)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY, KEY_LOWER);
  else
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RC_QUALITY,
                  KEY_LOWEST);
}

void FlowUnit::SetH264Profile(std::string value) {
  if (value == DB_VALUE_H264_BASELINE)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_H264_PROFILE, "66");
  else if (value == DB_VALUE_H264_MAIN)
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_H264_PROFILE, "77");
  else
    SetPropByName(stream_param_props_, NODE_STREAM_PARAM_H264_PROFILE, "100");
}

void FlowUnit::SetH264Profile2(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_H264_PROFILE, value);
}

void FlowUnit::SetSmart(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_SMART, value);
}

void FlowUnit::SetSVC(std::string value) {
  // TODO
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_SVC, value);
}

void FlowUnit::SetVideoType(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_VIDEO_TYPE, value);
}

void FlowUnit::SetRoiRegions(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_ROI_REGIONS, value);
}

void FlowUnit::SetRIRegions(std::string value) {
  SetPropByName(extra_props_, NODE_STREAM_RI_REGIONS, value);
}

void FlowUnit::SetEnable(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_ENABLE, value);
}

void FlowUnit::SetRegEnable(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_REG_ENABLE, value);
}

void FlowUnit::SetInterval(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_INTERVAL, value);
}

void FlowUnit::SetDuration(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_DURATION, value);
}

void FlowUnit::SetPercentage(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_PERCENTAGE, value);
}

void FlowUnit::SetSensitivity(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_SENSITIVITY, value);
}

void FlowUnit::SetFullRange(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_FULL_RANGE, value);
}

void FlowUnit::SetMoveDetectRoiCnt(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_MD_ROI_CNT, value);
}

void FlowUnit::SetMoveDetectRoiRect(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_MD_ROI_RECT, value);
}

void FlowUnit::SetPortNum(std::string value) {
  SetPropByName(flow_param_props_, NODE_FLOW_PARAM_PORT_NUM, value);
}

std::string FlowUnit::GetChannel() {
  std::string channel =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_CHANNEL);
  if (channel.empty()) {
    LOG_WARN("flow %s no found node channel\n", flow_name_.c_str());
  }
  return channel;
}

std::string FlowUnit::GetSampleFormat() {
  std::string sample_format =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_SAMPLE_FORMAT);
  if (sample_format.empty()) {
    LOG_WARN("flow %s no found node sample_format\n", flow_name_.c_str());
  }
  return sample_format;
}

std::string FlowUnit::GetSampleRate() {
  std::string sample_rate =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_SAMPLE_RATE);
  if (sample_rate.empty()) {
    LOG_WARN("flow %s no found node sample_rate\n", flow_name_.c_str());
  }
  return sample_rate;
}

void FlowUnit::SetChannel(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_CHANNEL, value);
}

void FlowUnit::SetSampleRate(std::string value) {
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_SAMPLE_RATE, value);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_SAMPLE_RATE, value);
}

void FlowUnit::SetSampleFormat(std::string value) {
  std::string format;
  if (!value.compare("mp3")) {
    format = "audio:pcm_fltp";
  } else if (!value.compare("G711A")) {
    format = "audio:pcm_s16";
  } else if (!value.compare("G711U")) {
    format = "audio:pcm_s16";
  } else {
    LOG_ERROR("Not Support\n");
    return;
  }
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_SAMPLE_FORMAT, format);
}

void FlowUnit::SetVolume(std::string value) {
  // TODO
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_VOLUME, value);
}

void FlowUnit::SetAudioSource(std::string value) {
  // TODO
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_AUDIO_SOURCE, value);
}

void FlowUnit::SetEncodeType(std::string value) {
  std::string output_data_type;
  std::string input_data_type;
  if (!value.compare("mp3")) {
    output_data_type = "audio:mp3";
    input_data_type = "audio:pcm_fltp";
  } else if (!value.compare("G711A")) {
    output_data_type = "audio:g711a";
    input_data_type = "audio:pcm_s16";
  } else if (!value.compare("G711U")) {
    output_data_type = "audio:g711U";
    input_data_type = "audio:pcm_s16";
  } else {
    LOG_ERROR("Not Support\n");
    return;
  }
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                output_data_type);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_OUTPUT_DATA_TYPE,
                output_data_type);
  SetPropByName(flow_param_props_, NODE_STREAM_PARAM_INPUT_DATA_TYPE,
                input_data_type);
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_INPUT_DATA_TYPE,
                input_data_type);
}

void FlowUnit::SetANS(std::string value) {
  // TODO
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_ANS, value);
}

std::string FlowUnit::GetRect() {
  std::string rect = GetPropByName(stream_param_props_, NODE_STREAM_PARAM_RECT);
  if (rect.empty()) {
    LOG_WARN("flow %s no found node rect\n", flow_name_.c_str());
  }
  return rect;
}

void FlowUnit::SetRect(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_RECT, value);
}

std::string FlowUnit::GetDetectRect() {
  std::string rect =
      GetPropByName(stream_param_props_, NODE_STREAM_PARAM_DETECT_RECT);
  if (rect.empty()) {
    LOG_WARN("flow %s no found node rect\n", flow_name_.c_str());
  }
  return rect;
}
void FlowUnit::SetDetectRect(std::string value) {
  SetPropByName(stream_param_props_, NODE_STREAM_PARAM_DETECT_RECT, value);
}

void FlowUnit::SetFilePath(std::string value) {
  SetPropByName(flow_param_props_, NODE_FLOW_PARAM_FILE_PATH, value);
}

void FlowUnit::SetFilePathInStreamParam(std::string value) {
  SetPropByName(stream_param_props_, NODE_FLOW_PARAM_FILE_PATH, value);
}

std::string FlowUnit::GetFilePath() {
  return GetPropByName(flow_param_props_, NODE_FLOW_PARAM_FILE_PATH);
}

void FlowUnit::SetFilePreFix(std::string value) {
  SetPropByName(flow_param_props_, NODE_FLOW_PARAM_FILE_PREFIX, value);
}

std::string FlowUnit::GetFilePreFix() {
  std::string file_prefix =
      GetPropByName(flow_param_props_, NODE_FLOW_PARAM_FILE_PREFIX);
  if (file_prefix.empty())
    file_prefix =
        GetPropByName(stream_param_props_, NODE_FLOW_PARAM_FILE_PREFIX);
  return file_prefix;
}

} // namespace mediaserver
} // namespace rockchip

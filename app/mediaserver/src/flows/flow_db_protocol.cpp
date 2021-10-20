// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_db_protocol.h"
#include <bitset>
#include <sstream>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_db_protocol.cpp"

namespace rockchip {
namespace mediaserver {

#if 0
int h264_qp_table[6][6] = { // qp_init,qp_min,qp_max,qp_step,min_i_qp,max_i_qp
    {4, 0, 8, 2, 0, 0},    {8, 0, 16, 4, 0, 0},   {16, 8, 24, 4, 0, 0},
    {24, 16, 32, 4, 0, 0}, {32, 24, 40, 4, 0, 0}, {40, 32, 51, 4, 0, 0}};
int h265_qp_table[6][6] = { // qp_init,qp_min,qp_max,qp_step,min_i_qp,max_i_qp
    {-1, 0, 8, 0, 2, 6},     {-1, 0, 16, 0, 8, 12},   {-1, 8, 24, 0, 16, 24},
    {-1, 16, 32, 0, 24, 32}, {-1, 24, 40, 0, 30, 40}, {-1, 32, 51, 0, 38, 51}};
#else
int h264_qp_table[6][6] = { // qp_init,qp_min,qp_max,qp_step,min_i_qp,max_i_qp
    {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48},
    {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48}};
int h265_qp_table[6][6] = { // qp_init,qp_min,qp_max,qp_step,min_i_qp,max_i_qp
    {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48},
    {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48}, {24, 6, 48, 4, 6, 48}};
#endif

#ifdef ENABLE_DBUS

static void AudioProcess(std::string key, std::string value) {
  if (key == DB_AUDIO_SAMPLE_RATE) {
    int sample_rate = atoi(value.c_str());
    SetSampleRate(sample_rate);
  } else if (key == DB_AUDIO_VOLUME) {
    int volume = atoi(value.c_str());
    SetVolume(volume);
  } else if (key == DB_AUDIO_BIT_RATE) {
    int bit_rate = atoi(value.c_str());
    SetBitRate(bit_rate);
  } else if (key == DB_AUDIO_SOURCE) {
    const char *audio_source = value.c_str();
    SetAudioSource(audio_source);
  } else if (key == DB_AUDIO_ENCODE_TYPE) {
    const char *encode_type = value.c_str();
    SetAudioEncodeType(encode_type);
  } else if (key == DB_AUDIO_ANS) {
    const char *ANS = value.c_str();
    SetANS(ANS);
  }
}

static void VideoProcess(int id, std::string key, std::string value) {
  if (key == DB_VIDEO_GOP) {
    int gop = atoi(value.c_str());
    SetGop(id, gop);
  } else if (key == DB_VIDEO_MAX_RATE) {
    int max_rate = atoi(value.c_str());
    SetMaxRate(id, max_rate);
  } else if (key == DB_VIDEO_STREAM_SMOOTH) {
    int stream_smooth = atoi(value.c_str());
    SetStreamSmooth(id, stream_smooth);
  } else if (key == DB_VIDEO_FRAME_RATE) {
    if (value.find("/") != std::string::npos) {
      int pos = value.find("/");
      int numerator = atoi(value.substr(0, pos).c_str());
      int denominator = atoi(value.substr(pos + 1, value.size() - 1).c_str());
      SetFrameRate(id, numerator, denominator);
    }
  } else if (key == DB_VIDEO_RESOLUTION) {
    const char *resolution = value.c_str();
    SetResolution(id, resolution);
  } else if (key == DB_VIDEO_RC_QUALITY) {
    const char *rc_quality = value.c_str();
    SetRCQuality(id, rc_quality);
  } else if (key == DB_VIDEO_OUTPUT_DATA_TYPE) {
    const char *output_data_type = value.c_str();
    SetVideoEncodeType(id, output_data_type);
  } else if (key == DB_VIDEO_RC_MODE) {
    for (int i = 0; i < value.size(); i++)
      value[i] = tolower(value[i]);
    const char *rc_mode = value.c_str();
    SetRCMode(id, rc_mode);
  } else if (key == DB_VIDEO_H264_PROFILE) {
    const char *h264_profile = value.c_str();
    SetH264Profile(id, h264_profile);
  } else if (key == DB_VIDEO_SMART) {
    const char *smart = value.c_str();
    SetSmart(id, smart);
  } else if (key == DB_VIDEO_SVC) {
    const char *SVC = value.c_str();
    SetSVC(id, SVC);
  } else if (key == DB_VIDEO_TYPE) {
    const char *video_type = value.c_str();
    SetVideoType(id, video_type);
  }
}

#if (defined(ENABLE_OSD_SERVER))

static void OsdProcess(int id, std::map<std::string, std::string> &map) {
  SetOsdRegion(id, map);
}

#endif

static void RoiProcess(int id, std::map<std::string, std::string> &map) {
  int stream_id;
  std::string roi_regions;
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);

  for (auto it : map) {
    if (!it.first.compare(DB_ROI_STREAM_TYPE)) {
      if (!it.second.compare(DB_VIDEO_MAIN_STREAM))
        stream_id = 0;
      else if (!it.second.compare(DB_VIDEO_SUB_STREAM))
        stream_id = 1;
      else if (!it.second.compare(DB_VIDEO_THIRD_STREAM))
        stream_id = 2;
    }
  }
  roi_regions = db_protocol->GetRoiRegions(stream_id);
  if (!roi_regions.empty())
    SetRoiRegion(stream_id, roi_regions);
}

#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))

static void RIProcess(int id, std::map<std::string, std::string> &map) {
  region_invade_s region_invade;
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  memset(&region_invade, 0, sizeof(region_invade_s));
  db_protocol->GetRegionInvade(map, region_invade);
  SetRegionInvade(id, region_invade);
}

#endif

static void MoveDetectionProcess(std::string key, std::string value) {
  if (key == DB_MOVE_DETECT_ENABLED) {
    int md_enabled = atoi(value.c_str());
    SetMDEnabled(md_enabled);
  } else if (key == DB_MOVE_DETECT_SENSITIVITY) {
    int md_sensitivity = atoi(value.c_str());
    SetMDSensitivity(md_sensitivity);
  } else if (key == DB_MOVE_DETECT_GRID_MAP) {
    SetMDRect();
  }
}

static void VideoAdvancedEncProcess(std::map<std::string, std::string> &map) {
  int stream_id;
  std::string function;
  nlohmann::json param;

  for (auto it : map) {
    if (!it.first.compare(DB_VIDEO_ADVANCED_ENC_FUNCTION)) {
      function = it.second;
    } else if (!it.first.compare(DB_VIDEO_ADVANCED_ENC_PARAMETERS)) {
      param = nlohmann::json::parse(it.second.c_str());
    } else if (!it.first.compare(DB_VIDEO_ADVANCED_ENC_STREAM_TYPE)) {
      if (!it.second.compare(DB_VIDEO_MAIN_STREAM))
        stream_id = 0;
      else if (!it.second.compare(DB_VIDEO_SUB_STREAM))
        stream_id = 1;
      else if (!it.second.compare(DB_VIDEO_THIRD_STREAM))
        stream_id = 2;
    }
  }
  if (!function.compare(DB_VIDEO_ADVANCED_ENC_FUNCTION_QP)) {
    VideoEncoderQp qp;
    qp.qp_init = param.at(DB_VIDEO_ADVANCED_ENC_IMAGE_QUALITY_INIT);
    qp.qp_step = param.at(DB_VIDEO_ADVANCED_ENC_IMAGE_QUALITY_STEP);
    qp.qp_min = param.at(DB_VIDEO_ADVANCED_ENC_IMAGE_QUALITY_MIN);
    qp.qp_max = param.at(DB_VIDEO_ADVANCED_ENC_IMAGE_QUALITY_MAX);
    qp.qp_min_i = param.at(DB_VIDEO_ADVANCED_ENC_IMAGE_QUALITY_MIN_I_QP);
    qp.qp_max_i = param.at(DB_VIDEO_ADVANCED_ENC_IMAGE_QUALITY_MAX_I_QP);
    SetQP(stream_id, qp);
  } else if (!function.compare(DB_VIDEO_ADVANCED_ENC_FUNCTION_SPLIT)) {
    int mode = param.at(DB_VIDEO_ADVANCED_ENC_SPLIT_MODE);
    int size = param.at(DB_VIDEO_ADVANCED_ENC_SPLIT_SIZE);
    SetSplit(stream_id, mode, size);
  }
}

void DBProtocolVideoProcess(int id, std::map<std::string, std::string> &map) {
  // If some Settings need to reset the pipe, there is no need to set others
  auto iter = map.find(DB_VIDEO_RESOLUTION);
  if (iter != map.end()) {
    VideoProcess(id, iter->first, iter->second);
    return;
  }

  for (auto it : map) {
    VideoProcess(id, it.first, it.second);
  }
}

void DBProtocolAudioProcess(int id, std::map<std::string, std::string> &map) {
  for (auto it : map) {
    AudioProcess(it.first, it.second);
  }
}

#if (defined(ENABLE_OSD_SERVER))

void DBProtocolOsdProcess(int id, std::map<std::string, std::string> &map) {
  OsdProcess(id, map);
}

#endif

void DBProtocolRoiProcess(int id, std::map<std::string, std::string> &map) {
  RoiProcess(id, map);
}

#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))

void DBProtocolRIProcess(int id, std::map<std::string, std::string> &map) {
  RIProcess(id, map);
}

#endif

void DBProtocolMDProcess(int id, std::map<std::string, std::string> &map) {
  for (auto it : map) {
    MoveDetectionProcess(it.first, it.second);
  }
}

void DBProtocolAEProcess(std::map<std::string, std::string> &map) {
  VideoAdvancedEncProcess(map);
}

void FlowDbProtocol::DbDataDispatch(std::string db_data) {
  int id = -1;
  std::string value;
  std::map<std::string, std::string> map;
  nlohmann::json context = nlohmann::json::parse(db_data);
  nlohmann::json data = context.at(DB_MEDIA_TABLE_DATACHANGE);
  std::string data_s = data.dump();

  nlohmann::json key_j = context.at(DB_MEDIA_TABLE_KEY);
  std::string key_s = key_j.dump();
  if (key_s.find(DB_MEDIA_TABLE_ID) != std::string::npos)
    id = atoi(key_j.at(DB_MEDIA_TABLE_ID).dump().c_str());

  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it.value().is_string())
      value = it.value().get<std::string>();
    else
      value = std::to_string(it.value().get<int>());
    map[it.key()] = value;
  }

  nlohmann::json table = context.at(DB_MEDIA_TABLE);
  std::string table_s = table.dump();
  if (table_s.find(DB_MEDIA_VIDEO_ADVANCED_ENC_TABLE) != std::string::npos) {
    DBProtocolAEProcess(map);
  } else if (table_s.find(DB_MEDIA_VIDEO_TABLE) != std::string::npos) {
    DBProtocolVideoProcess(id, map);
  } else if (table_s.find(DB_MEDIA_AUDIO_TABLE) != std::string::npos) {
    DBProtocolAudioProcess(id, map);
#if (defined(ENABLE_OSD_SERVER))
  } else if (table_s.find(DB_MEDIA_OSD_TABLE) != std::string::npos) {
    DBProtocolOsdProcess(id, map);
#endif
  } else if ((table_s.find(DB_MEDIA_ROI_TABLE) != std::string::npos) &&
             (data_s.find(DB_ROI_STREAM_ENABLED) == std::string::npos)) {
    DBProtocolRoiProcess(id, map);
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))
  } else if ((table_s.find(DB_MEDIA_REGION_INVADE_TABLE) !=
              std::string::npos)) {
    DBProtocolRIProcess(id, map);
#endif
  } else if (table_s.find(DB_MEDIA_MOVE_DETECTION_TABLE) != std::string::npos) {
    DBProtocolMDProcess(id, map);
  }
}

void FlowDbProtocol::DbDataToMap(std::string db_data,
                                 std::map<std::string, std::string> &map) {
  nlohmann::json context = nlohmann::json::parse(db_data);
  nlohmann::json data = context.at(DB_MEDIA_TABLE_DATA);
  std::string data_s = data.dump();
  std::string key, value;
  for (auto it = data[0].begin(); it != data[0].end(); ++it) {
    if (it.value().is_string())
      value = it.value().get<std::string>();
    else
      value = std::to_string(it.value().get<int>());
    map[it.key()] = value;
  }
}

std::string
FlowDbProtocol::GetValueByKey(std::string key,
                              std::map<std::string, std::string> &map) {
  for (auto it : map) {
    if (it.first == key)
      return it.second;
  }
  return nullptr;
}

std::string FlowDbProtocol::GetValueByKey(std::string db_data,
                                          std::string key) {
  std::map<std::string, std::string> map;
  DbDataToMap(db_data, map);
  return GetValueByKey(key, map);
}

std::string FlowDbProtocol::GetRoiRegions(int stream_id) {
  int roi_id;
  int roi_enabled;
  int enc_flow_width;
  int enc_flow_height;
  int roi_enabled_num = 0;
  std::string db_config;
  std::string roi_region = "";
  std::string roi_regions = "";
  nlohmann::json roi_config;
  nlohmann::json db_config_js = nlohmann::json::array();
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();

  db_config = flow_manager->SelectVideoDb(stream_id);
  if (db_config.empty() ||
      db_config.find(DB_MEDIA_TABLE_ID) == std::string::npos) {
    return "";
  }
  db_config_js = nlohmann::json::parse(db_config).at(DB_MEDIA_TABLE_DATA);
  if (db_config_js.empty()) {
    LOG_INFO("select video database empty\n");
    return roi_regions;
  }
  std::string resolution = db_config_js.at(0).at(DB_VIDEO_RESOLUTION);
  int pos = resolution.find("*");
  enc_flow_width = atoi(resolution.substr(0, pos).c_str());
  enc_flow_height =
      atoi(resolution.substr(pos + 1, resolution.size() - 1).c_str());

  LOG_DEBUG("enc_flow_width is %d, enc_flow_height is %d\n", enc_flow_width,
            enc_flow_height);
  float gradient_x = (float)enc_flow_width / WEB_VIEW_RECT_W;
  float gradient_y = (float)enc_flow_height / WEB_VIEW_RECT_H;

  EncROIRegion region_data[ROI_REGION_NUM];
  memset(region_data, 0, sizeof(region_data));
  for (int i = 0; i < ROI_REGION_NUM; i++) {
    db_config = flow_manager->SelectRoiDb(stream_id * 4 + i);
    LOG_DEBUG("db_config is %s\n", db_config.c_str());
    roi_config = nlohmann::json::parse(db_config).at(DB_MEDIA_TABLE_DATA).at(0);
    roi_enabled = roi_config.at(DB_ROI_ENABLED);
    roi_enabled_num += roi_enabled;
    if (!roi_enabled)
      continue;

    int db_roi_x = roi_config.at(DB_ROI_POSITION_X);
    int db_roi_y = roi_config.at(DB_ROI_POSITION_Y);
    int db_roi_w = roi_config.at(DB_ROI_WIDTH);
    int db_roi_h = roi_config.at(DB_ROI_HTIGHT);
    int db_quality_level = roi_config.at(DB_ROI_QUALITY_LEVEL);
    LOG_DEBUG("db_roi_x is %d, db_roi_y is %d, "
              "db_roi_w is %d, db_roi_h is %d\n",
              db_roi_x, db_roi_y, db_roi_w, db_roi_h);
    float gradient_w = (float)db_roi_w / WEB_VIEW_RECT_W;
    float gradient_h = (float)db_roi_h / WEB_VIEW_RECT_H;
    LOG_DEBUG("gradient_x is %f, gradient_y is %f, "
              "gradient_w is %f, gradient_h is %f\n",
              gradient_x, gradient_y, gradient_w, gradient_h);

    region_data[i].x = db_roi_x * gradient_x;
    region_data[i].y = db_roi_y * gradient_y;
    region_data[i].w = enc_flow_width * gradient_w;
    region_data[i].h = enc_flow_height * gradient_h;
    region_data[i].x = UPALIGNTO16(region_data[i].x);
    region_data[i].y = UPALIGNTO16(region_data[i].y);
    region_data[i].w = UPALIGNTO16(region_data[i].w);
    region_data[i].h = UPALIGNTO16(region_data[i].h);
    if ((region_data[i].x + region_data[i].w) > enc_flow_width)
      region_data[i].w -= 16;
    if ((region_data[i].y + region_data[i].h) > enc_flow_height)
      region_data[i].h -= 16;
    region_data[i].abs_qp_en = 0;
    switch (db_quality_level) {
    case 6:
      region_data[i].quality = -16;
      break;
    case 5:
      region_data[i].quality = -14;
      break;
    case 4:
      region_data[i].quality = -12;
      break;
    case 3:
      region_data[i].quality = -10;
      break;
    case 2:
      region_data[i].quality = -8;
      break;
    case 1:
    default:
      region_data[i].quality = -6;
    }
    LOG_INFO(
        "#%02d x:%d, y:%d, w:%d, h:%d, abs_qp_en:%d, intra:%d, quality:%d\n", i,
        region_data[i].x, region_data[i].y, region_data[i].w, region_data[i].h,
        region_data[i].abs_qp_en, region_data[i].intra, region_data[i].quality);
  }
  if (!roi_enabled_num)
    return roi_regions;

  for (int i = 0; i < roi_enabled_num; i++) {
    roi_region = "(" + std::to_string(region_data[i].x) + "," +
                 std::to_string(region_data[i].y) + "," +
                 std::to_string(region_data[i].w) + "," +
                 std::to_string(region_data[i].h) + "," +
                 std::to_string(region_data[i].intra) + "," +
                 std::to_string(region_data[i].quality) + "," +
                 std::to_string(region_data[i].qp_area_idx) + "," +
                 std::to_string(region_data[i].area_map_en) + "," +
                 std::to_string(region_data[i].abs_qp_en) + ")";
    roi_regions = roi_regions + roi_region;
  }

  return roi_regions;
}

#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))

void FlowDbProtocol::GetRegionInvade(std::map<std::string, std::string> map,
                                     region_invade_s &region_invade) {
  region_invade.enable = atoi(map[DB_REGION_INVADE_ENABLED].c_str());
  region_invade.width = atoi(map[DB_REGION_INVADE_WIDTH].c_str());
  region_invade.height = atoi(map[DB_REGION_INVADE_HEIGHT].c_str());
  region_invade.position_x = atof(map[DB_REGION_INVADE_POSITION_X].c_str());
  region_invade.position_y = atof(map[DB_REGION_INVADE_POSITION_Y].c_str());
  region_invade.proportion = atoi(map[DB_REGION_INVADE_PROPORTION].c_str());
  region_invade.sensitivity_level =
      atoi(map[DB_REGION_INVADE_SENSITIVITY_LEVEL].c_str());
  region_invade.time_threshold =
      atoi(map[DB_REGION_INVADE_TIME_THRESHOLD].c_str());
}

void FlowDbProtocol::GetRegionInvade(std::string db_data,
                                     region_invade_s &region_invade) {
  std::map<std::string, std::string> map;
  DbDataToMap(db_data, map);
  GetRegionInvade(map, region_invade);
}

std::string FlowDbProtocol::GetRegionInvadeRect(std::string db_data,
                                                std::string img_rect_str) {
  region_invade_s region_invade;
  std::map<std::string, std::string> map;
  DbDataToMap(db_data, map);
  GetRegionInvade(map, region_invade);

  ImageRect img_rect;
  sscanf(img_rect_str.c_str(), "(%d,%d,%d,%d)", &img_rect.x, &img_rect.y,
         &img_rect.w, &img_rect.h);

  float gradient_x_ = (float)img_rect.w / WEB_VIEW_RECT_W;
  float gradient_y_ = (float)img_rect.h / WEB_VIEW_RECT_H;

  ImageRect ri_rect;
  ri_rect.x = UPALIGNTO16((int)(gradient_x_ * region_invade.position_x));
  ri_rect.y = UPALIGNTO16((int)(gradient_y_ * region_invade.position_y));
  ri_rect.w = UPALIGNTO16((int)(gradient_x_ * region_invade.width));
  ri_rect.h = UPALIGNTO16((int)(gradient_y_ * region_invade.height));

  std::string ri_rect_str;
  ri_rect_str.append("(");
  ri_rect_str.append(std::to_string(ri_rect.x));
  ri_rect_str.append(",");
  ri_rect_str.append(std::to_string(ri_rect.y));
  ri_rect_str.append(",");
  ri_rect_str.append(std::to_string(ri_rect.w));
  ri_rect_str.append(",");
  ri_rect_str.append(std::to_string(ri_rect.h));
  ri_rect_str.append(")");
  return ri_rect_str;
}

#endif

std::string FlowDbProtocol::GetMoveDetectRegions(int id,
                                                 std::string move_detect_db,
                                                 int &md_roi_cnt) {
  std::string db_config;
  nlohmann::json db_config_js = nlohmann::json::array();
  FlowManagerPtr &flow_manager = FlowManager::GetInstance();

  // use one stream for move detection by default
  int md_pipe_index = -1;
  for (int i = 0; i < MAX_CAM_NUM; i++) {
    md_pipe_index = flow_manager->GetPipeIndexById(i, StreamType::MOVE_DETEC);
    if (md_pipe_index != -1) {
      LOG_INFO("md_pipe_index is %d\n", md_pipe_index);
      break;
    }
  }
  if (id != md_pipe_index) {
    LOG_INFO("pipe[%d] no found md_flow\n", id);
    return "";
  }

  db_config = flow_manager->SelectVideoDb(md_pipe_index);
  db_config_js = nlohmann::json::parse(db_config).at(DB_MEDIA_TABLE_DATA);
  if (db_config_js.empty()) {
    LOG_ERROR("select video database empty\n");
    return NULL;
  }
  std::string resolution = db_config_js.at(0).at(DB_VIDEO_RESOLUTION);
  int pos = resolution.find("*");
  int video_width = atoi(resolution.substr(0, pos).c_str());
  int video_height =
      atoi(resolution.substr(pos + 1, resolution.size() - 1).c_str());
  LOG_INFO("md_flow belongs to the pipe, height is %d, width is %d\n",
           video_height, video_width);

  std::map<std::string, std::string> map;
  DbDataToMap(move_detect_db, map);
  // if md is not enabled, it returns empty
  int md_enabled = atoi(map[DB_MOVE_DETECT_ENABLED].c_str());
  if (!md_enabled) {
    LOG_INFO("move detection is disabled\n");
    md_roi_cnt = 0;
    return "";
  }

  int row_num = atoi(map[DB_MOVE_DETECT_ROW_GRANULARITY].c_str());
  int column_num = atoi(map[DB_MOVE_DETECT_COLUMN_GRANULARITY].c_str());
  std::string grid_map = map[DB_MOVE_DETECT_GRID_MAP];
  int grid_height = floor((float)video_height / (float)row_num);
  int grid_width = floor((float)video_width / (float)column_num);
  int per_row_hex_num = ceil((float)column_num / 4.0);
  LOG_INFO("grid_height is %d, grid_width is %d\n", grid_height, grid_width);
  LOG_INFO("grid_map is %s\n", grid_map.c_str());
  LOG_INFO("the num of hex char required per row is %d\n", per_row_hex_num);

  std::stringstream ss;
  std::string grid_bin = "";
  for (int i = 0; i < row_num; i++) {
    std::string grid_col_bin = "";
    std::string row_hex = grid_map.substr(i * per_row_hex_num, per_row_hex_num);
    for (int j = 0; j < per_row_hex_num; j++) {
      std::string grid_bin;
      std::string row_bit = row_hex.substr(j, 1);
      int num = strtol(row_bit.c_str(), NULL, 16);
      ss << std::bitset<4>(num);
      ss >> grid_bin;
      ss.clear();
      grid_col_bin = grid_col_bin + grid_bin;
    }
    grid_col_bin = grid_col_bin.substr(0, column_num);
    LOG_INFO("grid_col_bin is %s\n", grid_col_bin.c_str());
    grid_bin = grid_bin + grid_col_bin;
  }
  LOG_DEBUG("grid_bin is %s\n", grid_bin.c_str());

  std::string region = "";
  std::string md_roi_rect = "";
  for (int row = 0; row < row_num; row++) {
    for (int column = 0; column < column_num; column++) {
      std::string enabled_bit = grid_bin.substr(row * column_num + column, 1);
      if (!enabled_bit.compare("1")) {
        md_roi_cnt++;
        region = "(" + std::to_string(column * grid_width) + "," +
                 std::to_string(row * grid_height) + "," +
                 std::to_string(grid_width) + "," +
                 std::to_string(grid_height) + ")";
        md_roi_rect = md_roi_rect + region;
      }
    }
  }

  return md_roi_rect;
}

#endif

} // namespace mediaserver
} // namespace rockchip

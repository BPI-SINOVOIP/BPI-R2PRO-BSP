// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_api.h"
#include <dbserver.h>
#include <mediaserver.h>

namespace rockchip {
namespace cgi {

nlohmann::json video_region_clip_info_get() {
  nlohmann::json region_clip_info;
  char *str = dbserver_video_region_clip_get();
  nlohmann::json region_clip = nlohmann::json::parse(str).at("jData").at(0);
  str = dbserver_media_get((char *)TABLE_NORMALIZED_SCREEN_SIZE);
  nlohmann::json normalized_screen_size =
      nlohmann::json::parse(str).at("jData").at(0);
  region_clip.erase("id");
  normalized_screen_size.erase("id");
  region_clip_info.emplace("normalizedScreenSize", normalized_screen_size);
  region_clip_info.emplace("regionClip", region_clip);

  return region_clip_info;
}

void video_smart_rules(nlohmann::json &config) {
  int bitrate, bitrate_max, bitrate_min;
  bitrate_max = atoi(config.at("iMaxRate").dump().c_str());
  bitrate_min = atoi(config.at("iMinRate").dump().c_str());
  bitrate = (bitrate_max + bitrate_min) >> 1;
  config.emplace("iTargetRate", bitrate);
}

void video_setting_item_set(nlohmann::json config, int id) {
  if (!config.empty()) {
    nlohmann::json setting_config = config;
    if (setting_config.dump().find("id") != setting_config.dump().npos)
      setting_config.erase("id");
    // bitrate set need min, max, target
    mediaserver_bitrate_set(id, (char *)setting_config.dump().c_str());
    mediaserver_set((char *)TABLE_VIDEO, id, (char *)setting_config.dump().c_str());
  }
}

void video_advanced_item_set(nlohmann::json config, int id, int functions_id) {
  if (!config.empty()) {
    int param[256];
    int size;
    std::string params_str = config.at("sParameters").get<std::string>();
    nlohmann::json param_config = nlohmann::json::parse(params_str.c_str());
    if (functions_id == 0) {
      size = 6;
      param[0] = atoi(param_config.at("qp_init").dump().c_str());
      param[1] = atoi(param_config.at("qp_step").dump().c_str());
      param[2] = atoi(param_config.at("qp_min").dump().c_str());
      param[3] = atoi(param_config.at("qp_max").dump().c_str());
      param[4] = atoi(param_config.at("min_i_qp").dump().c_str());
      param[5] = atoi(param_config.at("max_i_qp").dump().c_str());
    } else if (functions_id == 1) {
      size = 2;
      param[0] = atoi(param_config.at("mode").dump().c_str());
      param[1] = atoi(param_config.at("size").dump().c_str());
    }
    mediaserver_advanced_enc_set(id, functions_id, param, size);
  }
}

void VideoApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  std::string path_api_resource;
  std::string path_stream_resource;
  std::string path_specific_resource;
  std::string path_function_resource;
  char *str;
  nlohmann::json content;
#ifdef ENABLE_JWT
  int user_level = Req.UserLevel;
#endif

  // Get Path Information
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_stream_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_stream_resource.find_first_of("/");
    if (pos_first != -1) {
      path_specific_resource = path_stream_resource.substr(
          pos_first + 1, path_stream_resource.size());
      pos_first = path_specific_resource.find_first_of("/");
      if (pos_first != -1) {
        path_function_resource = path_specific_resource.substr(
            pos_first + 1, path_specific_resource.size());
      }
    }
  }

  if (Req.Method == "GET") {
    str = dbserver_video_get();
    nlohmann::json video_all_config = nlohmann::json::parse(str).at("jData");
    nlohmann::json video_current_config;
    if (path_stream_resource.empty()) { // path is video
      content = video_all_config;
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else {
      if (path_specific_resource.empty()) { // path example is video/0
        int id = stoi(path_stream_resource);
        video_current_config = video_all_config.at(id);
        content = video_current_config;
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!path_stream_resource.compare("2/region-clip")) {
        // path must is video/2/region-clip
        content = video_region_clip_info_get();
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!path_specific_resource.compare("advanced-enc")) {
        // path example is viedo/0/advanced-enc
        str = dbserver_media_get((char *)TABLE_VIDEO_ADVANCED_ENC);
        content = nlohmann::json::parse(str).at("jData");
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    nlohmann::json video_config = Req.PostObject; // must be json::object

    if (path_stream_resource.empty()) { // path is video
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    } else {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      int id = stoi(path_stream_resource.substr(0, 1));
      if (path_specific_resource.empty()) { // path example is video/0
        // id and sStreamType must correspond
        char *stream_type = new char[20];
        strcpy(stream_type,
               (video_config.at("sStreamType").get<std::string>()).c_str());
        if ((id == 0) && (strcmp(stream_type, "mainStream")))
          strcpy(stream_type, "mainStream");
        if ((id == 1) && (strcmp(stream_type, "subStream")))
          strcpy(stream_type, "subStream");
        if ((id == 2) && (strcmp(stream_type, "thirdStream")))
          strcpy(stream_type, "thirdStream");

        // get depend object
        bool depend_bitrate = false;
        if (!video_config.empty())
          if (video_config.dump().find("\"sSmart\":\"open\"") !=
              std::string::npos)
            depend_bitrate = true;

        // Erase unchanged data
        char *prev = dbserver_video_get();
        nlohmann::json cfg_old_all = nlohmann::json::parse(prev).at("jData");
        nlohmann::json diff =
            nlohmann::json::diff(cfg_old_all.at(id), video_config);
        for (auto &x : nlohmann::json::iterator_wrapper(cfg_old_all.at(id))) {
          if (depend_bitrate &&
              (!x.key().compare("iTargetRate") ||
               !x.key().compare("iMinRate") || !x.key().compare("iMaxRate")))
            continue;
          if (diff.dump().find("\"replace\",\"path\":\"/" + x.key()) ==
              diff.dump().npos)
            video_config.erase(x.key());
        }

        // smart rules
        if (depend_bitrate) {
          video_smart_rules(video_config);
        } else if ((!depend_bitrate) && (video_config.dump().find("iMaxRate") !=
                                         std::string::npos)) {
          video_config.emplace("iTargetRate", 0);
          video_config.emplace("iMinRate", 0);
        }
        // Set new config, only one table is transmitted at a time.
        if (!video_config.empty())
          dbserver_video_set((char *)video_config.dump().c_str(), stream_type);
        video_setting_item_set(video_config, id);
        delete[] stream_type;
        // get new info
        char *str = dbserver_video_get();
        nlohmann::json video_all_config =
            nlohmann::json::parse(str).at("jData");
        content = video_all_config.at(id);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!path_stream_resource.compare("2/region-clip")) {
        // path must is video/2/region-clip
        if (!video_config.empty())
          dbserver_video_region_clip_set((char *)video_config.dump().c_str(),
                                         0); // temp only one table
        // get new info
        content = video_region_clip_info_get();
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (path_specific_resource.find("advanced-enc") !=
                 path_specific_resource.npos) {
        if (path_function_resource.empty()) {
          // path is video/0/advanced-enc
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        } else {
          // path example is video/0/advanced-enc/qp
          minilog_debug("path_function_resource is %s\n",
                        path_function_resource.c_str());
          int functions_id = 0;
          if (!path_function_resource.compare("qp"))
            functions_id = 0;
          else if (!path_function_resource.compare("split"))
            functions_id = 1;
          if (!video_config.empty())
            dbserver_media_set((char *)TABLE_VIDEO_ADVANCED_ENC,
                               (char *)video_config.dump().c_str(),
                               functions_id);
          video_advanced_item_set(video_config, id, functions_id);
          // get new info
          str = dbserver_media_get((char *)TABLE_VIDEO_ADVANCED_ENC);
          content = nlohmann::json::parse(str).at("jData");
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(content);
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      }
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip

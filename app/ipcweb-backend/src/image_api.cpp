// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_api.h"
#include <dbserver.h>
#include <mediaserver.h>

namespace rockchip {
namespace cgi {

nlohmann::json image_specific_config_get(std::string string) {
  nlohmann::json specific_config = nlohmann::json::array();
  char *table;

  if (!string.compare(PATH_IMAGE_SCENARIO))
    table = TABLE_IMAGE_SCENARIO;
  if (!string.compare(PATH_IMAGE_ADJUSTMENT))
    table = TABLE_IMAGE_ADJUSTMENT;
  if (!string.compare(PATH_IMAGE_EXPOSURE))
    table = TABLE_IMAGE_EXPOSURE;
  if (!string.compare(PATH_IMAGE_NIGHT_TO_DAY))
    table = TABLE_IMAGE_NIGHT_TO_DAY;
  if (!string.compare(PATH_IMAGE_BLC))
    table = TABLE_IMAGE_BLC;
  if (!string.compare(PATH_IMAGE_WHITE_BLANCE))
    table = TABLE_IMAGE_WHITE_BLANCE;
  if (!string.compare(PATH_IMAGE_ENHANCEMENT))
    table = TABLE_IMAGE_ENHANCEMENT;
  if (!string.compare(PATH_IMAGE_VIDEO_ADJUSTMEN))
    table = TABLE_IMAGE_VIDEO_ADJUSTMEN;
  specific_config =
      nlohmann::json::parse(dbserver_media_get(table)).at("jData");

  return specific_config;
}

nlohmann::json image_specific_resource_get(std::string string, int id) {
  nlohmann::json specific_config =
      nlohmann::json::array(); /* array of all the same resources */
  nlohmann::json specific_resource =
      nlohmann::json::object(); /* one of all the same resources */

  specific_config = image_specific_config_get(string);
  specific_config.at(id).erase("id");
  specific_resource = specific_config.at(id);

  return specific_resource;
}

nlohmann::json image_channel_resource_get(int channel_id, int id) {
  nlohmann::json resource = nlohmann::json::object();
  nlohmann::json image_adjustment =
      image_specific_config_get(PATH_IMAGE_ADJUSTMENT);
  nlohmann::json image_exposure =
      image_specific_config_get(PATH_IMAGE_EXPOSURE);
  nlohmann::json image_night_to_day =
      image_specific_config_get(PATH_IMAGE_NIGHT_TO_DAY);
  nlohmann::json image_BLC = image_specific_config_get(PATH_IMAGE_BLC);
  nlohmann::json image_white_blance =
      image_specific_config_get(PATH_IMAGE_WHITE_BLANCE);
  nlohmann::json image_enhancement =
      image_specific_config_get(PATH_IMAGE_ENHANCEMENT);
  nlohmann::json image_video_adjustment =
      image_specific_config_get(PATH_IMAGE_VIDEO_ADJUSTMEN);

  image_adjustment.at(id).erase("id");
  image_exposure.at(id).erase("id");
  image_night_to_day.at(id).erase("id");
  image_BLC.at(id).erase("id");
  image_white_blance.at(id).erase("id");
  image_enhancement.at(id).erase("id");
  image_video_adjustment.at(id).erase("id");

  resource.emplace("id", channel_id);
  resource.emplace(KEY_IMAGE_ADJUSTMENT, image_adjustment.at(id));
  resource.emplace(KEY_IMAGE_EXPOSURE, image_exposure.at(id));
  resource.emplace(KEY_IMAGE_NIGHT_TO_DAY, image_night_to_day.at(id));
  resource.emplace(KEY_IMAGE_BLC, image_BLC.at(id));
  resource.emplace(KEY_IMAGE_WHITE_BLANCE, image_white_blance.at(id));
  resource.emplace(KEY_IMAGE_ENHANCEMENT, image_enhancement.at(id));
  resource.emplace(KEY_IMAGE_VIDEO_ADJUSTMEN, image_video_adjustment.at(id));

  return resource;
}

void image_specific_resource_set(std::string current, nlohmann::json data,
                                 int id) {
  char *table;

  if (!current.compare(PATH_IMAGE_SCENARIO))
    table = TABLE_IMAGE_SCENARIO;
  if (!current.compare(PATH_IMAGE_ADJUSTMENT))
    table = TABLE_IMAGE_ADJUSTMENT;
  if (!current.compare(PATH_IMAGE_EXPOSURE))
    table = TABLE_IMAGE_EXPOSURE;
  if (!current.compare(PATH_IMAGE_NIGHT_TO_DAY))
    table = TABLE_IMAGE_NIGHT_TO_DAY;
  if (!current.compare(PATH_IMAGE_BLC))
    table = TABLE_IMAGE_BLC;
  if (!current.compare(PATH_IMAGE_WHITE_BLANCE))
    table = TABLE_IMAGE_WHITE_BLANCE;
  if (!current.compare(PATH_IMAGE_ENHANCEMENT))
    table = TABLE_IMAGE_ENHANCEMENT;
  if (!current.compare(PATH_IMAGE_VIDEO_ADJUSTMEN))
    table = TABLE_IMAGE_VIDEO_ADJUSTMEN;

  dbserver_media_set(table, (char *)data.dump().c_str(), id);
  mediaserver_set((char *)TABLE_VIDEO, 0, (char *)data.dump().c_str());
}

void image_channel_resource_set(nlohmann::json json_config, int id) {
  for (auto &x : nlohmann::json::iterator_wrapper(json_config)) {
    auto val = x.value(); /* string or int */
    std::string resource;
    if (x.key() == KEY_IMAGE_ADJUSTMENT)
      resource = PATH_IMAGE_ADJUSTMENT;
    else if (x.key() == KEY_IMAGE_EXPOSURE)
      resource = PATH_IMAGE_EXPOSURE;
    else if (x.key() == KEY_IMAGE_NIGHT_TO_DAY)
      resource = PATH_IMAGE_NIGHT_TO_DAY;
    else if (x.key() == KEY_IMAGE_BLC)
      resource = PATH_IMAGE_BLC;
    else if (x.key() == KEY_IMAGE_WHITE_BLANCE)
      resource = PATH_IMAGE_WHITE_BLANCE;
    else if (x.key() == KEY_IMAGE_ENHANCEMENT)
      resource = PATH_IMAGE_ENHANCEMENT;
    else if (x.key() == KEY_IMAGE_VIDEO_ADJUSTMEN)
      resource = PATH_IMAGE_VIDEO_ADJUSTMEN;
    else
      continue;

    /* Erase unchanged data */
    nlohmann::json resource_old = image_specific_resource_get(resource, id);
    nlohmann::json resource_diff = nlohmann::json::diff(resource_old, val);
    for (auto &y : nlohmann::json::iterator_wrapper(resource_old)) {
      if (resource_diff.dump().find(y.key()) == resource_diff.dump().npos)
        val.erase(y.key());
    }
    image_specific_resource_set(resource, val, id);
  }
}

void ImageApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  int id;
  int channel_id = 0;
  nlohmann::json content;
  nlohmann::json scenario;
  std::string scenario_str = "";
  std::string path_api_resource = "";
  std::string path_channel_resource = "";
  std::string path_specific_resource = "";
#ifdef ENABLE_JWT
  int user_level = Req.UserLevel;
#endif

  /* Get the current scene and the corresponding table id */
  scenario = image_specific_resource_get(PATH_IMAGE_SCENARIO, 0);
  scenario_str = scenario.at("sScenario").dump();
  scenario_str.erase(0, 1); /* erase " */
  scenario_str.erase(scenario_str.end() - 1, scenario_str.end());
  if (!scenario_str.compare(IMAGE_SCENARIO_NORMAL))
    id = 0;
  if (!scenario_str.compare(IMAGE_SCENARIO_BACKLIGHT))
    id = 1;
  if (!scenario_str.compare(IMAGE_SCENARIO_FRONTLIGHT))
    id = 2;
  if (!scenario_str.compare(IMAGE_SCENARIO_LOW_ILLUMINATION))
    id = 3;
  if (!scenario_str.compare(IMAGE_SCENARIO_CUSTOM1))
    id = 4;
  if (!scenario_str.compare(IMAGE_SCENARIO_CUSTOM2))
    id = 5;

  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_channel_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_channel_resource.find_first_of("/");
    if (pos_first != -1)
      path_specific_resource = path_channel_resource.substr(
          pos_first + 1, path_channel_resource.size());
  }
  if (!path_channel_resource.empty())
    channel_id = atoi(
        path_channel_resource.substr(0, path_channel_resource.size()).c_str());
  id = id + 6 * channel_id; /* 6 scenarios for each channel */

  if (Req.Method == "GET") {
    /* Get based on path information */
    if (path_channel_resource.empty()) { /* path info is /image */
      content = nlohmann::json::array();
      /* TODO: channel 1 need dbserver to create 6 more tables and test */
      if (channel_id == 0) {
        content.push_back(image_channel_resource_get(0, id));
        // content.push_back(image_channel_resource_get(1, id + 6));
      } else if (channel_id == 1) {
        content.push_back(image_channel_resource_get(0, id - 6));
        // content.push_back(image_channel_resource_get(1, id));
      }
      // content.push_back(image_channel_resource_get(1, id));
    } else {
      if (path_specific_resource.empty()) { /* path info is /image/0 */
        content = image_channel_resource_get(channel_id, id);
      } else { /* path example is /image/0/blc */
        if (!path_specific_resource.compare(PATH_IMAGE_SCENARIO))
          id = 0;
        content = image_specific_resource_get(path_specific_resource, id);
      }
    }

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    nlohmann::json diff;
    nlohmann::json cfg_old;
    nlohmann::json image_config = Req.PostObject; /* must be json::object */

    if (path_specific_resource
            .empty()) { /* path info is /image/0 or /image/1 */
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      cfg_old = image_channel_resource_get(channel_id, id);
      /* Erase unexist data */
      for (auto &x : nlohmann::json::iterator_wrapper(image_config)) {
        if (cfg_old.dump().find(x.key()) == cfg_old.dump().npos)
            image_config.erase(x.key());
      }
      /* Erase unchanged data */
      diff = nlohmann::json::diff(cfg_old, image_config);
      for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
        if (diff.dump().find(x.key()) == diff.dump().npos)
          image_config.erase(x.key());
      }
      /* Set */
      if (!image_config.empty())
        image_channel_resource_set(image_config, id);
      /* Get new info */
      content = image_channel_resource_get(channel_id, id);
    } else { /* path example is /image/0/blc */
      if (!path_specific_resource.compare(PATH_IMAGE_SCENARIO))
        id = 0;
      cfg_old = image_specific_resource_get(path_specific_resource, id);
      /* Erase unexist data */
      for (auto &x : nlohmann::json::iterator_wrapper(image_config)) {
        if (cfg_old.dump().find(x.key()) == cfg_old.dump().npos)
            image_config.erase(x.key());
      }
      /* Erase unchanged data */
      diff = nlohmann::json::diff(cfg_old, image_config);
      for (auto &x : nlohmann::json::iterator_wrapper(cfg_old)) {
        if (diff.dump().find(x.key()) == diff.dump().npos)
          image_config.erase(x.key());
      }
      /* Set */
      if (!image_config.empty())
        image_specific_resource_set(path_specific_resource, image_config, id);
      /* Get new info */
      content = image_specific_resource_get(path_specific_resource, id);
    }

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchips

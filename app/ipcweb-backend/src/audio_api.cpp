// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "audio_api.h"
#include <dbserver.h>
#include <mediaserver.h>

namespace rockchip {
namespace cgi {

static void audio_setting_item_set(nlohmann::json config, int id) {
  if (!config.empty()) {
    nlohmann::json setting_config = config;
    if (setting_config.dump().find("id") != setting_config.dump().npos)
      setting_config.erase("id");
    mediaserver_set((char *)TABLE_AUDIO, id, (char *)setting_config.dump().c_str());
  }
}

void AudioApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
#ifdef ENABLE_JWT
  int user_level = Req.UserLevel;
#endif
  if (Req.Method == "GET") {
    nlohmann::json content;
    char *str = dbserver_audio_get();
    nlohmann::json audio_config = nlohmann::json::parse(str).at("jData");
    int pos_first = Req.PathInfo.find_first_of("/");
    int pos_last = Req.PathInfo.find_last_of("/");

    if (pos_first != pos_last)
      content = audio_config.at(0);
    else
      content = audio_config;

    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
#ifdef ENABLE_JWT
    if (user_level > 1) {
      Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
      return;
    }
#endif
    nlohmann::json content = nlohmann::json::array();
    nlohmann::json audio_config = Req.PostObject; /* must be json::object */
    int id = 0;
    int pos_first = Req.PathInfo.find_first_of("/");
    int pos_last = Req.PathInfo.find_last_of("/");

    if (pos_first != pos_last)
      id = atoi(Req.PathInfo.substr(pos_last + 1, Req.PathInfo.size()).c_str());

    /* Erase unchanged data */
    char *prev = dbserver_audio_get();
    nlohmann::json cfg_old = nlohmann::json::parse(prev).at("jData");
    nlohmann::json diff = nlohmann::json::diff(cfg_old.at(id), audio_config);
    for (auto &x : nlohmann::json::iterator_wrapper(cfg_old.at(id))) {
      if (diff.dump().find(x.key()) == diff.dump().npos)
        audio_config.erase(x.key());
    }

    /* set */
    if (!audio_config.empty())
      dbserver_audio_set((char *)audio_config.dump().c_str());
    audio_setting_item_set(audio_config, id);

    /* get new info */
    char *str = dbserver_audio_get();
    audio_config = nlohmann::json::parse(str).at("jData");
    content = audio_config.at(id);
    Resp.setHeader(HttpStatus::kOk, "OK");
    Resp.setApiData(content);
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip

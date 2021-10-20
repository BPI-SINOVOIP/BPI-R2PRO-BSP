// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "system_api.h"
#include "common.h"
#include <dbserver.h>
#include <fstream>
#include <netserver.h>
#include <sys/statfs.h>
#include <system_manager.h>

#ifdef MEDIASERVER_ROCKFACE
#include <storage_manager.h>
#include <mediaserver.h>
#endif

namespace rockchip {
namespace cgi {

std::string create_fireware_location() {
  int file_id;
  int exist = 1;
  char *str;
  std::ofstream file;
  // Make sure files are not duplicated
  srand((int)time(NULL));
  while (exist) {
    file_id = rand() % 1000;
    file.open("/data/" + file_id);
    if (!file) // open fail
      exist = 0;
    file.close();
  }
  // Create new file
  std::string file_name = "/data/" + std::to_string(file_id);
  std::ofstream new_file(file_name.c_str());
  new_file.close();
  // Get ip address and location
  std::string ipv4_address = ipv4_address_get();
  std::string location = "http://" + ipv4_address +
                         "/cgi-bin/entry.cgi/system/firmware-upgrade?id=" +
                         std::to_string(file_id);

  return location;
}

nlohmann::json is_register_user(nlohmann::json put_info) {
  std::string user_name = put_info.at("sUserName");
  std::string password = put_info.at("sPassword");
  char *str;
  nlohmann::json res;
  int id_mark;
  str = dbserver_system_get((char *)TABLE_SYSTEM_USER);
  nlohmann::json user_info = nlohmann::json::parse(str).at("jData");
  for (auto &x : nlohmann::json::iterator_wrapper(user_info)) {
    id_mark = x.value().at("id");
    std::string db_name = x.value().at("sUserName");
    if (!user_name.compare(db_name)) {
      std::string db_password = x.value().at("sPassword");
      if (!password.compare(db_password)) {
        int auth = x.value().at("iUserLevel");
        res.emplace("status", id_mark);
        res.emplace("auth", auth);
        return res;
      } else {
        res.emplace("status", -1);
        res.emplace("auth", 0);
        return res;
      }
    }
  }
  // |status| - 1 = id for add
  id_mark = -2 - id_mark;
  res.emplace("status", id_mark);
  res.emplace("auth", 0);
  return res;
}

void create_new_user(nlohmann::json user_info) {
  int user_id = 0;
  char *str;
  str = dbserver_sql((char *)"select * from SystemUser ORDER BY id DESC LIMIT 1",
                     (char *)DBSERVER_SYSTEM_INTERFACE);
  nlohmann::json user_last = nlohmann::json::parse(str).at("jData");
  if (!user_last.empty()) {
    user_id = user_last.at(0).at("id");
    user_id++;
  }
  dbserver_system_set((char *)TABLE_SYSTEM_USER,
                      (char *)user_info.dump().c_str(), user_id);
}

// -1:admin password wrong, >=0:ok, -2:find no item have the same user name
int user_alter_verify(nlohmann::json put_info) {
  char *str;
  int user_id = 0;
  std::string in_admin_password = put_info.at("sPassword");
  str = dbserver_sql(
      (char *)"select sPassword from SystemUser WHERE sUserName='admin'",
      (char *)DBSERVER_SYSTEM_INTERFACE);
  nlohmann::json admin_json = nlohmann::json::parse(str).at("jData").at(0);
  std::string db_admin_password = admin_json.at("sPassword");
  if (in_admin_password.compare(db_admin_password)) {
    return -1;
  }
  std::string in_user_name = put_info.at("newUserName");
  std::string cmd =
      "select id from SystemUser WHERE sUserName='" + in_user_name + "'";
  str = dbserver_sql((char *)cmd.c_str(), (char *)DBSERVER_SYSTEM_INTERFACE);
  nlohmann::json new_json = nlohmann::json::parse(str).at("jData");
  if (!new_json.empty()) {
    user_id = new_json.at(0).at("id");
  } else {
    user_id = -2;
  }
  return user_id;
}

nlohmann::json GetPara(std::string para_name) {
  std::string para_key = "para";
  char *str = dbserver_system_get((char *)TABLE_SYSTEM_PARA);
  nlohmann::json data = nlohmann::json::parse(str).at("jData");
  nlohmann::json content;
  if (!para_name.compare("screenshot")) {
    content = data.at(1).at(para_key);
  } else if (!para_name.compare("screenshot-schedule")) {
    content = data.at(2).at(para_key);
  } else if (!para_name.compare("video-plan-schedule")) {
    content = data.at(3).at(para_key);
  } else if (!para_name.compare("smart-cover")) {
    content = data.at(14).at(para_key);
  } else if (!para_name.compare("video-encoder")) {
    content = data.at(4).at(para_key);
  } else if (!para_name.compare("isp")) {
    for (int i = 7; i <= 13; i++) {
      std::string name = data.at(i).at("name");
      content[name] = data.at(i).at(para_key);
    }
  } else {
    str = dbserver_system_para_get_by_name((char *)para_name.c_str());
    data = nlohmann::json::parse(str).at("jData");
    if (!data.empty()) {
        content = data.at(0).at(para_key);
    }
  }
  return content;
}

void SystemApiHandler::handler(const HttpRequest &Req, HttpResponse &Resp) {
  char *str;
  std::string path_api_resource;
  std::string path_specific_resource;
  std::string para_channel;
  nlohmann::json content;
#ifdef ENABLE_JWT
  int user_level = Req.UserLevel;
#endif

  /* Get Path Information */
  int pos_first = Req.PathInfo.find_first_of("/");
  path_api_resource = Req.PathInfo.substr(pos_first + 1, Req.PathInfo.size());
  pos_first = path_api_resource.find_first_of("/");
  if (pos_first != -1) {
    path_specific_resource =
        path_api_resource.substr(pos_first + 1, path_api_resource.size());
    pos_first = path_specific_resource.find_first_of("/");
    if (pos_first != -1)
      para_channel = path_specific_resource.substr(
          pos_first + 1, path_specific_resource.size() + 1);
  }

  if (Req.Method == "GET") {
    if (!path_specific_resource.compare("device-info")) {
      str = dbserver_system_get((char *)TABLE_SYSTEM_DEVICE_INFO);
      content = nlohmann::json::parse(str).at("jData");
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (!path_specific_resource.compare("remain-space")) {
      struct statfs diskInfo;
      statfs("/userdata/", &diskInfo);
      // Free space for non-root users
      unsigned long long availableDisk = diskInfo.f_bavail * diskInfo.f_bsize;
      content.emplace("availableDisk", availableDisk);
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else if (path_specific_resource.find("para") != std::string::npos) {
      if (para_channel.empty()) {
        Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
      } else {
        content = GetPara(para_channel);
        if (!content.empty()) {
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.setApiData(content);
        } else {
          Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
        }
      }
    } else if (!path_specific_resource.compare("login")) {
      /* path is system/login */
      str = dbserver_sql((char *)"SELECT id, sUserName, iAuthLevel, iUserLevel FROM SystemUser",
                         (char *)DBSERVER_SYSTEM_INTERFACE);
      content = nlohmann::json::parse(str).at("jData");
      Resp.setHeader(HttpStatus::kOk, "OK");
      Resp.setApiData(content);
    } else {
      Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
    }
  } else if ((Req.Method == "POST") || (Req.Method == "PUT")) {
    nlohmann::json system_config = Req.PostObject; /* must be json::object */
    if (!path_specific_resource.compare("device-info")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      int id = system_config.at("id");
      str = dbserver_system_get((char *)TABLE_SYSTEM_DEVICE_INFO);
      nlohmann::json cfg_old = nlohmann::json::parse(str).at("jData").at(id);
      if (!cfg_old.at("ro").dump().compare("\"true\"")) {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Value is read-only!");
      } else {
        /* Erase can't change data */
        for (auto &x : nlohmann::json::iterator_wrapper(Req.PostObject)) {
          if (x.key() != "value")
            system_config.erase(x.key());
        }
        /* Set */
        dbserver_system_set((char *)TABLE_SYSTEM_DEVICE_INFO,
                            (char *)system_config.dump().c_str(), id);
        /* Get new info */
        str = dbserver_system_get((char *)TABLE_SYSTEM_DEVICE_INFO);
        content = nlohmann::json::parse(str).at("jData").at(id);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      }
    } else if (!path_specific_resource.compare("firmware-upgrade")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      // int total_length = 0;
      int transmitted_length;
      int begin_position;
      int end_position;
      int current_transfer_length;
      std::string progress_rate = "";
      // for (auto p : Req.Params) {
      //   if (p.Key == "total-length")
      //     total_length = atoi(p.Value.c_str());
      // }
      for (auto p : Req.Params) {
        if ((p.Key == "upload-type") && (p.Value == "resumable")) {
          // Create a transmission resource
          Resp.setHeader(HttpStatus::kOk, "OK");
          Resp.addHeader("X-Location", create_fireware_location());
        } else if (p.Key == "id") {
          std::ofstream file;
          std::string file_id = p.Value;

          file.open("/data/" + file_id, std::ofstream::app); // append write
          if (!file) {
            Resp.setErrorResponse(HttpStatus::kNotFound, "Not Found");
          } else if (Req.ContentLength == 0) {
            // Query the current progress
            file.seekp(0, std::ofstream::end);
            transmitted_length = file.tellp();
            progress_rate = "bytes 0-" + std::to_string(transmitted_length - 1);
            Resp.setHeader(HttpStatus::kResumeIncomplete, "Resume Incomplete");
            content.emplace("range", progress_rate);
            Resp.setApiData(content);
          } else {
            // Transfer
            begin_position = file.tellp();
            file << Req.PostData;
            end_position = file.tellp();
            current_transfer_length = end_position - begin_position;
            // transmitted_length is not necessarily equal to end_position,
            // it is possible to change a paragraph in the middle of the file
            file.seekp(0, std::ofstream::end);
            transmitted_length = file.tellp();
            if (Req.ContentLength != current_transfer_length) {
              // Request for retransmission due to transmission error
              progress_rate = "bytes 0-" + std::to_string(begin_position - 1);
              Resp.setHeader(HttpStatus::kResumeIncomplete,
                             "Resume Incomplete");
            } else {
              progress_rate =
                  "bytes 0-" + std::to_string(transmitted_length - 1);
              if (Req.ContentLength < 512 * 1024) {
                // Transfer completed
                Resp.setHeader(HttpStatus::kCreated, "Created");
              } else if (Req.ContentLength == 512 * 1024) {
                Resp.setHeader(HttpStatus::kResumeIncomplete,
                               "Resume Incomplete");
              } else {
                Resp.setHeader(HttpStatus::kResumeIncomplete,
                               "More than 512KB!");
              }
            }
            content.emplace("range", progress_rate);
            Resp.setApiData(content);
          }
          file.close();
        } else if (p.Key == "start") {
          int fireware_id = stoi(p.Value);
          std::string path = "/userdata/" + std::to_string(fireware_id);
          system_upgrade(path.c_str());
          Resp.setHeader(HttpStatus::kOk, "OK");
        }
      }
    } else if (!path_specific_resource.compare("reboot")) {
      system_reboot();
    } else if (!path_specific_resource.compare("pre-factory-reset")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      // for face db
#ifdef MEDIASERVER_ROCKFACE
      char *str = storage_manager_get_media_path();
      std::string mount_path = nlohmann::json::parse(str).at("sMountPath");
      minilog_debug("format when reset, path is %s\n", (char *)mount_path.c_str());
      storage_manager_diskformat((char *)mount_path.c_str(), (char *)"fat32");
      mediaserver_clear_face_db();
#else
      dbserver_face_reset((char *)TABLE_FACE_LIST);
#endif
    } else if (!path_specific_resource.compare("factory-reset")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      system_factory_reset();
    } else if (!path_specific_resource.compare("export-log")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      const char *path = (const char *)"/userdata/export.log";
      system_export_log(path);
      std::string ipv4_address = ipv4_address_get();
      std::string location = "http://" + ipv4_address + path;
      content.emplace("location", location);
      Resp.setApiData(content);
    } else if (!path_specific_resource.compare("export-db")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      const char *path = (const char *)"/userdata/export.db";
      system_export_db(path);
      std::string ipv4_address = ipv4_address_get();
      std::string location = "http://" + ipv4_address + path;
      content.emplace("location", location);
      Resp.setApiData(content);
    } else if (!path_specific_resource.compare("import-db")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      if (Req.Params.empty()) {
        int end_position = 0;
        // overwrite
        std::ofstream db_file("/userdata/import.db", std::ofstream::out);
        db_file << Req.Files.at(0).getData();
        end_position = db_file.tellp();
        db_file.close();
        if (end_position == Req.Files.at(0).getDataLength()) {
          Resp.setHeader(HttpStatus::kOk, "OK");
        } else {
          Resp.setErrorResponse(HttpStatus::kBadRequest, "db upload failed!");
        }
      }
      for (auto p : Req.Params) {
        if (p.Key == "start")
          system_import_db((const char *)"/userdata/import.db");
      }
    } else if (path_specific_resource.find("login") != std::string::npos) {
      if (para_channel.empty()) { /* path is login */
        std::string username = system_config.at("sUserName");
        content = is_register_user(system_config);
        int auth = content.at("auth");
        int status = content.at("status");
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
        long expire_time = EXPIRE_SECONDS;
        if (status >= 0) {
          for (auto p : Req.Params) {
            if (p.Key == "expire") {
              std::string expire_stirng = p.Value;
              if (!expire_stirng.compare("day")) {
                expire_time = 86400;
              } else if (!expire_stirng.compare("week")) {
                expire_time = 604800;
              } else if (!expire_stirng.compare("month")) {
                expire_time = 2592000;
              }
              break;
            }
          }
          std::string token = jwt_token_get(username, auth, expire_time);
          Resp.setCookie("token", token, expire_time);
        }
      } else if (!para_channel.compare("modify")) { /* path is login/modify */
#ifdef ENABLE_JWT
        if (user_level > 0) {
          Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
          return;
        }
#endif
        int res = user_alter_verify(system_config);
        if (res >= 0) {
          std::string new_name = system_config.at("newUserName");
          std::string new_pw = system_config.at("newPassword");
          int user_level = system_config.at("iUserLevel");
          nlohmann::json modify_json;
          modify_json.emplace("sPassword", new_pw);
          modify_json.emplace("iUserLevel", user_level);
          dbserver_system_set((char *)TABLE_SYSTEM_USER,
                              (char *)modify_json.dump().c_str(), res);
        }
        content.emplace("status", res);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!para_channel.compare("add")) { /* path is login/add */
#ifdef ENABLE_JWT
        if (user_level > 0) {
          Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
          return;
        }
#endif
        int res = user_alter_verify(system_config);
        if (res == -2) {
          std::string new_name = system_config.at("newUserName");
          std::string new_pw = system_config.at("newPassword");
          nlohmann::json add_json;
          add_json.emplace("sUserName", new_name);
          add_json.emplace("sPassword", new_pw);
          create_new_user(add_json);
        }
        content.emplace("status", res);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else if (!para_channel.compare("delete")) { /* path is login/delete */
#ifdef ENABLE_JWT
        if (user_level > 0) {
          Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
          return;
        }
#endif
        int res = user_alter_verify(system_config);
        if (res > 0) {
          dbserver_system_user_delete(res);
        }
        content.emplace("status", res);
        Resp.setHeader(HttpStatus::kOk, "OK");
        Resp.setApiData(content);
      } else {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Not Implemented");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kBadRequest, "Not Implemented");
    }
  } else if (Req.Method == "DELETE") {
    if (!path_specific_resource.compare("firmware-upgrade")) {
#ifdef ENABLE_JWT
      if (user_level > 1) {
        Resp.setErrorResponse(HttpStatus::kUnauthorized, "Unauthorized");
        return;
      }
#endif
      if (!Req.Params.empty()) {
        for (auto p : Req.Params) {
          if (p.Key == "id") {
            std::string file_id_s = p.Value;
            std::string file_name = "/data/" + file_id_s;
            if (!remove(file_name.c_str()))
              Resp.setHeader(HttpStatus::kOk, "OK");
          }
        }
      } else {
        Resp.setErrorResponse(HttpStatus::kBadRequest, "Not Implemented");
      }
    } else {
      Resp.setErrorResponse(HttpStatus::kBadRequest, "Not Implemented");
    }
  } else {
    Resp.setErrorResponse(HttpStatus::kNotImplemented, "Not Implemented");
  }
}

} // namespace cgi
} // namespace rockchip

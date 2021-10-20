// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_sm_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_sm_protocol.cpp"

namespace rockchip {
namespace mediaserver {

#ifdef ENABLE_DBUS

std::string FlowSMProtocol::storage_path_ = "";

void FlowSMProtocol::DisksStatusChanged(std::string dbus_data) {
  int disk_id = -1;
  int disk_status = -1;
  std::string mount_path;

  dbus_data = dbus_data.replace(dbus_data.find("["), 1, "");
  dbus_data = dbus_data.replace(dbus_data.find("]"), 1, "");

  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();
  if (context_str.find(SM_TABLE_KEY_ID) != std::string::npos) {
    disk_id = atoi(context.at(SM_TABLE_KEY_ID).dump().c_str());
  }
  if (context_str.find(SM_TABLE_KEY_STATUS) != std::string::npos) {
    disk_status = atoi(context.at(SM_TABLE_KEY_STATUS).dump().c_str());
  }
  if (context_str.find(SM_TABLE_KEY_MOUNT_PATH) != std::string::npos) {
    mount_path = context.at(SM_TABLE_KEY_MOUNT_PATH).dump();
  }

  if (storage_path_.compare(mount_path))
    return;

  if (disk_status != DISK_INSERT) {
    StopRecord();
  } else {
#ifdef ENABLE_SCHEDULES_SERVER
    SyncSchedules();
#endif
  }
}

void FlowSMProtocol::MediaPathChanged(std::string dbus_data) {
  std::string mount_path;
  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();

  if (!context.size())
    return;

  if (context_str.find(SM_TABLE_KEY_MOUNT_PATH) != std::string::npos) {
    mount_path = context.at(SM_TABLE_KEY_MOUNT_PATH).dump();
  } else {
    return;
  }
  if (!storage_path_.compare(mount_path))
    return;
  ResetPipes();
}

std::string FlowSMProtocol::GetStoragePath(std::string dbus_data) {
  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();
  std::string storage_path = "";
  if (context_str.find(SM_TABLE_KEY_MOUNT_PATH) != std::string::npos) {
    storage_path = context.at(SM_TABLE_KEY_MOUNT_PATH).dump();
  } else {
    LOG_ERROR("GetStoragePath no found %s data:[%s]\n", SM_TABLE_KEY_MOUNT_PATH,
              context_str.c_str());
  }
  return storage_path;
}

std::string FlowSMProtocol::GetMediaPath(std::string dbus_data, int type,
                                         int id) {
  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();
  nlohmann::json scan_paths;
  if (context_str.find(SM_TABLE_KEY_SCAN_PATH) != std::string::npos) {
    scan_paths = context.at(SM_TABLE_KEY_SCAN_PATH);
  } else {
    LOG_ERROR("GetMediaPath no found %s data:[%s]\n", SM_TABLE_KEY_SCAN_PATH,
              context_str.c_str());
  }
  if (context_str.find(SM_TABLE_KEY_MOUNT_PATH) != std::string::npos) {
    storage_path_ = context.at(SM_TABLE_KEY_MOUNT_PATH).dump();
  } else {
    LOG_ERROR("GetMediaPath no found %s data:[%s]\n", SM_TABLE_KEY_MOUNT_PATH,
              context_str.c_str());
  }
  for (auto path = scan_paths.cbegin(); path != scan_paths.cend(); path++) {
    int cam_id = atoi(path->at(SM_TABLE_KEY_CAM_ID).dump().c_str());
    int file_type = atoi(path->at(SM_TABLE_KEY_FILE_TYPE).dump().c_str());
    if (cam_id == id && type == file_type) {
      std::string media_path = path->at(SM_TABLE_KEY_MEDIA_PATH).dump();
      media_path = media_path.replace(media_path.find("\""), 1, "");
      media_path = media_path.replace(media_path.find("\""), 1, "");
      return media_path;
    }
  }
  return "";
}

int FlowSMProtocol::GetDiskStatus(std::string dbus_data) {
  int disk_status = -1;
  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();
  if (context_str.find(SM_TABLE_KEY_STATUS) != std::string::npos) {
    disk_status = atoi(context.at(SM_TABLE_KEY_STATUS).dump().c_str());
  }
  return disk_status;
}

std::string FlowSMProtocol::FreeSizeNotice(std::string dbus_data) {
  LOG_DEBUG("%s\n", (char *)dbus_data.c_str());
  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();
  std::string status = "";
  bool low_space = false;
  if (context_str.find(SM_TABLE_KEY_NO_SPARE_SPACE) != std::string::npos) {
    status = context.at(SM_TABLE_KEY_NO_SPARE_SPACE).dump();
  } else {
    LOG_ERROR("FreeSizeNotice no found %s data:[%s]\n",
              SM_TABLE_KEY_NO_SPARE_SPACE, context_str.c_str());
  }
  low_space = atoi(status.c_str());
  LOG_ERROR("low_space:[%d]\n", low_space);
  if (low_space)
    StopRecord();
#ifdef ENABLE_SCHEDULES_SERVER
  else
    SyncSchedules();
#endif

  return status;
}

std::string FlowSMProtocol::FormatNotice(std::string dbus_data) {
  LOG_DEBUG("%s\n", (char *)dbus_data.c_str());
  nlohmann::json context = nlohmann::json::parse(dbus_data);
  std::string context_str = context.dump();
  std::string status = "";
  bool storage_formating = false;
  if (context_str.find(SM_TABLE_KEY_FORMATING_DISK) != std::string::npos) {
    status = context.at(SM_TABLE_KEY_FORMATING_DISK).dump();
    storage_formating = atoi(status.c_str());
    LOG_WARN("storage_formating:[%d]\n", storage_formating);
  } else {
    LOG_ERROR("FormatNotice no found %s data:[%s]\n",
              SM_TABLE_KEY_NO_SPARE_SPACE, context_str.c_str());
  }

  if (storage_formating) {
    TakePhotoEnableSet(false);
#ifdef ENABLE_SCHEDULES_SERVER
    StopSchedules();
#endif
    StopRecord();
#ifdef ENABLE_DBUS
    SendMediaStorageStopMsg();
#endif
  } else {
    TakePhotoEnableSet(true);
#ifdef ENABLE_SCHEDULES_SERVER
    SyncSchedules();
#endif
  }

  return status;
}

#endif

} // namespace mediaserver
} // namespace rockchip

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_SM_KEY_H_
#define _RK_DBUS_SM_KEY_H_

namespace rockchip {
namespace mediaserver {

enum MediaPathType {
  UNKNOW = -1,
  MEDIA_PATH_VIDEO,
  MEDIA_PATH_PHOTO,
  MEDIA_PATH_SNAP = 3
};

enum DiskStatus { DISK_REMOVE = 0, DISK_INSERT };

#define SM_TABLE_KEY_ID "id"
#define SM_TABLE_KEY_MOUNT_PATH "sMountPath"
#define SM_TABLE_KEY_NAME "sName"
#define SM_TABLE_KEY_DEV "sDev"
#define SM_TABLE_KEY_SCAN_PATH "sScanPath"
#define SM_TABLE_KEY_MEDIA_PATH "sMediaPath"
#define SM_TABLE_KEY_STATUS "iStatus"
#define SM_TABLE_KEY_NO_SPARE_SPACE "iLowFreeStatus"
#define SM_TABLE_KEY_FORMATING_DISK "iFormating"

#define SM_TABLE_KEY_CAM_ID "iCamId"
#define SM_TABLE_KEY_FILE_TYPE "iType"
// TODO FILL

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_DBUS_SM_KEY_H_
// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_ispserver.h"
#include "flow_export.h"
#include "flow_sm_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_ispserver.cpp"

namespace rockchip {
namespace mediaserver {

DBusIspserver::DBusIspserver(DBus::Connection &connection,
                                       const char *adaptor_path,
                                       const char *adaptor_name)
    : DBus::ObjectProxy(connection, adaptor_path, adaptor_name) {}

DBusIspserver::~DBusIspserver() {}

std::string DBusIspserver::GetIspserverExposureDump() {
  return GetDumpExposureInfo();
}

void DBusIspserver::SendIspserverTurnoffSignal() {
  LOG_INFO("cms in\n");
  SendTurnoffIspSignal();
}

} // namespace mediaserver
} // namespace rockchip

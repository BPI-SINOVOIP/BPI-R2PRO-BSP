// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_ISPSERVER_H_
#define _RK_DBUS_ISPSERVER_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_dispatcher.h"
#include "dbus_ispserver_proxy.h"
#include <dbus-c++/dbus.h>

namespace rockchip {
namespace mediaserver {

class DBusIspserver : public rockchip::ispserver::server_proxy,
                           public DBus::IntrospectableProxy,
                           public DBus::ObjectProxy {
public:
  DBusIspserver(DBus::Connection &connection, const char *adaptor_path,
                     const char *adaptor_name);
  virtual ~DBusIspserver();

  std::string GetIspserverExposureDump();
  void SendIspserverTurnoffSignal();

private:
  DBus::Pipe *pipe_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MEDIA_CONTROL_H_

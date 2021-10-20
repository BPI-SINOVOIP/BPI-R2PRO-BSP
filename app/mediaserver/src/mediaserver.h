// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_MEDIA_SERVER_H_
#define _RK_MEIDA_SERVER_H_

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <memory>

#ifdef ENABLE_DBUS
#include "dbus_server.h"
#endif
#include "flow_manager.h"
#ifdef LINK_API_ENABLE
#include "link_manager.h"
#endif
#include "server.h"
#include "thread.h"

namespace rockchip {
namespace mediaserver {

class MediaServer {
public:
  MediaServer();
  virtual ~MediaServer();

#ifdef LINK_API_ENABLE
  int InitMediaLink();
  int DeInitMediaLink();
#endif

#ifdef ENABLE_DBUS
  int InitDbusServer();
  int DeInitDbusServer();
  int RegisterDbusProxy(FlowManagerPtr &flow_manager);
#endif

private:
#ifdef ENABLE_DBUS
  std::unique_ptr<DBusServer> dbus_server_;
#endif
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MEIDA_SERVER_H_

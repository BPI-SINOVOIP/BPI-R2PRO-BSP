// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/prctl.h>

#include "dbus_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_server.cpp"

namespace rockchip {
namespace mediaserver {

DBusServer::DBusServer(bool session, bool need_dbserver)
    : session_(session), need_dbserver_(need_dbserver) {
  LOG_DEBUG("dbus server setup\n");
  flow_manager_ = FlowManager::GetInstance();
  media_control_.reset(new DBusMediaControl());
  RegisteredDBusAdaptor();
}

DBusServer::~DBusServer() {
  UnRegisteredDBusAdaptor();
  media_control_.reset(nullptr);
}

static void *ServerProcess(void *arg) {
  char thread_name[40];
  snprintf(thread_name, sizeof(thread_name), "DBusServer");
  prctl(PR_SET_NAME, thread_name);
  DbusDispatcher::dispatcher_.enter();
  return nullptr;
}

void DBusServer::start(void) {
  LOG_DEBUG("dbus server start\n");
  service_thread_.reset(new Thread(ServerProcess, this));
  service_thread_->set_status(kThreadRunning);
}

void DBusServer::stop(void) {
  DbusDispatcher::dispatcher_.leave();
  service_thread_->set_status(kThreadStopping);
  service_thread_->join();
  LOG_DEBUG("dbus server stop\n");
}

ThreadStatus DBusServer::status(void) {
  if (service_thread_)
    return service_thread_->status();
  else
    return kThreadRunning;
}

int DBusServer::RegisteredDBusAdaptor() {
  DbusDispatcher(100);
  DBus::Connection conn =
      session_ ? DBus::Connection::SessionBus() : DBus::Connection::SystemBus();
  conn.request_name(MEDIA_CONTROL_BUS_NAME);
  media_control_->ConnectDBusServer(conn);
  if (need_dbserver_) {
    dbserver_proxy_ =
        std::make_shared<DBusDbServer>(conn, DBSERVE_PATH, DBSERVE_BUS_NAME);
    dbserver_listen_.reset(new DBusDbListen(conn));
    dbevent_proxy_ =
        std::make_shared<DBusDbEvent>(conn, DBSERVE_PATH, DBSERVE_BUS_NAME);
    dbevent_listen_.reset(new DBusDbEventListen(conn));
    strorage_proxy_ = std::make_shared<DBusStorageManager>(
        conn, STORAGE_MANAGER_PATH, STORAGE_MANAGER_BUS_NAME);
    strorage_listen_.reset(new DBusStorageManagerListen(conn));
    ispserver_proxy_ = std::make_shared<DBusIspserver>(
        conn, ISPSERVER_PATH, ISPSERVER_BUS_NAME);
  }
  return 0;
}

int DBusServer::UnRegisteredDBusAdaptor() {
  if (need_dbserver_) {
    if (strorage_listen_) {
      strorage_listen_.reset(nullptr);
    }
    if (strorage_proxy_) {
      strorage_proxy_.reset();
      strorage_proxy_ = nullptr;
    }
    if (dbevent_listen_) {
      dbevent_listen_.reset(nullptr);
    }
    if (dbserver_listen_) {
      dbserver_listen_.reset(nullptr);
    }
    if (dbevent_proxy_) {
      dbevent_proxy_.reset();
      dbevent_proxy_ = nullptr;
    }
    if (dbserver_proxy_) {
      dbserver_proxy_.reset();
      dbserver_proxy_ = nullptr;
    }
    if (ispserver_proxy_) {
      ispserver_proxy_.reset();
      ispserver_proxy_ = nullptr;
    }
  }
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

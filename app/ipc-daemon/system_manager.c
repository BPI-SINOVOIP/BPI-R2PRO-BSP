// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <gdbus.h>
#include <glib.h>

#include "common.h"
#include "daemon_service.h"
#include "db_manager.h"
#include "json-c/json.h"
#include "system_manager.h"

#define MSG_CMD_ADD_TECH 1
#define ILLEGAL_ATTR -1

static DBusConnection *connection = 0;

int system_export_log(const char *path) {
  char cmd[128] = {'\0'};
  char *cmd_list[] = {
      "cat /tmp/messages",
      "cat /proc/uptime",
      "cat /proc/meminfo",
      "cat /proc/net/snmp",
      "cat /proc/interrupts",
      "top -b -n 1",
      "cat /sys/class/net/eth0/speed",
      "netstat -an",
      "ifconfig -a",
      "route -n",
      "cat /etc/resolv.conf",
      "cat /proc/net/wireless",
  };
  if (!access(path, F_OK)) {
    snprintf(cmd, 127, "rm -f %s", path);
    system(cmd);
  }
  for (int i = 0; i < sizeof(cmd_list) / sizeof(cmd_list[0]); i++) {
    snprintf(cmd, 127, "echo %s >> %s", cmd_list[i], path);
    system(cmd);
    snprintf(cmd, 127, "%s >> %s", cmd_list[i], path);
    system(cmd);
  }
}

int system_upgrade(const char *path) {
  char cmd[128] = {'\0'};

  /* TODO: check firmware */

  if (path)
    snprintf(cmd, 127, "updateEngine --image_url=%s --misc=update --savepath=%s --reboot &", path, path);
  else
    return ILLEGAL_ATTR;

  return system(cmd);
}

DBusMessage *method_reboot(DBusConnection *conn, DBusMessage *msg, void *data) {
  system("reboot");
  return NULL;
}

DBusMessage *method_factory_reset(DBusConnection *conn, DBusMessage *msg,
                                  void *data) {
  system("update factory");
  return NULL;
  /*
      daemon_services_stop();
      db_manager_reset_db();
      system("reboot");
  */
}

DBusMessage *method_export_db(DBusConnection *conn, DBusMessage *msg,
                              void *data) {
  const char *sender;
  DBusMessage *reply;
  DBusMessageIter array;
  const char *str;
  char *json_str;
  json_object *j_cfg;
  json_object *j_ret;

  sender = dbus_message_get_sender(msg);

  dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                        DBUS_TYPE_INVALID);

  j_cfg = json_tokener_parse(json_str);
  j_ret = json_object_new_object();

  char *path =
      (char *)json_object_get_string(json_object_object_get(j_cfg, "sPath"));
  LOGD("export db path = %s\n", path);
  int ret = db_manager_export_db(path);
  json_object_object_add(j_ret, "iReturn", json_object_new_int(ret));
  json_object_object_add(j_ret, "sErrMsg", json_object_new_string(""));

  str = json_object_to_json_string(j_ret);

  reply = dbus_message_new_method_return(msg);
  if (!reply)
    return NULL;

  dbus_message_iter_init_append(reply, &array);
  dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

  json_object_put(j_ret);

  return reply;
}

DBusMessage *method_import_db(DBusConnection *conn, DBusMessage *msg,
                              void *data) {
  const char *sender;
  DBusMessage *reply;
  DBusMessageIter array;
  const char *str;
  char *json_str;
  json_object *j_cfg;
  json_object *j_ret;

  sender = dbus_message_get_sender(msg);

  dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                        DBUS_TYPE_INVALID);

  j_cfg = json_tokener_parse(json_str);
  j_ret = json_object_new_object();

  char *path =
      (char *)json_object_get_string(json_object_object_get(j_cfg, "sPath"));
  LOGD("export db path = %s\n", path);

  daemon_services_stop();
  int ret = db_manager_import_db(path);
  daemon_services_start(SERVICE_CHECK_PERIOD_MS);

  json_object_object_add(j_ret, "iReturn", json_object_new_int(ret));
  json_object_object_add(j_ret, "sErrMsg", json_object_new_string(""));

  str = json_object_to_json_string(j_ret);

  reply = dbus_message_new_method_return(msg);
  if (!reply)
    return NULL;

  dbus_message_iter_init_append(reply, &array);
  dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

  json_object_put(j_ret);

  return reply;
}

DBusMessage *method_export_log(DBusConnection *conn, DBusMessage *msg,
                               void *data) {
  const char *sender;
  DBusMessage *reply;
  DBusMessageIter array;
  const char *str;
  char *json_str;
  json_object *j_cfg;
  json_object *j_ret;

  sender = dbus_message_get_sender(msg);

  dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                        DBUS_TYPE_INVALID);

  j_cfg = json_tokener_parse(json_str);
  j_ret = json_object_new_object();

  char *path =
      (char *)json_object_get_string(json_object_object_get(j_cfg, "sPath"));
  LOGD("export log path = %s\n", path);
  int ret = system_export_log(path);
  json_object_object_add(j_ret, "iReturn", json_object_new_int(ret));
  json_object_object_add(j_ret, "sErrMsg", json_object_new_string(""));

  str = json_object_to_json_string(j_ret);

  reply = dbus_message_new_method_return(msg);
  if (!reply)
    return NULL;

  dbus_message_iter_init_append(reply, &array);
  dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

  json_object_put(j_ret);

  return reply;
}

DBusMessage *method_upgrade(DBusConnection *conn, DBusMessage *msg,
                            void *data) {
  const char *sender;
  DBusMessage *reply;
  DBusMessageIter array;
  const char *str;
  char *json_str;
  json_object *j_cfg;
  json_object *j_ret;

  sender = dbus_message_get_sender(msg);

  dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                        DBUS_TYPE_INVALID);

  j_cfg = json_tokener_parse(json_str);
  j_ret = json_object_new_object();

  char *path =
      (char *)json_object_get_string(json_object_object_get(j_cfg, "sPath"));
  LOGD("export db path = %s\n", path);
  int ret = system_upgrade(path);
  json_object_object_add(j_ret, "iReturn", json_object_new_int(ret));
  if (ret == ILLEGAL_ATTR) {
    json_object_object_add(j_ret, "sErrMsg", json_object_new_string("update path is empty"));
  } else {
    json_object_object_add(j_ret, "sErrMsg", json_object_new_string(""));
  }

  str = json_object_to_json_string(j_ret);

  reply = dbus_message_new_method_return(msg);
  if (!reply)
    return NULL;

  dbus_message_iter_init_append(reply, &array);
  dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

  json_object_put(j_ret);

  return reply;
}

static const GDBusMethodTable server_methods[] = {
    {GDBUS_NOREPLY_METHOD("Reboot", NULL, NULL, method_reboot)},
    {GDBUS_NOREPLY_METHOD("FactoryReset", NULL, NULL, method_factory_reset)},
    {GDBUS_METHOD("ExportDB", GDBUS_ARGS({"json", "s"}),
                  GDBUS_ARGS({"json", "s"}), method_export_db)},
    {GDBUS_METHOD("ImportDB", GDBUS_ARGS({"json", "s"}),
                  GDBUS_ARGS({"json", "s"}), method_import_db)},
    {GDBUS_METHOD("ExportLog", GDBUS_ARGS({"json", "s"}),
                  GDBUS_ARGS({"json", "s"}), method_export_log)},
    {GDBUS_ASYNC_METHOD("Upgrade", GDBUS_ARGS({"json", "s"}),
                        GDBUS_ARGS({"json", "s"}), method_upgrade)},
    {},
};

static const GDBusSignalTable server_signals[] = {
    {GDBUS_SIGNAL("StatusChanged", GDBUS_ARGS({"name", "s"}, {"value", "v"}))},
    {},
};

static int dbus_system_init(void) {
  g_dbus_register_interface(connection, "/", SYSTEM_MANAGER_INTERFACE,
                            server_methods, server_signals, NULL, NULL, NULL);

  return 0;
}

void system_manager_init(void) {
  pthread_t tid;
  DBusError dbus_err;
  DBusConnection *dbus_conn;

  dbus_error_init(&dbus_err);
  dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, SYSTEM_MANAGER, &dbus_err);
  connection = dbus_conn;
  if (!connection) {
    ERROR("%s connect %s fail\n", __func__, SYSTEM_MANAGER);
    return;
  }
  dbus_system_init();
}

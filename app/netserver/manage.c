#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/prctl.h>

#include <glib.h>

#include <pthread.h>
#include <gdbus.h>

#include "json-c/json.h"
#include "manage.h"
#include "dbus.h"
#include "dbus_helpers.h"
#include "netctl.h"
#include "db_monitor.h"
#include "network_func.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "manage.c"

#define MSG_CMD_ADD_TECH   1

static DBusConnection *connection = 0;

static gboolean power_send_changed(char *name, int power)
{
    DBusMessage *signal;
    DBusMessageIter iter;

    signal = dbus_message_new_signal(NETSERVER_PATH,
                                     NETSERVER_INTERFACE, "PowerChanged");
    if (!signal)
        return FALSE;
    LOG_INFO("PowerChanged: name: %s, power: %d\n", name, power);
    json_object *j_obj = json_object_new_object();
    json_object_object_add(j_obj, "name", json_object_new_string(name));
    json_object_object_add(j_obj, "power", json_object_new_int(power));

    char *json_str = (char *)json_object_to_json_string(j_obj);
    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(connection, signal, NULL);
    dbus_message_unref(signal);

    json_object_put(j_obj);
    return FALSE;
}

static DBusMessage *get_config(DBusConnection *conn,
                               DBusMessage *msg, void *data)
{
    const char *sender, *service;
    const char *str;
    DBusMessage *reply;
    DBusMessageIter array;
    dbus_bool_t onoff;
    json_object *j_array = json_object_new_array();
    GList* list = g_list_first(netctl_get_service_list());

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &service,
                          DBUS_TYPE_INVALID);

    if (g_str_equal(service, "")) {
        while (list) {
            struct PropertiesStatus *status = (struct PropertiesStatus *)list->data;
            json_object *j_cfg = json_object_new_object();
            json_object *j_ipv4 = json_object_new_object();
            json_object *j_eth = json_object_new_object();

            if (status->service)
                json_object_object_add(j_cfg, "sService", json_object_new_string(status->service));
            if (status->Nameservers)
                json_object_object_add(j_cfg, "sDNS", json_object_new_string(status->Nameservers));
            if (status->Security)
                json_object_object_add(j_cfg, "sSecurity", json_object_new_string(status->Security));
            if (status->Ethernet.Method)
                json_object_object_add(j_eth, "sNicSpeed", json_object_new_string(status->Ethernet.Method));
            if (status->Ethernet.Interface)
                json_object_object_add(j_eth, "sInterface", json_object_new_string(status->Ethernet.Interface));
            if (status->Ethernet.Address)
                json_object_object_add(j_eth, "sAddress", json_object_new_string(status->Ethernet.Address));
            if (status->IPv4.Method)
                json_object_object_add(j_ipv4, "sV4Method", json_object_new_string(status->IPv4.Method));
            if (status->IPv4.Address)
                json_object_object_add(j_ipv4, "sV4Address", json_object_new_string(status->IPv4.Address));
            if (status->IPv4.Netmask)
                json_object_object_add(j_ipv4, "sV4Netmask", json_object_new_string(status->IPv4.Netmask));
            if (status->IPv4.Gateway)
                json_object_object_add(j_ipv4, "sV4Gateway", json_object_new_string(status->IPv4.Gateway));
            json_object_object_add(j_cfg, "ipv4", j_ipv4);
            json_object_object_add(j_cfg, "link", j_eth);

            json_object *j_db = (json_object *)database_networkservice_json_get(status->service);
            if (j_db)
                json_object_object_add(j_cfg, "dbconfig", j_db);
            json_object_array_add(j_array, j_cfg);
            list = list->next;
        }
    } else {
        while (list) {
            struct PropertiesStatus *status = (struct PropertiesStatus *)list->data;
            if (g_str_equal(status->service, service)) {
                json_object *j_cfg = json_object_new_object();
                json_object *j_ipv4 = json_object_new_object();
                json_object *j_eth = json_object_new_object();

                if (status->service)
                    json_object_object_add(j_cfg, "sService", json_object_new_string(status->service));
                if (status->Nameservers)
                    json_object_object_add(j_cfg, "sDNS", json_object_new_string(status->Nameservers));
                if (status->Security)
                    json_object_object_add(j_cfg, "sSecurity", json_object_new_string(status->Security));
                if (status->Ethernet.Method)
                    json_object_object_add(j_eth, "sNicSpeed", json_object_new_string(status->Ethernet.Method));
                if (status->Ethernet.Interface)
                    json_object_object_add(j_eth, "sInterface", json_object_new_string(status->Ethernet.Interface));
                if (status->Ethernet.Address)
                    json_object_object_add(j_eth, "sAddress", json_object_new_string(status->Ethernet.Address));
                if (status->IPv4.Method)
                    json_object_object_add(j_ipv4, "sV4Method", json_object_new_string(status->IPv4.Method));
                if (status->IPv4.Address)
                    json_object_object_add(j_ipv4, "sV4Address", json_object_new_string(status->IPv4.Address));
                if (status->IPv4.Netmask)
                    json_object_object_add(j_ipv4, "sV4Netmask", json_object_new_string(status->IPv4.Netmask));
                if (status->IPv4.Gateway)
                    json_object_object_add(j_ipv4, "sV4Gateway", json_object_new_string(status->IPv4.Gateway));
                json_object_object_add(j_cfg, "ipv4", j_ipv4);
                json_object_object_add(j_cfg, "link", j_eth);

                json_object *j_db = (json_object *)database_networkservice_json_get(status->service);
                if (j_db)
                    json_object_object_add(j_cfg, "dbconfig", j_db);
                json_object_array_add(j_array, j_cfg);
            }
            list = list->next;
        }
    }

    str = json_object_to_json_string(j_array);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    json_object_put(j_array);
    netctl_free_service_list();

    return reply;
}

static DBusMessage *get_service(DBusConnection *conn,
                                DBusMessage *msg, void *data)
{
    const char *sender, *type;
    const char *str;
    DBusMessage *reply;
    DBusMessageIter array;
    dbus_bool_t onoff;
    json_object *j_array = json_object_new_array();
    GList* list = g_list_first(netctl_get_service_list());

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &type,
                          DBUS_TYPE_INVALID);

    if (g_str_equal(type, "")) {
        while (list) {
            struct PropertiesStatus *status = (struct PropertiesStatus *)list->data;

            json_object *j_cfg = json_object_new_object();
            if (status->service)
                json_object_object_add(j_cfg, "sService", json_object_new_string(status->service));
            if (status->Type)
                json_object_object_add(j_cfg, "sType", json_object_new_string(status->Type));
            if (status->State)
                json_object_object_add(j_cfg, "sState", json_object_new_string(status->State));
            if (status->Name)
                json_object_object_add(j_cfg, "sName", json_object_new_string(status->Name));
            if (status->Security)
                json_object_object_add(j_cfg, "sSecurity", json_object_new_string(status->Security));
            json_object_object_add(j_cfg, "Favorite", json_object_new_int(status->Favorite));
            json_object_object_add(j_cfg, "Strength", json_object_new_int(status->Strength));
            json_object *j_db = (json_object *)database_networkpower_json_get(status->Type);
            if (j_db)
                json_object_object_add(j_cfg, "dbconfig", j_db);

            json_object_array_add(j_array, j_cfg);
            list = list->next;
        }
    } else {
        while (list) {
            struct PropertiesStatus *status = (struct PropertiesStatus *)list->data;

            if (g_str_equal(status->Type, type)) {
                json_object *j_cfg = json_object_new_object();
                if (status->service)
                    json_object_object_add(j_cfg, "sService", json_object_new_string(status->service));
                if (status->Type)
                    json_object_object_add(j_cfg, "sType", json_object_new_string(status->Type));
                if (status->State)
                    json_object_object_add(j_cfg, "sState", json_object_new_string(status->State));
                if (status->Name)
                    json_object_object_add(j_cfg, "sName", json_object_new_string(status->Name));
                if (status->Security)
                    json_object_object_add(j_cfg, "sSecurity", json_object_new_string(status->Security));
                json_object_object_add(j_cfg, "Favorite", json_object_new_int(status->Favorite));
                json_object_object_add(j_cfg, "Strength", json_object_new_int(status->Strength));
                json_object *j_db = (json_object *)database_networkpower_json_get(status->Type);
                if (j_db)
                    json_object_object_add(j_cfg, "dbconfig", j_db);
                json_object_array_add(j_array, j_cfg);
            }
            list = list->next;
        }
    }

    str = json_object_to_json_string(j_array);

    reply = dbus_message_new_method_return(msg);
    if (reply) {
        dbus_message_iter_init_append(reply, &array);
        dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);
    }

    json_object_put(j_array);
    netctl_free_service_list();
    return reply;
}

json_object *get_networkip_json(char *interface)
{
    char *type;
    struct NetworkPower *networkpower;
    json_object *j_cfg;
    json_object *j_ipv4;
    json_object *j_eth;
    char *v4ip = NULL;
    char *mac;
    char *netmask = NULL;
    char *gateway = NULL;
    char *dns1 = NULL;
    char *dns2 = NULL;

    mac = get_local_mac(interface);

    if (mac == NULL)
        return NULL;

    j_cfg = json_object_new_object();
    j_ipv4 = json_object_new_object();
    j_eth = json_object_new_object();

    json_object_object_add(j_eth, "sInterface", json_object_new_string(interface));

    json_object_object_add(j_eth, "sAddress", json_object_new_string(mac));
    g_free(mac);

    netctl_getdns(interface, &dns1, &dns2);

    if (strstr(interface, "eth"))
        type = "ethernet";
    else
        type = "wifi";

    networkpower = database_networkpower_get(type);
    if (networkpower && networkpower->power) {
        if (g_str_equal(type, "ethernet")) {
            struct ethtool_cmd ep;
            char speed[256];
            memset(speed, 0, 256);
            memset(&ep, 0, sizeof(struct ethtool_cmd));
            get_ethernet_tool(interface, &ep);
            get_ethernet_speedsupport(speed, &ep);
            json_object_object_add(j_eth, "sNicSpeedSupport", json_object_new_string(speed));
            memset(speed, 0, 256);
            get_ethernet_speed(speed, &ep);
            json_object_object_add(j_eth, "sNicSpeed", json_object_new_string(speed));
            json_object_object_add(j_eth, "iNicSpeed", json_object_new_int(ep.speed));
            json_object_object_add(j_eth, "iDuplex", json_object_new_int(ep.duplex));

        } else {
            json_object_object_add(j_eth, "sNicSpeed", json_object_new_string(""));
            json_object_object_add(j_eth, "sNicSpeedSupport", json_object_new_string(""));
            json_object_object_add(j_eth, "iNicSpeed", json_object_new_int(-1));
            json_object_object_add(j_eth, "iDuplex", json_object_new_int(-1));
        }
        v4ip = get_local_ip(interface);
        netmask = get_local_netmask(interface);
        gateway = get_gateway(interface);
        json_object_object_add(j_eth, "iPower", json_object_new_int(networkpower->power));
    } else {
        json_object_object_add(j_eth, "iNicSpeed", json_object_new_int(-1));
        json_object_object_add(j_eth, "iDuplex", json_object_new_int(-1));
        json_object_object_add(j_eth, "iPower", json_object_new_int(0));
    }

    if (v4ip) {
        json_object_object_add(j_ipv4, "sV4Address", json_object_new_string(v4ip));
        g_free(v4ip);
    } else {
        json_object_object_add(j_ipv4, "sV4Address", json_object_new_string(""));
    }

    if (netmask) {
        json_object_object_add(j_ipv4, "sV4Netmask", json_object_new_string(netmask));
        g_free(netmask);
    } else {
        json_object_object_add(j_ipv4, "sV4Netmask", json_object_new_string(""));
    }

    if (gateway) {
        json_object_object_add(j_ipv4, "sV4Gateway", json_object_new_string(gateway));
        g_free(gateway);
    } else {
        json_object_object_add(j_ipv4, "sV4Gateway", json_object_new_string(""));
    }

    if (dns1) {
        json_object_object_add(j_eth, "sDNS1", json_object_new_string(dns1));
        g_free(dns1);
    } else {
        json_object_object_add(j_eth, "sDNS1", json_object_new_string(""));
    }

    if (dns2) {
        json_object_object_add(j_eth, "sDNS2", json_object_new_string(dns2));
        g_free(dns2);
    } else {
        json_object_object_add(j_eth, "sDNS2", json_object_new_string(""));
    }

    json_object_object_add(j_cfg, "link", j_eth);
    json_object_object_add(j_cfg, "ipv4", j_ipv4);

    json_object *j_db = (json_object *)database_networkip_json_get(interface);
    if (j_db)
        json_object_object_add(j_cfg, "dbconfig", j_db);

    return j_cfg;
}

void *get_networkip_json_array(char *interface)
{
    json_object *j_array = json_object_new_array();

    if (g_str_equal(interface, "")) {
        GList *list, *values;
        list = values = g_hash_table_get_values(database_hash_network_ip_get());
        while (values) {
            struct NetworkIP *networkip = (struct NetworkIP *)values->data;
            if (networkip != NULL) {
                json_object *j_cfg = get_networkip_json(networkip->interface);
                if (j_cfg)
                    json_object_array_add(j_array, j_cfg);
            }
            values = values->next;
        }
        g_list_free(list);
    } else if (database_networkip_get(interface)) {
        json_object *j_cfg = get_networkip_json(interface);
        json_object_array_add(j_array, j_cfg);
    }

    return j_array;
}

static DBusMessage *get_networkip(DBusConnection *conn,
                                DBusMessage *msg, void *data)
{
    const char *sender;
    char *interface;
    char const *str;
    DBusMessage *reply;
    DBusMessageIter array;
    dbus_bool_t onoff;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &interface,
                          DBUS_TYPE_INVALID);

    json_object *j_array = (json_object *)get_networkip_json_array(interface);
    str = json_object_to_json_string(j_array);
    LOG_INFO("%s, %s\n", __func__, str);
    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    json_object_put(j_array);

    netctl_free_service_list();

    return reply;
}

static DBusMessage *scanwifi(DBusConnection *conn,
                             DBusMessage *msg, void *data)
{
    netctl_wifi_scan();
    return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static const GDBusMethodTable server_methods[] = {
    {
        GDBUS_METHOD("GetService",
        GDBUS_ARGS({ "type", "s" }), GDBUS_ARGS({ "json", "s" }),
        get_service)
    },
    {
        GDBUS_METHOD("GetNetworkIP",
        GDBUS_ARGS({ "type", "s" }), GDBUS_ARGS({ "json", "s" }),
        get_networkip)
    },
    {
        GDBUS_METHOD("GetConfig",
        GDBUS_ARGS({ "service", "s" }), GDBUS_ARGS({ "json", "s" }),
        get_config)
    },
    {
        GDBUS_ASYNC_METHOD("ScanWifi",
        NULL, NULL, scanwifi)
    },
    { },
};

static const GDBusSignalTable server_signals[] = {
    {
        GDBUS_SIGNAL("PowerChanged",
        GDBUS_ARGS({ "name", "s" }, { "value", "v" }))
    },
    { },
};

static int dbus_manager_init(void)
{
    g_dbus_register_interface(connection, "/",
                              NETSERVER_INTERFACE,
                              server_methods,
                              server_signals, NULL, NULL, NULL);

    return 0;
}

void manage_init(void)
{
    pthread_t tid;
    DBusError dbus_err;
    DBusConnection *dbus_conn;

    dbus_error_init(&dbus_err);
    dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NETSERVER, &dbus_err);
    connection = dbus_conn;
    if (!connection) {
        LOG_INFO("%s connect %s fail\n", __func__, NETSERVER);
        return;
    }
    dbus_manager_init();
    netctl_power_change_cb_register(&power_send_changed);
}

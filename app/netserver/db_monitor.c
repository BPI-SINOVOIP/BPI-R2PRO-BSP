#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <glib.h>

#include "json-c/json.h"
#include "netctl.h"
#include "db_monitor.h"
#include "network_func.h"
#include "dbserver.h"
#include "dbus_signal.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "db_monitor.c"

static GHashTable *db_networkpower_hash = NULL;
static GHashTable *db_networkip_hash = NULL;
static GHashTable *db_networkservice_hash = NULL;
static struct NtpCfg *ntp = NULL;

GHashTable *database_hash_network_ip_get(void)
{
    return db_networkip_hash;
}

void *database_networkip_get(char *interface)
{
    void *val = g_hash_table_lookup(db_networkip_hash, interface);

    return val;
}

void *database_networkservice_get(char *service)
{
    void *val = g_hash_table_lookup(db_networkservice_hash, service);

    return val;
}

void *database_networkpower_get(char *type)
{
    void *val = g_hash_table_lookup(db_networkpower_hash, type);

    return val;
}

struct NtpCfg *database_ntp_get(void)
{
    return ntp;
}

void *database_networkservice_json_get(char *service)
{
    struct NetworkService *networkservice = g_hash_table_lookup(db_networkservice_hash, service);
    if (networkservice) {
        json_object *j_cfg = json_object_new_object();

        json_object_object_add(j_cfg, "sPassword", json_object_new_string(networkservice->password));
        json_object_object_add(j_cfg, "iFavorite", json_object_new_int(networkservice->Favorite));
        json_object_object_add(j_cfg, "iAutoconnect", json_object_new_int(networkservice->AutoConnect));

        return (void *)j_cfg;
    }

    return NULL;
}

void *database_networkip_json_get(char *interface)
{
    struct NetworkIP *networkip = g_hash_table_lookup(db_networkip_hash, interface);

    if (networkip) {
        json_object *j_cfg = json_object_new_object();

        json_object_object_add(j_cfg, "sV4Method", json_object_new_string(networkip->IPv4.Method));
        json_object_object_add(j_cfg, "sV4Address", json_object_new_string(networkip->IPv4.Address));
        json_object_object_add(j_cfg, "sV4Netmask", json_object_new_string(networkip->IPv4.Netmask));
        json_object_object_add(j_cfg, "sV4Gateway", json_object_new_string(networkip->IPv4.Gateway));
        json_object_object_add(j_cfg, "sDNS1", json_object_new_string(networkip->dns1));
        json_object_object_add(j_cfg, "sDNS2", json_object_new_string(networkip->dns2));
        json_object_object_add(j_cfg, "sNicSpeed", json_object_new_string(networkip->nicspeed));

        return (void *)j_cfg;
    }

    return NULL;
}

void *database_networkpower_json_get(char *type)
{
    struct NetworkPower *networkpower = g_hash_table_lookup(db_networkpower_hash, type);

    if (networkpower) {
        json_object *j_cfg = json_object_new_object();

        json_object_object_add(j_cfg, "iPower", json_object_new_int(networkpower->power));

        return (void *)j_cfg;
    }

    return NULL;
}

void database_networkservice_remove(char *service)
{
    char *json_str = dbserver_network_service_delete(service);

    if (json_str)
        g_free(json_str);
}

int database_network_config(struct NetworkConfig *config)
{
    GList *list, *values;
    list = values = g_hash_table_get_values(db_networkip_hash);
    while (values) {
        struct NetworkIP *networkip = (struct NetworkIP *)values->data;
        if (networkip != NULL) {
             char *hwaddr = get_local_mac(networkip->interface);
             if (hwaddr && g_str_equal(hwaddr, config->hwaddr)) {
                 if (config->method || config->ip || config->mask || config->gate) {
                     char *json_str = dbserver_network_ipv4_set(networkip->interface, config->method, config->ip, config->mask, config->gate);
                     if (json_str)
                         g_free(json_str);
                 }

                 if (config->dns1 || config->dns2) {
                     char *json_str = dbserver_network_dns_set(networkip->interface, config->dns1, config->dns2);
                     if (json_str)
                         g_free(json_str);
                 }
                 g_list_free(list);

                 return 0;
             }
        }
        values = values->next;
    }
    g_list_free(list);

    return -1;
}

static void updatehash_networkservice(char *service, char *name, void *data)
{
    struct NetworkService *networkservice;

    networkservice = g_hash_table_lookup(db_networkservice_hash, service);

    if (networkservice == NULL) {
        networkservice = malloc(sizeof(struct NetworkService));
        memset(networkservice, 0, sizeof(struct NetworkService));
        networkservice->service = g_strdup(service);
        networkservice->password = g_strdup("");
        g_hash_table_replace(db_networkservice_hash, g_strdup(service), (gpointer)networkservice);
    }

    if (g_str_equal(name, "sPassword")) {
        if (networkservice->password)
            g_free(networkservice->password);
        networkservice->password = g_strdup(data);
    } else if (g_str_equal(name, "iAutoconnect")) {
        networkservice->AutoConnect = *(int *)data;
    } else if (g_str_equal(name, "iFavorite")) {
        networkservice->Favorite = *(int *)data;
    }
}

static void updatehash_network_ip(char *interface, char *name, void *data)
{
    struct NetworkIP *networkip = g_hash_table_lookup(db_networkip_hash, interface);

    if (networkip == NULL) {
        networkip = malloc(sizeof(struct NetworkIP));
        memset(networkip, 0, sizeof(struct NetworkIP));
        networkip->interface = g_strdup(interface);
        networkip->type = g_strdup("");
        networkip->IPv4.Method = g_strdup("");
        networkip->IPv4.Address = g_strdup("");
        networkip->IPv4.Netmask = g_strdup("");
        networkip->IPv4.Gateway = g_strdup("");
        networkip->dns1 = g_strdup("");
        networkip->dns2 = g_strdup("");
        networkip->nicspeed = g_strdup("");
        g_hash_table_replace(db_networkip_hash, g_strdup(interface), (gpointer)networkip);
    }
    if (g_str_equal(name, "sV4Method")) {
        if (networkip->IPv4.Method)
            g_free(networkip->IPv4.Method);
        networkip->IPv4.Method = g_strdup(data);
    } else if (g_str_equal(name, "sV4Address")) {
        if (networkip->IPv4.Address)
            g_free(networkip->IPv4.Address);
        networkip->IPv4.Address = g_strdup(data);
    } else if (g_str_equal(name, "sV4Netmask")) {
        if (networkip->IPv4.Netmask)
            g_free(networkip->IPv4.Netmask);
        networkip->IPv4.Netmask = g_strdup(data);
    } else if (g_str_equal(name, "sV4Gateway")) {
        if (networkip->IPv4.Gateway)
            g_free(networkip->IPv4.Gateway);
        networkip->IPv4.Gateway = g_strdup(data);
    } else if (g_str_equal(name, "sDNS1")) {
        if (networkip->dns1)
            g_free(networkip->dns1);
        networkip->dns1 = g_strdup(data);
    } else if (g_str_equal(name, "sDNS2")) {
        if (networkip->dns2)
            g_free(networkip->dns2);
        networkip->dns2 = g_strdup(data);
    } else if (g_str_equal(name, "sType")) {
        if (networkip->type)
            g_free(networkip->type);
        networkip->type = g_strdup(data);
    } else if (g_str_equal(name, "sNicSpeed")) {
        if (networkip->nicspeed)
            g_free(networkip->nicspeed);
        networkip->nicspeed = g_strdup(data);
    } 
}

static void updatehash_network_power(char *type, char *name, void *data)
{
    struct NetworkPower *networkpower = g_hash_table_lookup(db_networkpower_hash, type);

    if (networkpower == NULL) {
        networkpower = malloc(sizeof(struct NetworkPower));
        memset(networkpower, 0, sizeof(struct NetworkPower));
        networkpower->type = g_strdup(type);
        g_hash_table_replace(db_networkpower_hash, g_strdup(type), (gpointer)networkpower);
    }
    if (g_str_equal(name, "iPower")) {
        networkpower->power = *(int *)data;
    }
}

static void updatentp(char *name, void *data)
{
    if (ntp == NULL) {
        ntp = malloc(sizeof(struct NtpCfg));
        memset(ntp, 0, sizeof(struct NtpCfg));
    }

    if (g_str_equal(name, "sNtpServers")) {
        if (ntp->servers)
            g_free(ntp->servers);
        ntp->servers = g_strdup(data);
    } else if (g_str_equal(name, "sTimeZone")) {
        if (ntp->timezone)
            g_free(ntp->timezone);
        ntp->timezone = g_strdup(data);
    } else if (g_str_equal(name, "sTimeZoneFile")) {
        if (ntp->timezonefile)
            g_free(ntp->timezonefile);
        ntp->timezonefile = g_strdup(data);
    } else if (g_str_equal(name, "sTimeZoneFileDst")) {
        if (ntp->timezonefiledst)
            g_free(ntp->timezonefiledst);
        ntp->timezonefiledst = g_strdup(data);
    } else if (g_str_equal(name, "iAutoDst")) {
        ntp->autodst = *(int *)data;
    } else if (g_str_equal(name, "iAutoMode")) {
        ntp->automode = *(int *)data;
    } else if (g_str_equal(name, "iRefreshTime")) {
        ntp->time = *(int *)data;
    }
}

static int networkservice_get(void)
{
    char *json_str = dbserver_network_service_get(NULL);

    if (json_str) {
        json_object *j_array;
        json_object *j_ret;

        j_ret = json_tokener_parse(json_str);
        j_array = json_object_object_get(j_ret, "jData");
        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);
            char *service = (char *)json_object_get_string(json_object_object_get(j_obj, "sService"));
            json_object_object_foreach(j_obj, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatehash_networkservice(service, key, data);
            }
        }
        json_object_put(j_ret);
        g_free(json_str);
        return 0;
    }

    return -1;
}

static int networkip_get(void)
{
    char *json_str = dbserver_network_ip_get(NULL);

    if (json_str) {
        json_object *j_array;
        json_object *j_ret;

        j_ret = json_tokener_parse(json_str);
        j_array = json_object_object_get(j_ret, "jData");

        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);
            char *interface = (char *)json_object_get_string(json_object_object_get(j_obj, "sInterface"));

            json_object_object_foreach(j_obj, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatehash_network_ip(interface, key, data);
            }
        }
        json_object_put(j_ret);
        g_free(json_str);
        return 0;
    }

    return -1;
}

static int networkpower_get(void)
{
    char *json_str = dbserver_network_power_get(NULL);

    if (json_str) {
        json_object *j_array;
        json_object *j_ret;

        j_ret = json_tokener_parse(json_str);
        j_array = json_object_object_get(j_ret, "jData");

        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);
            char *type = (char *)json_object_get_string(json_object_object_get(j_obj, "sType"));

            json_object_object_foreach(j_obj, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatehash_network_power(type, key, data);
            }
        }
        json_object_put(j_ret);
        g_free(json_str);
        return 0;
    }

    return -1;
}

static int ntp_get(void)
{
    char *json_str = dbserver_ntp_get();

    if (json_str) {
        json_object *j_array;
        json_object *j_ret;

        j_ret = json_tokener_parse(json_str);
        j_array = json_object_object_get(j_ret, "jData");
        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_data = json_object_array_get_idx(j_array, i);
            json_object_object_foreach(j_data, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatentp(key, data);
            }
        }
        json_object_put(j_ret);
        g_free(json_str);
        return 0;
    }

    return -1;
}

static void deletehash_networkservice(char *service)
{
    struct NetworkService *networkservice = g_hash_table_lookup(db_networkservice_hash, service);
    if (networkservice) {
        g_hash_table_remove(db_networkservice_hash, service);
        if (networkservice->service)
            g_free(networkservice->service);
        if (networkservice->password)
            g_free(networkservice->password);
        g_free(networkservice);
    }
}

void signal_network_datachanged(void *user_data)
{
    json_object *j_cfg;
    json_object *j_key = 0;
    json_object *j_data = 0;
    char *table = 0;
    char *json_str = (char *)user_data;

    j_cfg = json_tokener_parse(json_str);

    table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    j_key = json_object_object_get(j_cfg, "key");
    j_data = json_object_object_get(j_cfg, "data");

    if (g_str_equal(table, TABLE_NETWORK_IP)) {
        char *interface = (char *)json_object_get_string(json_object_object_get(j_key, "sInterface"));
        char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));

        if (g_str_equal(cmd, "Delete")) {

        } else if (g_str_equal(cmd, "Update")) {
            json_object_object_foreach(j_data, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatehash_network_ip(interface, key, data);
            }
            SyncAllNetconfig();
        }
    } else if (g_str_equal(table, TABLE_NETWORK_POWER)) {
        char *type = (char *)json_object_get_string(json_object_object_get(j_key, "sType"));
        char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));

        if (g_str_equal(cmd, "Delete")) {

        } else if (g_str_equal(cmd, "Update")) {
            json_object_object_foreach(j_data, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatehash_network_power(type, key, data);
                if (g_str_equal(key, "iPower"))
                    netctl_set_power(type, tmp);
            }
        }
    } else if (g_str_equal(table, TABLE_NETWORK_SERVICE)) {
        char *service = (char *)json_object_get_string(json_object_object_get(j_key, "sService"));
        char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));

        if (g_str_equal(cmd, "Delete")) {
            deletehash_networkservice(service);
        } else if (g_str_equal(cmd, "Update")) {
            json_object_object_foreach(j_data, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatehash_networkservice(service, key, data);
            }
            ConnectService(service);
        }
    } else if (g_str_equal(table, "ntp")) {
        char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));
        if (g_str_equal(cmd, "Update")) {
            json_object_object_foreach(j_data, key, val) {
                void *data;
                int tmp;
                if (json_object_get_type(val) == json_type_int) {
                    tmp = (int)json_object_get_int(val);
                    data = (void *)&tmp;
                } else
                    data = (void *)json_object_get_string(val);
                updatentp(key, data);
            }
            syncntp();
            synczone();
        }
    }
    json_object_put(j_cfg);
}

void database_init(void)
{
    disable_loop();
    while (networkservice_get() != 0) {
        LOG_INFO("dbserver_networkservice_get, wait dbserver.\n");
        usleep(50000);
    }

    while (networkip_get() != 0) {
        LOG_INFO("dbserver_networkip_get, wait dbserver.\n");
        usleep(50000);
    }

    while (networkpower_get() != 0) {
        LOG_INFO("dbserver_networkpower_get, wait dbserver.\n");
        usleep(50000);
    }

    while (ntp_get() != 0) {
        LOG_INFO("dbserver_ntp_get, wait dbserver.\n");
        usleep(50000);
    }
    dbus_monitor_signal_registered(DBSERVER_NET_INTERFACE, DS_SIGNAL_DATACHANGED, &signal_network_datachanged);
}

void database_hash_init(void)
{
    db_networkpower_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                          g_free, NULL);
    db_networkip_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                          g_free, NULL);
    db_networkservice_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              g_free, NULL);
}

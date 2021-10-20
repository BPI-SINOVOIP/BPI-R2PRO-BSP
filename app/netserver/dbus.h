#ifndef __DBUS_H__
#define __DBUS_H__

#define NETSERVER   "rockchip.netserver"
#define NETSERVER_PATH      "/"
#define NETSERVER_INTERFACE    NETSERVER ".server"

#define ERROR_INTERFACE     NETSERVER ".Error"

DBusConnection *connman_dbus_get_connection(void);

void connman_dbus_property_append_basic(DBusMessageIter *iter,
                                        const char *key, int type, void *val);

void connman_dbus_property_append_fixed_array(DBusMessageIter *iter,
                                              const char *key, int type, void *val, int len);

void dbus_property_append_basic(DBusMessageIter *iter,
                                const char *key, int type, void *val);

DBusMessage *__error_invalid_arguments(DBusMessage *msg);

static inline void connman_dbus_dict_open(DBusMessageIter *iter,
                                          DBusMessageIter *dict)
{
    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
                                     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                                     DBUS_DICT_ENTRY_END_CHAR_AS_STRING, dict);
}

static inline void connman_dbus_dict_open_variant(DBusMessageIter *iter,
                                                  DBusMessageIter *dict)
{
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     DBUS_TYPE_ARRAY_AS_STRING
                                     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                                     DBUS_DICT_ENTRY_END_CHAR_AS_STRING, dict);
}

static inline void connman_dbus_array_open(DBusMessageIter *iter,
                                           DBusMessageIter *dict)
{
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     DBUS_TYPE_ARRAY_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING,
                                     dict);
}

static inline void connman_dbus_dict_close(DBusMessageIter *iter,
                                           DBusMessageIter *dict)
{
    dbus_message_iter_close_container(iter, dict);
}

static inline void connman_dbus_dict_append_basic(DBusMessageIter *dict,
                                                  const char *key, int type, void *val)
{
    DBusMessageIter entry;

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY,
                                     NULL, &entry);
    connman_dbus_property_append_basic(&entry, key, type, val);
    dbus_message_iter_close_container(dict, &entry);
}

#endif
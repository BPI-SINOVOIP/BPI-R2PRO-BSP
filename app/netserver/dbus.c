#include <string.h>
#include <errno.h>
#include <gdbus.h>

#include "dbus.h"

dbus_bool_t connman_dbus_validate_ident(const char *ident)
{
    unsigned int i;

    if (!ident)
        return FALSE;

    for (i = 0; i < strlen(ident); i++) {
        if (ident[i] >= '0' && ident[i] <= '9')
            continue;
        if (ident[i] >= 'a' && ident[i] <= 'z')
            continue;
        if (ident[i] >= 'A' && ident[i] <= 'Z')
            continue;
        return FALSE;
    }

    return TRUE;
}

char *connman_dbus_encode_string(const char *value)
{
    GString *str;
    unsigned int i, size;

    if (!value)
        return NULL;

    size = strlen(value);

    str = g_string_new(NULL);
    if (!str)
        return NULL;

    for (i = 0; i < size; i++) {
        const char tmp = value[i];
        if ((tmp < '0' || tmp > '9') && (tmp < 'A' || tmp > 'Z') &&
            (tmp < 'a' || tmp > 'z'))
            g_string_append_printf(str, "_%02x", tmp);
        else
            str = g_string_append_c(str, tmp);
    }

    return g_string_free(str, FALSE);
}

DBusMessage *__error_invalid_arguments(DBusMessage *msg)
{
    return g_dbus_create_error(msg, ERROR_INTERFACE
                               ".InvalidArguments", "Invalid arguments");
}

void connman_dbus_property_append_basic(DBusMessageIter *iter,
                                        const char *key, int type, void *val)
{
    DBusMessageIter value;
    const char *signature;

    dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &key);

    switch (type) {
    case DBUS_TYPE_BOOLEAN:
        signature = DBUS_TYPE_BOOLEAN_AS_STRING;
        break;
    case DBUS_TYPE_STRING:
        signature = DBUS_TYPE_STRING_AS_STRING;
        break;
    case DBUS_TYPE_BYTE:
        signature = DBUS_TYPE_BYTE_AS_STRING;
        break;
    case DBUS_TYPE_UINT16:
        signature = DBUS_TYPE_UINT16_AS_STRING;
        break;
    case DBUS_TYPE_INT16:
        signature = DBUS_TYPE_INT16_AS_STRING;
        break;
    case DBUS_TYPE_UINT32:
        signature = DBUS_TYPE_UINT32_AS_STRING;
        break;
    case DBUS_TYPE_INT32:
        signature = DBUS_TYPE_INT32_AS_STRING;
        break;
    case DBUS_TYPE_UINT64:
        signature = DBUS_TYPE_UINT64_AS_STRING;
        break;
    case DBUS_TYPE_INT64:
        signature = DBUS_TYPE_INT64_AS_STRING;
        break;
    case DBUS_TYPE_OBJECT_PATH:
        signature = DBUS_TYPE_OBJECT_PATH_AS_STRING;
        break;
    default:
        signature = DBUS_TYPE_VARIANT_AS_STRING;
        break;
    }

    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     signature, &value);
    dbus_message_iter_append_basic(&value, type, val);
    dbus_message_iter_close_container(iter, &value);
}

void connman_dbus_property_append_fixed_array(DBusMessageIter *iter,
                                              const char *key, int type, void *val, int len)
{
    DBusMessageIter value, array;
    const char *variant_sig, *array_sig;

    switch (type) {
    case DBUS_TYPE_BYTE:
        variant_sig = DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_BYTE_AS_STRING;
        array_sig = DBUS_TYPE_BYTE_AS_STRING;
        break;
    default:
        return;
    }

    dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &key);

    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     variant_sig, &value);

    dbus_message_iter_open_container(&value, DBUS_TYPE_ARRAY,
                                     array_sig, &array);
    dbus_message_iter_append_fixed_array(&array, type, val, len);
    dbus_message_iter_close_container(&value, &array);

    dbus_message_iter_close_container(iter, &value);
}

void dbus_property_append_basic(DBusMessageIter *iter,
                                const char *key, int type, void *val)
{
    DBusMessageIter value;
    const char *signature;

    dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &key);

    switch (type) {
    case DBUS_TYPE_BOOLEAN:
        signature = DBUS_TYPE_BOOLEAN_AS_STRING;
        break;
    case DBUS_TYPE_STRING:
        signature = DBUS_TYPE_STRING_AS_STRING;
        break;
    case DBUS_TYPE_BYTE:
        signature = DBUS_TYPE_BYTE_AS_STRING;
        break;
    case DBUS_TYPE_UINT16:
        signature = DBUS_TYPE_UINT16_AS_STRING;
        break;
    case DBUS_TYPE_INT16:
        signature = DBUS_TYPE_INT16_AS_STRING;
        break;
    case DBUS_TYPE_UINT32:
        signature = DBUS_TYPE_UINT32_AS_STRING;
        break;
    case DBUS_TYPE_INT32:
        signature = DBUS_TYPE_INT32_AS_STRING;
        break;
    case DBUS_TYPE_UINT64:
        signature = DBUS_TYPE_UINT64_AS_STRING;
        break;
    case DBUS_TYPE_INT64:
        signature = DBUS_TYPE_INT64_AS_STRING;
        break;
    case DBUS_TYPE_OBJECT_PATH:
        signature = DBUS_TYPE_OBJECT_PATH_AS_STRING;
        break;
    default:
        signature = DBUS_TYPE_VARIANT_AS_STRING;
        break;
    }

    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     signature, &value);
    dbus_message_iter_append_basic(&value, type, val);
    dbus_message_iter_close_container(iter, &value);
}
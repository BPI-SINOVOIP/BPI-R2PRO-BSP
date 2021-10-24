/****************************************************************************
*
*    Copyright (c) 2017 - 2019 by Rockchip Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Rockchip Corporation. This is proprietary information owned by
*    Rockchip Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Rockchip Corporation.
*
*****************************************************************************/

#ifndef _ROCKX_CONFIG_UTIL_H
#define _ROCKX_CONFIG_UTIL_H

#include "../rockx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Config Key of rockx data path
 */
#define ROCKX_CONFIG_DATA_PATH "ROCKX_DATA_PATH"

/**
 * @brief Config Key of rockx bin path
 */
#define ROCKX_CONFIG_BIN_PATH "ROCKX_BIN_PATH"

/**
 * @brief Config Key of rockx target device id
 */
#define ROCKX_CONFIG_TARGET_DEVICE_ID "ROCKX_TARGET_DEVICE_ID"

/**
 * @brief Max number of config item
 */
#define ROCKX_CONFIG_MAX_ITEM 8

/**
 * @brief Max size of config key
 */
#define ROCKX_CONFIG_KEY_MAX 32

/**
 * @brief Max size of config value
 */
#define ROCKX_CONFIG_VALUE_MAX 256


/**
 * @brief Congfig item
 */
typedef struct rockx_config_item_t {
    char key[ROCKX_CONFIG_KEY_MAX];     ///< Key
    char value[ROCKX_CONFIG_VALUE_MAX]; ///< Value
} rockx_config_item_t;

/**
 * @brief Congfig
 */
typedef struct rockx_config_t {
    rockx_config_item_t configs[ROCKX_CONFIG_MAX_ITEM];
    int count;
} rockx_config_t;

/// Create a rockx_config_t
/// \return pointer of @ref rockx_config_t
rockx_config_t *rockx_create_config();

/// Release rockx_config_t
/// \param config [in] pointer of @ref rockx_config_t
/// \return @ref rockx_ret_t
rockx_ret_t rockx_release_config(rockx_config_t *config);

/// Add a config item to rockx_config_t
/// \param config [in] pointer of @ref rockx_config_t
/// \param key [in] config key
/// \param value [in] config value
/// \return @ref rockx_ret_t
rockx_ret_t rockx_add_config(rockx_config_t *config, char *key, char *value);

/// Get a config item value of rockx_config_t
/// \param config [in] pointer of @ref rockx_config_t
/// \param key [in] config key
/// \param value [in] config value
/// \return @ref rockx_ret_t
rockx_ret_t rockx_get_config(rockx_config_t *config, char *key, char *value);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //_ROCKX_CONFIG_UTIL_H

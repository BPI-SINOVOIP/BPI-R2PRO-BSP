/*
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Bin Yang <yangbin@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>

#define   PCBA_TEST_PLATFORM_PX             "PX3SE"
#define   PCBA_TEST_PLATFORM_ECHO           "ECHO"
#define   PCBA_TEST_PLATFORM_CVR            "CVR"
#define   PCBA_TEST_PLATFORM_GVA            "GVA"
#define   PCBA_TEST_PLATFORM_3308           "3308"

typedef enum
{
    /* format parse*/
    CMD_CHK_OK          =  0,
    CMD_TYPE_ERR        = -1,
    TEST_ITEM_ERR       = -2,
    CMD_ERR             = -3,
    MSG_ERR             = -4,
    SEND_FORMAT_ERR     = -5,
    TCP_SEND_ERR        = -6,
    QUERY_RESULT_ERR    = -7,
    RECV_FORMAT_ERR     = -8,
    CMD_OVERLONG        = -9,

    /* process  parse */
    TEST_MODE_ERR       = -10,
    ENTER_TEST_ERR      = -11,
    EXIT_TEST_ERR       = -12,
    TEST_ITEM_BUSY      = -13,
    STOP_TEST_ERR       = -14,
    SAVE_RESULE_ERR     = -15,
    TEST_FORK_ERR       = -16,
    TEST_RESULT_EXIST   = -17,

    /* write storage */
    WRITE_VENDOR_ERR    = -20,
    READ_VENDOR_ERR     = -21,
    /* ptz  test */
    PTZ_TEST_ERR        = -30,
    /* key test */
    KEY_OPEN_FAIL       = -40,
    KEY_EVENT_TIMEOUT   = -41,
    KEY_PROC_ERR        = -42,
    KEY_QUERY_FAIL      = -43,
    /* ROTARY test */
    ROTARY_OPEN_FAIL    = -50,
    ROTARY_EVENT_TIMEOUT = -51,
    ROTARY_QUERY_FAIL   = -52,
    ROTARY_PROC_ERR     = -53,
//	/* light test */
//	LIGHT_PATH_ERR = -50,
//	LIGHT_DEV_ERR = -51,
//	/* ircut test */
//	IRCUT_STATE_ERR = -60,
//	IRCUT_OPEN_ERR	= -61,
//	IRCUT_SET_ERR	= -62,
    /* sdcard test */
    SDCARD_SIZE_ERR     = -70,
    SDCARD_WRITE_ERR    = -71,
    SDCARD_READ_ERR     = -72,
    SDCARD_UNKNOWN_ERR  = -73,
    SDCARD_TIMEOUT_ERR  = -74,
    SDCARD_MOUNT_ERR    = -75,
    /* audio test */
    AUDIO_PLAY_ERR      = -80,
    AUDIO_RECORD_ERR    = -81,
    AUDIO_TYPE_ERR      = -82,
    /* wifi test */
    WIFI_RES_FILE_ERR   = -90,
    WIFI_RUN_ERR        = -91,
    WIFI_PROC_ERR       = -92,
    WIFI_UNKNOWN_ERR    = -93,

    /*usb host test*/
    USBHOST_UNKNOWN_ERR = -100,
    USBHOST_TIMEOUT_ERR = -101,
    USBHOST_MOUNT_ERR   = -102,

    /*infrard ray test*/
    IR_OPEN_FAIL        = -110,
    IR_EVENT_TIMEOUT    = -111,
    IR_QUERY_FAIL       = -112,

    /*audio line in test*/
    AUDIO_LINEIN_TIMEOUT = -120,

} RET_STA;

typedef enum _SDCARD_EVENT_ID
{
    SDCARD_NOT_EVENT        = 0,
    SDCARD_UNMOUNT_EVENT    = 1,
    SDCARD_MOUNT_EVENT      = 2,
    SDCARD_MOUNT_NOTFIT     = 3,
    SDCARD_MOUNT_FAIL       = 4,
} SDCARD_EVENT_ID;

typedef enum _HDMI_EVENT_ID
{
    HDMI_NOT_EVENT          = 0,
    HDMI_IN_EVENT           = 1,
    HDMI_OUT_EVENT          = 2,
} HDMI_EVENT_ID;

#define TEXT_SIZE           10
struct shared_use_st
{
    int written; //作为一个标志，非0表示可读，0表示可写
    char text[TEXT_SIZE];
};


#define COMMAND_NAMESIZE    20
#define COMMAND_VALUESIZE   100

#define FACTORYMODE "/tmp/testMode"

#define SDCARD_PATH             "/mnt/sdcard"
#define TEST_RESULT_SAVE_PATH   "/tmp"    //IPC使用这个
//#define TEST_RESULT_SAVE_PATH "/data/cfg/rk_pcba_test"  //echo使用这个路径

#if 0
    #define PCBA_TEST_PATH SDCARD_PATH
#else
    //#define PCBA_TEST_PATH "/tmp"    //IPC使用这个
    //#define PCBA_TEST_PATH "/data/cfg/rk_pcba_test"  //echo使用这个路径
    #define PCBA_TEST_PATH          "/tmp/factory/"  //echo使用这个路径

#endif
#define MANUAL_TEST_TIMEOUT     60000000   //人工测试项60s超时

#define RESULT_TESTING          "TESTING"
#define RESULT_PASS             "PASS"
#define RESULT_FAIL             "FAIL"
#define RESULT_VERIFY           "VERIFY"

#define RESULT_KEY_PRESS        "PRESS"

#define TESTITEM_SEND_PARSE	    "%*[^<]<%[^>]>,<%[^>]>,<%d>"
#define TESTITEM_SEND_PARSE_NOMSG "%*[^<]<>,<%[^>]>,<%d>"
#define TESTITEM_SEND_HEAD      "[tcp_server]"
#define TESTITEM_SEND_FORMAT    "%s<%s>,<%s>,<%d>\n"

#define send_msg_to_server(msg, result, err_code) \
    printf(TESTITEM_SEND_FORMAT, TESTITEM_SEND_HEAD, msg, result, err_code)
#define send_msg_to_buf(buf, msg, result, err_code) \
    snprintf(buf, COMMAND_VALUESIZE, TESTITEM_SEND_FORMAT, TESTITEM_SEND_HEAD, msg, result, err_code)


#define LOG_DEBUG_LEVEL         (1)
#define LOG_ERROR_FLAG          (4)
#define LOG_WARING_FLAG         (3)
#define LOG_INFO_FLAG           (2)
#define LOG_DEBUG_FLAG          (1)

#define LOG_PRINTF(level, format, ...) \
    do { \
        if (level > LOG_DEBUG_LEVEL) { \
            printf("[%s]: "format, LOG_TAG, ##__VA_ARGS__); \
        } \
    } while(0)
#define log_info(format, ...) LOG_PRINTF(LOG_INFO_FLAG, format, ##__VA_ARGS__)
#define log_dbg(format, ...) LOG_PRINTF(LOG_DEBUG_FLAG, format, ##__VA_ARGS__)
#define log_warn(format, ...) LOG_PRINTF(LOG_WARING_FLAG, format, ##__VA_ARGS__)
#define log_err(format, ...) LOG_PRINTF(LOG_ERROR_FLAG, format, ##__VA_ARGS__)

#endif

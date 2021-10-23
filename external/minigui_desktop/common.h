/*
 * Rockchip App
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#define _ID_TIMER_MAIN                  100
#define _ID_TIMER_DESKTOP               101
#define _ID_TIMER_AUDIOPLAY             102
#define _ID_TIMER_BROWSER               103
#define _ID_TIMER_PICPREVIEW            104
#define _ID_TIMER_VIDEOPLAY             105
#define _ID_TIMER_SETTING               106
#define _ID_TIMER_SETTING_VERSION       107
#define _ID_TIMER_SETTING_LANGUAGE      108
#define _ID_TIMER_SETTING_EQ            109
#define _ID_TIMER_SETTING_BACKLIGHT     110
#define _ID_TIMER_SETTING_SCREENOFF     111
#define _ID_TIMER_VIDEOPLAY_HW          112
#define _ID_TIMER_SETTING_WIFI          113
#define _ID_TIMER_SETTING_THEMESTYLE    114
#define _ID_TIMER_SETTING_SYSTEMTIME    115
#define _ID_TIMER_LOWPOWER              116
#define _ID_TIMER_SETTING_VOLUME        117
#define _ID_TIMER_SETTING_RECOVERY      118
#define _ID_TIMER_SETTING_AIRKISS       119
#define _ID_TIMER_SETTING_GENERAL       120
#define _ID_TIMER_INPUT       121

#define TIMING_NUM  3

#define MSG_VIDEOPLAY_END         (MSG_USER + 1)
#define MSG_MEDIA_UPDATE          (MSG_USER + 2)

//500ms
#define TIMER_MAIN                  100
#define TIMER_DESKTOP               50
#define TIMER_AUDIOPLAY             50
#define TIMER_BROWSER               50
#define TIMER_PICPREVIEW            5
#define TIMER_VIDEOPLAY             50
#define TIMER_SETTING               50
#define TIMER_SETTING_VERSION       50
#define TIMER_SETTING_LANGUAGE      50
#define TIMER_SETTING_EQ            50
#define TIMER_SETTING_BACKLIGHT     50
#define TIMER_SETTING_SCREENOFF     50
#define TIMER_VIDEOPLAY_HW          50
#define TIMER_SETTING_WIFI          20
#define TIMER_SETTING_THEMESTYLE    50
#define TIMER_SETTING_VOLUME        50
#define TIMER_SETTING_RECOVERY      50
#define TIMER_SETTING_SYSTEMTIME    50
#define TIMER_SETTING_AIRKISS       50
#define TIMER_SETTING_GENERAL       50
#define TIMER_LOWPOWER              50
#define TIMER_INPUT                 50


enum RES_STR_ID
{
    RES_STR_RES = 0,
    RES_STR_TITLE_GAME,
    RES_STR_TITLE_MUSIC,
    RES_STR_TITLE_PIC,
    RES_STR_TITLE_VIDEO,
    RES_STR_TITLE_BROWSER,
    RES_STR_TITLE_SETTING,
    RES_STR_TITLE_GENERAL,
    RES_STR_TITLE_WIFI,
    RES_STR_TITLE_AIRKISS,
    RES_STR_TITLE_SYSTEMTIME,
    RES_STR_TITLE_RESTORE,
    RES_STR_TITLE_INFO,
    RES_STR_TITLE_LANGUAGE,
    RES_STR_TITLE_VOLUME,
    RES_STR_TITLE_THEMESTYLE,
    RES_STR_TITLE_SCREENOFF,
    RES_STR_TITLE_BACKLIGHT,
    RES_STR_SHUTDOWN,
    RES_STR_LANGUAGE_CN,
    RES_STR_LANGUAGE_EN,
    RES_STR_LANGUAGE_JP,
    RES_STR_LANGUAGE_KO,
    RES_STR_EQ_1,
    RES_STR_EQ_2,
    RES_STR_EQ_3,
    RES_STR_EQ_4,
    RES_STR_EQ_5,
    RES_STR_SCREENOFF_1,
    RES_STR_SCREENOFF_2,
    RES_STR_SCREENOFF_3,
    RES_STR_SCREENOFF_4,
    RES_STR_SCREENOFF_5,
    RES_STR_SCREENOFF_6,
    RES_STR_BACKLIGHT_1,
    RES_STR_BACKLIGHT_2,
    RES_STR_BACKLIGHT_3,
    RES_STR_BACKLIGHT_4,
    RES_STR_INFO_MODEL,
    RES_STR_INFO_UDISKCAP,
    RES_STR_INFO_UDISKAVACAP,
    RES_STR_INFO_VERSION,
    RES_STR_SYSTEM_UPGRAD,
    RES_STR_WIFI_CONNECTION,
    RES_STR_ENABLE,
    RES_STR_DISABLE,
    RES_STR_WARNING,
    RES_STR_WARNING_RECOVERY,
    RES_STR_YES,
    RES_STR_NO,
    RES_STR_OK,
    RES_STR_CANCEL,
    RES_STR_LOCAL,
    RES_STR_SDCARD,
    RES_STR_UDISK,
    RES_STR_NO_CONTENT,
    RES_STR_THEMESTYLE_THEME1,
    RES_STR_THEMESTYLE_THEME2,
    RES_STR_SYNC_NET_TIME,
    RES_STR_SYSTEMTIME_DATA,
    RES_STR_SYSTEMTIME_TIME,
    RES_STR_SYSTEMTIME_FORMAT,
    RES_STR_SYSTEMTIME_ON1,
    RES_STR_SYSTEMTIME_OFF1,
    RES_STR_SYSTEMTIME_ON2,
    RES_STR_SYSTEMTIME_OFF2,
    RES_STR_SYSTEMTIME_ON3,
    RES_STR_SYSTEMTIME_OFF3,
    RES_STR_SYSTEMTIME_FORMAT_24,
    RES_STR_SYSTEMTIME_FORMAT_12,
    RES_STR_LOWPOWER,
    RES_STR_POWEROFF,
    RES_STR_CONNECTED,
    RES_STR_CONNECTING,
    RES_STR_WRONG_PWD,
    RES_STR_TOO_SHORT,
    RES_STR_WIFI_SSID,
    RES_STR_WIFI_PWD,
    RES_STR_SCANING,
    RES_STR_NO_UPDATE_FILE,
    RES_STR_RECOVERY_SOON,
    RES_STR_MAX
};

enum filter_filetype
{
    FILTER_FILE_NO = 0,
    FILTER_FILE_MUSIC = 1,
    FILTER_FILE_GAME = 2,
    FILTER_FILE_PIC = 3,
    FILTER_FILE_ZIP = 4,
    FILTER_FILE_VIDEO = 5
};

enum filetype
{
    FILE_FOLDER = 0,
    FILE_MUSIC = 1,
    FILE_GAME = 2,
    FILE_PIC = 3,
    FILE_ZIP = 4,
    FILE_VIDEO = 5,
    FILE_OTHER = 6,
    FILE_TYPE_MAX
};

enum languagetype
{
    LANGUAGE_CH = 0,
    LANGUAGE_EN,
    LANGUAGE_JA,
    LANGUAGE_KO,
    LANGUAGE_MAX
};

struct file_node
{
    struct file_node *pre_node;
    struct file_node *next_node;
    char *name;
    int type;
};

struct directory_node
{
    struct directory_node *pre_node;
    struct directory_node *next_node;
    struct file_node *file_node_list;
    char *patch;
    int total;
    int file_sel;
};

enum MEDIA_CMD
{
    MEDIA_CMD_CUR_TIME = 0,
    MEDIA_CMD_TOTAL_TIME,
    MEDIA_CMD_END,
    MEDIA_CMD_QUIT,
    MEDIA_CMD_READY
};

typedef struct
{
    int x;
    int y;
} touch_pos;

typedef struct
{
    int status;
    int timing;
} rtc_timing;

struct wifi_info
{
    char ssid[64];
    char psk[64];
};

struct wifi_avaiable
{
    char ssid[64];
    int rssi;
};

enum INPUT_TYPE
{
    WIFI_PWD = 1,
    SYSTEMTIME_DATE,
};

#if 1
#define BROWSER_PATH_ROOT     "/oem/file"
#define BROWSER_PATH_PIC      "/oem/file/pic"
#define BROWSER_PATH_MUSIC    "/oem/file/music"
#define BROWSER_PATH_GAME     "/oem/file/game"
#define BROWSER_PATH_VIDEO    "/oem/file/video"
#define SDCARD_PATH_ROOT      "/sdcard"
#define SDCARD_PATH_PIC       "/sdcard/pic"
#define SDCARD_PATH_MUSIC     "/sdcard/music"
#define SDCARD_PATH_GAME      "/sdcard/game"
#define SDCARD_PATH_VIDEO     "/sdcard/video"
#define UDISK_PATH_ROOT       "/udisk"
#define UDISK_PATH_PIC        "/udisk/pic"
#define UDISK_PATH_MUSIC      "/udisk/music"
#define UDISK_PATH_GAME       "/udisk/game"
#define UDISK_PATH_VIDEO      "/udisk/video"
#else
#define BROWSER_PATH_ROOT     "/sdcard"
#define BROWSER_PATH_PIC      "/sdcard/pic"
#define BROWSER_PATH_MUSIC    "/sdcard/music"
#define BROWSER_PATH_GAME     "/sdcard/game"
#define BROWSER_PATH_VIDEO    "/sdcard/video"
#endif

#define REC_FILE_CN    "/usr/local/share/minigui/res/string/CN-UTF8.bin"
#define REC_FILE_EN    "/usr/local/share/minigui/res/string/EN-UTF8.bin"
#define REC_FILE_JA    "/usr/local/share/minigui/res/string/JP-UTF8.bin"
#define REC_FILE_KO    "/usr/local/share/minigui/res/string/KO-UTF8.bin"

#define TIMING_FILE    "/data/timing"

#define VERSION_FILE   "/etc/version"
#define DEFRETROARCH   "/data/retroarch/retroarch.cfg"
#define DEFRETROARCHNAME   ".cfg"

#include "ui_1024x600.h"
//#include "ui_480x320.h"
//#include "ui_480x272.h"
#include "key_map_rk3128.h"

#include "parameter.h"
#include "desktop_dialog.h"
#include "pic_preview_dialog.h"
#include "browser_dialog.h"

#ifdef ENABLE_VIDEO
#include "videoplay_dialog.h"
#endif

#include "audioplay_dialog.h"
#include "setting_dialog.h"
#include "setting_language_dialog.h"
#include "setting_screenoff_dialog.h"
#include "setting_backlight_dialog.h"
#include "setting_version_dialog.h"
#include "setting_wifi_dialog.h"
#include "setting_themestyle_dialog.h"
#include "setting_volume_dialog.h"
#include "setting_systemtime_dialog.h"
#include "setting_airkiss_dialog.h"
#include "setting_general_dialog.h"
#include "message_dialog.h"
#include "videoplay_hw_dialog.h"
#include "system.h"
#include "ffplay_ipc.h"
#include "poweroff_dialog.h"
#include "input_dialog.h"
#include "time_input_dialog.h"
#include "systime.h"
#include "rtc_demo.h"

#include <DeviceIo/Rk_wifi.h>
#include <pthread.h>

extern int loadstringres(void);
extern int loadtimefile(void);
extern int loadversion(char **model, char **version);
extern int main_loadres(void);
extern void main_unloadres(void);
extern void DisableScreenAutoOff(void);
extern void EnableScreenAutoOff(void);

extern BITMAP batt_bmap[6];
extern BITMAP wifi_connected_bmap;
extern BITMAP wifi_disconnected_bmap;
extern BITMAP wifi_disabled_bmap;
extern BITMAP back_bmap;
extern BITMAP volume_0;
extern BITMAP volume_1;
extern BITMAP volume_2;
extern BITMAP volume_3;

extern HWND setting_wiif_dialog_hWnd;
extern HWND mhWnd, nhWnd;

extern struct wifi_info input_wifi_date;
extern struct wifi_info connect_wifi_date;
extern struct wifi_avaiable wifiavaiable_list[50];  // result of scan
extern int wifiavaiable_size;  // size of wifiavaiable_list

extern struct tm *now_time;
extern struct tm *last_time;
extern int battery;
extern BITMAP background_bmap;
extern RECT msg_rcBg;
extern RECT msg_rcBatt;
extern RECT msg_rcWifi;
extern RECT msg_rcTitle;
extern RECT msg_rcDialog;
extern RECT msg_rcStatusBar;
extern int status_bar_offset;
extern int status_bar_time_str[10];
extern int status_bar_date_str[20];

extern char *res_str[RES_STR_MAX];
extern rtc_timing timing_power_on[TIMING_NUM + 1];
extern rtc_timing timing_power_off[TIMING_NUM + 1];

extern LOGFONT  *logfont_cej;
extern LOGFONT  *logfont_k;
extern LOGFONT  *logfont_cej_title;
extern LOGFONT  *logfont_k_title;
extern LOGFONT  *logfont;
extern LOGFONT  *logfont_title;

extern int input_dialog_type;
extern int pwd_short_flag;
extern int wifi_connect_flag;
extern int cur_page;
extern int avaiable_wifi_display_mode;

#define BUTTON_MAXNUM    41
extern CTRLDATA KeyboardCtrl[BUTTON_MAXNUM];

extern pthread_t thread1;
extern char *message1;

#endif

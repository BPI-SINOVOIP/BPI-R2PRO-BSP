// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_LIBMEDIASERVER_API_H_
#define _RK_LIBMEDIASERVER_API_H_

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRIVACYMASK_NUM 1 // 隐私遮挡最大个数
#define MAX_TITLE_LINE 1      // OSD最大行数
#define MAX_TITLE_LENGTH 1024 // 每行OSD字幕最大长度

typedef struct _PRIVACYMASK_DATA_S {
  int32_t s32X[MAX_PRIVACYMASK_NUM];
  int32_t s32Y[MAX_PRIVACYMASK_NUM];
  int32_t s32W[MAX_PRIVACYMASK_NUM];
  int32_t s32H[MAX_PRIVACYMASK_NUM];
} PRIVACYMASK_DATA_S;

typedef struct _OSD_COLOR_S {
  uint32_t u32FgColor; // osd 前景颜色
} OSD_COLOR_S;

typedef struct _OSD_POS_S {
  uint16_t u16PosX; // osd 区域的左上角X坐标
  uint16_t u16PosY; // osd 区域的左上角Y坐标
} OSD_POS_S;

typedef struct _OSD_CFG_S {
  uint16_t u16Status;                               // 1 显示 0 隐藏
  uint16_t u16OsdRegW;                              // osd 区域的宽度
  uint16_t u16OsdRegH;                              // osd 区域的高度
  uint16_t u16FontPixelSize;                        // osd 字体像素尺寸
  char cOsdTitle[MAX_TITLE_LINE][MAX_TITLE_LENGTH]; // osd 叠加字幕信息
  OSD_POS_S stOsdPos;
  OSD_COLOR_S stOsdColor;
  int32_t s32OsdRegHdl;                 // osd 区域句柄
  uint16_t u16DisplayWeek;              // 1 显示星期 0 隐藏星期
  char cOsdColorMode[MAX_TITLE_LENGTH]; // "customize" or "auto"
  char cTimeStyle[MAX_TITLE_LENGTH];    // "24hour" or "12hour"
  char cDateStyle[MAX_TITLE_LENGTH];
  // YYYY-MM-DD, MM-DD-YYYY, DD-MM-YYYY, CHR-YYYY-MM-DD, CHR-MM-DD-YYYY,
  // CHR-DD-MM-YYYY, CHR-YYYY/MM/DD, CHR-MM/DD/YYYY, CHR-DD/MM/YYYY
} OSD_CFG_S;

typedef struct {
  uint16_t x;           /**< horizontal position of top left corner */
  uint16_t y;           /**< vertical position of top left corner */
  uint16_t w;           /**< width of ROI rectangle */
  uint16_t h;           /**< height of ROI rectangle */
  uint16_t intra;       /**< flag of forced intra macroblock */
  int16_t quality;      /**<  qp of macroblock */
  uint16_t qp_area_idx; /**< qp min max area select*/
  uint8_t area_map_en;  /**< enable area map */
  uint8_t abs_qp_en;    /**< absolute qp enable flag*/
} ROI_REGION_S;

typedef struct {
  int x, y; // left, top
  int w, h; // width, height
} IMAGE_RECT_S;

typedef struct {
  char cBMPPath[MAX_TITLE_LENGTH];
  char cTTCPath[MAX_TITLE_LENGTH];
} RESOURCE_S;

typedef struct {
  unsigned short x;
  unsigned short y;
  unsigned short w;
  unsigned short h;
} MOVE_DETEC_INFO_S;

typedef struct {
  unsigned short info_cnt;
  unsigned short ori_width;
  unsigned short ori_height;
  unsigned short ds_width;
  unsigned short ds_height;
  MOVE_DETEC_INFO_S *data;
} MOVE_DETECT_EVENT_S;

typedef void (*AudioHandler)(unsigned char *, unsigned int, int64_t);
typedef void (*VideoHandler)(unsigned char *, unsigned int, int64_t, int);
typedef void (*CaptureHandler)(unsigned char *, unsigned int, int,
                               const char *);
typedef void (*MDCallbackPtr)(MOVE_DETECT_EVENT_S *);

int libMediaInit(const char *json_file, RESOURCE_S env);
int libMediaStart();
int libMediaSaveConfig(const char *json_file);
int libMediaCleanup();

// {
//     "sResolution": "1920*1080",
//     "sRCMode": "CBR",   // CBR or VBR
//     "sRCQuality": "high", // lowest/lower/low/medium/high/higher/highest
//     "sFrameRate": "25",
//     "sFrameRateIn": "25",
//     "iMaxRate": 2048,
//     "sOutputDataType": "H.265", // H.265/H.264/MJPEG
//     "sSmart": "close",
//     "sH264Profile": "high",  // high/main/baseline
//     "iGOP": 50
// }
int libMediaSyncVideoData(int id, char *json);
// {
//     "sEncodeType":"G711U"   // mp3/G711A/G711U
// }
int libMediaSyncAudioData(char *json);

int libMediaAudioStart(AudioHandler handler);
int libMediaAudioStop();
int libMediaVideoStart(int id, VideoHandler handler);
int libMediaVideoStop(int id);
int libMediaCaptureStart(int id, CaptureHandler handler);
int libMediaCaptureStop(int id);

int libMediaGetSnapshot(int id);

int libMediaAudioChangeEncodeType(char *type); // mp3/G711A/G711U

int libMediaVideoChangeOutputDataType(int id, char *type); // H.265/H.264/MJPEG
int libMediaVideoChangeResolution(int id, int w, int h);
int libMediaVideoChangeBitrate(int id, int bitrate);
int libMediaVideoChangeFramerate(int id, int num, int den);
int libMediaVideoChangeIFrameInterval(int id, int value);
int libMediaVideoForceIFrame(int id);
int libMediaVideoSetSmartPFrame(int id, int value);
int libMediaVideoChangeProfile(int id, int value);
int libMediaVideoRateControl(int id, char *mode);
int libMediaVideoSetROI(int id, ROI_REGION_S *regions, int region_cnt);
int libMediaVideoSetUserdata(int id, void *data, int len, int all_frames);

int libMediaSetMDCallback(MDCallbackPtr callback);
int libMediaVideoSetMDEnabled(int enabled);
int libMediaVideoSetMDSensitivity(int sensitivity);
int libMediaVideoSetMDRect(IMAGE_RECT_S *rects, int rect_cnt);

int libMediaVideoSetTextOSD(int id, OSD_CFG_S osd);
int libMediaVideoSetBMPOSD(int id, OSD_CFG_S osd);
int libMediaVideoSetDateOSD(int id, OSD_CFG_S osd);
int libMediaVideoSetPrivacyMask(int id, PRIVACYMASK_DATA_S mask);
int libMediaVideoRmPrivacyMask(int id);

#ifdef __cplusplus
};
#endif

#endif

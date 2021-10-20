#ifndef _RKAIQ_COMMON_H__
#define _RKAIQ_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera_capture.h"
#include "camera_infohw.h"
#include "logger/log.h"
#include "rkaiq_cmdid.h"
#if 0
#include "rkaiq_manager.h"
#endif
#ifdef ENABLE_RSTP_SERVER
#include "rtsp_server.h"
#endif
#include "tcp_server.h"

typedef enum { KNOCK_KNOCK = 0x80, READY, BUSY, RES_FAILED = 0x00, RES_SUCCESS } cmdStatus;

typedef enum { PACKET_TYPE_SET = 0x00, PACKET_TYPE_GET = 0x01, PACKET_TYPE_STATUS = 0x80 } packeType;

typedef enum { PC_TO_DEVICE = 0x00, DEVICE_TO_PC } cmdType;

#define RKAIQ_TOOL_VERSION "v0.0.1"

#define STOP_RKLUNCH_CMD "sh /oem/RkLunch-stop.sh"

#if 0
extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;
#endif

#define RKID_ISP_ON "ISP_ON"
#define RKID_ISP_OFF "ISP_OFF"
#define RKID_CHECK "IGNORE"
#define RKID_SEND_FILE "SendFile"

#pragma pack(1)
typedef struct CommandData_s {
  uint8_t RKID[8];
  uint16_t cmdType;
  uint16_t cmdID;
  uint8_t version[8];
  uint16_t datLen;
  uint8_t dat[48];
  uint16_t checkSum;
} CommandData_t;
#pragma pack()

typedef enum {
  CMD_TYPE_UAPI_SET = 0x00,
  CMD_TYPE_UAPI_GET = 0x01,
  CMD_TYPE_CAPTURE = 0x02,
  CMD_TYPE_DUMP_RAW = 0x03,
  CMD_TYPE_STATUS = 0x80,
  CMD_TYPE_STREAMING = 0xff,
} FuncType_e;

typedef enum {
  CMD_ID_CAPTURE_STATUS = 0x0001,
  CMD_ID_CAPTURE_RAW_CAPTURE = 0x0002,
  CMD_ID_CAPTURE_YUV_CAPTURE = 0x0006,
} RkispCmdCaptureID_e;

typedef enum {
  CMD_ID_GET_STATUS = 0x0100,
  CMD_ID_GET_MODE = 0x0101,
  CMD_ID_START_RTSP = 0x0102,
  CMD_ID_STOP_RTSP = 0x0103,
} RkispCmdStatusID_e;

typedef enum {
  DATA_ID_CAPTURE_RAW_STATUS = 0x00,
  DATA_ID_CAPTURE_RAW_GET_PARAM = 0x01,
  DATA_ID_CAPTURE_RAW_SET_PARAM = 0x02,
  DATA_ID_CAPTURE_RAW_START = 0x03,
  DATA_ID_CAPTURE_RAW_CHECKSUM = 0x04,
} RkispCmdRawCaptureProcID_e;

typedef enum {
  DATA_ID_CAPTURE_YUV_STATUS = 0x00,
  DATA_ID_CAPTURE_YUV_GET_PARAM = 0x01,
  DATA_ID_CAPTURE_YUV_SET_PARAM = 0x02,
  DATA_ID_CAPTURE_YUV_START = 0x03,
  DATA_ID_CAPTURE_YUV_CHECKSUM = 0x04,
} RkispCmdYuvCaptureProcID_e;

typedef enum {
  RKISP_FORMAT_NV12 = 0x0,
  RKISP_FORMAT_NV16,
} RkispFmt_e;

#define VICAP_COMPACT_TEST_OFF "echo 0 > /sys/devices/platform/rkcif_mipi_lvds/compact_test"

#define VICAP2_COMPACT_TEST_OFF "echo 0 > /sys/devices/platform/rkcif_lite_mipi_lvds/compact_test"

#endif

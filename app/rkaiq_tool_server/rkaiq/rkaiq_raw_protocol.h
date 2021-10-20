#ifndef _RKAIQ_RAW_PROTOCOL_H__
#define _RKAIQ_RAW_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger/log.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_common.h"

typedef enum {
  CAPTURE_NORMAL = 0,
  CAPTUER_AVERAGE,
} captureMode;

typedef enum { VIDEO_APP_OFF = 0x80, VIDEO_APP_ON } videoAppStatus;

typedef enum {
  APP_RUN_STATUS_INIT = -1,
  APP_RUN_STATUS_TUNRING = 0,
  APP_RUN_STATUS_CAPTURE = 1,
  APP_RUN_STATUS_STREAMING = 2,
} appRunStatus;

typedef enum { RAW_CAP = 0x00, AVALIABLE } runStaus;

#pragma pack(1)
typedef struct Sensor_Params_s {
  uint8_t status;
  uint32_t fps;
  uint32_t hts;
  uint32_t vts;
  uint32_t bits;
  uint8_t endianness;
} Sensor_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Params_s {
  uint32_t gain;
  uint32_t time;
  uint8_t lhcg;
  uint8_t bits;
  uint8_t framenumber;
  uint8_t multiframe;
  uint32_t vblank;
  uint32_t focus_position;
} Capture_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Reso_s {
  uint16_t width;
  uint16_t height;
} Capture_Reso_t;
#pragma pack()

#define VIDEO_RAW0 "/dev/video0"
#define SAVE_RAW0_PATH "/data/output.raw"

#define TAG_PC_TO_DEVICE RKID_ISP_OFF
#define TAG_DEVICE_TO_PC RKID_ISP_OFF

int StopProcess(const char* process, const char* str);
int WaitProcessExit(const char* process, int sec);

class RKAiqRawProtocol {
 public:
  RKAiqRawProtocol() = default;
  virtual ~RKAiqRawProtocol() = default;
  static void HandlerRawCapMessage(int sockfd, char* buffer, int size);
};

#endif

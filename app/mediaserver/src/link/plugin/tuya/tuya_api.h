#ifndef _RK_TUYA_API_H_
#define _RK_TUYA_API_H_

#include "tuya_cloud_base_defs.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_cloud_error_code.h"
#include "tuya_cloud_types.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_media.h"
#include "tuya_ipc_p2p.h"
#include "tuya_ring_buffer.h"
#include "wifi_hwl.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "flow_common.h"
#include "link_common.h"

namespace rockchip {
namespace mediaserver {

class TuyaApi {

public:
  TuyaApi() {}
  ~TuyaApi() {}

  void FillMediaParam(MediaParamType type, unsigned int value);
  int FillLicenseKey(pLicenseKey plicense);
  int InitDevice();
  int DeInitDevice();
  int ConnectLink();
  int StartLink();
  int StopLink();

  void SetMediaControlCb(MediaControlCB callback) {
    media_control_callback_ = callback;
  }
  static void MediaControl(IpcMediaCmd cmd, const IpcMediaParam *param);

  /*Push data process func*/
  static void PushVideoHandler(unsigned char *buffer, unsigned int buffer_size,
                               int64_t present_time, int nal_type);
  static void PushAudioHandler(unsigned char *buffer, unsigned int buffer_size,
                               int64_t present_time);
  static void PushCaptureHandler(unsigned char *buffer,
                                 unsigned int buffer_size, int type,
                                 const char *id);

  int IPC_APP_Init_SDK(WIFI_INIT_MODE_E init_mode, char *p_token);
  void IPC_APP_Set_Media_Info();
  int TUYA_APP_Init_Ring_Buffer();
  int TUYA_APP_Enable_P2PTransfer(IN UINT_T max_users);
  int TUYA_APP_LOW_POWER_ENABLE();
  int TUYA_APP_LOW_POWER_DISABLE();
  void ReportWakeUpData1(void);

  static bool cloud_connected_;

private:
  LicenseKey license_;
  IPC_MEDIA_INFO_S s_media_info;
  static MediaControlCB media_control_callback_;
};

} // namespace mediaserver
} // namespace rockchip

#endif

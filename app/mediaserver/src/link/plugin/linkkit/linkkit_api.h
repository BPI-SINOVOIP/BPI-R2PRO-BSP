#ifndef _RK_LINKKIT_API_H_
#define _RK_LINKKIT_API_H_

#include <string>

#include <string.h>

#include "flow_common.h"
#include "link_common.h"

#include "iot_export.h"
#include "iot_export_linkkit.h"
#include "iot_import.h"
#include "link_visual_def.h"
#include "link_visual_ipc.h"

namespace rockchip {
namespace mediaserver {

class LinkKitApi {

public:
  LinkKitApi() : log_level_(LV_LOG_ERROR) {}
  ~LinkKitApi() {}

  void FillMediaParam(MediaParamType type, unsigned int value);
  int FillLicenseKey(pLicenseKey plicense);
  int InitDevice();
  int DeInitDevice();
  int ConnectLink();
  int StartLink();
  int StopLink();
  void InvasionPictureUpload(const char *fpath);
  void FacePictureUpload(const char *fpath);
  void FailPictureUpload(const char *fpath);

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

  /*CallBack for linklit Cmd*/
  static int OnPushLiveStreamCmdCb(int service_id,
                                   lv_on_push_streaming_cmd_type_e cmd,
                                   const lv_on_push_streaming_cmd_param_s *param);
  static int StartPushLiveStreamCmdCb(int service_id, lv_stream_type_e cmd_type,
                                      const lv_stream_param_s *param);
  static int StopPushLiveStreamCmdCb(int service_id);

  static int StartVoiceIntercomCb(int service_id);
  static int StopVoiceIntercomCb(int service_id);
  static int
  VoiceIntercomReceiveMetadataCb(int service_id,
                                 const lv_audio_param_s *audio_param);
  static void VoiceIntercomReceiveDataCb(const char *buffer,
                                         unsigned int buffer_size);

  static void QueryStorageRecordCb(
      unsigned int start_time, unsigned int stop_time, int num, const char *id,
      int (*on_complete)(int num, const char *id,
                         const lv_query_storage_record_response_s *response));
  static void QueryStorageRecordByMonthCb(
      const char *month, const char *id,
      int (*on_complete)(const char *id, const int *response));

  static int TriggerPicCaptureCb(const char *trigger_id);
  static int TriggerRecordCb(int type, int duration, int pre_duration);

  /*Handler for linklit event*/
  static int UserConnectedEventHandler(void);
  static int UserDisConnectedEventHandler(void);
  static int UserLinkVisualHandler(const int devid, const char *service_id,
                                    const int service_id_len, const char *payload,
                                    const int payload_len);
  static int UserServiceRequestHandler(const int devid, const char *id,
                                       const int id_len, const char *serviceid,
                                       const int serviceid_len,
                                       const char *request,
                                       const int request_len, char **response,
                                       int *response_len);
  static int UserPropertySetHandler(const int devid, const char *request,
                                    const int request_len);
  static int UserTimestampReplyHandler(const char *timestamp);
  static int UserFotaHandler(int type, const char *version);
  static void *UserDispatchYield(void *args);
  static void *UserReport(void *args);
  static void ReportProperty(const char *key, const int *value);
  static int ReportStorageCapacity(void);
  static void ReportWakeUpData0(void);
  static void ReportWakeUpData1(void);
  static int ReportBatteryLevel(void);
  static int GetAllProperty(void);
  static void PostEventPicUploadResult(std::string payload);
  static void PictureUpload(const char *fpath, lv_event_type_e type);
  static void PostAlarmEvent();

private:
  LicenseKey license_;
  lv_log_level_e log_level_;

  static int linkkit_dev_id_;
  static bool cloud_connected_;
  static bool msg_thread_running_;
  static bool msg_thread_not_quit_;
  static void *msg_thread_;
  static bool report_thread_running_;
  static bool report_thread_not_quit_;
  static void *report_thread_;
  static lv_video_param_s vparam_;
  static lv_audio_param_s aparam_;
  static double duration_;
  static int live_service_id_;
  static int voice_intercom_service_id_;
  static int face_event_id_;

  static MediaControlCB media_control_callback_;
};

} // namespace mediaserver
} // namespace rockchip

#endif

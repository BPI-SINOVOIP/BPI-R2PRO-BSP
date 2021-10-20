#include "linkkit_api.h"
#include "flow_pipe.h"
#include <cjson/cJSON.h>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef THUNDER_BOOT
#include <storage_manager.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "linkkit_api.cpp"

namespace rockchip {
namespace mediaserver {

static int g_init = 0;

// 这个字符串数组用于说明LinkVisual需要处理的物模型服务
static const char *link_visual_service[] = {
    "TriggerPicCapture",   //触发设备抓图
    "StartVoiceIntercom",  //开始语音对讲
    "StopVoiceIntercom",   //停止语音对讲
    "StartVod",            //开始录像观看
    "StartVodByTime",      //开始录像按时间观看
    "StopVod",             //停止录像观看
    "QueryRecordList",     //查询录像列表
    "StartP2PStreaming",   //开始P2P直播
    "StartPushStreaming",  //开始直播
    "StopPushStreaming",   //停止直播
    "QueryRecordTimeList", //按月查询卡录像列表
};

// 用于启动时上报所有属性的初始值
static std::map<std::string, std::string> property_map = {
    {"AlarmNotifyPlan", "{{\"DayOfWeek\":0,\"BeginTime\":21600,\"EndTime\":"
                        "36000},{\"DayOfWeek\":1,\"BeginTime\":21600,"
                        "\"EndTime\":36000}}"},

};

static std::map<std::string, int> property_map_int = {
    {"AlarmFrequencyLevel", 0}, {"AlarmPromptSwitch", 1},
    {"AlarmSwitch", 1},         {"DayNightMode", 0},
    {"EncryptSwitch", 0},       {"ImageFlipState", 0},
    {"MicSwitch", 1},           {"MotionDetectSensitivity", 0},
    {"SpeakerSwitch", 1},       {"StatusLightSwitch", 0},
    {"StreamVideoQuality", 1},  {"SubStreamVideoQuality", 0},
    {"StorageRecordMode", 0},   {"VoiceDetectionSensitivity", 0}};

void string_safe_copy(char *target, std::string source, int max_len) {
  if (target && !source.empty() && max_len) {
    int len = max_len;
    source.size() > max_len ? len = max_len : len = source.size();
    memcpy(target, source.c_str(), len);
  }
}

int LinkKitApi::linkkit_dev_id_ = -1;
bool LinkKitApi::cloud_connected_;
bool LinkKitApi::msg_thread_running_;
bool LinkKitApi::msg_thread_not_quit_;
void *LinkKitApi::msg_thread_;
bool LinkKitApi::report_thread_running_;
bool LinkKitApi::report_thread_not_quit_;
void *LinkKitApi::report_thread_;
int LinkKitApi::live_service_id_ = -1;
int LinkKitApi::voice_intercom_service_id_ = -1;
int LinkKitApi::face_event_id_ = 8;
lv_video_param_s LinkKitApi::vparam_;
lv_audio_param_s LinkKitApi::aparam_;
double LinkKitApi::duration_;
MediaControlCB LinkKitApi::media_control_callback_ = nullptr;

int LinkKitApi::FillLicenseKey(pLicenseKey plicense) {
  if (plicense) {
    license_.product_key = plicense->product_key;
    license_.device_name = plicense->device_name;
    license_.device_secret = plicense->device_secret;
    license_.product_secret = plicense->product_secret;
  }
  return 0;
}

static void query_storage_record_cb(unsigned int start_time,
                                    unsigned int stop_time,
                                    int num,
                                    lv_storage_record_type_e type,
                                    const char *id,
                                    int (*on_complete)(int num,
                                                       const char *id,
                                                       const lv_query_storage_record_response_s *response)) {
    printf("start_time:%d stop_time:%d num:%d type:%d\n", start_time, stop_time, num, type);
#ifdef DUMMY_IPC
    dummy_ipc_report_vod_list(start_time, stop_time, num, id, on_complete);
#endif // DUMMY_IPC
}

static int cloud_event_cb(lv_cloud_event_type_e type, const lv_cloud_event_param_s *param) {
    printf("cloud_event_cb: %d \n", type);
    if (type == LV_CLOUD_EVENT_DOWNLOAD_FILE) {
        printf("cloud_event_cb %d %s %s\n", param->file_download.file_type, param->file_download.file_name, param->file_download.url);
    } else if (type == LV_CLOUD_EVENT_UPLOAD_FILE) {
        printf("cloud_event_cb %d %s %s\n", param->file_upload.file_type, param->file_upload.file_name, param->file_upload.url);
    }
    return 0;
}

int LinkKitApi::InitDevice() {
  LOG_INFO("before init linkvisual\n");

  if (g_init) {
    printf("linkvisual already init\n");
    return 0;
  }

  memset(&vparam_, 0, sizeof(lv_video_param_s));
  memset(&aparam_, 0, sizeof(lv_audio_param_s));

  lv_config_s config;
  memset(&config, 0, sizeof(lv_config_s));

  /* 设备三元组 */
  string_safe_copy(config.product_key, license_.product_key, PRODUCT_KEY_LEN);
  string_safe_copy(config.device_name, license_.device_name, DEVICE_NAME_LEN);
  string_safe_copy(config.device_secret, license_.device_secret,
                   DEVICE_SECRET_LEN);

  /* SDK的日志配置 */
  config.log_level = log_level_;
  config.log_dest = LV_LOG_DESTINATION_STDOUT;

  /* 码流路数限制 */
  config.storage_record_mode = LV_STREAM_CMD_STORAGE_RECORD_BY_UTC_TIME;
  config.live_p2p_num = 4;
  config.storage_record_source_num = 1;

  /* 码流检查功能 */
  config.stream_auto_check = 1;
#if 1 /* 码流保存为文件功能，按需使用 */
  config.stream_auto_save = 0;
#else
  config.stream_auto_save = 1;
  char *path = "/tmp/";
  memcpy(config.stream_auto_save_path, path, strlen(path));
#endif

  /* 设备取证服务功能（Device Attestation Service）默认开启 */
  config.das_close = 0;

  /* 未实现预录事件录像 */
  config.pre_event_record_support = 0;

  /* 音视频推流服务 */
  config.start_push_streaming_cb = StartPushLiveStreamCmdCb;
  config.stop_push_streaming_cb = StopPushLiveStreamCmdCb;
  config.on_push_streaming_cmd_cb = OnPushLiveStreamCmdCb;

  /* 语音对讲服务 */
  config.start_voice_intercom_cb = StartVoiceIntercomCb;
  config.stop_voice_intercom_cb = StopVoiceIntercomCb;
  config.voice_intercom_receive_metadata_cb = VoiceIntercomReceiveMetadataCb;
  config.voice_intercom_receive_data_cb = VoiceIntercomReceiveDataCb;

  /* 获取存储录像录像列表 */
  config.query_storage_record_cb = query_storage_record_cb;
  config.query_storage_record_by_month_cb = QueryStorageRecordByMonthCb;

  /* 触发设备抓图 */
  config.trigger_pic_capture_cb = TriggerPicCaptureCb;
  /* 触发设备录像 */
  config.trigger_record_cb = TriggerRecordCb;
  /* 云端事件通知 */
  config.cloud_event_cb = cloud_event_cb;

  int ret = lv_init(&config);
  if (ret < 0) {
    LOG_ERROR("lv_init failed, result = %d\n", ret);
    return -1;
  }
  g_init = 1;

  LOG_INFO("after init linkvisual\n");
  return 0;
}

int LinkKitApi::DeInitDevice() {
  printf("before destroy linkvisual\n");
  if (!g_init) {
    printf("linkvisual_demo_destroy is not init\n");
    return -1;
  }
  lv_destroy();
  printf("after destroy linkvisual\n");

  return 0;
}

void LinkKitApi::FillMediaParam(MediaParamType type, unsigned int value) {
  switch (type) {
  case MEDIA_PARAM_VIDEO_FPS:
    vparam_.fps = value;
    break;
  case MEDIA_PARAM_VIDEO_FMT:
    vparam_.format = (lv_video_format_e)value;
    break;
  case MEDIA_PARAM_VIDEO_FRAME_INTERVAL:
    vparam_.key_frame_interval_ms = value;
    break;
  case MEDIA_PARAM_AUDIO_FMT:
    aparam_.format = (lv_audio_format_e)value;
    break;
  case MEDIA_PARAM_AUDIO_SAMPLE_RATE:
    aparam_.sample_rate = (lv_audio_sample_rate_e)value;
    break;
  case MEDIA_PARAM_AUDIO_SAMPLE_BITS:
    aparam_.sample_bits = (lv_audio_sample_bits_e)value;
    break;
  case MEDIA_PARAM_AUDIO_CHANNLE:
    aparam_.channel = (lv_audio_channel_e)value;
    break;
  case MEDIA_PARAM_FILE_DURATION:
    duration_ = value;
    break;
  default:
    break;
  }
}

void LinkKitApi::PushVideoHandler(unsigned char *buffer,
                                  unsigned int buffer_size,
                                  int64_t present_time, int nal_type) {
  if (live_service_id_ >= 0) {
    lv_stream_send_video(live_service_id_, LV_VIDEO_FORMAT_H264, buffer, buffer_size, nal_type,
                         present_time);
  }
}

void LinkKitApi::PushAudioHandler(unsigned char *buffer,
                                  unsigned int buffer_size,
                                  int64_t present_time) {
  if (live_service_id_ >= 0) {
    lv_stream_send_audio(live_service_id_, LV_AUDIO_FORMAT_G711A, buffer, buffer_size, present_time);
  }
  if (voice_intercom_service_id_ >= 0) {
    lv_voice_intercom_send_audio(voice_intercom_service_id_,
                                 (const char *)buffer, buffer_size,
                                 present_time);
  }
}

void LinkKitApi::PushCaptureHandler(unsigned char *buffer,
                                    unsigned int buffer_size, int type,
                                    const char *id) {}

void LinkKitApi::MediaControl(IpcMediaCmd cmd, const IpcMediaParam *param) {
  if (media_control_callback_)
    media_control_callback_(cmd, param);
}

int LinkKitApi::OnPushLiveStreamCmdCb(int service_id,
                                      lv_on_push_streaming_cmd_type_e cmd,
                                      const lv_on_push_streaming_cmd_param_s *param) {
  LOG_INFO("OnPushLiveStreamCmdCb service_id %d cmd %d\n", service_id, cmd);
  IpcMediaParam ipc_param = {0};
  ipc_param.service_id = service_id;
  if (cmd == LV_LIVE_REQUEST_I_FRAME) {
    lv_stream_send_config(service_id, 1000, duration_, &vparam_, &aparam_);
    MediaControl(IPC_MEDIA_REQUEST_I_FRAME, &ipc_param);
  }
  return 0;
}

int LinkKitApi::StartPushLiveStreamCmdCb(int service_id,
                                         lv_stream_type_e cmd_type,
                                         const lv_stream_param_s *param) {
  LOG_INFO("StartPushLiveStreamCmdCb service_id %d\n", service_id);
  IpcMediaParam ipc_param = {0};
  ipc_param.service_id = service_id;
  if (cmd_type == LV_STREAM_CMD_LIVE) {
    MediaControl(IPC_MEDIA_START, &ipc_param);
    live_service_id_ = service_id;
  }
  lv_stream_send_config(service_id, 1000, duration_, &vparam_, &aparam_);
#ifdef THUNDER_BOOT
  FILE *fp;
  fp = fopen("/tmp/rtmp_live","a");
  if (fp)
    fclose(fp);
#endif
  return 0;
}

int LinkKitApi::StopPushLiveStreamCmdCb(int service_id) {
  LOG_INFO("StopPushLiveStreamCmdCb service_id %d\n", service_id);
  IpcMediaParam ipc_param = {0};
  if (service_id == live_service_id_) {
    live_service_id_ = -1;
    MediaControl(IPC_MEDIA_STOP, &ipc_param);
  }
#ifdef THUNDER_BOOT
  unlink("/tmp/rtmp_live");
#endif
  return 0;
}

// 语音对讲的全局变量
std::shared_ptr<easymedia::Stream> out_stream;
std::string alsa_device = "default";
MediaConfig pcm_config;
AudioConfig &aud_cfg = pcm_config.aud_cfg;
SampleInfo &sample_info = aud_cfg.sample_info;

int LinkKitApi::StartVoiceIntercomCb(int service_id) {
  // 收到开始语音对讲请求时，主动开启对讲服务
  LOG_INFO("start voice intercom callback service_id:%d\n", service_id);
  voice_intercom_service_id_ = service_id;
  lv_audio_param_s audio_param;
  memset(&aparam_, 0, sizeof(lv_audio_param_s));
  aparam_.format = LV_AUDIO_FORMAT_G711A;
  aparam_.channel = LV_AUDIO_CHANNEL_MONO;
  aparam_.sample_bits = LV_AUDIO_SAMPLE_BITS_16BIT;
  aparam_.sample_rate = LV_AUDIO_SAMPLE_RATE_8000;
  int ret = lv_voice_intercom_start_service(service_id, &aparam_);
  if (ret < 0)
    return -1;

  // 输出流的格式配置
  sample_info.fmt = SAMPLE_FMT_G711A;
  sample_info.channels = 1;
  sample_info.sample_rate = 8000;

  easymedia::REFLECTOR(Stream)::DumpFactories();

  // 创建输出流
  std::string stream_name("alsa_playback_stream");
  std::string fmt_str = SampleFmtToString(sample_info.fmt);
  std::string rule;
  PARAM_STRING_APPEND(rule, KEY_INPUTDATATYPE, fmt_str);
  if (!easymedia::REFLECTOR(Stream)::IsMatch(stream_name.c_str(),
                                             rule.c_str())) {
    fprintf(stderr, "unsupport data type\n");
    exit(EXIT_FAILURE);
  }
  std::string params;
  PARAM_STRING_APPEND(params, KEY_DEVICE, alsa_device);
  PARAM_STRING_APPEND(params, KEY_SAMPLE_FMT, fmt_str);
  PARAM_STRING_APPEND_TO(params, KEY_CHANNELS, sample_info.channels);
  PARAM_STRING_APPEND_TO(params, KEY_SAMPLE_RATE, sample_info.sample_rate);
  LOG_INFO("params:\n%s\n", params.c_str());
  out_stream = easymedia::REFLECTOR(Stream)::Create<easymedia::Stream>(
      stream_name.c_str(), params.c_str());
  if (!out_stream) {
    fprintf(stderr, "Create stream %s failed\n", stream_name.c_str());
    exit(EXIT_FAILURE);
  }

  return 0;
}

int LinkKitApi::StopVoiceIntercomCb(int service_id) {
  LOG_INFO("stop voice intercom, service_id=%d\n", service_id);
  lv_voice_intercom_stop_service(service_id);
  if (service_id == voice_intercom_service_id_) {
    voice_intercom_service_id_ = -1;
  }
  out_stream.reset();

  return 0;
}

int LinkKitApi::VoiceIntercomReceiveMetadataCb(
    int service_id, const lv_audio_param_s *audio_param) {
  LOG_INFO("voice intercom receive metadata, service_id:%d\n", service_id);
  LOG_DEBUG("format is %d, channel is %d, "
            "sample_bits is %d, sample_rate is %d\n",
            audio_param->format, audio_param->channel, audio_param->sample_bits,
            audio_param->sample_rate);

  return 0;
}

void LinkKitApi::VoiceIntercomReceiveDataCb(const char *buffer,
                                            unsigned int buffer_size) {
  LOG_DEBUG("voice receive data callback buffer size is %d\n", buffer_size);
  out_stream->Write(buffer, buffer_size, 1);
}

void LinkKitApi::QueryStorageRecordCb(
    unsigned int start_time, unsigned int stop_time, int num, const char *id,
    int (*on_complete)(int num, const char *id,
                       const lv_query_storage_record_response_s *response)) {
#ifndef THUNDER_BOOT
  char *str;

  LOG_INFO("start_time is %d, stop_time is %d\n", start_time, stop_time);
  str = storage_manager_get_media_path();
  cJSON *media_path = cJSON_Parse(str);
  cJSON *scan_path = cJSON_GetObjectItem(media_path, "sScanPath");
  cJSON *scan_path_0 = cJSON_GetArrayItem(scan_path, 0);
  cJSON *files_path = cJSON_GetObjectItem(scan_path_0, "sMediaPath");
  std::string files_path_s = files_path->valuestring;
  LOG_INFO("media path is %s\n", files_path_s.c_str());
  free(str);

  str = storage_manager_get_filelist_path((char *)files_path_s.c_str(),
                                          (int *)&start_time, (int *)&stop_time,
                                          1, 0);
  LOG_DEBUG("storage_manager_get_filelist_path is %s\n", str);
  cJSON *file_list_path = cJSON_Parse(str);
  cJSON *files = cJSON_GetObjectItem(file_list_path, "FILES");
  int files_size = cJSON_GetArraySize(files);
  LOG_INFO("storage record file size is %d\n", files_size);
  free(str);

  if (!files_size) {
    LOG_ERROR("storage record file list is null!\n");
    return;
  }

  auto *response = new lv_query_storage_record_response_s[files_size];
  memset(response, 0, sizeof(lv_query_storage_record_response_s) * files_size);
  double duration = 60; // 默认录像时长60s
  cJSON *file;
  cJSON *file_name;
  cJSON *file_size;
  cJSON *file_time;
  for (int i = 0; i < files_size; i++) {
    file = cJSON_GetArrayItem(files, i);
    file_name = cJSON_GetObjectItem(file, "sNAME");
    file_size = cJSON_GetObjectItem(file, "sSIZE");
    file_time = cJSON_GetObjectItem(file, "iTIME");
    response[i].file_size = file_size->valueint;
    response[i].record_type = LV_STORAGE_RECORD_INITIATIVE;
    snprintf(response[i].file_name, 64, file_name->valuestring, i); //注意别溢出
    response[i].start_time = file_time->valueint - duration;
    response[i].stop_time = file_time->valueint;
    // LOG_DEBUG("start time is %d, stop time is %d\n",
    //           response[i].start_time, response[i].stop_time);
  }
  int result = on_complete(files_size, id, response);
  if (result)
    LOG_ERROR("ipc_report_vod_list query fail\n");
  delete[] response;

  cJSON_Delete(media_path);
  cJSON_Delete(file_list_path);
#endif
}

void LinkKitApi::QueryStorageRecordByMonthCb(
    const char *month, const char *id,
    int (*on_complete)(const char *id, const int *response)) {}

int LinkKitApi::TriggerPicCaptureCb(const char *trigger_id) { return 0; }

int LinkKitApi::TriggerRecordCb(int type, int duration, int pre_duration) {
  return 0;
}

int LinkKitApi::ConnectLink() {
  // 连接到服务器
  int ret = IOT_Linkkit_Connect(linkkit_dev_id_);
  // 对主设备/网关来说, 将会建立设备与云端的通信.
  // 对于子设备来说, 将向云端注册该子设备(若需要), 并添加主子设备拓扑关系
  if (ret < 0) {
    LOG_ERROR("IOT_Linkkit_Connect Failed\n");
    return -1;
  }

  // 创建线程，线程用于轮训消息
  msg_thread_running_ = true;
  msg_thread_not_quit_ = true;
  ret = HAL_ThreadCreate(&msg_thread_, UserDispatchYield, NULL, NULL, NULL);
  if (ret != 0) { //!= 0 而非 < 0
    LOG_ERROR("HAL_ThreadCreate Failed, ret = %d\n", ret);
    msg_thread_running_ = false;
    msg_thread_not_quit_ = false;
    IOT_Linkkit_Close(linkkit_dev_id_);
    return -1;
  }
  // 创建线程，线程用于定时上报属性
  report_thread_running_ = true;
  report_thread_not_quit_ = true;
  ret = HAL_ThreadCreate(&report_thread_, UserReport, NULL, NULL, NULL);
  if (ret != 0) { //!= 0 而非 < 0
    LOG_ERROR("HAL_ThreadCreate Failed, ret = %d\n", ret);
    report_thread_running_ = false;
    report_thread_not_quit_ = false;
    IOT_Linkkit_Close(linkkit_dev_id_);
    return -1;
  }
  // 等待linkkit链接成功
  // (demo做了有限时长的等待，实际产品中，可设置为在网络可用时一直等待)
  for (;;) {
    if (!cloud_connected_) {
      HAL_SleepMs(200);
    } else {
      break;
    }
  }

  if (!cloud_connected_) {
    LOG_ERROR("linkkit connect Failed\n");
    StopLink();
    return -1;
  }

  return 0;
}

int LinkKitApi::StartLink() {
  LOG_INFO("StartLink\n");
  // 设置调试的日志级别
  IOT_SetLogLevel(IOT_LOG_NONE);
  // 注册链接状态的回调
  IOT_RegisterCallback(ITE_CONNECT_SUCC, UserConnectedEventHandler);
  IOT_RegisterCallback(ITE_DISCONNECTED, UserDisConnectedEventHandler);
  // 注册消息通知
  // linkvisual自定义消息
  IOT_RegisterCallback(ITE_LINK_VISUAL, UserLinkVisualHandler);
  // 物模型服务类消息
  IOT_RegisterCallback(ITE_SERVICE_REQUST, UserServiceRequestHandler);
  // 物模型属性设置
  IOT_RegisterCallback(ITE_PROPERTY_SET, UserPropertySetHandler);
  // NTP时间
  IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, UserTimestampReplyHandler);
  // 固件OTA升级事件
  IOT_RegisterCallback(ITE_FOTA, UserFotaHandler);

  iotx_linkkit_dev_meta_info_t master_meta_info;
  memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
  string_safe_copy(master_meta_info.product_key, license_.product_key,
                   PRODUCT_KEY_MAXLEN - 1);
  string_safe_copy(master_meta_info.product_secret, license_.product_secret,
                   PRODUCT_SECRET_MAXLEN - 1);
  string_safe_copy(master_meta_info.device_name, license_.device_name,
                   DEVICE_NAME_MAXLEN - 1);
  string_safe_copy(master_meta_info.device_secret, license_.device_secret,
                   DEVICE_SECRET_MAXLEN - 1);
  // 选择服务器地址，当前使用上海服务器
  int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
  IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);
  // 动态注册
  int dynamic_register = 0;
  IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);
  // 创建linkkit资源
  linkkit_dev_id_ =
      IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
  // 创建本地资源, 在进行网络报文交互之前, 必须先调用此接口, 得到一个会话的句柄
  if (linkkit_dev_id_ < 0) {
    LOG_ERROR("IOT_Linkkit_Open Failed\n");
    return -1;
  }

  return ConnectLink();
}

int LinkKitApi::StopLink() {
  LOG_INFO("StopLink\n");
  // 等待线程退出，并释放线程资源，
  // 也可用分离式线程，但需要保证线程不使用linkkit资源后，再去释放linkkit
  msg_thread_running_ = false;
  while (msg_thread_not_quit_) {
    HAL_SleepMs(20);
  }
  if (msg_thread_) {
    HAL_ThreadDelete(msg_thread_);
    msg_thread_ = NULL;
  }
  report_thread_running_ = false;
  while (report_thread_not_quit_) {
    HAL_SleepMs(20);
  }
  if (report_thread_) {
    HAL_ThreadDelete(report_thread_);
    report_thread_ = NULL;
  }

  LOG_INFO("IOT_Linkkit_Close\n");
  IOT_Linkkit_Close(linkkit_dev_id_);
  //若入参中的会话句柄为主设备/网关,
  //则关闭网络连接并释放SDK为该会话所占用的所有资源
  linkkit_dev_id_ = -1;
  LOG_INFO("After destroy linkkit\n");
  return 0;
}

int LinkKitApi::UserConnectedEventHandler(void) {
  LOG_INFO("Cloud Connected\n");
  cloud_connected_ = true;
  // Linkkit连接后，上报设备的属性值。
  // 当APP查询设备属性时，会直接从云端获取到设备上报的属性值，而不会下发查询指令。
  // 对于设备自身会变化的属性值（存储使用量等），设备可以主动隔一段时间进行上报。

  // 开机获取和上报所有属性值
  GetAllProperty();
#ifdef THUNDER_BOOT
  FILE *fp;
  fp = fopen("/tmp/wifi_wake","r");
  if (!fp)
    PostAlarmEvent(); // 开机上报事件，用于触发事件录像
  else 
    fclose(fp);
#endif

  // Linkkit连接后，查询下ntp服务器的时间戳，用于同步服务器时间。
  // 查询结果在UserTimestampReplyHandler中
  IOT_Linkkit_Query(linkkit_dev_id_, ITM_MSG_QUERY_TIMESTAMP, NULL, 0);
  // linkkit上线消息同步给LinkVisual
  lv_ipc_linkkit_adapter(LV_LINKKIT_ADAPTER_TYPE_CONNECTED, NULL);
  return 0;
}

int LinkKitApi::UserDisConnectedEventHandler(void) {
  LOG_INFO("Cloud Disconnected\n");
  cloud_connected_ = false;
  return 0;
}

int LinkKitApi::UserLinkVisualHandler(const int devid, const char *service_id,
                                    const int service_id_len, const char *payload,
                                    const int payload_len) {
  // Linkvisual自定义的消息，直接全交由LinkVisual来处理
  if (payload == NULL || payload_len == 0) {
    return 0;
  }

  lv_linkkit_adapter_property_s in = {0};
  in.service_name = service_id;
  in.service_name_len = service_id_len;
  in.request = payload;
  in.request_len = payload_len;
  int ret = lv_ipc_linkkit_adapter(LV_LINKKIT_ADAPTER_TYPE_LINK_VISUAL, &in);
  if (ret < 0) {
    LOG_ERROR("LinkVisual process service request failed, ret = %d\n", ret);
    return -1;
  }
  return 0;
}

int LinkKitApi::UserServiceRequestHandler(
    const int devid, const char *id, const int id_len, const char *serviceid,
    const int serviceid_len, const char *request, const int request_len,
    char **response, int *response_len) {
  LOG_INFO("Service Request Received, Devid: %d, ID %.*s, Service ID: %.*s, "
           "Payload: %s\n",
           devid, id_len, id, serviceid_len, serviceid, request);
  // 部分物模型服务消息由LinkVisual处理，部分需要自行处理。
  int link_visual_process = 0;
  for (unsigned int i = 0;
       i < sizeof(link_visual_service) / sizeof(link_visual_service[0]); i++) {
    // 这里需要根据字符串的长度来判断
    if (!strncmp(serviceid, link_visual_service[i],
                 strlen(link_visual_service[i]))) {
      link_visual_process = 1;
      break;
    }
  }

  if (link_visual_process) {
    // ISV将某些服务类消息交由LinkVisual来处理，不需要处理response
    lv_linkkit_adapter_property_s in = {0};
    in.dev_id = devid;
    in.msg_id = id;
    in.msg_id_len = id_len;
    in.service_name = serviceid;
    in.service_name_len = serviceid_len;
    in.request = request;
    in.request_len = request_len;
    int ret = lv_ipc_linkkit_adapter(LV_LINKKIT_ADAPTER_TYPE_TSL_SERVICE, &in);
    if (ret < 0) {
      LOG_ERROR("LinkVisual process service request failed, ret = %d\n", ret);
      return -1;
    }
  } else {
    // 非LinkVisual处理的消息示例
    if (!strncmp(serviceid, "PTZActionControl",
                 (serviceid_len > 0) ? serviceid_len : 0)) {
      cJSON *root = cJSON_Parse(request);
      if (root == NULL) {
        LOG_ERROR("JSON Parse Error\n");
        return -1;
      }

      cJSON *child = cJSON_GetObjectItem(root, "ActionType");
      if (!child) {
        LOG_ERROR("JSON Parse Error\n");
        cJSON_Delete(root);
        return -1;
      }
      int action_type = child->valueint;

      child = cJSON_GetObjectItem(root, "Step");
      if (!child) {
        LOG_ERROR("JSON Parse Error\n");
        cJSON_Delete(root);
        return -1;
      }
      int step = child->valueint;

      cJSON_Delete(root);
      LOG_INFO("PTZActionControl %d %d\n", action_type, step);
    }
  }

  if (!strncmp(serviceid, "TriggerPicCapture", strlen("TriggerPicCapture"))) {
    // 根据云端触发抓图回调的url，上报图片上传结果
    PostEventPicUploadResult(serviceid);
  }

  return 0;
}

int LinkKitApi::UserPropertySetHandler(const int devid, const char *request,
                                       const int request_len) {
  cJSON *root = NULL;
  cJSON *value = NULL;
  LOG_INFO("Property Set Received, Devid: %d, Request: %s\n", devid, request);
  // Parse Root
  root = cJSON_Parse(request);
  if (root == NULL || !cJSON_IsObject(root)) {
    LOG_ERROR("JSON Parse Error");
    return -1;
  }
  // 用户可在此处对收到的属性值进行处理。
  // 当收到属性设置时，开发者需要修改设备配置、改写已存储的属性值，并上报最新属性值。demo只上报了最新属性值。
  // Parse StreamVideoQuality
  value = cJSON_GetObjectItem(root, "StreamVideoQuality");
  if (value != NULL && cJSON_IsNumber(value)) {
    LOG_INFO("StreamVideoQuality = %d\n", value->valueint);
    IpcMediaParam ipc_param = {0};
    ipc_param.stream_type = value->valueint;
    MediaControl(IPC_MEDIA_CHANGE_STREAM, &ipc_param);
  }

  value = cJSON_GetObjectItem(root, "PIRSensitivity");
  if (value != NULL && cJSON_IsNumber(value)) {
    char cmd[50];
    sprintf(cmd, "echo %d > /proc/pir/sensibility", value->valueint);
    LOG_INFO("cmd is %s\n", cmd);
    system(cmd);
  }

  // 原样返回
  int res = IOT_Linkkit_Report(linkkit_dev_id_, ITM_MSG_POST_PROPERTY,
                               (unsigned char *)request, request_len);
  LOG_INFO("Post Property Message ID: %d\n", res);
  cJSON_Delete(root);

  return 0;
}

int LinkKitApi::UserTimestampReplyHandler(const char *timestamp) {
  LOG_INFO("Current Timestamp: %s \n", timestamp);
  // 时间戳为字符串格式，单位：毫秒
  return 0;
}

int LinkKitApi::UserFotaHandler(int type, const char *version) {
  char buffer[1024] = {0};
  int buffer_length = 1024;
  if (type == 0) {
    LOG_INFO("New Firmware Version: %s\n", version);
    // 向云端发送存在云端业务数据下发的查询报文,
    // 包括OTA状态查询/OTA固件下载/子设备拓扑查询/NTP时间查询等各种报文
    IOT_Linkkit_Query(linkkit_dev_id_, ITM_MSG_QUERY_FOTA_DATA,
                      (unsigned char *)buffer, buffer_length);
  }
  return 0;
}

void *LinkKitApi::UserDispatchYield(void *args) {
  //int cnt = 0;
  while (msg_thread_running_) {
    IOT_Linkkit_Yield(10); // 10ms
    // 若SDK占有独立线程, 该函数内容为空,
    // 否则表示将CPU交给SDK让其接收网络报文并将消息分发到用户的回调函数中

    // // Post Proprety Example
    // if ((cnt % 200) == 0) { // 2s
    //     /* user_post_property(); */
    // }

    //cnt++;
  }

  msg_thread_not_quit_ = 0;
  return NULL;
}

void *LinkKitApi::UserReport(void *args) {
  for (;;) {
    if (!cloud_connected_)
      HAL_SleepMs(200);
    else
      break;
  }
  int cnt = 0;
  while (report_thread_running_) {
    if ((cnt % 6) == 0) { // 60s
      ReportBatteryLevel();
      cnt = 0;
    }
    cnt++;
    sleep(10);
  }

  report_thread_not_quit_ = 0;
  return NULL;
}

// 属性上报
void LinkKitApi::ReportProperty(const char *key, const int *value) {
  char result[1024] = {0};
  snprintf(result, 1024, "{\"%s\":%d}", key, *value);
  int res = IOT_Linkkit_Report(linkkit_dev_id_, ITM_MSG_POST_PROPERTY,
                               (unsigned char *)result, strlen(result));
  LOG_INFO("Post Property Message : %s\n", result);
  LOG_INFO("Post Property Message ID: %d\n", res);
}

int LinkKitApi::ReportStorageCapacity() {
#ifndef THUNDER_BOOT
  int size;
  int status = 0;
  char *disks_status = storage_manager_get_disks_status((char *)"/userdata");
  cJSON *disks_js = cJSON_Parse(disks_status);
  free(disks_status);
  if (disks_js == NULL) {
    LOG_ERROR("JSON Parse Error\n");
    cJSON_Delete(disks_js);
    return -1;
  }
  cJSON *disks = cJSON_GetArrayItem(disks_js, 0);
  if (!disks) {
    LOG_ERROR("JSON Parse Error\n");
    cJSON_Delete(disks_js);
    return -1;
  }
  cJSON *total_size = cJSON_GetObjectItem(disks, "iTotalSize");
  if (!total_size) {
    LOG_ERROR("JSON Parse Error\n");
    cJSON_Delete(disks_js);
    return -1;
  }
  cJSON *free_size = cJSON_GetObjectItem(disks, "iFreeSize");
  if (!total_size) {
    LOG_ERROR("JSON Parse Error\n");
    cJSON_Delete(disks_js);
    return -1;
  }
  cJSON *iStatus = cJSON_GetObjectItem(disks, "iStatus");
  if (!iStatus) {
    LOG_ERROR("JSON Parse Error\n");
    cJSON_Delete(disks_js);
    return -1;
  }
  cJSON *iFormatStatus = cJSON_GetObjectItem(disks, "iFormatStatus");
  if (!iFormatStatus) {
    LOG_ERROR("JSON Parse Error\n");
    cJSON_Delete(disks_js);
    return -1;
  }

  size = total_size->valueint / 1024; // MByte
  ReportProperty("StorageTotalCapacity", &size);
  size = free_size->valueint / 1024; // MByte
  ReportProperty("StorageRemainCapacity", &size);
  if (iFormatStatus->valueint == 1)
    status = 3;
  else if ((iStatus->valueint == 1) && (iFormatStatus->valueint == 0))
    status = 1;
  else if (iStatus->valueint == 0)
    status = 0;
  ReportProperty("StorageStatus", &status); // TODO: Add unformatted judgment
  cJSON_Delete(disks_js);
#endif
  return 0;
}

void LinkKitApi::ReportWakeUpData0() {
  const char data[1024] = "{\"WakeUpData\":\"rockchip\"}";
  int res = IOT_Linkkit_Report(linkkit_dev_id_, ITM_MSG_POST_PROPERTY,
                               (unsigned char *)data, strlen(data));
  LOG_INFO("Post Property Message : %s\n", data);
  LOG_INFO("Post Property Message ID: %d\n", res);
  const char result[1024] = "{\"LowPowerSwitch\":0}";
  res = IOT_Linkkit_Report(linkkit_dev_id_, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)result, strlen(result));
  LOG_INFO("Post Property Message : %s\n", result);
  LOG_INFO("Post Property Message ID: %d\n", res);
}

void LinkKitApi::ReportWakeUpData1() {
  const char data[1024] = "{\"WakeUpData\":\"rockchip\"}";
  int res = IOT_Linkkit_Report(linkkit_dev_id_, ITM_MSG_POST_PROPERTY,
                               (unsigned char *)data, strlen(data));
  LOG_INFO("Post Property Message : %s\n", data);
  LOG_INFO("Post Property Message ID: %d\n", res);
  const char result[1024] = "{\"LowPowerSwitch\":1}";
  res = IOT_Linkkit_Report(linkkit_dev_id_, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)result, strlen(result));
  LOG_INFO("Post Property Message : %s\n", result);
  LOG_INFO("Post Property Message ID: %d\n", res);
}

int LinkKitApi::ReportBatteryLevel() {
  int fd;
  int battery_level;
  char battery_level_buf[4];

  fd = open("/sys/class/power_supply/rk-bat/capacity", O_RDONLY);
  if (fd < 0) {
    LOG_ERROR("failed to open /sys/class/power_supply/rk-bat/capacity\n");
    return -1;
  }
  if (read(fd, battery_level_buf, 4) < 0) {
    LOG_ERROR("failed to read battery level\n");
    return -1;
  }
  close(fd);
  battery_level = atoi(battery_level_buf);
  LOG_INFO("battery_level is %d\n", battery_level);
  ReportProperty("BatteryLevel", &battery_level);

  return 0;
}

int LinkKitApi::GetAllProperty() {
  // for (const auto& search:property_map) {
  //     SetProperty(search.first.c_str(), search.second.c_str());
  // }
  int ret = 0;
  for (const auto &search : property_map_int) {
    ReportProperty(search.first.c_str(), &search.second);
  }
  ReportWakeUpData0();
  ret &= ReportBatteryLevel();
  ret &= ReportStorageCapacity();
  LOG_INFO("ret is %d\n", ret);

  return ret;
}

void LinkKitApi::InvasionPictureUpload(const char *fpath) {
  LOG_INFO("InvasionPictureUpload, fpath is %s\n", fpath);
  PictureUpload(fpath, LV_EVENT_REGIONAL_INVASION);
}

void LinkKitApi::FacePictureUpload(const char *fpath) {
  LOG_INFO("FacePictureUpload, fpath is %s\n", fpath);
  switch (face_event_id_) {
  case 12:
    PictureUpload(fpath, LV_EVENT_LAUGH);
    break;
  case 11:
    PictureUpload(fpath, LV_EVENT_CRY);
    break;
  case 10:
    PictureUpload(fpath, LV_EVENT_ABNORMAL_SOUND);
    break;
  case 9:
    PictureUpload(fpath, LV_EVENT_SMILING);
    break;
  case 8:
  default:
    PictureUpload(fpath, LV_EVENT_FACE);
  }
  LOG_INFO("face_event_id_ is %d\n", face_event_id_);
  if (face_event_id_ < 12)
    face_event_id_++;
  else
    face_event_id_ = 8;
}

void LinkKitApi::FailPictureUpload(const char *fpath) {
  LOG_INFO("FailPictureUpload, fpath is %s\n", fpath);
  PictureUpload(fpath, LV_EVENT_FALL);
}

// 读取一个图片文件并上传
void LinkKitApi::PictureUpload(const char *fpath, lv_event_type_e type) {
  char *buf = new char[1024 * 1024];
  FILE *fp = nullptr;
  int ret = 0;
  if ((fp = fopen(fpath, "r")) == nullptr) {
    LOG_ERROR("Failed to open file.\n");
    delete[] buf;
    return;
  }
  ret = fseek(fp, 0, SEEK_SET);
  if (ret != 0) {
    fclose(fp);
    delete[] buf;
    return;
  }
  ret = fread(buf, 1, 1024 * 1024, fp);
  fclose(fp);

  if (ret > 0)
    // lv_post_alarm_image(buf, ret, type, NULL);
  delete[] buf;
}

void LinkKitApi::PostEventPicUploadResult(std::string payload) {
  int res = 0;
  char *event_id = (char *)"PicUploadResult";
  std::string search_str = "triggerTime";
  std::string trigger_time = payload.substr(payload.find(search_str) + 12, 13);
  search_str = "eventType";
  int event_type = stoi(payload.substr(payload.find(search_str) + 10, 2));
  if (event_type >= 8)
    event_type = 8;

  // cJSON can't deserialize " to \"
  std::string payload_t = "{\"Result\": 0,\"RKNNInfo\": \"{"
                          "\\\"TriggerTime\\\": \\\"" +
                          trigger_time + "\\\","
                                         "\\\"EventType\\\": " +
                          std::to_string(event_type);
  if (event_type == 8) {
    payload_t = payload_t + ", \\\"Id\\\": 1," + "\\\"Score\\\": 98.00," +
                "\\\"Age\\\": 24,"
                "\\\"Gender\\\": \\\"male\\\"";
  }
  payload_t = payload_t + "}\"}";
  LOG_INFO("payload_t is %s\n", payload_t.c_str());

  res = IOT_Linkkit_TriggerEvent(0, event_id, strlen(event_id),
                                 (char *)payload_t.c_str(), payload_t.size());
  LOG_INFO("Post Event Message ID: %d\n", res);
}

void LinkKitApi::PostAlarmEvent() {
  int res = 0;
  char *event_id = (char *)"AlarmEvent";
  std::string payload_t = "{\"AlarmType\":1,\"AlarmPicID\":\"wake\",\"Data\":\"all\"}";

  res = IOT_Linkkit_TriggerEvent(0, event_id, strlen(event_id),
                                 (char *)payload_t.c_str(), payload_t.size());
  LOG_INFO("Post Event Message ID: %d\n", res);
}

} // namespace mediaserver
} // namespace rockchip

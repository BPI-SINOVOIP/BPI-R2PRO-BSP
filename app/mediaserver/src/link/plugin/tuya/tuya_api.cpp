#include "tuya_api.h"
#include "buffer.h"
#include "fcntl.h"
#include "flow.h"
#include "stream.h"

#include "flow_common.h"
#include "flow_unit.h"
#include "thread.h"

#ifdef ENABLE_CY43438
extern "C"{
#include  "../../../utils/CY_WL_API/wifi.h"
}
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "tuya_api.cpp"

// Path to save tuya sdk DB files, should be readable, writeable and storable
#define IPC_APP_STORAGE_PATH "/oem/"
// File with path to download file during OTA
#define IPC_APP_UPGRADE_FILE "/tmp/upgrade.file"
// SD card mount directory
#define IPC_APP_SD_BASE_PATH "/mnt/sdcard/"
// Firmware version displayed on TUYA APP
#define IPC_APP_VERSION "1.2.3"
// TUYA P2P supports 5 users at the same time, include live preview and playback
#define MAX_P2P_USER 5
#define TUYA_DP_DOOR_STATUS 149

namespace rockchip {
namespace mediaserver {
static int g_init = 0;
// IPC_MEDIA_INFO_S TuyaApi::s_media_info;
bool TuyaApi::cloud_connected_ = 0;
MediaControlCB TuyaApi::media_control_callback_ = nullptr;

void TuyaApi::MediaControl(IpcMediaCmd cmd, const IpcMediaParam *param) {
  if (media_control_callback_)
    media_control_callback_(cmd, param);
}

void TuyaApi::FillMediaParam(MediaParamType type, unsigned int value) {
  LOG_INFO("FillMediaParam\n");
  switch (type) {
  case MEDIA_PARAM_VIDEO_FPS:
    s_media_info.video_fps[E_CHANNEL_VIDEO_MAIN] = value;
    break;
  case MEDIA_PARAM_VIDEO_FMT: {
    if (value == MEDIA_PARAM_VIDEO_FORMAT_H264)
      s_media_info.video_fps[E_CHANNEL_VIDEO_MAIN] = TUYA_CODEC_VIDEO_H264;
    else if (value == MEDIA_PARAM_VIDEO_FORMAT_H265)
      s_media_info.video_fps[E_CHANNEL_VIDEO_MAIN] = TUYA_CODEC_VIDEO_H265;
    else
      LOG_ERROR("MEDIA_PARAM_VIDEO_FMT %d is not support\n", value);
    break;
  }
  case MEDIA_PARAM_VIDEO_WIDTH: {
    s_media_info.video_width[E_CHANNEL_VIDEO_MAIN] = value;
    break;
  }
  case MEDIA_PARAM_VIDEO_HEIGHT: {
    s_media_info.video_height[E_CHANNEL_VIDEO_MAIN] = value;
    break;
  }
  case MEDIA_PARAM_VIDEO_GOP: {
    s_media_info.video_gop[E_CHANNEL_VIDEO_MAIN] = value;
    break;
  }
  case MEDIA_PARAM_AUDIO_FMT: {
    if (value == MEDIA_PARAM_AUDIO_FORMAT_PCM)
      s_media_info.audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_PCM;
    else if (value == MEDIA_PARAM_AUDIO_FORMAT_G711A)
      s_media_info.audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_G711A;
    else if (value == MEDIA_PARAM_AUDIO_FORMAT_MP3)
      s_media_info.audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_MP3_RAW;
    else
      LOG_ERROR("MEDIA_PARAM_AUDIO_FMT %d is not support\n", value);
    break;
  }
  case MEDIA_PARAM_AUDIO_SAMPLE_RATE: {
    if (value == MEDIA_PARAM_AUDIO_SAMPLE_RATE_8000)
      s_media_info.audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_8K;
    else if (value == MEDIA_PARAM_AUDIO_SAMPLE_RATE_16000)
      s_media_info.audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_16K;
    else if (value == MEDIA_PARAM_AUDIO_SAMPLE_RATE_48000)
      s_media_info.audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_48K;
    else
      LOG_ERROR("MEDIA_PARAM_AUDIO_SAMPLE_RATE %d is not support\n", value);
    break;
  }
  case MEDIA_PARAM_AUDIO_SAMPLE_BITS: {
    if (value == MEDIA_PARAM_AUDIO_SAMPLE_BITS_8BIT)
      s_media_info.audio_databits[E_CHANNEL_AUDIO] = TUYA_AUDIO_DATABITS_8;
    else if (value == MEDIA_PARAM_AUDIO_SAMPLE_BITS_16BIT)
      s_media_info.audio_databits[E_CHANNEL_AUDIO] = TUYA_AUDIO_DATABITS_16;
    else
      LOG_ERROR("MEDIA_PARAM_AUDIO_SAMPLE_BITS %d is not support\n", value);
    break;
  }
  case MEDIA_PARAM_AUDIO_CHANNLE: {
    if (value == MEDIA_PARAM_AUDIO_CHANNEL_STEREO)
      s_media_info.audio_channel[E_CHANNEL_AUDIO] = TUYA_AUDIO_CHANNEL_STERO;
    else
      s_media_info.audio_channel[E_CHANNEL_AUDIO] = TUYA_AUDIO_CHANNEL_MONO;
    break;
  }
  default:
    break;
  }
}

/* Set audio and video properties */
void TuyaApi::IPC_APP_Set_Media_Info(void) {
  /* main stream(HD), video configuration*/
  /* NOTE
  FIRST:If the main stream supports multiple video stream configurations, set
  each item to the upper limit of the allowed configuration.
  SECOND:E_CHANNEL_VIDEO_MAIN must exist.It is the data source of SDK.
  please close the E_CHANNEL_VIDEO_SUB for only one stream*/
  s_media_info.channel_enable[E_CHANNEL_VIDEO_MAIN] = TRUE;
  s_media_info.video_bitrate[E_CHANNEL_VIDEO_MAIN] = TUYA_VIDEO_BITRATE_1_5M;
  s_media_info.video_freq[E_CHANNEL_VIDEO_MAIN] = 90000; /* Clock frequency */

  /* Audio stream configuration.
  Note: The internal P2P preview, cloud storage, and local storage of the SDK
  are all use E_CHANNEL_AUDIO data. */
  s_media_info.channel_enable[E_CHANNEL_AUDIO] = TRUE;
  s_media_info.audio_fps[E_CHANNEL_AUDIO] = 25; /* Fragments per second */

  LOG_INFO("channel_enable:%d %d %d\n", s_media_info.channel_enable[0],
           s_media_info.channel_enable[1], s_media_info.channel_enable[2]);

  LOG_INFO("fps:%u\n", s_media_info.video_fps[E_CHANNEL_VIDEO_MAIN]);
  LOG_INFO("gop:%u\n", s_media_info.video_gop[E_CHANNEL_VIDEO_MAIN]);
  LOG_INFO("bitrate:%u kbps\n",
           s_media_info.video_bitrate[E_CHANNEL_VIDEO_MAIN]);
  LOG_INFO("video_main_width:%u\n",
           s_media_info.video_width[E_CHANNEL_VIDEO_MAIN]);
  LOG_INFO("video_main_height:%u\n",
           s_media_info.video_height[E_CHANNEL_VIDEO_MAIN]);
  LOG_INFO("video_freq:%u\n", s_media_info.video_freq[E_CHANNEL_VIDEO_MAIN]);
  LOG_INFO("video_codec:%d\n", s_media_info.video_codec[E_CHANNEL_VIDEO_MAIN]);

  LOG_INFO("audio_codec:%d\n", s_media_info.audio_codec[E_CHANNEL_AUDIO]);
  LOG_INFO("audio_sample:%d\n", s_media_info.audio_sample[E_CHANNEL_AUDIO]);
  LOG_INFO("audio_databits:%d\n", s_media_info.audio_databits[E_CHANNEL_AUDIO]);
  LOG_INFO("audio_channel:%d\n", s_media_info.audio_channel[E_CHANNEL_AUDIO]);
}

int TuyaApi::TUYA_APP_Init_Ring_Buffer(void) {
  static bool s_ring_buffer_inited = false;
  if (s_ring_buffer_inited == true) {
    LOG_INFO("The Ring Buffer Is Already Inited\n");
    return OPRT_OK;
  }

  int i;
  int ret;
  for (i = E_CHANNEL_VIDEO_MAIN; i < E_CHANNEL_MAX; i++) {
    CHANNEL_E channel = CHANNEL_E(i);
    LOG_DEBUG("init ring buffer Channel:%d Enable:%d\n", channel,
              s_media_info.channel_enable[channel]);
    if (s_media_info.channel_enable[channel] == TRUE) {
      if (channel == E_CHANNEL_AUDIO) {
        LOG_INFO("audio_sample %d, audio_databits %d, audio_fps %d\n",
                 s_media_info.audio_sample[E_CHANNEL_AUDIO],
                 s_media_info.audio_databits[E_CHANNEL_AUDIO],
                 s_media_info.audio_fps[E_CHANNEL_AUDIO]);
        ret = tuya_ipc_ring_buffer_init(
            channel, s_media_info.audio_sample[E_CHANNEL_AUDIO] *
                         s_media_info.audio_databits[E_CHANNEL_AUDIO] / 1024,
            s_media_info.audio_fps[E_CHANNEL_AUDIO], 0, NULL);
      } else {
        LOG_INFO("video_bitrate %d, video_fps %d\n",
                 s_media_info.video_bitrate[channel],
                 s_media_info.video_fps[channel]);
        ret = tuya_ipc_ring_buffer_init(
            channel, s_media_info.video_bitrate[channel],
            s_media_info.video_fps[channel], 0, NULL);
      }
      if (ret != 0) {
        LOG_ERROR("init ring buffer fails. %d %d\n", channel, ret);
        return OPRT_MALLOC_FAILED;
      }
      LOG_INFO("init ring buffer success. channel:%d\n", channel);
    }
  }

  s_ring_buffer_inited = TRUE;

  return OPRT_OK;
}

void __IPC_APP_Get_Net_Status_cb(unsigned char stat) {
  LOG_INFO("Net status change to:%d\n", stat);
  switch (stat) {
  case STAT_CLOUD_CONN:  // for wifi ipc
  case STAT_MQTT_ONLINE: // for low-power wifi ipc
  case GB_STAT_CLOUD_CONN: // for wired ipc
  {
    // IPC_APP_Notify_LED_Sound_Status_CB(IPC_MQTT_ONLINE);
    LOG_INFO("mqtt is online\r\n");
    TuyaApi::cloud_connected_ = 1;
    break;
  }
  default: { break; }
  }
}

int TuyaApi::IPC_APP_Init_SDK(WIFI_INIT_MODE_E init_mode, char *p_token) {
  LOG_INFO("SDK Version:%s\r\n", tuya_ipc_get_sdk_info());

  IPC_APP_Set_Media_Info();
  TUYA_APP_Init_Ring_Buffer();
  // IPC_APP_Notify_LED_Sound_Status_CB(IPC_BOOTUP_FINISH);

  TUYA_IPC_ENV_VAR_S env;
  memset(&env, 0, sizeof(TUYA_IPC_ENV_VAR_S));
  strcpy(env.storage_path, IPC_APP_STORAGE_PATH);
  strcpy(env.product_key, license_.pid.c_str());
  strcpy(env.uuid, license_.uuid.c_str());
  strcpy(env.auth_key, license_.authkey.c_str());
  strcpy(env.dev_sw_version, IPC_APP_VERSION);
  strcpy(env.dev_serial_num, "tuya_ipc");
  // env.dev_obj_dp_cb = IPC_APP_handle_dp_cmd_objs;
  // env.dev_dp_query_cb = IPC_APP_handle_dp_query_objs;
  env.status_changed_cb = __IPC_APP_Get_Net_Status_cb;
  // env.gw_ug_cb = IPC_APP_Upgrade_Inform_cb;
  // env.gw_rst_cb = IPC_APP_Reset_System_CB;
  // env.gw_restart_cb = IPC_APP_Restart_Process_CB;
  env.mem_save_mode = FALSE;
  LOG_INFO("tuya_ipc_init_sdk\n");
  tuya_ipc_init_sdk(&env);

  LOG_INFO("init_mode is %d, p_token is %s\n", init_mode, p_token);
  tuya_ipc_start_sdk(init_mode, p_token);

  return OPRT_OK;
}

int IPC_APP_Sync_Utc_Time(VOID) {
  TIME_T time_utc;
  INT_T time_zone;
  LOG_INFO("Get Server Time\n");
  int ret = tuya_ipc_get_service_time_force(&time_utc, &time_zone);
  if (ret != OPRT_OK) {
    return ret;
  }
  // The API returns OK, indicating that UTC time has been successfully
  // obtained.
  // If it return not OK, the time has not been fetched.

  LOG_INFO("Get Server Time Success: %lu %d\n", time_utc, time_zone);
  return OPRT_OK;
}

typedef struct {
  BOOL_T enabled;
  TRANSFER_VIDEO_CLARITY_TYPE_E live_clarity;
  UINT_T max_users;
  TUYA_CODEC_ID p2p_audio_codec;
} TUYA_APP_P2P_MGR;

STATIC TUYA_APP_P2P_MGR s_p2p_mgr = {0};

STATIC VOID __depereated_online_cb(IN TRANSFER_ONLINE_E status) {
  LOG_INFO("status is %d\n", status);
}

// 语音对讲的全局变量
std::shared_ptr<easymedia::Stream> out_stream;
std::string alsa_device = "default";
MediaConfig pcm_config;
AudioConfig &aud_cfg = pcm_config.aud_cfg;
SampleInfo &sample_info = aud_cfg.sample_info;

int TUYA_APP_Enable_Speaker_CB(int enable) {
  if (enable) {
    // 输出流的格式配置
    sample_info.fmt = SAMPLE_FMT_G711U;
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
  } else {
    out_stream.reset();
  }

  return 0;
}

/* Callback functions for transporting events */
STATIC VOID __TUYA_APP_p2p_event_cb(IN CONST TRANSFER_EVENT_E event,
                                    IN CONST PVOID_T args) {
  LOG_INFO("p2p rev event cb=[%d]\n", event);
  switch (event) {
  case TRANS_LIVE_VIDEO_START: {
    C2C_TRANS_CTRL_VIDEO_START *parm = (C2C_TRANS_CTRL_VIDEO_START *)args;
    LOG_INFO("chn[%u] video start\n", parm->channel);
#ifdef THUNDER_BOOT
    FILE *fp;
    fp = fopen("/tmp/rtmp_live", "a");
    if (fp)
      fclose(fp);
#endif
    break;
  }
  case TRANS_LIVE_VIDEO_STOP: {
    C2C_TRANS_CTRL_VIDEO_STOP *parm = (C2C_TRANS_CTRL_VIDEO_STOP *)args;
    LOG_INFO("chn[%u] video stop\n", parm->channel);
#ifdef THUNDER_BOOT
    unlink("/tmp/rtmp_live");
#endif
    break;
  }
  case TRANS_LIVE_AUDIO_START: {
    C2C_TRANS_CTRL_AUDIO_START *parm = (C2C_TRANS_CTRL_AUDIO_START *)args;
    LOG_INFO("chn[%u] audio start\n", parm->channel);
    break;
  }
  case TRANS_LIVE_AUDIO_STOP: {
    C2C_TRANS_CTRL_AUDIO_STOP *parm = (C2C_TRANS_CTRL_AUDIO_STOP *)args;
    LOG_INFO("chn[%u] audio stop\n", parm->channel);
    break;
  }
  case TRANS_SPEAKER_START: {
    LOG_INFO("enbale audio speaker\n");
    TUYA_APP_Enable_Speaker_CB(TRUE);
    break;
  }
  case TRANS_SPEAKER_STOP: {
    LOG_INFO("disable audio speaker\n");
    TUYA_APP_Enable_Speaker_CB(FALSE);
    break;
  }
  case TRANS_LIVE_LOAD_ADJUST: {
    C2C_TRANS_LIVE_LOAD_PARAM_S *quality = (C2C_TRANS_LIVE_LOAD_PARAM_S *)args;
    LOG_INFO("live quality %d -> %d\n", quality->curr_load_level,
             quality->new_load_level);
    break;
  }
  case TRANS_PLAYBACK_LOAD_ADJUST: {
    C2C_TRANS_PB_LOAD_PARAM_S *quality = (C2C_TRANS_PB_LOAD_PARAM_S *)args;
    LOG_INFO("pb idx:%d quality %d -> %d\n", quality->client_index,
             quality->curr_load_level, quality->new_load_level);
    break;
  }
  case TRANS_ABILITY_QUERY: {
    C2C_TRANS_QUERY_FIXED_ABI_REQ *pAbiReq;
    pAbiReq = (C2C_TRANS_QUERY_FIXED_ABI_REQ *)args;
    pAbiReq->ability_mask = TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_VIDEO |
                            TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_SPEAKER |
                            TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_MIC;
    break;
  }
  case TRANS_PLAYBACK_QUERY_MONTH_SIMPLIFY:
  case TRANS_PLAYBACK_QUERY_DAY_TS:
  case TRANS_PLAYBACK_START_TS:
  case TRANS_PLAYBACK_PAUSE:
  case TRANS_PLAYBACK_RESUME:
  case TRANS_PLAYBACK_MUTE:
  case TRANS_PLAYBACK_UNMUTE:
  case TRANS_PLAYBACK_STOP:
    break;
  case TRANS_LIVE_VIDEO_CLARITY_SET: {
    C2C_TRANS_LIVE_CLARITY_PARAM_S *pParam =
        (C2C_TRANS_LIVE_CLARITY_PARAM_S *)args;
    LOG_INFO("set clarity:%d\n", pParam->clarity);
    if ((pParam->clarity == TY_VIDEO_CLARITY_STANDARD) ||
        (pParam->clarity == TY_VIDEO_CLARITY_HIGH)) {
      LOG_INFO("set clarity:%d OK\n", pParam->clarity);
      s_p2p_mgr.live_clarity = pParam->clarity;
    }
    break;
  }
  case TRANS_LIVE_VIDEO_CLARITY_QUERY: {
    C2C_TRANS_LIVE_CLARITY_PARAM_S *pParam =
        (C2C_TRANS_LIVE_CLARITY_PARAM_S *)args;
    pParam->clarity = s_p2p_mgr.live_clarity;
    LOG_INFO("query larity:%d\n", pParam->clarity);
    break;
  }
  case TRANS_DOWNLOAD_START:
  case TRANS_DOWNLOAD_STOP:
  case TRANS_DOWNLOAD_PAUSE:
  case TRANS_DOWNLOAD_RESUME:
  case TRANS_DOWNLOAD_CANCLE:
    break;
  case TRANS_STREAMING_VIDEO_START: {
    TRANSFER_SOURCE_TYPE_E *pSrcType = (TRANSFER_SOURCE_TYPE_E *)args;
    LOG_INFO("streaming start type %d\n", *pSrcType);
    break;
  }
  case TRANS_STREAMING_VIDEO_STOP: {
    TRANSFER_SOURCE_TYPE_E *pSrcType = (TRANSFER_SOURCE_TYPE_E *)args;
    LOG_INFO("streaming stop type %d\n", *pSrcType);
    break;
  }
  default:
    break;
  }
}

STATIC VOID __TUYA_APP_rev_audio_cb(
    IN CONST TRANSFER_AUDIO_FRAME_S *p_audio_frame, IN CONST UINT_T frame_no) {
  // MEDIA_FRAME_S audio_frame;
  // audio_frame.p_buf = p_audio_frame->p_audio_buf;
  // audio_frame.size = p_audio_frame->buf_len;
  // LOG_DEBUG("Rev Audio. size:%u audio_codec:%d audio_sample:%d "
  //          "audio_databits:%d audio_channel:%d\n",
  //          p_audio_frame->buf_len, p_audio_frame->audio_codec,
  //          p_audio_frame->audio_sample, p_audio_frame->audio_databits,
  //          p_audio_frame->audio_channel);
  out_stream->Write(p_audio_frame->p_audio_buf, p_audio_frame->buf_len, 1);
}

int TuyaApi::TUYA_APP_Enable_P2PTransfer(IN UINT_T max_users) {
  if (s_p2p_mgr.enabled == TRUE) {
    LOG_INFO("P2P Is Already Inited\n");
    return OPRT_OK;
  }

  LOG_INFO("Init P2P With Max Users:%u\n", max_users);

  s_p2p_mgr.enabled = TRUE;
  s_p2p_mgr.max_users = max_users;
  s_p2p_mgr.p2p_audio_codec = s_media_info.audio_codec[E_CHANNEL_AUDIO];

  TUYA_IPC_TRANSFER_VAR_S p2p_var = {0};
  p2p_var.online_cb = __depereated_online_cb;
  p2p_var.on_rev_audio_cb = __TUYA_APP_rev_audio_cb;
  /*speak data format  app->ipc*/
  p2p_var.rev_audio_codec = TUYA_CODEC_AUDIO_PCM;
  p2p_var.audio_sample = TUYA_AUDIO_SAMPLE_8K;
  p2p_var.audio_databits = TUYA_AUDIO_DATABITS_16;
  p2p_var.audio_channel = TUYA_AUDIO_CHANNEL_MONO;
  /*end*/
  p2p_var.on_event_cb = __TUYA_APP_p2p_event_cb;
  p2p_var.live_quality = TRANS_LIVE_QUALITY_MAX;
  p2p_var.max_client_num = max_users;
  memcpy(&p2p_var.AVInfo, &s_media_info, sizeof(IPC_MEDIA_INFO_S));
  tuya_ipc_tranfser_init(&p2p_var);

  return OPRT_OK;
}

int TuyaApi::FillLicenseKey(pLicenseKey plicense) {
  if (plicense) {
    license_.pid = plicense->pid;
    license_.uuid = plicense->uuid;
    license_.authkey = plicense->authkey;
  }
  return 0;
}

static void *p2p_init(void *arg) {
  auto os = reinterpret_cast<TuyaApi *>(arg);
  // char thread_name[40];
  // snprintf(thread_name, sizeof(thread_name), "p2p_init");
  // prctl(PR_SET_NAME, thread_name);

  /* Enable TUYA P2P service after the network is CONNECTED.
     Note: For low-power camera, invoke this API as early as possible(can be
     before mqtt online) */
  LOG_INFO("TUYA_APP_Enable_P2PTransfer begin\n");
  os->TUYA_APP_Enable_P2PTransfer(MAX_P2P_USER);
  LOG_INFO("TUYA_APP_Enable_P2PTransfer over\n");

  return 0;
}

int TuyaApi::InitDevice() {
  LOG_INFO("before init tuya\n");
  if (g_init) {
    printf("tuya already init\n");
    return 0;
  }

  WIFI_INIT_MODE_E mode = WIFI_INIT_AUTO; // for wired and wifi qrcode
  // The demo mode is set to WIFI_INIT_DEBUG,
  // token needs to be scanned with WeChat to get a ten-minute valid.
  // so before running this main process,
  // developers need to make sure that devices are connected to the Internet.
#if 1
  IPC_APP_Init_SDK(mode, NULL);
#else
  IPC_APP_Init_SDK(WIFI_INIT_DEBUG, "AYgTyyjwMfeZ2W");
#endif
  LOG_INFO("after IPC_APP_Init_SDK\n");

  Thread::UniquePtr p2p_init_thread;
  p2p_init_thread.reset(new Thread(p2p_init, this));


  MediaControl(IPC_MEDIA_START, NULL);

  /* whether SDK is connected to MQTT */
  while (cloud_connected_ != 1) {
    usleep(100000);
  }

  TUYA_APP_LOW_POWER_DISABLE(); // report device online
  /*At least one system time synchronization after networking*/
  IPC_APP_Sync_Utc_Time();
  LOG_INFO("after IPC_APP_Sync_Utc_Time\n");
  /* Upload all local configuration item (DP) status when MQTT connection is
   * successful */
  // IPC_APP_upload_all_status();
  // TUYA_APP_Enable_CloudStorage();
  // TUYA_APP_Enable_AI_Detect();
  /*!!!very important! After all module inited, update skill to tuya cloud */
  tuya_ipc_upload_skills();
  g_init = 1;
  LOG_INFO("tuya init ok\n");

  return 0;
}

int TuyaApi::DeInitDevice() {
  LOG_INFO("DeInitDevice\n");
  if (!g_init) {
    LOG_ERROR("Tuya device is not init\n");
    return -1;
  }

  return 0;
}

int TUYA_APP_Put_Frame(IN CONST CHANNEL_E channel,
                       IN CONST MEDIA_FRAME_S *p_frame) {
  LOG_DEBUG("Put Frame. Channel:%d type:%d size:%u pts:%llu ts:%llu\n", channel,
            p_frame->type, p_frame->size, p_frame->pts, p_frame->timestamp);

  int ret = tuya_ipc_ring_buffer_append_data(
      channel, p_frame->p_buf, p_frame->size, p_frame->type, p_frame->pts);

  if (ret != OPRT_OK) {
    LOG_INFO("Put Frame Fail.%d Channel:%d type:%d size:%u pts:%llu ts:%llu\n",
             ret, channel, p_frame->type, p_frame->size, p_frame->pts,
             p_frame->timestamp);
  }
  return ret;
}

static MEDIA_FRAME_S h264_frame;
void TuyaApi::PushVideoHandler(unsigned char *buffer, unsigned int buffer_size,
                               int64_t present_time, int nal_type) {
  memset(&h264_frame, 0, sizeof(h264_frame));
  if (nal_type)
    h264_frame.type = E_VIDEO_I_FRAME;
  else
    h264_frame.type = E_VIDEO_PB_FRAME;
  h264_frame.p_buf = buffer;
  h264_frame.size = buffer_size;
  h264_frame.pts = present_time;
  TUYA_APP_Put_Frame(E_CHANNEL_VIDEO_MAIN, &h264_frame);
}

static MEDIA_FRAME_S pcm_frame;
void TuyaApi::PushAudioHandler(unsigned char *buffer, unsigned int buffer_size,
                               int64_t present_time) {
  memset(&pcm_frame, 0, sizeof(pcm_frame));
  pcm_frame.type = E_AUDIO_FRAME;
  pcm_frame.p_buf = buffer;
  pcm_frame.size = buffer_size;
  pcm_frame.pts = present_time;
  TUYA_APP_Put_Frame(E_CHANNEL_AUDIO, &pcm_frame);
}

void TuyaApi::PushCaptureHandler(unsigned char *buffer,
                                 unsigned int buffer_size, int type,
                                 const char *id) {}

int TuyaApi::ConnectLink() { return 0; }

int TuyaApi::StartLink() { return 0; }

int TuyaApi::StopLink() {
  LOG_INFO("StopLink\n");
  return 0;
}

static unsigned char s_wakeup_data[32] = {0};
static unsigned int s_heartbeat_len = 32;
static unsigned char s_heartbeat_data[32] = {0};
static unsigned int s_wakeup_len = 32;
static int s_mqtt_socket_fd = -1;

int TuyaApi::TUYA_APP_LOW_POWER_ENABLE() {
  LOG_INFO("tuya low power mode enable\n");
  BOOL_T doorStat = FALSE;
  int ret = 0;

  // Report sleep status to tuya
  ret = tuya_ipc_dp_report(NULL, TUYA_DP_DOOR_STATUS, PROP_BOOL, &doorStat, 1);
  if (OPRT_OK != ret) {
    // This dp point is very important. The user should repeat call the report
    // interface until the report is successful when it is fail.
    // If it is failure continues, the user needs to check the network
    // connection.
    LOG_ERROR("dp report failed\n");
    return ret;
  }
  ret = tuya_ipc_book_wakeup_topic();
  if (OPRT_OK != ret) {
    LOG_ERROR("tuya_ipc_book_wakeup_topic failed\n");
    return ret;
  }

  // Get fd for server to wakeup
  s_mqtt_socket_fd = tuya_ipc_get_mqtt_socket_fd();
  if (-1 == s_mqtt_socket_fd) {
    LOG_ERROR("tuya_ipc_get_mqtt_socket_fd failed\n");
    return ret;
  }
  LOG_INFO("s_mqtt_socket_fd is %d\n", s_mqtt_socket_fd);

#ifdef ENABLE_CY43438
  ret = WIFI_Suspend(s_mqtt_socket_fd, true);
  LOG_INFO("WIFI_Suspend, ret is %d\n", ret);
#endif

  ret = tuya_ipc_get_wakeup_data(s_wakeup_data, &s_wakeup_len);
  if (OPRT_OK != ret) {
    LOG_ERROR("tuya_ipc_get_wakeup_data failed\n");
    return ret;
  }
  LOG_INFO("s_wakeup_data is \n");
  for (int i = 0; i < s_wakeup_len; i++) {
    printf("%x ", s_wakeup_data[i]);
  }
  printf("\n");

  ret = tuya_ipc_get_heartbeat_data(s_heartbeat_data, &s_heartbeat_len);
  if (OPRT_OK != ret) {
    LOG_ERROR("tuya_ipc_get_heartbeat_data failed\n");
    return ret;
  }
  LOG_INFO("s_heartbeat_data is \n");
  for (int i = 0; i < s_heartbeat_len; i++) {
    printf("%x ", s_heartbeat_data[i]);
  }
  printf("\n");

  return ret;
}

int TuyaApi::TUYA_APP_LOW_POWER_DISABLE() {
  BOOL_T doorStat = TRUE;
  int ret = 0;
  ret = tuya_ipc_dp_report(NULL, TUYA_DP_DOOR_STATUS, PROP_BOOL, &doorStat, 1);

  return ret;
}

void TuyaApi::ReportWakeUpData1() {
  // 通知服务器，设备将要准备进入睡眠状态
  TUYA_APP_LOW_POWER_ENABLE();
}

} // namespace mediaserver
} // namespace rockchip

#ifndef _RK_LINK_COMMON_H_
#define _RK_LINK_COMMON_H_

namespace rockchip {
namespace mediaserver {

typedef struct {
#ifdef LINK_API_ENABLE_LINKKIT
  std::string product_key;
  std::string device_name;
  std::string device_secret;
  std::string product_secret;
#elif defined LINK_API_ENABLE_TUYA
  std::string pid;
  std::string uuid;
  std::string authkey;
#endif
} LicenseKey, *pLicenseKey;

typedef enum {
  MEDIA_PARAM_VIDEO_FPS = 0,
  MEDIA_PARAM_VIDEO_FMT,
  MEDIA_PARAM_VIDEO_FRAME_INTERVAL,
  MEDIA_PARAM_VIDEO_GOP,
  MEDIA_PARAM_VIDEO_WIDTH,
  MEDIA_PARAM_VIDEO_HEIGHT,
  MEDIA_PARAM_AUDIO_FMT,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE,
  MEDIA_PARAM_AUDIO_SAMPLE_BITS,
  MEDIA_PARAM_AUDIO_CHANNLE,
  MEDIA_PARAM_FILE_DURATION,
} MediaParamType;

typedef enum {
  MEDIA_PARAM_VIDEO_FORMAT_H264 = 0,
  MEDIA_PARAM_VIDEO_FORMAT_H265,
} MediaParamVideoFormat;

typedef enum {
  MEDIA_PARAM_AUDIO_FORMAT_PCM = 0,
  MEDIA_PARAM_AUDIO_FORMAT_G711A = 1,
  MEDIA_PARAM_AUDIO_FORMAT_MP3 = 2,
} MediaParamAudioFormat;

typedef enum {
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_96000 = 0,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_88200 = 1,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_64000 = 2,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_48000 = 3,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_44100 = 4,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_32000 = 5,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_24000 = 6,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_22050 = 7,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_16000 = 8,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_12000 = 9,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_11025 = 10,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_8000 = 11,
  MEDIA_PARAM_AUDIO_SAMPLE_RATE_7350 = 12,
} MediaParamAudioSampleRate;

typedef enum {
  MEDIA_PARAM_AUDIO_SAMPLE_BITS_8BIT = 0,
  MEDIA_PARAM_AUDIO_SAMPLE_BITS_16BIT = 1,
} MediaParamAudioSampleBits;

typedef enum {
  MEDIA_PARAM_AUDIO_CHANNEL_MONO = 0,
  MEDIA_PARAM_AUDIO_CHANNEL_STEREO = 1,
} MediaParamAudioChannel;

typedef enum {
  IPC_MEDIA_START = 0,
  IPC_MEDIA_PAUSE,
  IPC_MEDIA_UNPAUSE,
  IPC_MEDIA_STOP,
  IPC_MEDIA_SEEK,
  IPC_MEDIA_REQUEST_I_FRAME,
  IPC_MEDIA_CHANGE_STREAM, //切换主子码流
} IpcMediaCmd;

typedef struct {
  int service_id;
  unsigned int stream_type;
  unsigned int seek_timestamp_ms;
  unsigned int base_time_ms;
  char source_file[256 + 1];
} IpcMediaParam;

using MediaControlCB =
    std::add_pointer<void(IpcMediaCmd cmd, const IpcMediaParam *param)>::type;

} // namespace mediaserver
} // namespace rockchip

#endif

#include "link_manager.h"

#include <memory>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "link_manager.cpp"

namespace rockchip {
namespace mediaserver {

LinkDeviceStatus LinkManager::device_status = LINK_DEVICE_STATUS_NOEXIST;
LinkStatus LinkManager::link_status = LINK_STATUS_UNINIT;

std::shared_ptr<LinkManager> LinkManager::instance_;

LinkManager::LinkManager() : camera_stream_id_(0), audio_stream_id_(0) {
#ifdef LINK_API_ENABLE_LINKKIT
  LinkApi_.reset(new LinkKitApi());
#elif defined LINK_API_ENABLE_TUYA
  LinkApi_.reset(new TuyaApi());
#else
  LinkApi_.reset(new LinkVirApi());
#endif
}

LinkManager::~LinkManager() {}

int LinkManager::FillLicenseKey(pLicenseKey plicense) {
  LinkApi_->FillLicenseKey(plicense);
  return 0;
}

void LinkManager::FillMediaParam(MediaParamType param, unsigned int value) {
  LinkApi_->FillMediaParam(param, value);
}

std::shared_ptr<FlowPipe> LinkManager::GetLinkPipe(int id,
                                                   std::string input_type) {
  int cnt = 0;
  auto &flow_manager = FlowManager::GetInstance();
  auto pipes = flow_manager->GetPipes();
  for (int pipe_index = 0; pipe_index < pipes.size(); pipe_index++) {
    auto pipe = pipes[pipe_index];
    auto flow_units = pipe->GetFlowUnits();
    for (int flow_index = 0; flow_index < flow_units.size(); flow_index++) {
      auto &flow_unit = flow_units[flow_index];
      auto stream_type = flow_unit->GetStreamType();
      auto stream_name = flow_unit->GetStreamName();
      std::string input_data_type;
      input_data_type = flow_unit->GetFlowInputDataType();
      if (input_data_type.empty())
        input_data_type = flow_unit->GetInputDataType();
      if (stream_type == StreamType::LINK &&
          (input_data_type.find(input_type) != std::string::npos)) {
        if (id == cnt) {
          return pipe;
        } else {
          cnt++;
        }
      }
    }
  }
  return nullptr;
}

std::shared_ptr<FlowUnit> LinkManager::GetLinkFlowUnit(int id,
                                                       std::string input_type) {
  int cnt = 0;
  auto &flow_manager = FlowManager::GetInstance();
  auto pipes = flow_manager->GetPipes();
  for (int pipe_index = 0; pipe_index < pipes.size(); pipe_index++) {
    auto pipe = pipes[pipe_index];
    auto flow_units = pipe->GetFlowUnits();
    for (int flow_index = 0; flow_index < flow_units.size(); flow_index++) {
      auto &flow_unit = flow_units[flow_index];
      auto stream_type = flow_unit->GetStreamType();
      auto stream_name = flow_unit->GetStreamName();
      std::string input_data_type;
      input_data_type = flow_unit->GetFlowInputDataType();
      if (input_data_type.empty())
        input_data_type = flow_unit->GetInputDataType();
      if (stream_type == StreamType::LINK &&
          (input_data_type.find(input_type) != std::string::npos)) {
        if (id == cnt) {
          return flow_unit;
        } else {
          cnt++;
        }
      }
    }
  }
  return nullptr;
}

std::shared_ptr<easymedia::Flow>
LinkManager::GetLinkFlow(int id, std::string input_type) {
  auto flow_unit = GetLinkFlowUnit(id, input_type);
  if (flow_unit) {
    return flow_unit->GetFlow();
  }
  return nullptr;
}

int LinkManager::FillMediaParam() {
  auto camera_pipe = GetLinkPipe(camera_stream_id_, "video:");
  if (camera_pipe) {
    auto link_flow = camera_pipe->GetFlowByInput(StreamType::LINK, "video:");
    if (link_flow) {
      LOG_INFO("link_flow find video\n");
      auto enc_flow_unit = camera_pipe->GetFlowunit(StreamType::VIDEO_ENCODER);
      if (enc_flow_unit) {
        auto &flow = enc_flow_unit->GetFlow();
        auto value = enc_flow_unit->GetFrameRate();
        if (!value.empty()) {
          int fps = atoi(value.c_str());
          FillMediaParam(MEDIA_PARAM_VIDEO_FPS, fps);
        } else {
          FillMediaParam(MEDIA_PARAM_VIDEO_FPS, 30);
        }

        std::string enc_output_data_type = enc_flow_unit->GetOutputDataType();
        if (enc_output_data_type.find("h264") != std::string::npos)
          FillMediaParam(MEDIA_PARAM_VIDEO_FMT, MEDIA_PARAM_VIDEO_FORMAT_H264);
        else if (enc_output_data_type.find("h265") != std::string::npos)
          FillMediaParam(MEDIA_PARAM_VIDEO_FMT, MEDIA_PARAM_VIDEO_FORMAT_H265);
        else
          FillMediaParam(MEDIA_PARAM_VIDEO_FMT, MEDIA_PARAM_VIDEO_FORMAT_H264);

        std::string width_s;
        std::string height_s;
        enc_flow_unit->GetResolution(width_s, height_s);
        FillMediaParam(MEDIA_PARAM_VIDEO_WIDTH, stoi(width_s));
        FillMediaParam(MEDIA_PARAM_VIDEO_HEIGHT, stoi(height_s));

        std::string enc_gop = enc_flow_unit->GetGop();
        FillMediaParam(MEDIA_PARAM_VIDEO_GOP, stoi(enc_gop));
      }
    }
  }

  auto audio_pipe = GetLinkPipe(audio_stream_id_, "audio:");
  if (audio_pipe) {
    auto link_flow = audio_pipe->GetFlowByInput(StreamType::LINK, "audio:");
    if (link_flow) {
      LOG_INFO("link_flow find audio\n");
      auto audio_flow_unit = audio_pipe->GetFlowunit(StreamType::AUDIO);
      if (audio_flow_unit) {
        auto flow = audio_flow_unit->GetFlow();
        FillMediaParam(MEDIA_PARAM_AUDIO_FMT, MEDIA_PARAM_AUDIO_FORMAT_PCM);

        std::string sample_format = audio_flow_unit->GetSampleFormat();
        if (sample_format.find("audio:pcm_s16") != std::string::npos)
          FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_BITS,
                         MEDIA_PARAM_AUDIO_SAMPLE_BITS_16BIT);
        else
          FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_BITS,
                         MEDIA_PARAM_AUDIO_SAMPLE_BITS_8BIT);

        auto value = audio_flow_unit->GetSampleRate();
        if (!value.empty()) {
          int sample_rate = atoi(value.c_str());
          if (sample_rate == 8000)
            FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_RATE,
                           MEDIA_PARAM_AUDIO_SAMPLE_RATE_8000);
          else if (sample_rate == 16000)
            FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_RATE,
                           MEDIA_PARAM_AUDIO_SAMPLE_RATE_16000);
          else if (sample_rate == 44100)
            FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_RATE,
                           MEDIA_PARAM_AUDIO_SAMPLE_RATE_44100);
          else if (sample_rate == 48000)
            FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_RATE,
                           MEDIA_PARAM_AUDIO_SAMPLE_RATE_48000);
          else
            FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_RATE,
                           MEDIA_PARAM_AUDIO_SAMPLE_RATE_8000);
        } else {
          FillMediaParam(MEDIA_PARAM_AUDIO_SAMPLE_RATE,
                         MEDIA_PARAM_AUDIO_SAMPLE_RATE_8000);
        }

        value = audio_flow_unit->GetChannel();
        if (!value.empty()) {
          int channel = atoi(value.c_str());
          if (channel == 1)
            FillMediaParam(MEDIA_PARAM_AUDIO_CHANNLE,
                           MEDIA_PARAM_AUDIO_CHANNEL_MONO);
          else
            FillMediaParam(MEDIA_PARAM_AUDIO_CHANNLE,
                           MEDIA_PARAM_AUDIO_CHANNEL_STEREO);
        } else {
          FillMediaParam(MEDIA_PARAM_AUDIO_CHANNLE,
                         MEDIA_PARAM_AUDIO_CHANNEL_STEREO);
        }
      }
    }
  }

  return 0;
}

bool LinkManager::CheckVideoStreamId(int id) {
  auto video_flow = GetLinkFlow(id, "video:");
  if (video_flow) {
    return true;
  }
  return false;
}

int LinkManager::PushStreamSwitch(bool on) {
  auto video_flow = GetLinkFlow(camera_stream_id_, "video:");
  if (video_flow) {
    LOG_INFO("link_flow find video_flow\n");
    if (on)
      video_flow->SetVideoHandler(PushVideoHandler);
    else
      video_flow->SetVideoHandler(nullptr);
  } else {
    LOG_ERROR("LinkManager camera id %d link no found\n", camera_stream_id_);
    return -1;
  }

  auto audio_flow = GetLinkFlow(audio_stream_id_, "audio:");
  if (audio_flow) {
    LOG_INFO("link_flow find audio_flow\n");
    if (on)
      audio_flow->SetAudioHandler(PushAudioHandler);
    else
      audio_flow->SetAudioHandler(nullptr);
  } else {
    LOG_ERROR("LinkManager audio id %d link no found\n", audio_stream_id_);
    return -1;
  }

  return 0;
}

#ifdef LINK_API_ENABLE_LINKKIT
void LinkManager::InvasionPictureUpload(const char *fpath) {
  LinkApi_->InvasionPictureUpload(fpath);
}

void LinkManager::FacePictureUpload(const char *fpath) {
  LinkApi_->FacePictureUpload(fpath);
}

void LinkManager::FailPictureUpload(const char *fpath) {
  LinkApi_->FailPictureUpload(fpath);
}
#endif

void LinkManager::ReportWakeUpData1() {
  LOG_INFO("entry low-power mode\n");
  LinkApi_->ReportWakeUpData1();
}

int LinkManager::InitDevice() {
  int flow_index;
  LinkApi_->SetMediaControlCb(MediaCtrolHandler);
#ifdef LINK_API_ENABLE_TUYA
  FillMediaParam();
#endif
  int ret = LinkApi_->InitDevice();
  if (ret) {
    LOG_ERROR("LinkManager LinkApi InitDevice faild\n");
    return -1;
  }

#ifdef LINK_API_ENABLE_LINKKIT
  FillMediaParam();
#endif
  device_status = LINK_DEVICE_STATUS_EXIST;

  return ret;
}

int LinkManager::DeInitDevice() {
  if (device_status != LINK_DEVICE_STATUS_EXIST)
    return 0;

  auto &flow_manager = FlowManager::GetInstance();
  auto pipes = flow_manager->GetPipes();
  for (int pipe_index = 0; pipe_index < pipes.size(); pipe_index++) {
    int link_flow_index;
    auto pipe = pipes[pipe_index];
    link_flow_index = pipe->GetFlowIndex(StreamType::LINK);
    auto link_flow = pipe->GetFlow(link_flow_index);
    link_flow->SetVideoHandler(nullptr);
    link_flow->SetAudioHandler(nullptr);
    link_flow->SetCaptureHandler(nullptr);
  }

  LinkApi_->DeInitDevice();
  device_status = LINK_DEVICE_STATUS_NOEXIST;
  return 0;
}

void PushVideoHandler(unsigned char *buffer, unsigned int buffer_size,
                      int64_t present_time, int nal_type) {
  auto link_manager_ = LinkManager::GetInstance();
  link_manager_->LinkApi_->PushVideoHandler(buffer, buffer_size,
                                            present_time / 1000, nal_type);
}

void PushAudioHandler(unsigned char *buffer, unsigned int buffer_size,
                      int64_t present_time) {
  auto link_manager_ = LinkManager::GetInstance();
  link_manager_->LinkApi_->PushAudioHandler(buffer, buffer_size,
                                            present_time / 1000);
}

void PushCaptureHandler(unsigned char *buffer, unsigned int buffer_size,
                        int type, const char *id) {
  auto link_manager_ = LinkManager::GetInstance();
  link_manager_->LinkApi_->PushCaptureHandler(buffer, buffer_size, type, id);
}

void MediaCtrolHandler(IpcMediaCmd cmd, const IpcMediaParam *para) {
  LOG_INFO("cmd is %d\n", cmd);
  if (cmd == IPC_MEDIA_START) {
    auto link_manager_ = LinkManager::GetInstance();
    link_manager_->PushStreamSwitch(true);
  } else if (cmd == IPC_MEDIA_STOP) {
    auto link_manager_ = LinkManager::GetInstance();
    link_manager_->PushStreamSwitch(false);
  } else if (cmd == IPC_MEDIA_REQUEST_I_FRAME) {
    SetForceIdrFrame(0);
  } else if (cmd == IPC_MEDIA_CHANGE_STREAM) {
    auto link_manager_ = LinkManager::GetInstance();
    if (link_manager_->CheckVideoStreamId(para->stream_type)) {
      link_manager_->PushStreamSwitch(false);
      link_manager_->SetVideoStreamId(para->stream_type);
      link_manager_->FillMediaParam();
      link_manager_->PushStreamSwitch(true);
      SetForceIdrFrame(para->stream_type);
    }
  }
}

int LinkManager::ConnectLink() {
  LOG_DEBUG("link manager connect link");
  int ret = LinkApi_->ConnectLink();
  if (ret) {
    LOG_ERROR("LinkManager LinkApi ConnectLink faild\n");
    return -1;
  }
  link_status = LINK_STATUS_INIT;
  return 0;
}

int LinkManager::StartLink() {
  LOG_DEBUG("link manager start link");
  int ret = LinkApi_->StartLink();
  if (ret) {
    LOG_ERROR("LinkManager LinkApi StartLink faild\n");
    LinkApi_->StartLink();
    return -1;
  }
  link_status = LINK_STATUS_INIT;
  return 0;
}

int LinkManager::StopLink() {
  if (link_status != LINK_STATUS_INIT)
    return 0;

  LinkApi_->StopLink();
  link_status = LINK_STATUS_UNINIT;
  LOG_DEBUG("link manager stop link");
  return 0;
}

} // namespace mediaserver
} // namespace rockchip

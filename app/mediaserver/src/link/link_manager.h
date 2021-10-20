// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_LINKKIT_MANAGER_H_
#define _RK_LINKKIT_MANAGER_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <string>

#include "flow_export.h"
#include "flow_manager.h"

#include "link_common.h"
#include "link_interface.h"
#ifdef LINK_API_ENABLE_LINKKIT
#include "linkkit_api.h"
#elif defined LINK_API_ENABLE_TUYA
#include "tuya_api.h"
#endif

typedef enum {
  LINK_DEVICE_STATUS_NOEXIST = 0,
  LINK_DEVICE_STATUS_EXIST = 1,
} LinkDeviceStatus;

typedef enum {
  LINK_STATUS_UNINIT = 0,
  LINK_STATUS_INIT = 1,
} LinkStatus;

namespace rockchip {
namespace mediaserver {

void PushVideoHandler(unsigned char *buffer, unsigned int buffer_size,
                      int64_t present_time, int nal_type);
void PushAudioHandler(unsigned char *buffer, unsigned int buffer_size,
                      int64_t present_time);
void PushCaptureHandler(unsigned char *buffer, unsigned int buffer_size,
                        int type, const char *id);
void MediaCtrolHandler(IpcMediaCmd cmd, const IpcMediaParam *para);

class LinkManager : public LinkInterFace {
public:
  LinkManager();
  virtual ~LinkManager();
  static std::shared_ptr<LinkManager> &GetInstance() {
    if (instance_ == nullptr) {
      instance_ = std::make_shared<LinkManager>();
    }
    return instance_;
  }
  std::shared_ptr<FlowPipe> GetLinkPipe(int id, std::string input_type);
  std::shared_ptr<FlowUnit> GetLinkFlowUnit(int id, std::string input_type);
  std::shared_ptr<easymedia::Flow> GetLinkFlow(int id, std::string input_type);
  bool CheckVideoStreamId(int id);

  virtual int FillLicenseKey(pLicenseKey plicense);
  virtual void FillMediaParam(MediaParamType param, unsigned int value);
  int FillMediaParam();

  virtual int InitDevice();
  virtual int DeInitDevice();

  void SetVideoStreamId(int id) { camera_stream_id_ = id; }

  int PushStreamSwitch(bool on);

  void InvasionPictureUpload(const char *fpath);
  void FacePictureUpload(const char *fpath);
  void FailPictureUpload(const char *fpath);
  void ReportWakeUpData1(void);

  friend void PushVideoHandler(unsigned char *buffer, unsigned int buffer_size,
                               unsigned int present_time, int nal_type);
  friend void PushAudioHandler(unsigned char *buffer, unsigned int buffer_size,
                               unsigned int present_time);
  friend void PushCaptureHandler(unsigned char *buffer,
                                 unsigned int buffer_size, int type,
                                 const char *id);

  friend void MediaCtrolHandler(IpcMediaCmd cmd, const IpcMediaParam *para);

  virtual int ConnectLink();
  virtual int StartLink();
  virtual int StopLink();

#ifdef LINK_API_ENABLE_LINKKIT
  std::unique_ptr<LinkKitApi> LinkApi_;
#elif defined LINK_API_ENABLE_TUYA
  std::unique_ptr<TuyaApi> LinkApi_;
#else
  std::unique_ptr<LinkVirApi> LinkApi_;
#endif

private:
  static LinkDeviceStatus device_status;
  static LinkStatus link_status;

  static std::shared_ptr<LinkManager> instance_;

  int camera_stream_id_;
  int audio_stream_id_;
};

typedef std::shared_ptr<LinkManager> LinkManagerPtr;

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_MEIDA_SERVER_H_

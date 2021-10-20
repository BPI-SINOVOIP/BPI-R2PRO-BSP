// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_EXPORT_H_
#define _RK_FLOW_EXPORT_H_

#include "flow_manager.h"

namespace rockchip {
namespace mediaserver {

#define KB_UNITS 1024

std::shared_ptr<FlowPipe> GetFlowPipe(int id, StreamType type);
std::shared_ptr<FlowPipe> GetFlowPipe(int id, StreamType type,
                                      std::string name);

std::shared_ptr<CameraControl> GetCameraControl(int id);
std::shared_ptr<EncoderControl> GetEncoderControl(int id);
std::shared_ptr<AudioControl> GetAudioControl(int id);

void ResetPipes();
void StopPipes();
void RestartPipes();

std::shared_ptr<easymedia::Flow> GetLinkFlow(int id, StreamType type,
                                             std::string input_data_type);
std::shared_ptr<FlowUnit> GetLinkFlowUnit(int id, StreamType type,
                                          std::string input_type);

/* audio functions */
void SetSampleRate(int val);
void SetVolume(int val);
void SetBitRate(int val);
void SetAudioSource(const char *param);
void SetAudioEncodeType(const char *param);
void SetANS(const char *param);

/* video functions */
void SetGop(int id, int val);
void SetMaxRate(int id, int val);
void SetStreamSmooth(int id, int val);
void SetForceIdrFrame(int id);
void SetForceIdrFrame(easymedia::Flow *search_flow, StreamType search_type);
void SetFrameRate(int id, int val);
void SetFrameRate(int id, int num, int den);
void SetResolution(int id, const char *param);
void SetRCQuality(int id, std::string level);
std::string GetVideoEncoderType(int id);
void SetVideoEncodeType(int id, const char *param);
void SetRCMode(int id, const char *param);
void SetH264Profile(int id, const char *param);
void SetSmart(int id, const char *param);
void SetSVC(int id, const char *param);
void SetVideoType(int id, const char *param);

int StartRecord(int id);
int StopRecord(int id);
int GetRecordStatus(int id);
int StartRecord();
int StopRecord();

int TakePhotoEnableSet(bool en);
int TakePhoto(int id, int count);

#ifdef ENABLE_SCHEDULES_SERVER
int SyncSchedules();
int StopSchedules();
int SetFileDuration(int id, int duration);
int SetFilePath(int id, std::string path);
int SetFilePrefix(int id, std::string prefix);
#endif

#ifdef ENABLE_OSD_SERVER
/* osd functions */
void SetOsdRegion(int id, const std::map<std::string, std::string> &map);
#endif

/* roi functions */
void SetRoiRegion(int stream_id, std::string roi_regions);
/* move detection functions */
void SetMDEnabled(int enabled);
void SetMDSensitivity(int sensitivity);
void SetMDRect();
void SetMDRect2(ImageRect *rects, int rect_cnt);
/* video advanced encoder functions*/
void SetQP(int stream_id, VideoEncoderQp qp);
void SetSplit(int stream_id, int mode, int size);

#ifdef USE_ROCKFACE
int GetFaceDetectArg(int id, easymedia::FaceDetectArg &fda);
int SetFaceDetectArg(int id, easymedia::FaceDetectArg fda);
int GetNNInputArg(int id, easymedia::NNinputArg &fda);
int SetNNInputArg(int id, easymedia::NNinputArg fda);
int GetFaceCaptureArg(int id, easymedia::FaceCaptureArg &cfa);
int SetFaceCaptureArg(int id, easymedia::FaceCaptureArg fda);
int GetFaceRegArg(int id, easymedia::FaceRegArg &cfa);
int SetFaceRegArg(int id, easymedia::FaceRegArg fda);
int GetDrawFilterArg(int id, easymedia::DrawFilterArg &cfa);
int SetDrawFilterArg(int id, easymedia::DrawFilterArg fda);
#endif

/* region invade functions */
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER) &&                    \
     defined(ENABLE_DBUS))
int GetRegionInvade(int id, region_invade_s &region_invade);
#endif
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))
void SetRegionInvade(int stream_id, region_invade_s region_invade);
#endif

#ifdef USE_ROCKFACE
void SetNNResultInput(RknnResult *result, int size);
#endif
#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void SetFacePicUpload(char *path, int size);
#endif
#if (defined(USE_ROCKFACE) && defined(ENABLE_OSD_SERVER))
void SetOsdNNResult(RknnResult *result, int type, int size);
#endif

#if (defined(USE_ROCKX))
void SetRockXNNResultInput(RknnResult *result, int size);
void SetRockxStatus(std::string model_name, int status);
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void SetImageToRecognize(const int32_t &id, const std::string &path);
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void DeleteFaceInfoInDB(const int32_t &id, const int32_t &faceId);
#endif

#if (defined(USE_ROCKFACE) && defined(ENABLE_DBUS))
void ClearFaceDBInfo();
#endif

#ifdef ENABLE_DBUS
std::string SendMediaStorageStopMsg();
#endif

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_FLOW_MANAGER_H_

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <nn_data.pb.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "buffer.h"
#include "link_config.h"
#include "shm_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "shm_control.cpp"

namespace ShmControl {
int setNNGeneralInfos(NNData *mNNData, RknnResult bufptr, int size) {
  if (size <= 0)
    return -1;

  auto nn_result = bufptr;
  int type = nn_result.type;
  int npu_w = 0, npu_h = 0;
  npu_w = nn_result.img_w;
  npu_h = nn_result.img_h;
  LOG_DEBUG("setNNGeneralInfos type=%d w=%d h=%d ********** \n", type, npu_w,
            npu_h);
  if (npu_w <= 0 || npu_h <= 0) {
    npu_w = 300;
    npu_h = 300;
  }
  mNNData->set_nn_width(npu_w);
  mNNData->set_nn_height(npu_h);
  mNNData->set_model_type(type);
  return 0;
}

void pushFaceDetectInfo(NNData *mNNData, RknnResult bufptr, int size) {
  if (!mNNData)
    return;
  auto nn_result_face_detect = bufptr;
  FaceDetect *facedetect;
  for (int i = 0; i < size; i++) {
    auto face_detect_item = nn_result_face_detect.face_info.object;
    float score = face_detect_item.score;
    if (score < 0.5f) {
      LOG_DEBUG(
          "%s %d, ****************************************face score %f\n",
          __FUNCTION__, __LINE__, score);
      continue;
    }
    int x1 = face_detect_item.box.left;
    int y1 = face_detect_item.box.top;
    int x2 = face_detect_item.box.right;
    int y2 = face_detect_item.box.bottom;
    facedetect = mNNData->add_face_detect();
    facedetect->set_left(x1);
    facedetect->set_top(y1);
    facedetect->set_right(x2);
    facedetect->set_bottom(y2);
    facedetect->set_score(score);
    LOG_DEBUG("%s %d, face left top right bottom [%d %d %d %d] score %f\n",
              __FUNCTION__, __LINE__, x1, y1, x2, y2, score);
  }
}

void pushPoseBodyInfo(NNData *mNNData, RknnResult bufptr, int size) {
  if (!mNNData)
    return;
  auto nn_result_pose_body = bufptr;
  LandMark *landmark;
  for (uint32_t i = 0; i < size; i++) {
    auto keyPointsItem = nn_result_pose_body.body_info.object;
    landmark = mNNData->add_landmark();
    for (int j = 0; j < keyPointsItem.count; j++) {
      Points *mPoint = landmark->add_points();
      mPoint->set_x(keyPointsItem.points[j].x);
      mPoint->set_y(keyPointsItem.points[j].y);
      LOG_DEBUG("  %s [%d, %d] %f\n", ROCKX_POSE_BODY_KEYPOINTS_NAME[j],
                keyPointsItem.points[j].x, keyPointsItem.points[j].y,
                keyPointsItem.score[j]);
    }
  }
}

void pushLandMarkInfo(NNData *mNNData, RknnResult bufptr, int size) {
  if (!mNNData)
    return;
  auto nn_result_land_mark = bufptr;
  mNNData->set_model_type(2);
  LandMark *landmark;
  for (uint32_t i = 0; i < size; i++) {
    landmark = mNNData->add_landmark();
    auto face_landmark_item = nn_result_land_mark.landmark_info.object;
    for (int j = 0; j < face_landmark_item.landmarks_count; j++) {
      Points *mPoint = landmark->add_points();
      mPoint->set_x(face_landmark_item.landmarks[j].x);
      mPoint->set_y(face_landmark_item.landmarks[j].y);
    }
  }
}

void pushFingerDetectInfo(NNData *mNNData, RknnResult bufptr, int size) {
  if (!mNNData)
    return;
}

void PushUserHandler(void *handler, int type, void *buffer, int size) {
  static int cnt = 0;
  if (!buffer)
    return;
  auto link_nndata = (struct easymedia::linknndata *)(buffer);
  int valid_size = link_nndata->size;
  // const char *model_name = link_nndata->nn_model_name;
  const char *model_name = nullptr;
  bool hasNNInfoUpdate = true;
  NNData mNNData;
  if (!valid_size)
    return;
  auto rknn_result = (RknnResult *)(link_nndata->rknn_result);
  for (int i = 0; i < valid_size; i++) {
    if (rknn_result[i].type == NNRESULT_TYPE_FACE) {
      pushFaceDetectInfo(&mNNData, rknn_result[i], 1);
      model_name = "rockx_face_detect";
    } else if (rknn_result[i].type == NNRESULT_TYPE_BODY) {
      pushPoseBodyInfo(&mNNData, rknn_result[i], 1);
      model_name = "rockx_pose_body";
    } else if (rknn_result[i].type == NNRESULT_TYPE_LANDMARK) {
      pushLandMarkInfo(&mNNData, rknn_result[i], 1);
      model_name = "rockx_face_landmark";
    } else if (rknn_result[i].type == NNRESULT_TYPE_FINGER) {
      pushFingerDetectInfo(&mNNData, rknn_result[i], 1);
      model_name = "rockx_pose_finger";
    } else {
      hasNNInfoUpdate = false;
    }
  }
  if (hasNNInfoUpdate == true) {
    std::string sendbuf;
    int res = setNNGeneralInfos(&mNNData, rknn_result[0], 1);
    if (res >= 0) {
      mNNData.set_model_name(model_name);
      mNNData.SerializeToString(&sendbuf);
      queue_w_.Push(sendbuf);
      cnt++;
      LOG_DEBUG("---PushUserHandler , len = %d cnt %d \n", sendbuf.length(),
                cnt);
      if (cnt == 100000)
        cnt = 0;
    }
  }
}

} // namespace ShmControl

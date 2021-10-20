// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow_nn_handler.h"
#include "flow_common.h"
#include "flow_export.h"

#include <sys/prctl.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "flow_nn_handler.cpp"

namespace rockchip {
namespace mediaserver {

void DrawHandler::RectDebounce(RknnResult *input, int count) {
#ifdef USE_ROCKFACE
  for (int i = 0; i < count; i++) {
    if (input[i].type == NNRESULT_TYPE_FACE) {
      rockface_det_t *det = &input[i].face_info.base;
      for (RknnResult &last_result : last_results_) {
        if (last_result.type != NNRESULT_TYPE_FACE)
          continue;
        rockface_det_t *last = &last_result.face_info.base;
        if (det->id == last->id) {
          det->box.left = (det->box.left + last->box.left) / 2;
          det->box.top = (det->box.top + last->box.top) / 2;
          det->box.right = (det->box.right + last->box.right) / 2;
          det->box.bottom = (det->box.bottom + last->box.bottom) / 2;
          continue;
        }
      }
    }
  }
  last_results_.clear();
  for (int i = 0; i < count; i++)
    last_results_.push_back(input[i]);
#endif
}

DrawHandler::DrawHandler(std::shared_ptr<easymedia::Flow> flow, int width,
                         int height, int min_rect_size, float x_offset,
                         float y_offset)
    : flow_(flow), width_(width), height_(height),
      min_rect_size_(min_rect_size), x_offset_(x_offset), y_offset_(y_offset) {}

void NNHandler::NNResultInput(RknnResult *result, int size) {
#ifdef USE_ROCKFACE
  if (!draw_handlers_.size())
    return;

  for (auto &iter : draw_handlers_) {
    int w = iter->width_;
    int h = iter->height_;
    int min_rect_size = iter->min_rect_size_;
    float x_offset = iter->x_offset_;
    float y_offset = iter->y_offset_;

    if (iter->flow_) {
      if (size <= 0) {
        iter->flow_->SubControl(easymedia::S_NN_INFO, nullptr, 0);
      } else {
        RknnResult fix_result[size];
        int pos = 0;
        for (int pos = 0; pos < size; pos++) {
          float slope_x = (float)w / result[pos].img_w;
          float slope_y = (float)h / result[pos].img_h;
          fix_result[pos] = result[pos];
          rockface_rect_t *rect = &fix_result[pos].face_info.base.box;
          rect->left = (slope_x * rect->left) + x_offset;
          rect->right = (slope_x * rect->right) + x_offset;
          rect->top = (slope_y * rect->top) + y_offset;
          rect->bottom = (slope_y * rect->bottom) + y_offset;
          size_t face_size =
              (rect->right - rect->left) * (rect->bottom - rect->top);
          if (face_size < min_rect_size)
            memset(&fix_result[pos].face_info.base, 0, sizeof(rockface_det_t));
        }
        iter->flow_->SubControl(easymedia::S_NN_INFO, fix_result, size);
      }
    }
  }
#endif
}

void NNHandler::RockXNNResultInput(RknnResult *result, int size) {
#ifdef USE_ROCKX
  if (!draw_handlers_.size()) {
    return;
  }
  for (auto &iter : draw_handlers_) {
    int w = iter->width_;
    int h = iter->height_;
    float x_offset = iter->x_offset_;
    float y_offset = iter->y_offset_;

    if (iter->flow_) {
      if (size <= 0) {
        iter->flow_->SubControl(easymedia::S_NN_INFO, nullptr, 0);
      } else {
        RknnResult fix_result[size];
        int pos = 0;
        int i = 0;
        int no_marks = 0;
        for (int pos = 0; pos < size; pos++) {
          switch(result[pos].object_info.cls_idx) {
            case 1:
              /* people */
              break;
            case 3:
              /* car */
              break;
            case 6:
              /* bus */
              break;
            case 17:
              /* cat */
              break;
            case 18:
              /* dog */
              break;
            default:
              no_marks = 1;
              break;
          }
          if (no_marks == 0) {
            float slope_x = (float)w / result[pos].img_w;
            float slope_y = (float)h / result[pos].img_h;
            fix_result[i] = result[pos];
            Rect *rect = &fix_result[i].object_info.box;
            rect->left = (slope_x * rect->left) + x_offset;
            rect->right = (slope_x * rect->right) + x_offset;
            rect->top = (slope_y * rect->top) + y_offset;
            rect->bottom = (slope_y * rect->bottom) + y_offset;
            i++;
          }
          no_marks = 0;
        }
        iter->flow_->SubControl(easymedia::S_NN_INFO, fix_result, i);
      }
    }
  }
#endif
}

} // namespace mediaserver
} // namespace

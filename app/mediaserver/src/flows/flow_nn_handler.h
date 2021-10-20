// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FLOW_DRAW_HANDLER_H_
#define _RK_FLOW_DRAW_HANDLER_H_

#include <memory>
#include <queue>
#include <vector>

#include "flow.h"
#include "rknn_user.h"
#include "server.h"
#include "thread.h"

namespace rockchip {
namespace mediaserver {

class NNHandler;

class DrawHandler {
public:
  DrawHandler() = delete;
  DrawHandler(std::shared_ptr<easymedia::Flow> flow, int width, int height,
              int min_rect_size = 0, float x_offset = 0.0,
              float y_offset = 0.0);
  virtual ~DrawHandler() = default;
  friend NNHandler;

private:
  void RectDebounce(RknnResult *input, int count);

  int width_;
  int height_;
  int min_rect_size_;
  float x_offset_;
  float y_offset_;
  std::shared_ptr<easymedia::Flow> flow_;
  std::vector<RknnResult> last_results_;
};

class NNHandler {
public:
  NNHandler() = default;
  virtual ~NNHandler() = default;
  void NNResultInput(RknnResult *result, int size);
  void RockXNNResultInput(RknnResult *result, int size);
  std::vector<std::unique_ptr<DrawHandler>> &GetDrawHandler() {
    return draw_handlers_;
  }

private:
  std::vector<std::unique_ptr<DrawHandler>> draw_handlers_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_FLOW_PIPE_H_

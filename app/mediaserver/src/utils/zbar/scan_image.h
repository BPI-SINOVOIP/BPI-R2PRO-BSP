// Copyright 2020 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_SCAN_IMAGE_H_
#define _RK_SCAN_IMAGE_H_

#include "server.h"
#include "thread.h"
#include "rkbar_scan_api.h"

namespace rockchip {
namespace mediaserver {

class ScanImage : public Service {
public:
  ScanImage();
  virtual ~ScanImage();
  virtual void start(void) override;
  virtual void stop(void) override;
  ThreadStatus status(void);

  int key_fd_;
  int key_event_code_;
  std::string key_event_path_;

private:
  Thread::UniquePtr wait_key_event_thread_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_SCAN_IMAGE_H_

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_SCHEDULES_MANAGER_H_
#define _RK_SCHEDULES_MANAGER_H_

#include <assert.h>
#include <dbserver.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "flow.h"
#include "media_config.h"

#include "flow_common.h"
#include "flow_export.h"
#include "server.h"
#include "thread.h"

#define TIMED_RECORD_PREFIX "timed"
#define EVENT_RECORD_PREFIX "md"
#define EVENT_RECORD_DURATION_SECOND 30

namespace rockchip {
namespace mediaserver {

class SchedulesManager : public Service {
public:
  int day_index_;
  int now_minute_;
  int now_second_;
  int running_flag_;
  int event_snap_flag_;
  int record_stream_id_;
  int timed_video_enabled_;
  int timed_snap_enabled_;

  SchedulesManager();
  virtual ~SchedulesManager();
  virtual void start(void) override;
  virtual void stop(void) override;
  int running_status(void);
  ThreadStatus status(void);

  void SyncTime();
  void SyncSchedulesConfig();
  void CheckVideoPlan();
  void CheckPhotoPlan();
  void CheckMDPlan();
  void EventSnap();
  void MDEventCountAdd() { md_event_count_++; }
  int MDTriggerEnabled() { return md_trigger_enabled_; }
  int GetMDEventCount() { return md_event_count_; }

private:
  std::chrono::steady_clock::time_point md_last_event_time_;
  int md_event_count_;
  int md_record_enabled_;
  int md_trigger_enabled_;
  int timed_snap_quality_;
  int timed_snap_interval_;
  Thread::UniquePtr manager_thread_;
  struct week *schedule_video_;
  struct week *schedule_photo_;
  struct week *schedule_md_;
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_SCHEDULES_MANAGER_H_

// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/prctl.h>

#include "dbus_dbserver_key.h"
#include "flow_db_protocol.h"
#include "flow_manager.h"
#include "schedules_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "schedules_manager.cpp"

namespace rockchip {
namespace mediaserver {

SchedulesManager::SchedulesManager() {
  LOG_DEBUG("SchedulesManager init\n");
  day_index_ = 0;
  now_minute_ = 0;
  running_flag_ = 0;
  event_snap_flag_ = 0;
  record_stream_id_ = 0; // default main stream recording
  timed_video_enabled_ = 0;
  timed_snap_enabled_ = 0;
  timed_snap_quality_ = 10;
  timed_snap_interval_ = 1;
  md_event_count_ = 0;
  md_record_enabled_ = 0;
  md_trigger_enabled_ = 0;
  schedule_video_ = new struct week;
  schedule_photo_ = new struct week;
  schedule_md_ = new struct week;
  memset(schedule_video_, 0, sizeof(struct week));
  memset(schedule_photo_, 0, sizeof(struct week));
  memset(schedule_md_, 0, sizeof(struct week));
}

SchedulesManager::~SchedulesManager() {
  delete schedule_video_;
  delete schedule_photo_;
  delete schedule_md_;
  schedule_video_ = NULL;
  schedule_photo_ = NULL;
  schedule_md_ = NULL;
  LOG_DEBUG("SchedulesManager free\n");
}

void SchedulesManager::SyncTime() {
  auto time_now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  struct tm *ptm = localtime(&time_now);
  if (ptm->tm_wday == 0)
    day_index_ = 6;
  else
    day_index_ = ptm->tm_wday - 1;
  now_minute_ = ptm->tm_hour * 60 + ptm->tm_min;
  now_second_ = ptm->tm_sec;
  // LOG_DEBUG("ptm->tm_sec is %d\n", ptm->tm_sec);
}

void SchedulesManager::SyncSchedulesConfig() {
  LOG_DEBUG("sync schedules config\n");
  // sync record plan
  if (!timed_video_enabled_)
    return;
  dbserver_storage_video_plan_parse(&timed_video_enabled_);
  memset(schedule_video_, 0, sizeof(struct week));
  dbserver_event_schedules_parse(schedule_video_,
                                 DB_SCHEDULES_ID_STORAGE_VIDEO_PLAN);
  LOG_DEBUG("timed record ebaled is %d\n", timed_video_enabled_);
  // sync snap plan
  if (!timed_snap_interval_)
    return;
  dbserver_storage_snap_plan_parse(0, &timed_snap_enabled_,
                                   &timed_snap_quality_, &timed_snap_interval_,
                                   NULL);
  memset(schedule_photo_, 0, sizeof(struct week));
  dbserver_event_schedules_parse(schedule_photo_, DB_SCHEDULES_ID_SCREENSHOT);
  LOG_DEBUG("timed snap ebaled is %d, quality is %d, interval is %d\n",
            timed_snap_enabled_, timed_snap_quality_, timed_snap_interval_);
  // sync move detect plan
  if (!md_record_enabled_)
    return;
  dbserver_event_triggers_parse(0, &md_record_enabled_);
  memset(schedule_md_, 0, sizeof(struct week));
  dbserver_event_schedules_parse(schedule_md_, DB_SCHEDULES_ID_MOTION_DETECT);
  LOG_DEBUG("move detect record enabled is %d\n", md_record_enabled_);

  SyncTime();
  CheckVideoPlan();
  CheckMDPlan();
}

void SchedulesManager::CheckVideoPlan() {
  int video_plan_enabled = 0;
  for (int j = 0; j < MAX_DAY_SCHEDULE_NUM; j++) {
    std::string type = "";
    if (!schedule_video_)
      continue;
    else if (!schedule_video_->week_day[day_index_].day_period[j].end_minute)
      continue;
    else
      type = (schedule_video_->week_day[day_index_].day_period[j].type);
    int start_minute =
        schedule_video_->week_day[day_index_].day_period[j].start_minute;
    int end_minute =
        schedule_video_->week_day[day_index_].day_period[j].end_minute;
    LOG_DEBUG("video type is %s,minute now is %d, start is %d, end is %d\n",
              type.c_str(), now_minute_, start_minute, end_minute);
    if ((now_minute_ < start_minute) || (now_minute_ >= end_minute))
      continue;
    // Generally only within a period of time,
    // so the following code will only be executed once
    if (!type.compare(DB_SCHEDULES_TYPE_TIMING)) {
      video_plan_enabled = 1;
      if (!GetRecordStatus(record_stream_id_)) {
        LOG_INFO("start timed record\n");
        SetFilePrefix(0, TIMED_RECORD_PREFIX);
        StartRecord(record_stream_id_);
      } else {
        LOG_DEBUG("already timed recording\n");
      }
    } else if ((!type.compare(DB_SCHEDULES_TYPE_MOTION_DETECT)) &&
               (md_record_enabled_)) {
      video_plan_enabled = 1;
      if (md_event_count_) {
        md_last_event_time_ = std::chrono::steady_clock::now();
        SetFilePrefix(0, EVENT_RECORD_PREFIX);
        if (!GetRecordStatus(record_stream_id_)) {
          LOG_INFO("start move detect record\n");
          StartRecord(record_stream_id_);
        } else {
          LOG_DEBUG("already move detect recording\n");
        }
      } else {
        long int interval =
            (std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::steady_clock::now() - md_last_event_time_))
                .count();
        LOG_DEBUG("%ld seconds since the last md event\n", interval);
        if (interval >= EVENT_RECORD_DURATION_SECOND)
          video_plan_enabled = 0;
      }
    }
  }
  md_event_count_ = 0;
  if ((video_plan_enabled == 0) && (GetRecordStatus(record_stream_id_))) {
    LOG_INFO("SchedulesManager, stop record\n");
    StopRecord(record_stream_id_);
  }
}

void SchedulesManager::CheckPhotoPlan() {
  auto time_now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  for (int j = 0; j < MAX_DAY_SCHEDULE_NUM; j++) {
    std::string type = "";
    if (!schedule_photo_)
      continue;
    else if (strlen(schedule_photo_->week_day[day_index_].day_period[j].type) == 0)
      continue;
    else
      type = schedule_photo_->week_day[day_index_].day_period[j].type;
    int start_minute =
        schedule_photo_->week_day[day_index_].day_period[j].start_minute;
    int end_minute =
        schedule_photo_->week_day[day_index_].day_period[j].end_minute;
    // LOG_DEBUG("snap type is %s,minute now is %d, start is %d, end is %d\n",
    //           type.c_str(), now_minute_, start_minute, end_minute);
    // must be an integer multiple of 1000 milliseconds
    if ((now_minute_ >= start_minute) && (now_minute_ < end_minute) &&
        (time_now % ((int)timed_snap_interval_ / 1000) == 0)) {
      if (TakePhoto(0, 1))
        LOG_ERROR("TakePhoto Fail\n");
      else
        LOG_DEBUG("TakePhoto Success\n");
    }
  }
}

void SchedulesManager::CheckMDPlan() {
  int between_md_plan = 0;
  for (int j = 0; j < MAX_DAY_SCHEDULE_NUM; j++) {
    if (!schedule_md_)
      continue;
    else if (!schedule_md_->week_day[day_index_].day_period[j].end_minute)
      continue;
    int start_minute =
        schedule_md_->week_day[day_index_].day_period[j].start_minute;
    int end_minute =
        schedule_md_->week_day[day_index_].day_period[j].end_minute;
    LOG_DEBUG("md minute now is %d, start is %d, end is %d\n", now_minute_,
              start_minute, end_minute);
    if ((now_minute_ < start_minute) || (now_minute_ >= end_minute))
      continue;
    between_md_plan = 1;
  }
  if (between_md_plan)
    md_trigger_enabled_ = 1;
  else
    md_trigger_enabled_ = 0;
}

static void *ManagerProcess(void *arg) {
  auto os = reinterpret_cast<SchedulesManager *>(arg);
  char thread_name[40];
  snprintf(thread_name, sizeof(thread_name), "SchedulesManager");
  prctl(PR_SET_NAME, thread_name);
  LOG_DEBUG("Schedules ManagerProcess in\n");

  if (GetRecordStatus(os->record_stream_id_))
    StopRecord(os->record_stream_id_);
  // Synchronize configuration parameters from the database
  os->SyncSchedulesConfig();
  while (os->status() == kThreadRunning) {
    auto time_before = std::chrono::steady_clock::now();
    os->SyncTime();

    // check video plan every minute or trigger event
    if ((!os->now_second_) || (os->GetMDEventCount())) {
      if (os->timed_video_enabled_)
        os->CheckVideoPlan();
      else
        LOG_DEBUG("timed record is disabled\n");
      if (!os->now_second_) // check move detect plan every minute
        os->CheckMDPlan();
    }
    // check video plan every second
    if (os->timed_snap_enabled_)
      os->CheckPhotoPlan();
    if ((!os->timed_video_enabled_) && (!os->timed_snap_enabled_)) {
      LOG_DEBUG("timed record and snap are disabled, schedules manager exit\n");
      break;
    } else {
      os->running_flag_ = 1;
    }

    // prevent the program from accumulating time leading to skip a second
    auto time_later = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        time_later - time_before);
    // LOG_DEBUG("duration is %d ms\n", duration.count());
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000 - duration.count()));
  }
  LOG_DEBUG("Schedules ManagerProcess out\n");
  if (GetRecordStatus(os->record_stream_id_))
    StopRecord(os->record_stream_id_);
  os->running_flag_ = 0;
  return nullptr;
}

void SchedulesManager::start(void) {
  LOG_DEBUG("schedules manager start\n");
  LOG_INFO("schedules manager start\n");
  manager_thread_.reset(new Thread(ManagerProcess, this));
  manager_thread_->set_status(kThreadRunning);
}

void SchedulesManager::stop(void) {
  manager_thread_->set_status(kThreadStopping);
  manager_thread_->join();
  LOG_INFO("schedules manager stop\n");
  LOG_DEBUG("schedules manager stop\n");
}

int SchedulesManager::running_status(void) {
  if (manager_thread_)
    return manager_thread_->status() == kThreadRunning ? 1 : 0;
  return 0;
}

ThreadStatus SchedulesManager::status(void) {
  if (manager_thread_)
    return manager_thread_->status();
  else
    return kThreadStopping;
}

static void EventSnapProcess(void *arg, int interval, int num) {
  auto os = reinterpret_cast<SchedulesManager *>(arg);
  if (!os)
    return;
  os->event_snap_flag_ = 1;
  LOG_DEBUG("event snapshoot begin\n");
  while (num > 0) {
    TakePhoto(0, 1);
    LOG_DEBUG("event snapshoot remain num is %d\n", num);
    num--;
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
  }
  if (!os)
    return;
  os->event_snap_flag_ = 0;
  LOG_DEBUG("event snapshoot over\n");
}

void SchedulesManager::EventSnap() {
  int snap_enabled = 0;
  int snap_quality = 0;
  int snap_interval = 1;
  int snap_num = 1;

  dbserver_storage_snap_plan_parse(1, &snap_enabled, &snap_quality,
                                   &snap_interval, &snap_num);
  if (!snap_enabled) {
    LOG_DEBUG("event snapshot is not enabled\n");
    return;
  }
  if (event_snap_flag_) {
    LOG_DEBUG("event snapshoot already in progress\n");
    return;
  }
  std::thread t(EventSnapProcess, this, snap_interval, snap_num);
  t.detach();
}

} // namespace mediaserver
} // namespace rockchip

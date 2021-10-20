// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_OSD_COMMON_H_
#define _RK_OSD_COMMON_H_

#include <chrono>
#include <stdint.h>
#include <string>

#include "flow_common.h"

//#define OSD_INFO_EN
//#define OSD_DEBUG_EN

#ifdef OSD_INFO_EN
#define OSD_LOG_INFO LOG_INFO
#else
#define OSD_LOG_INFO(...)
#endif

#ifdef OSD_DEBUG_EN
#define OSD_LOG_DEBUG LOG_DEBUG
#else
#define OSD_LOG_DEBUG(...)
#endif

inline int64_t gettimeofday() {
  std::chrono::microseconds us =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch());
  return us.count();
}

class AutoDuration {
public:
  AutoDuration() { Reset(); }
  int64_t Get() { return gettimeofday() - start; }
  void Reset() { start = gettimeofday(); }
  int64_t GetAndReset() {
    int64_t now = gettimeofday();
    int64_t pretime = start;
    start = now;
    return now - pretime;
  }

private:
  int64_t start;
};

enum {
  OSD_TYPE_DATE = 0,
  OSD_TYPE_IMAGE = 1,
  OSD_TYPE_TEXT = 2,
  OSD_TYPE_BORDER = 3,
};

typedef struct text_data {
  wchar_t wch[128];
  uint32_t font_size;
  uint32_t font_color;
  uint32_t color_inverse;
  const char *font_path;
  char format[128];
} text_data_s;

typedef struct border_data {
  int color_index;
  int color_key;
  int thick;
  int display_style;
} border_data_s;

typedef struct osd_data {
  int type;
  union {
    const char *image;
    text_data_s text;
    border_data_s border;
  };
  int width;
  int height;
  uint8_t *buffer;
  uint32_t size;

  int origin_x;
  int origin_y;
  int enable;
} osd_data_s;

typedef struct Region_Rect {
  int left;
  int right;
  int top;
  int bottom;
} region_rect_s;

typedef struct Region_Invade {
  int enable;
  int width;
  int height;
  float position_x;
  float position_y;
  int proportion;
  int sensitivity_level;
  int time_threshold;
  int blink_cnt_;
  int blink_interval_ms_;
  int blink_wait_cnt_;
  bool blink_status_;
} region_invade_s;

#endif // _RK_OSD_COMMON_H_

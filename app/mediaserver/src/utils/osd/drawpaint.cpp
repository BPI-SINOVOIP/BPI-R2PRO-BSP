// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <codecvt>
#include <locale>

#include <sys/prctl.h>

#include "color_table.h"
#include "dbus_dbserver_key.h"
#include "flow_db_protocol.h"
#include "flow_manager.h"
#include "osd_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "drawpaint.cpp"

int DrawPaint::interval_offset_ = 0;

void DrawPaint::DrawSolidBorder(uint8_t *buffer, BorderInfo info) {
  int offset;
  int rect_w = info.rect.w;
  int rect_h = info.rect.h;
  int thick = info.thick;
  int color = info.color;

  for (int j = 0; j < thick; j++) {
    offset = j * rect_w;
    memset(buffer + offset, color, rect_w);
    offset = (rect_h - j - 1) * rect_w;
    memset(buffer + offset, color, rect_w);
  }

  for (int j = 0; j < rect_h; j++) {
    for (int k = 0; k < thick; k++) {
      buffer[j * rect_w + k] = color;
      buffer[(j + 1) * rect_w - thick + k] = color;
    }
  }
}

void DrawPaint::DrawDottedBorder(uint8_t *buffer, BorderInfo info) {
  int offset;
  int rect_w = info.rect.w;
  int rect_h = info.rect.h;
  int thick = info.thick;
  int color = info.color;
  int color_key = info.color_key;
  int interval = info.interval;
  uint8_t dotted_line[info.rect.w];

  for (int k = 0; k < rect_w; k++) {
    if (((k + interval_offset_) / interval) % 2)
      dotted_line[k] = color;
    else
      dotted_line[k] = color_key;
  }

  for (int j = 0; j < thick; j++) {
    for (int k = 0; k < rect_w; k++) {
      offset = j * rect_w;
      memcpy(buffer + offset, dotted_line, rect_w);
      offset = (rect_h - j - 1) * rect_w;
      memcpy(buffer + offset, dotted_line, rect_w);
    }
  }

  for (int j = 0; j < rect_h; j++) {
    for (int k = 0; k < thick; k++) {
      if (((j + interval_offset_) / interval) % 2) {
        buffer[j * rect_w + k] = color;
        buffer[(j + 1) * rect_w - thick + k] = color;
      }
    }
  }
}

void DrawPaint::DrawBorder(uint8_t *buffer, BorderInfo info) {
  if (info.display_style == BORDER_DOTTED) {
    interval_offset_ = 0;
    DrawDottedBorder(buffer, info);
  } else if (info.display_style == BORDER_LINE) {
    DrawSolidBorder(buffer, info);
  } else if (info.display_style == BORDER_WATERFULL_LIGHT) {
    interval_offset_ += 40;
    DrawDottedBorder(buffer, info);
  }
}

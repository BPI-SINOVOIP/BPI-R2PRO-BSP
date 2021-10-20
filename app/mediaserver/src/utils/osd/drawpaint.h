// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_OSD_DRAW_PAINT_H_
#define _RK_OSD_DRAW_PAINT_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum BorderEffect {
  BORDER_LINE = 0,
  BORDER_DOTTED,
  BORDER_WATERFULL_LIGHT,
};

typedef struct DrawRect {
  int x;
  int y;
  int w;
  int h;
} DrawRect;

typedef struct BorderInfo {
  DrawRect rect;
  int color;
  int color_key;
  int thick;
  int display_style;
  int dotted_offset;
  int interval;
} BorderInfo;

class DrawPaint {
public:
  DrawPaint() = default;
  ~DrawPaint() = default;
  void DrawSolidBorder(uint8_t *buffer, BorderInfo info);
  void DrawDottedBorder(uint8_t *buffer, BorderInfo info);
  void DrawBorder(uint8_t *buffer, BorderInfo info);

private:
  static int interval_offset_;
};

#endif // _RK_OSD_DRAW_PAINT_H_

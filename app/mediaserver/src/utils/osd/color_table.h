// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_COLOR_TABLES_H_
#define _RK_COLOR_TABLES_H_

#include <memory>
#include <vector>

#include "osd_common.h"

#define PALETTE_TABLE_LEN 256

extern const uint32_t rgb888_palette_table[PALETTE_TABLE_LEN];
extern const uint32_t yuv444_palette_table[PALETTE_TABLE_LEN];

/* Match an RGB value to a particular palette index */
uint8_t inline find_color(const uint32_t *pal, uint32_t len, uint8_t r,
                          uint8_t g, uint8_t b) {
  uint32_t i = 0;
  uint8_t pixel = 0;
  unsigned int smallest = 0;
  unsigned int distance = 0;
  int rd, gd, bd;
  uint8_t rp, gp, bp;

  smallest = ~0;

  // LOG_DEBUG("find_color rgba_value %8x", (0xFF << 24 | r << 16 | g <<8 | b
  // <<0));

  for (i = 0; i < len; ++i) {
    bp = (pal[i] & 0xff000000) >> 24;
    gp = (pal[i] & 0x00ff0000) >> 16;
    rp = (pal[i] & 0x0000ff00) >> 8;

    rd = rp - r;
    gd = gp - g;
    bd = bp - b;

    distance = (rd * rd) + (gd * gd) + (bd * bd);
    if (distance < smallest) {
      pixel = i;

      /* Perfect match! */
      if (distance == 0)
        break;

      smallest = distance;
    }
  }

  // LOG_DEBUG("find_color pixel %d pal[%d][%d] %8x", pixel, pixel/6, pixel%6,
  // pal[pixel]);

  return pixel;
}

#endif // _RK_OSD_MIDDLEWARE_H_

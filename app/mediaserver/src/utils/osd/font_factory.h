// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_FONT_FACTORY_H_
#define _RK_FONT_FACTORY_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <ft2build.h>
#ifdef __cplusplus
}
#endif
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <vector>

#include "osd_common.h"

class FontFactory {
public:
  FontFactory();
  virtual ~FontFactory();

  int CreateFont(const char *font_path, int font_size);
  int DestoryFont();

  int SetFontSize(int font_size);
  int GetFontSize();
  uint SetFontColor(uint font_color);
  uint GetFontColor();
  std::string GetFontPath() { return font_path_; }

  void DrawYuvMapBuffer(uint8_t *buffer, int buf_w, int buf_h);
  void DrawYuvMapWCharEx(uint8_t *buffer, int buf_w, int buf_h,
                         const wchar_t wch);
  void DrawYuvMapWChar(uint8_t *buffer, int buf_w, int buf_h,
                       const wchar_t wch);
  void DrawYuvMapText(uint8_t *buffer, int buf_w, int buf_h,
                      const wchar_t *wstr);

private:
  FT_Library library_;
  FT_Face face_;
  FT_GlyphSlot slot_;
  FT_Vector pen_;

  double font_angle_;
  int font_size_;
  uint font_color_;
  std::string font_path_;
  uint color_index_;
  uint trans_index_;
};

#endif // _RK_OSD_MIDDLEWARE_H_

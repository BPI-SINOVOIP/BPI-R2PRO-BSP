// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <wchar.h>

#include "color_table.h"
#include "font_factory.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "font_factory.cpp"

FontFactory::FontFactory() : face_(NULL), library_(NULL) {
  trans_index_ = PALETTE_TABLE_LEN - 1;
}

FontFactory::~FontFactory() { DestoryFont(); }

int FontFactory::CreateFont(const char *font_path, int font_size) {
  FT_Init_FreeType(&library_);
  FT_New_Face(library_, font_path, 0, &face_);
  if (!face_) {
    LOG_ERROR("please check font_path %s\n", font_path);
    return -1;
  }

  FT_Set_Pixel_Sizes(face_, font_size, 0);
  font_size_ = font_size;
  font_path_ = font_path;
  slot_ = face_->glyph;
  return 0;
}

int FontFactory::DestoryFont() {
  if (face_) {
    FT_Done_Face(face_);
    face_ = NULL;
  }
  if (library_) {
    FT_Done_FreeType(library_);
    library_ = NULL;
  }
  return 0;
}

int FontFactory::SetFontSize(int font_size) {
  FT_Set_Pixel_Sizes(face_, font_size, font_size);
  // FT_Set_Char_Size(face_, font_size * 64, font_size * 64, 0, 0);
  font_size_ = font_size;
  return 0;
}

int FontFactory::GetFontSize() { return font_size_; }

uint FontFactory::SetFontColor(uint font_color) {
  font_color_ = font_color;
  unsigned char cR = font_color_ >> 16 & 0xFF;
  unsigned char cG = font_color_ >> 8 & 0xFF;
  unsigned char cB = font_color_ >> 0 & 0xFF;
  color_index_ =
      find_color(rgb888_palette_table, PALETTE_TABLE_LEN, cR, cG, cB);
  return 0;
}

uint FontFactory::GetFontColor() { return font_color_; }

void FontFactory::DrawYuvMapBuffer(uint8_t *buffer, int buf_w, int buf_h) {
  int i, j, p, q;
  int left = slot_->bitmap_left;
  int top = (face_->size->metrics.ascender >> 6) - slot_->bitmap_top;
  int right = left + slot_->bitmap.width;
  int bottom = top + slot_->bitmap.rows;

  for (j = top, q = 0; j < bottom; j++, q++) {
    int offset = j * buf_w;
    int bmp_offset = q * slot_->bitmap.width;
    for (i = left, p = 0; i < right; i++, p++) {
      if (i < 0 || j < 0 || i >= buf_w || j >= buf_h)
        continue;
      if (slot_->bitmap.buffer[bmp_offset + p])
        buffer[offset + i] = color_index_;
      else
        buffer[offset + i] = trans_index_;
    }
  }
}

void FontFactory::DrawYuvMapWCharEx(uint8_t *buffer, int buf_w, int buf_h,
                                    const wchar_t wch) {
  FT_Error error;
  error = FT_Load_Char(face_, wch, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
  FT_Render_Glyph(slot_, FT_RENDER_MODE_NORMAL);
  if (error) {
    LOG_DEBUG("FT_Load_Char error\n");
    return;
  }
  DrawYuvMapBuffer(buffer, buf_w, buf_h);
}

void FontFactory::DrawYuvMapWChar(uint8_t *buffer, int buf_w, int buf_h,
                                  const wchar_t wch) {
  FT_Error error;
  FT_Set_Transform(face_, NULL, &pen_);
  error = FT_Load_Char(face_, wch, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
  FT_Render_Glyph(slot_, FT_RENDER_MODE_NORMAL);
  if (error) {
    LOG_DEBUG("FT_Load_Char error\n");
    return;
  }
  DrawYuvMapBuffer(buffer, buf_w, buf_h);
}

void FontFactory::DrawYuvMapText(uint8_t *buffer, int buf_w, int buf_h,
                                 const wchar_t *wstr) {
  int x_offset = 0;
  int len = wcslen(wstr);
  pen_.x = 0 * 64;
  pen_.y = 0 * 64;
  for (int i = 0; i < len; i++) {
    DrawYuvMapWChar(buffer, buf_w, buf_h, wstr[i]);
    pen_.x += slot_->advance.x;
    pen_.y += slot_->advance.y;
  }
}

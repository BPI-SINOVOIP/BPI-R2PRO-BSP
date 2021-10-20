// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_OSD_CACHE_H_
#define _RK_OSD_CACHE_H_

#include <memory>
#include <vector>

#include "font_factory.h"

/*UTF-8 code*/
#define COMMON_STRING                                                          \
  L"0123456789:-/*."                                                           \
  L"\x5E74\x6708\x65E5"                                                        \
  L"\x661F\x671F"                                                              \
  L"\x4E00\x4E8C\x4E09\x56DB\x4E94\x516D"                                      \
  L"MondayTuesWhrFiStPA"

class FontCache {
public:
  FontCache() {}
  ~FontCache() {}
  wchar_t wch;
  uint32_t font_color;
  uint32_t font_size;
  std::string font_path;
  int font_width;
  int font_height;
  uint8_t *yuvamap_buffer;
  int yuvamap_buffer_size;
};

typedef std::vector<std::pair<wchar_t, std::shared_ptr<FontCache>>> FontCache_P;

class OSDCache {
public:
  OSDCache();
  ~OSDCache() {}
  int initCommonYuvMap(std::string path, uint32_t size, uint32_t color);
  int deInitCommonYuvMap();
  std::shared_ptr<FontCache> GetCommonFontCache(wchar_t wch);
  FontCache_P GetCaches() { return caches_; }

private:
  FontCache_P caches_;
};

#endif // _RK_OSD_MIDDLEWARE_H_

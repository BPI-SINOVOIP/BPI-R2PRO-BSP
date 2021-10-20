// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <exiv2/exiv2.hpp>

#include <string>

#ifndef RK_EXIF_HANDLER_H_
#define RK_EXIF_HANDLER_H_

namespace rockchip {

class ExifHandler {
public:
  ExifHandler() {}
  virtual ~ExifHandler() {}
  int Wirte(const std::string &file, Exiv2::ExifData &ed);
  int Read(const std::string &file, Exiv2::ExifData &exifData);
  std::string Read(const std::string &file, const std::string &key);

private:
};

} // namespace rockchip

#endif // RK_UTILS_THREAD_H_

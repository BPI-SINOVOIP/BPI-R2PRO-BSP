// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "exifhandler.h"
#include "flow_common.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "exifhandler.cpp"

namespace rockchip {

int ExifHandler::Wirte(const std::string &file, Exiv2::ExifData &ed) {
  Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
  if (image.get() == 0) {
    LOG_ERROR("Wirte open %s failed\n", file.c_str());
    return -1;
  }
  image->setExifData(ed);
  image->writeMetadata();
  return 0;
}

int ExifHandler::Read(const std::string &file, Exiv2::ExifData &exifData) {
  Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file);
  if (image.get() == 0) {
    LOG_ERROR("Read open %s failed\n", file.c_str());
    return -1;
  }
  image->readMetadata();
  exifData = image->exifData();
  if (exifData.empty()) {
    LOG_ERROR("no metadata found in %s ", file.c_str());
    return -1;
  }
  return 0;
}

std::string ExifHandler::Read(const std::string &file, const std::string &key) {
  std::string empty;
  Exiv2::ExifData exifData;
  if (Read(file, exifData) != 0)
    return empty;
  Exiv2::ExifData::iterator pos = exifData.findKey(Exiv2::ExifKey(key));
  if (pos == exifData.end()) {
    LOG_ERROR("Key %s not found in %s ", key.c_str(), file.c_str());
    return empty;
  }
  Exiv2::ExifData::const_iterator end = exifData.end();
  for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
    if (i->key() == key) {
      return i->value().toString();
    }
  }
  return empty;
}
}

#if 0
int main(int argc, char*argv[]) {
  std::string file(argv[1]);

  std::unique_ptr<rockchip::ExifHandler> exif_handler;
  exif_handler.reset(new rockchip::ExifHandler);
 
  std::string face_info("1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18");

  Exiv2::ExifData ed;
  ed["Exif.Image.Model"] = "Rockchip image Face info";
  ed["Exif.Photo.UserComment"] = face_info.c_str();
  exif_handler->Wirte(file, ed);

  std::string model = exif_handler->Read(file, "Exif.Image.Model");
  printf("model ----------- %s\n", model.c_str());
  std::string comment = exif_handler->Read(file, "Exif.Photo.UserComment");
  printf("comment ----------- %s\n", comment.c_str());

  return 0;
}
#endif

// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_BMP_READER_H_
#define _RK_BMP_READER_H_

#include <memory>
#include <stdint.h>
#include <vector>

#include "osd_common.h"

#define WIDTHBYTES(bits) ((((bits) + 31) >> 5) * 4)

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

typedef struct tagBITMAPFILEHEADER {
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

class BMPReader {
public:
  BMPReader() : use_user_buffer(true) {}
  ~BMPReader() {}
  bool BmpCheck();
  int ReadBmpData();
  void FreeBmpData();
  void FreeBmpData2();
  void Bmp24ToARGB8888();
  void Bmp32ToARGB8888();
  void Bmp24ToYUVAMAP(osd_data_s *data);
  void Bmp32ToYUVAMAP(osd_data_s *data);
  int LoadBmpFromFile(osd_data_s *data);
  int LoadYuvaMapFromFile(osd_data_s *data);
  int GetBmpInfo(osd_data_s *data);

private:
  FILE *pfile;
  BITMAPFILEHEADER bitHead;
  BITMAPINFOHEADER bitInfoHead;
  long dataSize;
  BYTE *pColorData;
  tagRGBQUAD *dataOfBmp;
  uint8_t *dataOfYuvaMap;
  bool use_user_buffer;
};

#endif // _RK_BMP_READER_H_
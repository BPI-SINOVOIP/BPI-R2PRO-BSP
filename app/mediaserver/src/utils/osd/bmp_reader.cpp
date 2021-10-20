// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp_reader.h"
#include "color_table.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "bmp_reader.cpp"

void ShowBmpHead(BITMAPFILEHEADER *pBmpHead) {
  printf("bfSize:     %d\n", pBmpHead->bfSize);
  printf("bfReserved1:%d\n", pBmpHead->bfReserved1);
  printf("bfReserved2:%d\n", pBmpHead->bfReserved2);
  printf("bfOffBits:  %d\n", pBmpHead->bfOffBits);
  printf("\n\n");
}

void ShowBmpInforHead(tagBITMAPINFOHEADER *pBmpInforHead) {
  printf("BmpInforHead:\n");
  printf("biSize:         %d\n", pBmpInforHead->biSize);
  printf("biWidth:        %d\n", pBmpInforHead->biWidth);
  printf("biHeight:       %d\n", pBmpInforHead->biHeight);
  printf("biPlanes:       %d\n", pBmpInforHead->biPlanes);
  printf("biBitCount:     %d\n", pBmpInforHead->biBitCount);
  printf("biCompression:  %d\n", pBmpInforHead->biCompression);
  printf("biSizeImage:    %d\n", pBmpInforHead->biSizeImage);
  printf("biXPelsPerMeter:%d\n", pBmpInforHead->biXPelsPerMeter);
  printf("biYPelsPerMeter:%d\n", pBmpInforHead->biYPelsPerMeter);
  printf("biClrUsed:      %d\n", pBmpInforHead->biClrUsed);
  printf("biClrImportant: %d\n", pBmpInforHead->biClrImportant);
  printf("\n\n");
}

void ShowRgbQuan(tagRGBQUAD *pRGB) {
  printf("(%-3d,%-3d,%-3d)   ", pRGB->rgbRed, pRGB->rgbGreen, pRGB->rgbBlue);
}

bool BMPReader::BmpCheck() {
  LOG_INFO("BmpCheck\n");
  WORD fileType;
  fread(&fileType, 1, sizeof(WORD), pfile);
  if (fileType != 0x4d42) {
    printf("file is not .bmp file!");
    return false;
  }
  fread(&bitHead, 1, sizeof(BITMAPFILEHEADER), pfile);
  fread(&bitInfoHead, 1, sizeof(BITMAPINFOHEADER), pfile);

  if (bitInfoHead.biBitCount < 24) {
    printf("bmp must be 24/32 depth!");
    return false;
  }
  return true;
}

int BMPReader::ReadBmpData() {
  LOG_INFO("ReadBmpData\n");
  int width = bitInfoHead.biWidth;
  int height = bitInfoHead.biHeight;
  int pitch = WIDTHBYTES(width * bitInfoHead.biBitCount);
  dataSize = pitch * height;
  pColorData = (BYTE *)malloc(dataSize);
  if (pColorData == NULL) {
    printf("pColorData malloc fail!\n");
    return -1;
  }
  if (fread(pColorData, 1, dataSize, pfile) <= 0) {
    printf("pColorData fread fail!\n");
    return -1;
  }

  return 0;
}

void BMPReader::FreeBmpData() {
  LOG_INFO("FreeBmpData\n");
  fclose(pfile);
  if (dataOfBmp) {
    printf("111121\n");
    free(dataOfBmp);
    dataOfBmp = NULL;
  }
  if (pColorData) {
    printf("111121\n");
    free(pColorData);
    pColorData = NULL;
  }
}

void BMPReader::FreeBmpData2() {
  LOG_INFO("FreeBmpData2\n");
  fclose(pfile);
  if (pColorData) {
    free(pColorData);
    pColorData = NULL;
  }
}

void BMPReader::Bmp24ToARGB8888() {
  LOG_INFO("Bmp24ToARGB8888\n");
  int k;
  int index = 0;
  int width = bitInfoHead.biWidth;
  int height = bitInfoHead.biHeight;
  int pitch = WIDTHBYTES(width * bitInfoHead.biBitCount);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      k = (height - i) * pitch + j * 3;
      dataOfBmp[index].rgbReserved = 0XFF;
      dataOfBmp[index].rgbRed = pColorData[k + 2];
      dataOfBmp[index].rgbGreen = pColorData[k + 1];
      dataOfBmp[index].rgbBlue = pColorData[k];
      index++;
    }
  }
}

void BMPReader::Bmp32ToARGB8888() {
  LOG_INFO("Bmp32ToARGB8888\n");
  int k;
  int index = 0;
  int width = bitInfoHead.biWidth;
  int height = bitInfoHead.biHeight;
  int pitch = WIDTHBYTES(width * bitInfoHead.biBitCount);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      k = (height - i) * pitch + j * 4;
      dataOfBmp[index].rgbReserved = pColorData[k + 3];
      dataOfBmp[index].rgbRed = pColorData[k + 2];
      dataOfBmp[index].rgbGreen = pColorData[k + 1];
      dataOfBmp[index].rgbBlue = pColorData[k];
      index++;
    }
  }
}

void BMPReader::Bmp24ToYUVAMAP(osd_data_s *data) {
  LOG_INFO("Bmp24ToYUVAMAP\n");
  int k, offset;
  int width = bitInfoHead.biWidth;
  int height = bitInfoHead.biHeight;
  int pitch = WIDTHBYTES(width * bitInfoHead.biBitCount);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      k = (height - i - 1) * pitch + j * 3;
      offset = i * data->width + j;
      // printf("offset %d k %d\n", offset, k);
      data->buffer[offset] =
          find_color(rgb888_palette_table, PALETTE_TABLE_LEN, pColorData[k + 2],
                     pColorData[k + 1], pColorData[k]);
    }
  }
}

void BMPReader::Bmp32ToYUVAMAP(osd_data_s *data) {
  LOG_INFO("Bmp32ToYUVAMAP\n");
  int k, offset;
  int width = bitInfoHead.biWidth;
  int height = bitInfoHead.biHeight;
  int pitch = WIDTHBYTES(width * bitInfoHead.biBitCount);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      k = (height - i - 1) * pitch + j * 4;
      offset = i * data->width + j;
      // printf("offset %d k %d\n", offset, k);
      data->buffer[offset] =
          find_color(rgb888_palette_table, PALETTE_TABLE_LEN, pColorData[k + 2],
                     pColorData[k + 1], pColorData[k]);
      OSD_LOG_DEBUG("-%3d", data->buffer[offset]);
    }
    OSD_LOG_DEBUG("\n");
  }
}

int BMPReader::LoadBmpFromFile(osd_data_s *data) {
  LOG_INFO("LoadBmpFromFile\n");
  pfile = fopen(data->image, "rb");
  if (pfile == NULL) {
    printf("Bmp file open fail!\n");
    return -1;
  }
  if (!BmpCheck())
    return -1;
  ReadBmpData();

  dataOfBmp = (tagRGBQUAD *)malloc(bitInfoHead.biWidth * bitInfoHead.biHeight *
                                   sizeof(tagRGBQUAD));
  if (dataOfBmp == NULL) {
    printf("dataOfBmp malloc fail!\n");
    return -1;
  }

  if (bitInfoHead.biBitCount == 24) {
    Bmp24ToARGB8888();
  } else {
    Bmp32ToARGB8888();
  }

  data->width = bitInfoHead.biWidth;
  data->height = bitInfoHead.biHeight;
  data->buffer = (uint8_t *)dataOfBmp;
  data->size = data->width * data->height * sizeof(tagRGBQUAD);

  FreeBmpData();
  return 0;
}

int BMPReader::LoadYuvaMapFromFile(osd_data_s *data) {
  LOG_INFO("LoadYuvaMapFromFile\n");
  pfile = fopen(data->image, "rb");
  if (pfile == NULL) {
    printf("Bmp file open fail!\n");
    return -1;
  }
  if (!BmpCheck())
    return -1;
  ReadBmpData();

  if (data->buffer == NULL) {
    printf("data buffer is NULL, will do malloc!\n");
    data->buffer =
        (uint8_t *)malloc(bitInfoHead.biWidth * bitInfoHead.biHeight);
    if (data->buffer == NULL) {
      printf("LoadYuvaMapFromFile buffer malloc fail!\n");
      return -1;
    }
    use_user_buffer = false;
  }

  if (bitInfoHead.biBitCount == 24) {
    Bmp24ToYUVAMAP(data);
  } else {
    Bmp32ToYUVAMAP(data);
  }

  if (!use_user_buffer) {
    data->width = bitInfoHead.biWidth;
    data->height = bitInfoHead.biHeight;
    data->size = data->width * data->height;
  }

  FreeBmpData2();
  return 0;
}

int BMPReader::GetBmpInfo(osd_data_s *data) {
  LOG_INFO("FillBmpInfoToData\n");
  pfile = fopen(data->image, "rb");
  if (pfile == NULL) {
    printf("Bmp file open fail!\n");
    return -1;
  }
  if (!BmpCheck()) {
    fclose(pfile);
    pfile = NULL;
    return -1;
  }

  data->width = bitInfoHead.biWidth;
  data->height = bitInfoHead.biHeight;

  fclose(pfile);
  pfile = NULL;
  return 0;
}

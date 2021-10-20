// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_OSD_SERVER_H_
#define _RK_OSD_SERVER_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "flow.h"
#include "media_config.h"

#include "flow_common.h"
#include "osd/osd_producer.h"
#include "server.h"
#include "thread.h"

namespace rockchip {
namespace mediaserver {

#ifdef MEDIASERVER_CONF_PREFIX
#define OSD_RESOURCE_PATH MEDIASERVER_CONF_PREFIX "/usr/share/mediaserver/"
#else
#define OSD_RESOURCE_PATH "/oem/usr/share/mediaserver/"
#endif
#define FONT_PATH OSD_RESOURCE_PATH "simsun.ttc"
#define IMAGE_PATH OSD_RESOURCE_PATH "image.bmp"

#define WEB_VIEW_RECT_W 704
#define WEB_VIEW_RECT_H 480

#define COLOR_BLACK_INDEX 0xd7
#define COLOR_WHITE_INDEX 0x0

#define MULTI_UPALIGNTO16(grad, value) UPALIGNTO16((int)(grad * value))

#define OSD_MAX(x, y) ((x) < (y) ? (y) : (x))
#define OSD_MIN(X, Y)                                                          \
  ({                                                                           \
    typeof(X) x_ = (X);                                                        \
    typeof(Y) y_ = (Y);                                                        \
    (x_ < y_) ? x_ : y_;                                                       \
  })

enum {
  OSD_DB_REGION_ID_START = -1,
  OSD_DB_REGION_ID_CHANNEL = 0,
  OSD_DB_REGION_ID_DATETIME,
  OSD_DB_REGION_ID_TEXT0,
  OSD_DB_REGION_ID_TEXT1,
  OSD_DB_REGION_ID_TEXT2,
  OSD_DB_REGION_ID_TEXT3,
  OSD_DB_REGION_ID_TEXT4,
  OSD_DB_REGION_ID_TEXT5,
  OSD_DB_REGION_ID_TEXT6,
  OSD_DB_REGION_ID_TEXT7,
  OSD_DB_REGION_ID_MASK0,
  OSD_DB_REGION_ID_MASK1,
  OSD_DB_REGION_ID_MASK2,
  OSD_DB_REGION_ID_MASK3,
  OSD_DB_REGION_ID_IMAGE,
  OSD_DB_REGION_ID_END
};

enum {
  OSD_REGION_ID_START = -1, // useless, start mark
  OSD_REGION_ID_INVADE = 0, // ID_TEXT3-4 now use to region invade
  OSD_REGION_ID_MASK, // ID_MASK0  (1-2-3)
  OSD_REGION_ID_IMAGE, // ID_IMAGE
  OSD_REGION_ID_TIMEDATE, // timedate
  OSD_REGION_ID_TEXT0, // ID_TEXT0
  OSD_REGION_ID_TEXT1, // ID_TEXT1  (2)
  OSD_REGION_ID_CHANNEL, // ID_CHANNEL
  OSD_REGION_ID_TIP, // ID_TEXT5-6-7
  OSD_REGION_ID_END // useless, end mark
};

#define OSD_FMT_SPACE " "
#define OSD_FMT_TIME0 "24hour"
#define OSD_FMT_TIME1 "12hour"
#define OSD_FMT_WEEK0 "WEEKCN"
#define OSD_FMT_WEEK1 "WEEK"
#define OSD_FMT_CHR "CHR"
#define OSD_FMT_YMD0 "YYYY-MM-DD"
#define OSD_FMT_YMD1 "MM-DD-YYYY"
#define OSD_FMT_YMD2 "DD-MM-YYYY"
#define OSD_FMT_YMD3 "YYYY/MM/DD"
#define OSD_FMT_YMD4 "MM/DD/YYYY"
#define OSD_FMT_YMD5 "DD/MM/YYYY"

#define OSD_NN_TIP_AUTHORIZED "人脸算法软件未授权..."
#define OSD_NN_TIP_REGION_INVADE "检测到人员入侵..."

class OSDServer : public Service {
public:
  OSDServer() = delete;
  OSDServer(std::shared_ptr<easymedia::Flow> &flow, int w, int h);
  virtual ~OSDServer();
  virtual void start(void) override;
  virtual void stop(void) override;
  ThreadStatus status(void);

  void DumpOsdData();
  int GetWidth() { return view_width_; }
  int GetHeight() { return view_height_; }

  void DefaultConfig();
#ifdef ENABLE_DBUS
  void ParseDbConfig();
  void SyncDbConfig();
#endif

  void SetOsdTextRegion(int region_id, std::map<std::string, std::string> map);
  void SetOsdDateRegion(int region_id, std::map<std::string, std::string> map);
  void SetOsdImageRegion(int region_id, std::map<std::string, std::string> map);
  void SetOsdMaskRegion(int region_id, std::map<std::string, std::string> map);
  void SetOsdRegion(int region_id,
                    const std::map<std::string, std::string> map);

  void SetOsdTip(int x, int y, std::string tip);
  void SetOsdBorder(int x, int y, int w, int h, int color_index, int thick,
                    int display_style);
  void BlinkRegionInvade();
  void SetRegionInvade(region_invade_s region_invade,
                       int display_style = BORDER_DOTTED);
  void SetRegionInvadeBlink(int blink_cnt, int interval_ms);
  void CleanRegionInvade();

  int UpdateBlink();
  int UpdateText();
  int UpdateImage();
  int UpdateMask();
  void GenDataTime(const char *fmt, wchar_t *result, int r_size);
  int UpdateTimeDate();

  int GetDelayMs() { return delay_ms_; }
  std::shared_ptr<easymedia::Flow> &GetFlow() { return encoder_flow_; }

  static std::shared_ptr<OSDProducer> osd_producer_;
  static std::string bmp_path_;
  static std::string ttc_path_;

private:
  int delay_ms_;
  std::wstring datetime_wstr_;
  std::shared_ptr<easymedia::Flow> encoder_flow_;
  Thread::UniquePtr service_thread_;
  osd_data_s osds_db_data_[15];
  int osds_db_data_change_[15];
  OsdRegionData region_data_[8];
  int view_width_;
  int view_height_;
  float gradient_x_;
  float gradient_y_;
  region_invade_s region_invade_;
  bool only_show_authorized_info_;
#ifdef ENABLE_DBUS
  std::wstring temp_wstr_;
  std::wstring dump_wstr_;
#endif
};

} // namespace mediaserver
} // namespace rockchip

#endif // _RK_OSD_SERVER_H_

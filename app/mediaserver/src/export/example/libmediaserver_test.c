// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <uAPI/rk_aiq_user_api_imgproc.h>
#include <uAPI/rk_aiq_user_api_sysctl.h>

#include "libmediaserver_api.h"

static bool quit = false;

static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}

void video_process_0(unsigned char *buffer, unsigned int buffer_size,
                     int64_t present_time, int nal_type) {
  //printf("%s: size %d, timestamp is %lld\n", __func__, buffer_size, present_time);
}

void video_process_1(unsigned char *buffer, unsigned int buffer_size,
                     int64_t present_time, int nal_type) {
  //printf("%s: size %d, timestamp is %lld\n", __func__, buffer_size, present_time);
}

void video_process_2(unsigned char *buffer, unsigned int buffer_size,
                     int64_t present_time, int nal_type) {
  //printf("%s: size %d, timestamp is %lld\n", __func__, buffer_size, present_time);
}

void audio_process(unsigned char *buffer, unsigned int buffer_size,
                   int64_t present_time) {
  //printf("%s: size %d, timestamp is %lld\n", __func__, buffer_size, present_time);
}

void jpeg_process(unsigned char *buffer, unsigned int buffer_size,
                  int null, const char *null_ptr) {
  printf("%s: size %d\n", __func__, buffer_size);
}

void MDCallback(MOVE_DETECT_EVENT_S *mdevent) {
  printf("@@@ MD: Get movement info[%d]: ORI:%dx%d, DS:%dx%d\n",
            mdevent->info_cnt, mdevent->ori_width, mdevent->ori_height,
            mdevent->ds_width, mdevent->ds_height);
  MOVE_DETEC_INFO_S *mdinfo = mdevent->data;
  for (int i = 0; i < mdevent->info_cnt; i++) {
    printf("--> %d rect:(%d, %d, %d, %d)\n", i, mdinfo->x, mdinfo->y,
              mdinfo->w, mdinfo->h);
    mdinfo++;
  }
}

/* AIQ Init */
rk_aiq_sys_ctx_t *init_engine(const char *iq_file_dir, rk_aiq_working_mode_t mode) {
  /* Line buffered so that printf can flash every line if redirected to
   * no-interactive device.
   */
  setlinebuf(stdout);

  rk_aiq_sys_ctx_t *aiq_ctx;
  rk_aiq_static_info_t aiq_static_info;
  rk_aiq_uapi_sysctl_enumStaticMetas(0, &aiq_static_info);
  printf("sensor_name is %s, iqfiles is %s\n",
          aiq_static_info.sensor_info.sensor_name, iq_file_dir);

  aiq_ctx = rk_aiq_uapi_sysctl_init(aiq_static_info.sensor_info.sensor_name,
                                    iq_file_dir, NULL, NULL);

  if (rk_aiq_uapi_sysctl_prepare(aiq_ctx, NULL, NULL, mode)) {
    printf("rkaiq engine prepare failed !\n");
    exit(-1);
  }
  printf("rk_aiq_uapi_sysctl_start\n");

  rk_aiq_uapi_sysctl_start(aiq_ctx);
  if (aiq_ctx == NULL) {
    printf("rkisp_init engine failed\n");
    exit(-1);
  } else {
    printf("rkisp_init engine succeed\n");
  }

  return aiq_ctx;
}

void help_printf(void)
{
    printf("************************\n");
    printf("0.Help\n");
    printf("1.Change Bitrate\n");
    printf("2.Force IFrame\n");
    printf("3.Change Profile\n");
    printf("4.Change Resolution\n");
    printf("5.Rate Control VBR\n");
    printf("6.Rate Control CBR\n");
    printf("7.Set OSD\n");
    printf("8.Clean OSD\n");
    printf("9.Set ROI\n");
    printf("a.Set Move Detect\n");
    printf("b.Snapshot\n");
    printf("c.Set Userdata\n");
    printf("d.Change Video Encode Type\n");
    printf("e.Change Audio Encode Type\n");
    printf("f.Set Brightness\n");
    printf("q.Quit\n");
    printf("************************\n");
}

int main(int argc, char *argv[]) {
  signal(SIGQUIT, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  signal(SIGXCPU, sigterm_handler);
  signal(SIGPIPE, SIG_IGN);

  if (argc != 2) {
    fprintf(stderr, "%s config\n", argv[0]);
    return -1;
  }

  rk_aiq_sys_ctx_t *aiq_ctx = NULL;
  RESOURCE_S env;
  strcpy(env.cBMPPath, "/usr/share/mediaserver/image.bmp\0");
  strcpy(env.cTTCPath, "/usr/share/mediaserver/simsun.ttc\0");
  aiq_ctx = init_engine("/etc/iqfiles/", RK_AIQ_WORKING_MODE_ISP_HDR2);
  libMediaInit(argv[1], env);
  libMediaSyncVideoData(0, "{\"sOutputDataType\":\"MJPEG\",\"sResolution\":\"1920*1080\"}");
  //libMediaSyncVideoData(1, "{\"sOutputDataType\":\"H.265\",\"sResolution\":\"352*288\"}");
  //libMediaSyncVideoData(2, "{\"sOutputDataType\":\"H.264\",\"sResolution\":\"720*480\"}");
  libMediaSyncAudioData("{\"sEncodeType\":\"G711U\"}");
  libMediaStart();

  int result = 0;
  int id = 0;
  int bitrate = 20000000;
  int profile = 100;

  //libMediaVideoStart(0, video_process_0); // default H265
  //libMediaVideoStart(1, video_process_1); // default H264
  //libMediaVideoStart(2, video_process_2); // default MJPEG
  //libMediaAudioStart(audio_process);
  //libMediaCaptureStart(0, jpeg_process);

again:
  help_printf();
  while (!quit) {
    char cmd = 0;
    printf("please enter:");
    scanf("%c", &cmd);
    switch(cmd) {
      case '0':
        help_printf();
        break;
      case '1': {
        fprintf(stderr, "Change Bitrate\n\n");
        bitrate = bitrate + 2000;
        if (bitrate > 40000000)
            bitrate = 20000000;
        result = libMediaVideoChangeBitrate(id, bitrate);
        if (result < 0) {
          fprintf(stderr, "libMediaVideoChangeBitrate error\n");
        }
        break;
      }
      case '2': {
        fprintf(stderr, "Force I Frame\n\n");
        result = libMediaVideoForceIFrame(id);
        if (result < 0) {
          fprintf(stderr, "libMediaVideoForceIFrame error\n");
        }
        break;
      }
      case '3': {
        fprintf(stderr, "Change Profile\n\n");
        if (profile == 100)
          profile = 66;
        else if (profile == 66)
          profile = 77;
        else if (profile == 77)
          profile = 100;
        result = libMediaVideoChangeProfile(1, profile);
        if (result < 0) {
          fprintf(stderr, "libMediaVideoChangeProfile error\n");
        }
        break;
      }
      case '4': {
        fprintf(stderr, "Change Resolution\n\n");
        result = libMediaVideoChangeResolution(1, 1280, 720);
        if (result < 0) {
          fprintf(stderr, "libMediaVideoChangeResolution error\n");
        }
        break;
      }
      case '5': {
        fprintf(stderr, "Rate Control VBR\n\n");
        result = libMediaVideoRateControl(id, "vbr");
        if (result < 0) {
          fprintf(stderr, "libMediaVideoRateControl error\n");
        }
        break;
      }
      case '6': {
        fprintf(stderr, "Rate Control CBR\n\n");
        result = libMediaVideoRateControl(id, "cbr");
        if (result < 0) {
          fprintf(stderr, "libMediaVideoRateControl error\n");
        }
        break;
      }
      case '7': {
        fprintf(stderr, "Set OSD\n\n");
        OSD_CFG_S text;
        text.u16Status = 1;
        text.u16FontPixelSize = 32;
        text.stOsdColor.u32FgColor  = 0xfff799;
        text.s32OsdRegHdl = 0;
        text.stOsdPos.u16PosX = 384;
        text.stOsdPos.u16PosY = 16;
        strcpy(text.cOsdTitle[0], "test1\0");
        libMediaVideoSetTextOSD(id, text);
        text.s32OsdRegHdl = 2;
        text.stOsdPos.u16PosX = 384;
        text.stOsdPos.u16PosY = 64;
        strcpy(text.cOsdTitle[0], "test2\0");
        libMediaVideoSetTextOSD(id, text);
        text.s32OsdRegHdl = 3;
        text.stOsdPos.u16PosX = 512;
        text.stOsdPos.u16PosY = 16;
        strcpy(text.cOsdTitle[0], "test3\0");
        libMediaVideoSetTextOSD(id, text);

        OSD_CFG_S bmp;
        bmp.u16Status = 1;
        bmp.stOsdPos.u16PosX = 16;
        bmp.stOsdPos.u16PosY = 388;
        // The path is temporarily hard-coded as /oem/usr/share/mediaserver/image.bmp
        // required bit depth is 24 bits
        libMediaVideoSetBMPOSD(id, bmp);

        OSD_CFG_S date_time;
        date_time.u16Status = 1;
        date_time.u16DisplayWeek = 1;
        date_time.u16FontPixelSize = 32;
        date_time.stOsdColor.u32FgColor  = 0xfff799;
        date_time.stOsdPos.u16PosX = 16;
        date_time.stOsdPos.u16PosY = 16;
        strcpy(date_time.cOsdColorMode, "customize\0");
        strcpy(date_time.cTimeStyle, "24hour\0");
        strcpy(date_time.cDateStyle, "YYYY-MM-DD\0");
        libMediaVideoSetDateOSD(id, date_time);

        PRIVACYMASK_DATA_S mask;
        mask.s32X[0] = 64;
        mask.s32Y[0] = 64;
        mask.s32W[0] = 32;
        mask.s32H[0] = 32;
        libMediaVideoSetPrivacyMask(id, mask);
        break;
      }
      case '8': {
        fprintf(stderr, "Clean OSD\n\n");
        OSD_CFG_S text;
        text.u16Status = 0;
        text.s32OsdRegHdl = 0;
        libMediaVideoSetTextOSD(id, text);
        text.s32OsdRegHdl = 2;
        libMediaVideoSetTextOSD(id, text);
        text.s32OsdRegHdl = 3;
        libMediaVideoSetTextOSD(id, text);

        OSD_CFG_S bmp;
        bmp.u16Status = 0;
        libMediaVideoSetBMPOSD(id, bmp);

        OSD_CFG_S date_time;
        date_time.u16Status = 0;
        libMediaVideoSetDateOSD(id, date_time);

        libMediaVideoRmPrivacyMask(id);
        break;
      }
      case '9': {
        fprintf(stderr, "Set ROI\n\n");
        int region_cnt = 2;
        ROI_REGION_S regions [2] ={{32,32,32,32,0,-12,0,0,0}, {128,128,128,128,0,-8,0,0,0}};
        libMediaVideoSetROI(id, regions, region_cnt);
        break;
      }
      case 'a': {
        fprintf(stderr, "Set Move Detect\n\n");
        libMediaSetMDCallback(MDCallback);
        int enabled = 1;
        int sensitivity = 3;
        int rect_cnt = 4;
        IMAGE_RECT_S rects [4] = {{522,52,29,26}, {551,52,29,26}, {522,78,29,26}, {551,78,29,26}};
        libMediaVideoSetMDEnabled(enabled);
        libMediaVideoSetMDSensitivity(sensitivity);
        libMediaVideoSetMDRect(rects, rect_cnt);
        break;
      }
      case 'b': {
        fprintf(stderr, "Snapshot\n\n");
        libMediaGetSnapshot(0);
        break;
      }
      case 'c': {
        fprintf(stderr, "Set Userdata\n\n");
        char sei_str[] = "RockChip AVC/HEVC Codec";
        printf("Set userdata string:%s\n", sei_str);
        int len = 23;
        int all_frames = 1;
        libMediaVideoSetUserdata(1, sei_str, len, all_frames);
        break;
      }
      case 'd': {
        fprintf(stderr, "Change Video Encode Type\n\n");
        libMediaVideoChangeOutputDataType(1, "MJPEG");
        break;
      }
      case 'e': {
        fprintf(stderr, "Change Audio Encode Type\n\n");
        libMediaAudioChangeEncodeType("G711A");
        break;
      }
      case 'f': {
        rk_aiq_uapi_setBrightness(aiq_ctx, 90);
        break;
      }
      case 'q': {
        quit = true;
        break;
      }
      case 0xa:
        continue;
        break;
    }
    goto again;
  }
  libMediaVideoStop(0);
  libMediaVideoStop(1);
  libMediaVideoStop(2);
  libMediaAudioStop();
  libMediaCaptureStop(0);
  // libMediaSaveConfig(argv[1]);
  libMediaCleanup();

  printf("rk_aiq_uapi_sysctl_stop\n");
  rk_aiq_uapi_sysctl_stop(aiq_ctx);
  rk_aiq_uapi_sysctl_deinit(aiq_ctx);

  return 0;
}

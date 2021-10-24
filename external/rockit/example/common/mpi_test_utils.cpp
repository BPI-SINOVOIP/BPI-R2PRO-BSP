/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <sys/time.h>
#include <time.h>
#include <cstring>

#include "mpi_test_utils.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavformat/version.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
}

#define SIZE_ARRAY_ELEMS(a)          (sizeof(a) / sizeof((a)[0]))

typedef struct _rkCodecInfo {
    RK_CODEC_ID_E enRkCodecId;
    enum AVCodecID enAvCodecId;
    char mine[16];
} CODEC_INFO;

typedef struct _rkPixFmtInfo {
    PIXEL_FORMAT_E enRkPixFmt;
    enum AVPixelFormat enAvPixFmt;
    char                    mine[16];
} PIXFMT_INFO;

static CODEC_INFO gCodecMapList[] = {
    { RK_VIDEO_ID_MPEG1VIDEO,    AV_CODEC_ID_MPEG1VIDEO,  "mpeg1" },
    { RK_VIDEO_ID_MPEG2VIDEO,    AV_CODEC_ID_MPEG2VIDEO,  "mpeg2" },
    { RK_VIDEO_ID_H263,          AV_CODEC_ID_H263,  "h263"  },
    { RK_VIDEO_ID_MPEG4,         AV_CODEC_ID_MPEG4, "mpeg4"  },
    { RK_VIDEO_ID_WMV,           AV_CODEC_ID_WMV3,  "wmv3" },
    { RK_VIDEO_ID_AVC,           AV_CODEC_ID_H264,  "h264" },
    { RK_VIDEO_ID_MJPEG,         AV_CODEC_ID_MJPEG, "mjpeg" },
    { RK_VIDEO_ID_VP8,           AV_CODEC_ID_VP8,   "vp8" },
    { RK_VIDEO_ID_VP9,           AV_CODEC_ID_VP9,   "vp9" },
    { RK_VIDEO_ID_HEVC,          AV_CODEC_ID_HEVC,  "hevc" },
    { RK_VIDEO_ID_VC1,           AV_CODEC_ID_VC1,   "vc1" },
    { RK_VIDEO_ID_AVS,           AV_CODEC_ID_AVS,   "avs" },
    { RK_VIDEO_ID_AVS,           AV_CODEC_ID_CAVS,  "cavs" },
    { RK_VIDEO_ID_AVSPLUS,       AV_CODEC_ID_CAVS,  "avs+" },
    { RK_VIDEO_ID_FLV1,          AV_CODEC_ID_FLV1,  "flv1" },
};

RK_S32 utils_av_to_rk_codec(RK_S32 s32Id) {
    RK_BOOL bFound = RK_FALSE;
    RK_U32 i = 0;
    for (i = 0; i < SIZE_ARRAY_ELEMS(gCodecMapList); i++) {
        if (s32Id == gCodecMapList[i].enAvCodecId) {
            bFound = RK_TRUE;
            break;
        }
    }

    if (bFound)
        return gCodecMapList[i].enRkCodecId;
    else
        return RK_VIDEO_ID_Unused;
}

RK_U64 mpi_test_utils_get_now_us() {
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (RK_U64)time.tv_sec * 1000000 + (RK_U64)time.tv_nsec / 1000; /* microseconds */
}

URI_SCHEME_TYPE get_uri_scheme_type(const char* uri) {
    URI_SCHEME_TYPE schemeType = RK_URI_SCHEME_LOCAL;
    if ((RK_NULL == uri) || (strlen(uri) < 4)) {
        return schemeType;
    }

    if (!strncmp("http://", uri, 7) || !strncmp("https://", uri, 8)) {
        schemeType = RK_URI_SCHEME_HTTP;
    } else if (!strncmp("rtsp://", uri, 7) || !strncmp("rtmp://", uri, 7)) {
        RK_LOGD("uri is with rtsp or rtmp scheme type");
        schemeType = RK_URI_SCHEME_RTSP;
    } else if (!strncmp("/data/smb/", uri, 10)) {
        RK_LOGD("uri is with /data/smb scheme type");
        schemeType = RK_URI_SCHEME_SAMBA;
    } else if (!strncmp("/data/nfs/", uri, 10)) {
        RK_LOGD("uri is with /data/nfs schemeType (signed as samba)");
        schemeType = RK_URI_SCHEME_SAMBA;
    } else if (strstr(uri, "m3u8")) {
        RK_LOGD("uri is with m3u8 scheme type");
        schemeType = RK_URI_SCHEME_HLS;
    } else if (!strncmp("rtp:", uri, 4)) {
        RK_LOGD("uri is with rtp scheme type");
        schemeType = RK_URI_SCHEME_RTP;
    } else if (!strncmp("udp:", uri, 4)) {
        RK_LOGD("uri is with udp scheme type");
        schemeType = RK_URI_SCHEME_UDP;
    } else if (!strncmp("mms://", uri, 6)) {
        RK_LOGD("uri is with mms scheme type");
        schemeType = RK_URI_SCHEME_MMS;
    } else if (!strncmp("mmsh://", uri, 7)) {
        RK_LOGD("uri is with mmsh scheme type");
        schemeType = RK_URI_SCHEME_MMSH;
    } else if (!strncmp("mmst://", uri, 7)) {
        RK_LOGD("uri is with mmst scheme type");
        schemeType = RK_URI_SCHEME_MMST;
    } else if (strstr(uri, "app_tts-cache")) {
        RK_LOGD("uri is with tts scheme type");
        schemeType = RK_URI_SCHEME_TTS;
    }  else if (strstr(uri, "cache://")) {
        schemeType = RK_URI_SCHEME_IPTV;
    }
    return schemeType;
}


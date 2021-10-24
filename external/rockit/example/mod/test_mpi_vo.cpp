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
 *
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "rk_debug.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_mb.h"
#include "rk_common.h"
#include "mpi_test_utils.h"
#include "argparse.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavformat/version.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
}

#define MAX_FRAME_QUEUE              8
#define MAX_TIME_OUT_MS              20

#ifndef VDEC_INT64_MIN
#define VDEC_INT64_MIN               (-0x7fffffffffffffffLL-1)
#endif

#ifndef VDEC_INT64_MAX
#define VDEC_INT64_MAX               INT64_C(9223372036854775807)
#endif

#ifndef rk_safe_free
#define rk_safe_free(p)              { if (p) {free(p); (p)=RK_NULL;} }
#endif

#ifndef rk_safe_delete
#define rk_safe_delete(p)            { if (p) {delete(p); (p)=RK_NULL;} }
#endif

#define VDEC_ARRAY_ELEMS(a)          (sizeof(a) / sizeof((a)[0]))

typedef struct _rkWbcCfg {
    VO_WBC_SOURCE_S stWbcSource;
    VO_WBC_ATTR_S stWbcAttr;
    RK_S32 s32ChnId;
    RK_S32 s32VdecChnId;
} WBC_CFG;

typedef struct _rkVdecCfg {
    RK_U32 u32FrameBufferCnt;
    COMPRESS_MODE_E enCompressMode;
} VDEC_CFG;

typedef struct _rkVOCfg {
    RK_U32 u32Screen0VoLayer;
    RK_U32 u32Screen1VoLayer;

    RK_U32 u32Screen0Rows;
    RK_U32 u32Screen1Rows;
    RK_BOOL bDoubleScreen;
} VO_CFG;

typedef struct _rkParserCfg {
    char *srcFileUri[VDEC_MAX_CHN_NUM];
    RK_CODEC_ID_E enCodecId;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_U32 u32StreamIndex;
    AVFormatContext *pstAvfc;
} PARSER_CFG;

typedef struct _rkBorderCfg {
    RK_U32 u32LeftWidth;
    RK_U32 u32RightWidth;
    RK_U32 u32TopWidth;
    RK_U32 u32BottomWidth;
} Border_CFG;

#define RK356X_VOP_LAYER_CLUSTER_0      0
#define RK356X_VOP_LAYER_CLUSTER_1      2
#define RK356X_VOP_LAYER_ESMART_0       4
#define RK356X_VOP_LAYER_ESMART_1       5
#define RK356X_VOP_LAYER_SMART_0        6
#define RK356X_VOP_LAYER_SMART_1        7

#define RK356X_VO_DEV_HD0               0
#define RK356X_VO_DEV_HD1               1

#define DISPLAY_TYPE_HDMI                0
#define DISPLAY_TYPE_EDP                 1
#define DISPLAY_TYPE_VGA                 2

#define MAX_VO_FORMAT_RGB_NUM            4
#define TEST_VO_FORMAT_ARGB8888          0
#define TEST_VO_FORMAT_ABGR8888          1
#define TEST_VO_FORMAT_RGB888            2
#define TEST_VO_FORMAT_BGR888            3
#define TEST_VO_FORMAT_ARGB1555          4
#define TEST_VO_FORMAT_ABGR1555          5
#define TEST_VO_FORMAT_NV12              6
#define TEST_VO_FORMAT_NV21              7

#define VO_CHANNEL_PLAY_NORMAL           0
#define VO_CHANNEL_PLAY_PAUSE            1
#define VO_CHANNEL_PLAY_STEP             2
#define VO_CHANNEL_PLAY_SPEED            3

#define WBC_SOURCE_DEV                0
#define WBC_SOURCE_VIDEO              1
#define WBC_SOURCE_GRAPHIC            2

#define MAX_WINDOWS_NUM               64
#define MAX_STEP_FRAME_NUM            50

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

typedef struct _TEST_MODE {
    RK_U32 mode;
    VO_INTF_SYNC_E enIntfSync;
} TEST_MODE_S;

static VO_DEV VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
static VO_DEV VoLayer_second = RK356X_VOP_LAYER_CLUSTER_1;

TEST_MODE_S test_mode_table[] = {
    {0,     VO_OUTPUT_640x480_60},
    {1,     VO_OUTPUT_NTSC},
    {2,     VO_OUTPUT_PAL},
    {3,     VO_OUTPUT_480P60},
    {4,     VO_OUTPUT_576P50},
    {5,     VO_OUTPUT_800x600_60},
    {6,     VO_OUTPUT_1024x768_60},
    {7,     VO_OUTPUT_720P50},
    {8,     VO_OUTPUT_720P60},
    {9,     VO_OUTPUT_1280x800_60},
    {10,    VO_OUTPUT_1280x1024_60},
    {11,    VO_OUTPUT_1366x768_60},
    {12,    VO_OUTPUT_1440x900_60},
    {13,    VO_OUTPUT_1600x1200_60},
    {14,    VO_OUTPUT_1680x1050_60},
    {15,    VO_OUTPUT_1080I50},
    {16,    VO_OUTPUT_1080I60},
    {17,    VO_OUTPUT_1080P50},
    {18,    VO_OUTPUT_1080P60},
    {19,    VO_OUTPUT_1920x1200_60},
    {20,    VO_OUTPUT_3840x2160_24},
    {21,    VO_OUTPUT_3840x2160_25},
    {22,    VO_OUTPUT_3840x2160_30},
    {23,    VO_OUTPUT_3840x2160_50},
    {24,    VO_OUTPUT_3840x2160_60},
};

#define Sample_Print(format, ...)       RK_LOGI(format, ##__VA_ARGS__)

typedef struct _rkMpiVOCtx {
    RK_S32  VoDev;
    RK_S32  VoLayer;
    RK_S32  VoLayerMode;

    RK_S32  u32Windows;
    RK_U32  enIntfType; /* 0 HDMI 1 edp */
    RK_U32  enIntfSync; /* VO_OUTPUT_1080P50 */
    RK_U32  s32X;
    RK_U32  s32Y;
    RK_U32  u32DispWidth;
    RK_U32  u32DispHeight;
    RK_U32  u32ImgeWidth;
    RK_U32  u32ImageHeight;
    RK_S32  s32PixFormat;
    RK_U32  u32DispFrmRt;
    RK_U32  u32DispFrmRtRatio;
    RK_U32  uEnMode;

    RK_U32  wbc_enable;
    RK_BOOL wbc_auto;
    RK_U32  ui;
    RK_U32  ui_alpha;
    RK_U32  u32WbcWidth;
    RK_S32  u32WbcHeight;
    COMPRESS_MODE_E  u32WbcCompressMode;
    RK_S32  u32WbcPixFormat;
    RK_U32  u32WbcSourceType;
    RK_U32  u32WbcSourceId;

    VO_LAYER   VoVideoLayer;
    RK_U32     u32VideoWindows;
    RK_U32     u32GFXLayers;
    VO_LAYER   VOGfxLayer;
    RK_BOOL    threadExit;

    RK_BOOL     bVoPlay;
    const char *cfgFileUri;
    RK_S32      s32LoopCount;
    RK_U32      u32ChnIndex;
    RK_U32      u32Screen0Chn;
    RK_U32      u32ChnDismode;
    RK_U32      u32Screen1Chn;
    RK_BOOL     bThreadExit;

    RK_BOOL     bEnWbc;
    RK_BOOL     bEnWbcToVO;
    RK_BOOL     bChnPriority;

    PARSER_CFG  stParserCfg;
    VDEC_CFG    stVdecCfg;
    VO_CFG      stVoCfg;
    WBC_CFG     stWbcCfg;
    Border_CFG  stBorderCfg;
} TEST_VO_CTX_S;

typedef struct _VO_Send_Thread_Param {
    RK_U32 u32VideoWindows;
    VO_LAYER VoVideoLayer;
    RK_U32 u32GFXLayers;
    VO_LAYER VOGfxLayer;
    RK_U32 u32Exit;
} VoThreadParam;

static PIXEL_FORMAT_E Sample_vo_test_fmt_to_rtfmt(RK_S32 format) {
    switch (format) {
        case TEST_VO_FORMAT_ARGB8888:
            return RK_FMT_BGRA8888;
        case TEST_VO_FORMAT_ABGR8888:
            return RK_FMT_RGBA8888;
        case TEST_VO_FORMAT_RGB888:
            return RK_FMT_BGR888;
        case TEST_VO_FORMAT_BGR888:
            return RK_FMT_RGB888;
        case TEST_VO_FORMAT_ARGB1555:
            return RK_FMT_BGRA5551;
        case TEST_VO_FORMAT_ABGR1555:
            return RK_FMT_RGBA5551;
        case TEST_VO_FORMAT_NV12:
            return RK_FMT_YUV420SP;
        case TEST_VO_FORMAT_NV21:
            return RK_FMT_YUV420SP_VU;
        default:
            return RK_FMT_BUTT;
    }
}

static void* vochn_test_thread_func(void *pArgs) {
    TEST_VO_CTX_S *ctx = reinterpret_cast<TEST_VO_CTX_S *>(pArgs);
    RK_S32 i, j;

    while (1) {
        if (ctx->u32ChnDismode == VO_CHANNEL_PLAY_PAUSE) {
            for (i = 0; i < ctx->u32Screen0Chn; i++) {
                RK_MPI_VO_PauseChn(VoLayer, i);
            }

            usleep(1000llu * 2000);

            for (i = 0; i < ctx->u32Screen0Chn; i++) {
                RK_MPI_VO_ResumeChn(VoLayer, i);
            }
        } else if (ctx->u32ChnDismode == VO_CHANNEL_PLAY_STEP) {
            for (i = 0; i < ctx->u32Screen0Chn; i++) {
                for (RK_S32 j = 0; j < MAX_STEP_FRAME_NUM; j++) {
                    RK_MPI_VO_StepChn(VoLayer, i);
                }
            }

            usleep(1000llu * 2000);

            for (i = 0; i < ctx->u32Screen0Chn; i++) {
                RK_MPI_VO_ResumeChn(VoLayer, i);
            }
        } else if (ctx->u32ChnDismode == VO_CHANNEL_PLAY_SPEED) {
            for (i = 0; i < ctx->u32Screen0Chn; i++) {
                RK_MPI_VO_SetChnFrameRate(VoLayer, i, ctx->u32DispFrmRt * ctx->u32DispFrmRtRatio);
            }

            usleep(1000llu * 2000);

            for (i = 0; i < ctx->u32Screen0Chn; i++) {
                RK_MPI_VO_ResumeChn(VoLayer, i);
            }
        }

        if (ctx->stVoCfg.bDoubleScreen) {
            if (ctx->u32ChnDismode == VO_CHANNEL_PLAY_PAUSE) {
                for (i = 0; i < ctx->u32Screen1Chn; i++) {
                    RK_MPI_VO_PauseChn(VoLayer_second, i);
                }
                usleep(1000llu * 2000);
                for (i = 0; i < ctx->u32Screen1Chn; i++) {
                    RK_MPI_VO_ResumeChn(VoLayer_second, i);
                }
            } else if (ctx->u32ChnDismode == VO_CHANNEL_PLAY_STEP) {
                for (i = 0; i < ctx->u32Screen1Chn; i++) {
                    for (RK_S32 j = 0; j < MAX_STEP_FRAME_NUM; j++) {
                        RK_MPI_VO_StepChn(VoLayer_second, i);
                    }
                }

                usleep(1000llu * 2000);

                for (i = 0; i < ctx->u32Screen1Chn; i++) {
                    RK_MPI_VO_ResumeChn(VoLayer_second, i);
                }
           } else if (ctx->u32ChnDismode == VO_CHANNEL_PLAY_SPEED) {
                for (i = 0; i < ctx->u32Screen1Chn; i++) {
                    RK_MPI_VO_SetChnFrameRate(VoLayer_second, i, ctx->u32DispFrmRt * ctx->u32DispFrmRtRatio);
                }

                usleep(1000llu * 2000);

                for (i = 0; i < ctx->u32Screen1Chn; i++) {
                    RK_MPI_VO_ResumeChn(VoLayer_second, i);
                }
            }
        }
    }

    return RK_NULL;
}

static RK_S32 mpi_vo_init_sample(const TEST_VO_CTX_S *ctx, RK_S32 primRows , RK_S32 secondRows) {
    VO_PUB_ATTR_S VoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CSC_S  VideoCSC;
    VO_CHN_ATTR_S VoChnAttr[64];
    VO_BORDER_S border;
    RK_S32  i;
    RK_U32 count = 0;
    RK_U32 u32DispBufLen;
    RK_U32 s32Ret;
    RK_U32 disp_width = 1920;
    RK_U32 disp_height = 1080;

    // Bind Layer
    s32Ret = RK_MPI_VO_BindLayer(VoLayer, RK356X_VO_DEV_HD0, VO_LAYER_MODE_VIDEO);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    if (ctx->stVoCfg.bDoubleScreen) {
        s32Ret = RK_MPI_VO_BindLayer(VoLayer_second, RK356X_VO_DEV_HD1, VO_LAYER_MODE_VIDEO);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    VoPubAttr.enIntfType = VO_INTF_HDMI;
    VoPubAttr.enIntfSync = VO_OUTPUT_1080P50;

    s32Ret = RK_MPI_VO_SetPubAttr(RK356X_VO_DEV_HD0, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_Enable(RK356X_VO_DEV_HD0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    if (ctx->stVoCfg.bDoubleScreen) {
        VO_PUB_ATTR_S VoPubAttr1;
        VoPubAttr1.enIntfType = VO_INTF_EDP;
        VoPubAttr1.enIntfSync = VO_OUTPUT_1024x768_60;

        s32Ret = RK_MPI_VO_SetPubAttr(RK356X_VO_DEV_HD1, &VoPubAttr1);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VO_Enable(RK356X_VO_DEV_HD1);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }

        s32Ret = RK_MPI_VO_GetLayerDispBufLen(VoLayer_second, &u32DispBufLen);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Get display buf len failed with error code %d!\n", s32Ret);
            return s32Ret;
        }
        RK_LOGD("Get VoLayer %d disp buf len is %d.\n", VoLayer_second, u32DispBufLen);
        u32DispBufLen = 3;
        s32Ret = RK_MPI_VO_SetLayerDispBufLen(VoLayer_second, u32DispBufLen);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        RK_LOGD("Agin Get VoLayer %d disp buf len is %d.\n", VoLayer_second, u32DispBufLen);
    }

    s32Ret = RK_MPI_VO_GetLayerDispBufLen(VoLayer, &u32DispBufLen);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Get display buf len failed with error code %d!\n", s32Ret);
        return s32Ret;
    }
    RK_LOGD("Get VoLayer %d disp buf len is %d.\n", VoLayer, u32DispBufLen);
    u32DispBufLen = 3;
    s32Ret = RK_MPI_VO_SetLayerDispBufLen(VoLayer, u32DispBufLen);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    RK_LOGD("Agin Get VoLayer %d disp buf len is %d.\n", VoLayer, u32DispBufLen);

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stDispRect.u32Width = disp_width;
    stLayerAttr.stDispRect.u32Height = disp_height;
    stLayerAttr.stImageSize.u32Width = disp_width;
    stLayerAttr.stImageSize.u32Height = disp_height;

    stLayerAttr.u32DispFrmRt = 25;
    if (VoLayer == RK356X_VOP_LAYER_ESMART_0) {
        stLayerAttr.enPixFormat = RK_FMT_YUV420SP;
    } else {
       stLayerAttr.enPixFormat = RK_FMT_RGBA8888;
    }
    stLayerAttr.bDoubleFrame = RK_TRUE;
    VideoCSC.enCscMatrix = VO_CSC_MATRIX_IDENTITY;
    VideoCSC.u32Contrast =  50;
    VideoCSC.u32Hue = 50;
    VideoCSC.u32Luma = 50;
    VideoCSC.u32Satuature = 50;

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_SetLayerCSC(VoLayer, &VideoCSC);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    // configure and enable vo channel
    RK_S32 totalCH = primRows * primRows;
    for (i = 0; i < totalCH; i++) {
        VoChnAttr[i].bDeflicker = RK_FALSE;
        VoChnAttr[i].u32Priority = 1;
        VoChnAttr[i].stRect.s32X = (disp_width/primRows)*(i%primRows);
        VoChnAttr[i].stRect.s32Y = (disp_height/primRows)*(i/primRows);
        VoChnAttr[i].stRect.u32Width = disp_width/primRows;
        VoChnAttr[i].stRect.u32Width = VoChnAttr[i].stRect.u32Width;
        VoChnAttr[i].stRect.u32Height = disp_height/primRows;
        VoChnAttr[i].stRect.u32Height = VoChnAttr[i].stRect.u32Height;
    }

    for (i = 0; i < totalCH; i++) {
        // set attribute of vo chn
        RK_S32 s32ret = RK_MPI_VO_SetChnAttr(VoLayer, i, &VoChnAttr[i]);
        if (RK_SUCCESS != s32ret) {
            RK_LOGE("vo set dev %d chn %d attr failed! \n", VoLayer, i);
        }
        // set border
        border.bBorderEn = RK_TRUE;
        border.stBorder.u32Color = 0x191970;
        border.stBorder.u32LeftWidth =  ctx->stBorderCfg.u32LeftWidth;
        border.stBorder.u32RightWidth = ctx->stBorderCfg.u32RightWidth;
        border.stBorder.u32TopWidth =  ctx->stBorderCfg.u32TopWidth;
        border.stBorder.u32BottomWidth = ctx->stBorderCfg.u32BottomWidth;
        s32Ret = RK_MPI_VO_SetChnBorder(VoLayer, i, &border);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    if (ctx->stVoCfg.bDoubleScreen) {
        disp_width = 1024;
        disp_height = 768;

        stLayerAttr.stDispRect.s32X = 0;
        stLayerAttr.stDispRect.s32Y = 0;
        stLayerAttr.stDispRect.u32Width = disp_width;
        stLayerAttr.stDispRect.u32Height = disp_height;
        stLayerAttr.stImageSize.u32Width = disp_width;
        stLayerAttr.stImageSize.u32Height = disp_height;

        stLayerAttr.u32DispFrmRt = 25;
        if (VoLayer_second == RK356X_VOP_LAYER_ESMART_1) {
            stLayerAttr.enPixFormat = RK_FMT_YUV420SP;
        } else {
           stLayerAttr.enPixFormat = RK_FMT_RGBA8888;
        }
        stLayerAttr.bDoubleFrame = RK_TRUE;

        s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer_second, &stLayerAttr);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VO_EnableLayer(VoLayer_second);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        // configure and enable vo channel
        totalCH = secondRows * secondRows;
        for (i = 0; i < totalCH; i++) {
            VoChnAttr[i].bDeflicker = RK_FALSE;
            VoChnAttr[i].u32Priority = 1;
            VoChnAttr[i].stRect.s32X = (disp_width/secondRows)*(i%secondRows);
            VoChnAttr[i].stRect.s32Y = (disp_height/secondRows)*(i/secondRows);
            VoChnAttr[i].stRect.u32Width = disp_width/secondRows;
            VoChnAttr[i].stRect.u32Width = VoChnAttr[i].stRect.u32Width;
            VoChnAttr[i].stRect.u32Height = disp_height/secondRows;
            VoChnAttr[i].stRect.u32Height = VoChnAttr[i].stRect.u32Height;
        }

        if (ctx->bChnPriority) {
            VoChnAttr[0].u32Priority = 15;

            VoChnAttr[1].u32Priority = 10;
            VoChnAttr[1].stRect.s32X = (disp_width/secondRows)*(1%secondRows) -  100;
            VoChnAttr[1].stRect.s32Y = (disp_height/secondRows)*(1/secondRows) + 100;
            VoChnAttr[1].stRect.u32Width = disp_width/secondRows + 200;
            VoChnAttr[1].stRect.u32Width = VoChnAttr[1].stRect.u32Width;
            VoChnAttr[1].stRect.u32Height = disp_width/secondRows + 200;
            VoChnAttr[1].stRect.u32Height = VoChnAttr[1].stRect.u32Height;

            VoChnAttr[2].u32Priority = 5;
        }

        for (i = 0; i < totalCH; i++) {
            // set attribute of vo chn
            RK_S32 s32ret = RK_MPI_VO_SetChnAttr(VoLayer_second, i, &VoChnAttr[i]);
            if (RK_SUCCESS != s32ret) {
                RK_LOGE("vo set dev %d chn %d attr failed! \n", VoLayer_second, i);
            }
            // set border
            border.bBorderEn = RK_TRUE;
            border.stBorder.u32Color = 0x191970;
            border.stBorder.u32LeftWidth =  ctx->stBorderCfg.u32LeftWidth;
            border.stBorder.u32RightWidth = ctx->stBorderCfg.u32RightWidth;
            border.stBorder.u32TopWidth =  ctx->stBorderCfg.u32TopWidth;
            border.stBorder.u32BottomWidth = ctx->stBorderCfg.u32BottomWidth;
            s32Ret = RK_MPI_VO_SetChnBorder(VoLayer_second, i, &border);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
        }
    }

    return RK_SUCCESS;
}

static RK_S32 mpi_ffmpeg_free(void *opaque) {
    AVPacket* avPkt = reinterpret_cast<AVPacket*>(opaque);
    if (RK_NULL != avPkt) {
        av_packet_unref(avPkt);
        av_packet_free(&avPkt);
    }
    avPkt = RK_NULL;
    return 0;
}

static RK_S32 open_parser(TEST_VO_CTX_S *ctx, RK_U32 u32Ch) {
    RK_S32 s32Ret = 0;
    RK_U32 u32RetryNum = 5;
    RK_BOOL bFindStream = RK_FALSE;
    AVFormatContext *pstAvfc = RK_NULL;
    const AVStream* stream = RK_NULL;

    avformat_network_init();

    pstAvfc = avformat_alloc_context();

__RETRY:
    s32Ret = avformat_open_input(&(pstAvfc), ctx->stParserCfg.srcFileUri[u32Ch], NULL, NULL);
    if (s32Ret < 0) {
        if (s32Ret == -110 && u32RetryNum >= 0) {
            RK_LOGE("AGAIN");
            u32RetryNum--;
            goto __RETRY;
        } else {
            RK_LOGE("open input %s failed!", ctx->stParserCfg.srcFileUri[u32Ch]);
            goto __FAILED;
        }
    }

    if (avformat_find_stream_info(pstAvfc, NULL) < 0) {
        goto __FAILED;
    }

    RK_LOGD("found stream num: %d", pstAvfc->nb_streams);
    for (RK_S32 i = 0; i < pstAvfc->nb_streams; i++) {
        stream = pstAvfc->streams[i];

        if (RK_NULL != stream && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ctx->stParserCfg.enCodecId = (RK_CODEC_ID_E)utils_av_to_rk_codec(stream->codecpar->codec_id);
            ctx->stParserCfg.u32SrcWidth = stream->codecpar->width;
            ctx->stParserCfg.u32SrcHeight = stream->codecpar->height;
            ctx->stParserCfg.u32StreamIndex = stream->index;
            RK_LOGD("found video stream width %d height %d",
                ctx->stParserCfg.u32SrcWidth, ctx->stParserCfg.u32SrcHeight);
            bFindStream = RK_TRUE;
            break;
        }
    }

    if (!bFindStream)
        goto __FAILED;

    RK_LOGI("open success %d", bFindStream);
    ctx->stParserCfg.pstAvfc = pstAvfc;
    return RK_SUCCESS;

__FAILED:
    if (pstAvfc)
        avformat_close_input(&(pstAvfc));

    avformat_network_deinit();
    return RK_ERR_VDEC_ILLEGAL_PARAM;
}

static RK_S32 open_config(TEST_VO_CTX_S *ctx) {
    FILE *fp = RK_NULL;
    RK_U32 u32Count = 0;
    RK_U32 u32FileLen = 0;
    RK_U32 u32Len = 0;
    RK_S32 s32Ret = 0;
    char *line = RK_NULL;
    char *pBuffer = RK_NULL;

    if ((fp = fopen(ctx->cfgFileUri, "r")) == RK_NULL) {
        RK_LOGE("Error! opening file");
        return RK_FAILURE;
    }
    fseek(fp, 0L, SEEK_END);
    u32FileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    size_t sLen = 0;
    size_t read = 0;
    while (read = getline(&line, &sLen, fp) != -1) {
        pBuffer = reinterpret_cast<char *>(malloc(strlen(line)));
        RK_S32 len = strlen(line);
        snprintf(pBuffer, strlen(line), "%s", line);
        while (len) {
            if (pBuffer[len] == '\r') {
                memcpy(&(pBuffer[len]), &(pBuffer[len + 1]), strlen(line) - len);
            }
            len--;
        }
        ctx->stParserCfg.srcFileUri[u32Count] = pBuffer;

        RK_LOGI("url %s", ctx->stParserCfg.srcFileUri[u32Count]);
        u32Count++;

        if (u32Count >= VDEC_MAX_CHN_NUM)
            break;
    }

    fclose(fp);
    return RK_SUCCESS;
}

static RK_S32 close_config(TEST_VO_CTX_S *ctx) {
    RK_S32 i;

    for (i = 0; i < VDEC_MAX_CHN_NUM; i++) {
        if (ctx->stParserCfg.srcFileUri[i] != RK_NULL) {
            rk_safe_free(ctx->stParserCfg.srcFileUri[i]);
        }
    }
    return RK_SUCCESS;
}

static RK_S32 creat_wbc(TEST_VO_CTX_S *ctx, RK_U32 u32Ch) {
    RK_S32 s32Ret = RK_SUCCESS;
    VO_WBC VoWbc = u32Ch;

    switch (ctx->u32WbcSourceType) {
        case WBC_SOURCE_DEV:
            ctx->stWbcCfg.stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
            break;
        default:
            ctx->stWbcCfg.stWbcSource.enSourceType = VO_WBC_SOURCE_VIDEO;
    }
    s32Ret = RK_MPI_VO_SetWbcSource(VoWbc, &ctx->stWbcCfg.stWbcSource);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_SetWbcAttr(VoWbc, &ctx->stWbcCfg.stWbcAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_EnableWbc(VoWbc);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return RK_SUCCESS;
}

static RK_S32 destory_wbc(RK_U32 u32Ch) {
    VO_WBC VoWbc = u32Ch;
    RK_MPI_VO_DisableWbc(VoWbc);

    return RK_SUCCESS;
}

static RK_S32 creat_vdec(TEST_VO_CTX_S *ctx, RK_U32 u32Ch) {
    RK_S32 s32Ret = RK_SUCCESS;
    VDEC_CHN_ATTR_S pstAttr;
    VDEC_CHN_PARAM_S stVdecParam;

    pstAttr.enMode = VIDEO_MODE_FRAME;
    pstAttr.enType = ctx->stParserCfg.enCodecId;
    pstAttr.u32PicWidth = ctx->stParserCfg.u32SrcWidth;
    pstAttr.u32PicHeight = ctx->stParserCfg.u32SrcHeight;
    pstAttr.u32FrameBufCnt = ctx->stVdecCfg.u32FrameBufferCnt;

    s32Ret = RK_MPI_VDEC_CreateChn(u32Ch, &pstAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed! ", u32Ch);
        RK_LOGE(R"(enMode %d enType %d u32PicWidth %d u32PicHeight %d \
            u32PicVirWidth %d u32PicVirHeight %d u32FrameBufCnt %d)", \
            pstAttr.enMode, pstAttr.enType, pstAttr.u32PicWidth, pstAttr.u32PicHeight, \
            pstAttr.u32PicVirWidth, pstAttr.u32PicVirHeight, pstAttr.u32FrameBufCnt);
        return s32Ret;
    }

    stVdecParam.stVdecVideoParam.enCompressMode = ctx->stVdecCfg.enCompressMode;
    s32Ret = RK_MPI_VDEC_SetChnParam(u32Ch, &stVdecParam);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VDEC_StartRecvStream(u32Ch);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    if (get_uri_scheme_type(ctx->stParserCfg.srcFileUri[u32Ch]) != RK_URI_SCHEME_LOCAL) {
        s32Ret = RK_MPI_VDEC_SetDisplayMode(u32Ch, VIDEO_DISPLAY_MODE_PREVIEW);
    } else {
        s32Ret = RK_MPI_VDEC_SetDisplayMode(u32Ch, VIDEO_DISPLAY_MODE_PLAYBACK);
    }
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    RK_LOGI("create %d vdec success", u32Ch);
    return RK_SUCCESS;
}

static RK_S32 screen1_process(TEST_VO_CTX_S *ctx, RK_U32 u32Index) {
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stSrcChn, stDstChn;

    s32Ret = open_parser(ctx, u32Index);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("creat %d ch parser failed!", u32Index);
        return s32Ret;
    }

    s32Ret = creat_vdec(ctx, u32Index);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("creat %d ch vdec failed!", u32Index);
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_EnableChn(ctx->stVoCfg.u32Screen1VoLayer, u32Index - ctx->u32Screen0Chn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("creat %d layer %d ch vo failed!",
            ctx->stVoCfg.u32Screen1VoLayer, u32Index - ctx->u32Screen0Chn);
        return s32Ret;
    }
    RK_LOGD("create vo layer %d ch %d", ctx->stVoCfg.u32Screen1VoLayer,  u32Index - ctx->u32Screen0Chn);

    stSrcChn.enModId = RK_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32Index;

    RK_LOGD("vdec  Ch %d", stSrcChn.s32ChnId);
    stDstChn.enModId = RK_ID_VO;
    stDstChn.s32DevId = ctx->stVoCfg.u32Screen1VoLayer;
    stDstChn.s32ChnId = u32Index - ctx->u32Screen0Chn;

    RK_LOGD("VoLayer %d, voCh %d", ctx->stVoCfg.u32Screen1VoLayer,  u32Index - ctx->u32Screen0Chn);

    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vdec and vo bind error");
        return s32Ret;
    }

    return RK_SUCCESS;
}

static RK_S32 screen0_process(TEST_VO_CTX_S *ctx, RK_U32 u32Index) {
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stSrcChn, stDstChn;

    s32Ret = open_parser(ctx, u32Index);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("creat %d ch parser failed!", u32Index);
        return s32Ret;
    }
    s32Ret = creat_vdec(ctx, u32Index);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("creat %d ch vdec failed!", u32Index);
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_EnableChn(ctx->stVoCfg.u32Screen0VoLayer, u32Index);
    if (s32Ret != RK_SUCCESS) {
         RK_LOGE("creat %d layer %d ch vo failed!", ctx->stVoCfg.u32Screen0VoLayer, u32Index);
         return s32Ret;
    }

    RK_LOGD("create vo layer %d ch %d", ctx->stVoCfg.u32Screen0VoLayer, u32Index);

    stSrcChn.enModId = RK_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32Index;
    RK_LOGD("vdec  Ch %d", stSrcChn.s32ChnId);

    stDstChn.enModId = RK_ID_VO;
    stDstChn.s32DevId = ctx->stVoCfg.u32Screen0VoLayer;
    stDstChn.s32ChnId = u32Index;
    RK_LOGD("voLayer %d, voCh %d", stDstChn.s32DevId,  stDstChn.s32ChnId);

    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vdec and vo bind error ");
        return s32Ret;
    }

    return RK_SUCCESS;
}

static RK_S32 wbc_process(TEST_VO_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stSrcChn, stDstChn;

    s32Ret = creat_wbc(ctx, 0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create wbc failed");
        return s32Ret;
    }

    stSrcChn.enModId = RK_ID_WBC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    if (ctx->bEnWbcToVO) {
        stDstChn.enModId   = RK_ID_VO;

        if (ctx->stWbcCfg.stWbcSource.u32SourceId == RK356X_VO_DEV_HD1) {
            s32Ret = RK_MPI_VO_EnableChn(ctx->stVoCfg.u32Screen0VoLayer, ctx->u32Screen0Chn);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("creat %d layer %d ch vo failed!",
                    ctx->stVoCfg.u32Screen0VoLayer, ctx->u32Screen0Chn);
                return s32Ret;
            }
            stDstChn.s32DevId  = ctx->stVoCfg.u32Screen0VoLayer;
        } else {
            s32Ret = RK_MPI_VO_EnableChn(ctx->stVoCfg.u32Screen1VoLayer, ctx->u32Screen1Chn);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("creat %d layer %d ch vo failed!",
                    ctx->stVoCfg.u32Screen1VoLayer, ctx->u32Screen1Chn);
                return s32Ret;
            }
            stDstChn.s32DevId  = ctx->stVoCfg.u32Screen1VoLayer;
        }
        stDstChn.s32ChnId  = ctx->u32Screen1Chn;

        s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("creat wbc bind vo failed!");
            return RK_FAILURE;
        }
    }

    return RK_SUCCESS;
}

void* vdec_send_stream_thread(void *pArgs) {
    prctl(PR_SET_NAME, "vdec_send");

    TEST_VO_CTX_S *ctx = reinterpret_cast<TEST_VO_CTX_S *>(pArgs);
    RK_S32 s32Ret = 0;

    RK_U8 *data = RK_NULL;
    VDEC_CHN_ATTR_S pstAttr;
    VDEC_STREAM_S pstStream;
    MB_POOL_CONFIG_S pstMbPoolCfg;
    MB_BLK buffer = RK_NULL;
    MB_EXT_CONFIG_S pstMbExtConfig;
    AVPacket *avPacket = RK_NULL;
    RK_S32 s32LoopCount = ctx->s32LoopCount;

    while (!ctx->bThreadExit) {
        avPacket = av_packet_alloc();
        av_init_packet(avPacket);
        s32Ret = av_read_frame(ctx->stParserCfg.pstAvfc, avPacket);
        if (s32Ret == AVERROR_EOF || s32Ret == AVERROR_EXIT) {
            mpi_ffmpeg_free(avPacket);
            if (s32LoopCount > 0) {
                s32LoopCount--;
                RK_LOGI("finish dec count %d\n", ctx->s32LoopCount - s32LoopCount);

                if (s32LoopCount <= 0) {
                    break;
                }
            }

            RK_LOGD("eos %d\n", ctx->u32ChnIndex);
            avformat_seek_file(ctx->stParserCfg.pstAvfc, ctx->stParserCfg.u32StreamIndex,
                VDEC_INT64_MIN, 0, VDEC_INT64_MAX, AVSEEK_FLAG_BYTE);
            continue;
        }

       if (ctx->stParserCfg.u32StreamIndex == avPacket->stream_index) {
            pstMbExtConfig.pFreeCB = mpi_ffmpeg_free;
            pstMbExtConfig.pOpaque = avPacket;
            pstMbExtConfig.pu8VirAddr = avPacket->data;
            pstMbExtConfig.u64Size = avPacket->size;

            RK_MPI_SYS_CreateMB(&buffer, &pstMbExtConfig);

            pstStream.u64PTS = avPacket->pts;
            pstStream.pMbBlk = buffer;
            pstStream.u32Len = avPacket->size;
            pstStream.bEndOfStream = RK_FALSE;
            pstStream.bEndOfFrame = RK_FALSE;
            pstStream.bBypassMbBlk = RK_TRUE;
__RETRY:
            s32Ret = RK_MPI_VDEC_SendStream(ctx->u32ChnIndex, &pstStream, MAX_TIME_OUT_MS);
            if (s32Ret < 0) {
                if (ctx->bThreadExit) {
                     mpi_ffmpeg_free(avPacket);
                     RK_MPI_MB_ReleaseMB(pstStream.pMbBlk);
                     break;
                }
                usleep(10000llu);
                goto  __RETRY;
            }

            RK_MPI_MB_ReleaseMB(pstStream.pMbBlk);
        } else {
            mpi_ffmpeg_free(avPacket);
        }
    }


      if (ctx->stParserCfg.pstAvfc)
        avformat_close_input(&(ctx->stParserCfg.pstAvfc));

__FAILED:
    return RK_SUCCESS;
}

RK_S32 unit_mpi_voplay_test(TEST_VO_CTX_S *ctx) {
    RK_U32  i, k;

    RK_S32 s32Ret = RK_SUCCESS;
    RK_U32 u32MaxCh = 0;
    RK_U32 u32step = 0;
    RK_S32 u32VpssGrap = 0;

    MPP_CHN_S stSrcChn, stDstChn;

    TEST_VO_CTX_S mpiVdecCtx[VDEC_MAX_CHN_NUM];
    TEST_VO_CTX_S mpiVencCtx[VENC_MAX_CHN_NUM];

    pthread_t vdecSendThread[VDEC_MAX_CHN_NUM];
    pthread_t vdecGetThread[VDEC_MAX_CHN_NUM];

    pthread_t voThread;
    RK_U32 u32VdecSendThreadNum = 0;
    RK_U32 u32VdecGetThreadNum = 0;
    RK_U32 u32Ch = 0;

    if (open_config(ctx) < 0) {
        RK_LOGE("parser cfg failed!");
        return RK_FAILURE;
    }

    mpi_vo_init_sample(ctx, ctx->stVoCfg.u32Screen0Rows, ctx->stVoCfg.u32Screen1Rows);

    u32MaxCh =  ctx->u32Screen1Chn + ctx->u32Screen0Chn;

    RK_LOGD("screen0Ch %d screen1Ch %d maxCh %d", ctx->u32Screen0Chn, ctx->u32Screen1Chn, u32MaxCh);

    for (i = 0; i < ctx->u32Screen0Chn; i++) {
        s32Ret = screen0_process(ctx, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("screen0_process failed!", i);
            continue;
        }
        ctx->u32ChnIndex = i;
        memcpy(&(mpiVdecCtx[i]), ctx, sizeof(TEST_VO_CTX_S));
        pthread_create(&vdecSendThread[u32VdecSendThreadNum], 0,
            vdec_send_stream_thread, reinterpret_cast<void *>(&mpiVdecCtx[i]));
        u32VdecSendThreadNum++;
    }


    for (i = ctx->u32Screen0Chn; i < u32MaxCh; i++) {
            s32Ret = screen1_process(ctx, i);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("screen1_process failed!", i);
                continue;
            }
            ctx->u32ChnIndex = i;
            memcpy(&(mpiVdecCtx[i]), ctx, sizeof(TEST_VO_CTX_S));
            pthread_create(&vdecSendThread[u32VdecSendThreadNum], 0,
                vdec_send_stream_thread, reinterpret_cast<void *>(&mpiVdecCtx[i]));
            u32VdecSendThreadNum++;
        }

    pthread_create(&voThread, 0, vochn_test_thread_func, reinterpret_cast<void *>(ctx));

    if (ctx->bEnWbc) {
        s32Ret = wbc_process(ctx);
        if (s32Ret == RK_SUCCESS) {
            RK_LOGE("wb process ok");
        } else {
            RK_LOGE("wb process failed");
        }
    }

    u32VdecSendThreadNum = 0;
    for (i = 0; i< ctx->u32Screen0Chn; i++) {
        pthread_join(vdecSendThread[u32VdecSendThreadNum], RK_NULL);
        u32VdecSendThreadNum++;

        stSrcChn.enModId = RK_ID_VDEC;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = i;

        stDstChn.enModId = RK_ID_VO;
        stDstChn.s32DevId = ctx->stVoCfg.u32Screen0VoLayer;
        stDstChn.s32ChnId = i;

        s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }

        s32Ret = RK_MPI_VDEC_StopRecvStream(i);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VDEC_DestroyChn(i);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VO_DisableChn(ctx->stVoCfg.u32Screen0VoLayer, i);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    for (i = ctx->u32Screen0Chn; i < u32MaxCh; i++) {
            pthread_join(vdecSendThread[u32VdecSendThreadNum], RK_NULL);
            u32VdecSendThreadNum++;

            stSrcChn.enModId = RK_ID_VDEC;
            stSrcChn.s32DevId = 0;
            stSrcChn.s32ChnId = i;

            stDstChn.enModId = RK_ID_VO;
            stDstChn.s32DevId = ctx->stVoCfg.u32Screen1VoLayer;
            stDstChn.s32ChnId = i;

            s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
            s32Ret = RK_MPI_VDEC_StopRecvStream(i);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
            s32Ret = RK_MPI_VDEC_DestroyChn(i);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
            s32Ret = RK_MPI_VO_DisableChn(ctx->stVoCfg.u32Screen1VoLayer, i);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
        }

    if (ctx->bEnWbc) {
        if (ctx->bEnWbcToVO) {
            stSrcChn.enModId    = RK_ID_WBC;
            stSrcChn.s32DevId   = 0;
            stSrcChn.s32ChnId   = 0;

            stDstChn.enModId   = RK_ID_VO;
            stDstChn.s32DevId  = ctx->stVoCfg.u32Screen0VoLayer;
            stDstChn.s32ChnId  = 0;

            s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
            s32Ret = destory_wbc(0);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
            s32Ret = RK_MPI_VO_DisableChn(ctx->stVoCfg.u32Screen0VoLayer, 0);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
        }
    }

    close_config(ctx);
    return RK_SUCCESS;
}

static void Sample_VO_GetPictureData(VO_FRAME_INFO_S *pstFrameInfo, const char *pFileName) {
    RK_S32 u32Fd;
    RK_U32 u32Size, u32Bpp, u32Stride;

    switch (pstFrameInfo->enPixelFormat) {
    case RK_FMT_ARGB8888:
    case RK_FMT_ABGR8888:
        u32Bpp = 4;
        break;
    case RK_FMT_RGB888:
        case RK_FMT_BGR888:
        u32Bpp = 3;
        break;
    case RK_FMT_YUV420SP:
    case RK_FMT_YUV420SP_VU:
        u32Bpp = 1;
        break;
    default:
        Sample_Print("%s Unsupported Format %d\n", __func__, pstFrameInfo->enPixelFormat);
    return;
    }

    u32Fd = open(pFileName, O_RDONLY);
    if (u32Fd < 0) {
        Sample_Print("cat not find file\n");
        return;
    }
    u32Size = lseek(u32Fd, 0, SEEK_END);
    lseek(u32Fd, 0, SEEK_SET);

    Sample_Print("%dx%d %dx%d\n",
                  pstFrameInfo->u32Width, pstFrameInfo->u32Height,
                  pstFrameInfo->u32VirWidth, pstFrameInfo->u32VirHeight);

    if (pstFrameInfo->u32Width != pstFrameInfo->u32VirWidth) {
        u32Stride = pstFrameInfo->u32VirWidth * u32Bpp;

        for (RK_S32 i = 0; i < pstFrameInfo->u32Height; i++)
            read(u32Fd, reinterpret_cast<RK_S8 *>(pstFrameInfo->pData) + u32Stride * i,
                        pstFrameInfo->u32Width * u32Bpp);

            if (pstFrameInfo->enPixelFormat == RK_FMT_YUV420SP ||
                pstFrameInfo->enPixelFormat == RK_FMT_YUV420SP_VU) {
                for (RK_S32 i = 0; i < pstFrameInfo->u32Height / 2; i++)
                    read(u32Fd, reinterpret_cast<RK_S8 *>(pstFrameInfo->pData)
                                + u32Stride * pstFrameInfo->u32VirHeight + i * u32Stride,
                                pstFrameInfo->u32Width);
                }
            } else {
                read(u32Fd, pstFrameInfo->pData, u32Size);
            }
            close(u32Fd);
}

static void filewrite(void *buf, uint32_t size, char *file) {
    int fd = open(file, O_WRONLY | O_CREAT);

    if (fd < 0) {
        printf("cat not open file\n");
        return;
    }

    write(fd, buf, size);
    close(fd);
}

static void Sample_VO_DrawUI(VO_FRAME_INFO_S *pstFrameInfo, RK_S32 format, RK_S32 value) {
    RK_S32 PixelVal0, PixelVal1;
    RK_S8 *pData = reinterpret_cast<RK_S8 *>(pstFrameInfo->pData);
    RK_U32 u32Offset;

    for (RK_S32 i = 0; i < pstFrameInfo->u32Height; i++) {
        for (RK_S32 j = 0; j < pstFrameInfo->u32Width; j++) {
            u32Offset = i * pstFrameInfo->u32VirWidth + j;
            if (format == RK_FMT_BGRA5551) {
                u32Offset = u32Offset * 2;
                PixelVal0 = value | 0x8000;
                PixelVal1 = value & 0x7FFF;
            } else if (format == RK_FMT_BGRA8888) {
                u32Offset = u32Offset * 4;
                PixelVal0 = 0x000000FF;
            } else {
                continue;
            }

            if (j < pstFrameInfo->u32Width / 2) {
                if (format == RK_FMT_BGRA5551) {
                    pData[u32Offset] = PixelVal0 & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 8) & 0xff;
                } else if (format == RK_FMT_BGRA8888) {
                    pData[u32Offset] = PixelVal0 & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 8) & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 16) & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 24) & 0xff;
                }
            } else {
                if (format == RK_FMT_BGRA5551) {
                    pData[u32Offset] = PixelVal1 & 0xff;
                    pData[u32Offset + 1] = (PixelVal1 >> 8) & 0xff;
                } else if (format == RK_FMT_BGRA8888) {
                    pData[u32Offset] = PixelVal0 & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 8) & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 16) & 0xff;
                    pData[u32Offset + 1] = (PixelVal0 >> 24) & 0xff;
               }
            }
        }
    }
}

static RK_S32 Sample_VO_MultiGFXLayer_Start(VO_LAYER VoLayer, RK_U32 u32Layers) {
    VO_VIDEO_LAYER_ATTR_S    stLayerAttr;
    VO_CHN_ATTR_S            stChnAttr;
    VO_CHN_PARAM_S           stParam;
    VO_BORDER_S              stBorder;
    RK_U32                   u32Row, u32Column, i;
    RK_U32                   u32WinWidth, u32WinHeight;
    RK_S32                   s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_GetLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        Sample_Print("[%s] Get Layer Attr failed\n", __func__);
        return RK_FAILURE;
    }

    for (i = 0; i < u32Layers; i++) {
        stChnAttr.stRect.s32X = 0;
        stChnAttr.stRect.s32Y = 0;
        stChnAttr.stRect.u32Width = stLayerAttr.stDispRect.u32Width;
        stChnAttr.stRect.u32Height = stLayerAttr.stDispRect.u32Height;
        stChnAttr.u32Priority = i;
        if (i == 0) {
            stChnAttr.u32FgAlpha = 128;
            stChnAttr.u32BgAlpha = 0;
        } else {
            stChnAttr.u32FgAlpha = 0;
            stChnAttr.u32BgAlpha = 128;
        }

        s32Ret = RK_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
        if (s32Ret != RK_SUCCESS) {
            Sample_Print("[%s] set chn Attr failed\n", __func__);
            return RK_FAILURE;
        }

        s32Ret = RK_MPI_VO_EnableChn(VoLayer, i);
        if (s32Ret != RK_SUCCESS) {
            Sample_Print("[%s] Enalbe chn failed\n", __func__);
            return RK_FAILURE;
        }
    }

    Sample_Print("[%s] success\n", __func__);
    return s32Ret;
}

static RK_S32 Sample_VO_MultiWindowns_Start(TEST_VO_CTX_S *ctx) {
    VO_VIDEO_LAYER_ATTR_S    stLayerAttr;
    VO_CHN_ATTR_S            stChnAttr;
    VO_CHN_PARAM_S           stChnParam;
    VO_BORDER_S              stBorder;
    RK_U32                   u32Row, u32Column, i;
    RK_U32                   u32WinWidth, u32WinHeight;
    RK_U32                   u32X, u32Y;
    RK_S32                   s32Ret = RK_SUCCESS;
    RK_U32 disp_width = 1920;
    RK_U32 disp_height = 1080;

    if (ctx->u32Windows <= 2)
        u32Row = 1;
    else if (ctx->u32Windows <= 4)
        u32Row = 2;
    else if (ctx->u32Windows <= 9)
        u32Row = 3;
    else if (ctx->u32Windows <= 16)
        u32Row = 4;
    else if (ctx->u32Windows <= 36)
        u32Row = 6;
    else if (ctx->u32Windows <= 64)
        u32Row = 8;
    else
        return RK_FAILURE;

    u32Column = ctx->u32Windows / u32Row + ((ctx->u32Windows % u32Row)? 1: 0);

    s32Ret = RK_MPI_VO_GetLayerAttr(ctx->VoLayer, &stLayerAttr);

    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    u32WinWidth = stLayerAttr.stDispRect.u32Width / u32Column;
    u32WinHeight = stLayerAttr.stDispRect.u32Height / u32Row;

    for (i = 0; i <ctx->u32Windows; i++) {
        u32X = i % u32Column;
        u32Y = i / u32Column;
        stChnAttr.stRect.s32X = stLayerAttr.stDispRect.s32X + u32X * u32WinWidth;
        stChnAttr.stRect.s32Y = stLayerAttr.stDispRect.s32Y + u32Y * u32WinHeight;
        stChnAttr.stRect.u32Width = u32WinWidth;
        stChnAttr.stRect.u32Height = u32WinHeight;
        // set priority
        stChnAttr.u32Priority = 1;

        s32Ret = RK_MPI_VO_SetChnAttr(ctx->VoLayer, i, &stChnAttr);
        if (s32Ret != RK_SUCCESS)
            return RK_FAILURE;

        // set video aspect ratio
        if (ctx->uEnMode == 2) {
            stChnParam.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
            stChnParam.stAspectRatio.stVideoRect.s32X = stLayerAttr.stDispRect.s32X + u32X * u32WinWidth;
            stChnParam.stAspectRatio.stVideoRect.s32Y = stLayerAttr.stDispRect.s32Y + u32Y * u32WinHeight;
            stChnParam.stAspectRatio.stVideoRect.u32Width = u32WinWidth/2;
            stChnParam.stAspectRatio.stVideoRect.u32Height = u32WinHeight/2;
            RK_MPI_VO_SetChnParam(ctx->VoLayer, i, &stChnParam);
        } else if (ctx->uEnMode == 1) {
            stChnParam.stAspectRatio.enMode = ASPECT_RATIO_AUTO;
            stChnParam.stAspectRatio.stVideoRect.s32X = stLayerAttr.stDispRect.s32X + u32X * u32WinWidth;
            stChnParam.stAspectRatio.stVideoRect.s32Y = stLayerAttr.stDispRect.s32Y + u32Y * u32WinHeight;
            stChnParam.stAspectRatio.stVideoRect.u32Width = u32WinWidth;
            stChnParam.stAspectRatio.stVideoRect.u32Height = u32WinHeight;
            RK_MPI_VO_SetChnParam(ctx->VoLayer, i, &stChnParam);
        }

        stBorder.stBorder.u32Color = 0xFFFAFA;
        stBorder.stBorder.u32TopWidth = ctx->stBorderCfg.u32TopWidth;
        stBorder.stBorder.u32BottomWidth = ctx->stBorderCfg.u32BottomWidth;
        stBorder.stBorder.u32LeftWidth = ctx->stBorderCfg.u32LeftWidth;
        stBorder.stBorder.u32RightWidth = ctx->stBorderCfg.u32RightWidth;
        stBorder.bBorderEn = RK_TRUE;
        s32Ret = RK_MPI_VO_SetChnBorder(ctx->VoLayer, i, &stBorder);
        if (s32Ret != RK_SUCCESS)
            return RK_FAILURE;

        s32Ret = RK_MPI_VO_EnableChn(ctx->VoLayer, i);
        if (s32Ret != RK_SUCCESS)
            return RK_FAILURE;
    }

    return s32Ret;
}

static RK_S32 Sample_VO_MultiWindowns_Stop(VO_LAYER VoLayer, RK_U32 u32Windows) {
    RK_U32 i;
    RK_S32 s32Ret = RK_SUCCESS;

    for (i = 0; i < u32Windows; i++) {
        s32Ret = RK_MPI_VO_DisableChn(VoLayer, i);
        if (s32Ret != RK_SUCCESS)
            return RK_FAILURE;
    }

    return s32Ret;
}

static RK_S32 Sample_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr) {
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_SetPubAttr(VoDev, pstPubAttr);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    s32Ret = RK_MPI_VO_Enable(VoDev);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

     return s32Ret;
}

static RK_S32 Sample_VO_StopDev(VO_DEV VoDev) {
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_Disable(VoDev);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    return s32Ret;
}

static RK_S32 Sample_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr) {
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, pstLayerAttr);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    return s32Ret;
}

static RK_S32 Sample_VO_StopLayer(VO_LAYER VoLayer) {
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_DisableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    return s32Ret;
}

static RK_S32 Sample_VO_BindVoWbc(VO_DEV VoWbcDev, VO_LAYER VoLayer, VO_CHN VoChn) {
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId    = RK_ID_WBC;
    stSrcChn.s32DevId   = VoWbcDev;
    stSrcChn.s32ChnId   = 0;

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32ChnId  = VoChn;
    stDestChn.s32DevId  = VoLayer;

    return RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

static RK_S32 Sample_VO_UnBindVoWbc(VO_DEV VoWbcDev, VO_LAYER VoLayer, VO_CHN VoChn) {
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId    = RK_ID_WBC;
    stSrcChn.s32DevId   = VoWbcDev;
    stSrcChn.s32ChnId   = 0;

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32ChnId  = VoChn;
    stDestChn.s32DevId  = VoLayer;

    return RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
}

static RK_S32 Sample_VO_CreateGFXData(RK_U32 u32Width, RK_U32 u32Height, RK_U32 foramt, RK_U32 value, RK_VOID **pMblk) {
    VO_FRAME_INFO_S stFrameInfo;
    RK_U32 u32BuffSize;

    u32BuffSize = RK_MPI_VO_CreateGraphicsFrameBuffer(u32Width, u32Height, foramt, pMblk);
    if (u32BuffSize == 0) {
        Sample_Print("can not create gfx buffer\n");
        return RK_FAILURE;
    }

    RK_MPI_VO_GetFrameInfo(*pMblk, &stFrameInfo);
    if (foramt == RK_FMT_YUV420SP) {
        if (u32Width == 1920 && u32Height == 1080)
            Sample_VO_GetPictureData(&stFrameInfo, "/userdata/res_1080P.yuv");
        else
            Sample_VO_GetPictureData(&stFrameInfo, "/userdata/res_480P.yuv");
    } else {
        Sample_VO_DrawUI(&stFrameInfo, foramt, value);
    }

    return RK_SUCCESS;
}

static RK_VOID * Sample_VO_SendPicture(RK_VOID *pArg) {
    TEST_VO_CTX_S *pCtx = reinterpret_cast<TEST_VO_CTX_S *>(pArg);
    VIDEO_FRAME_INFO_S *pstVFrame;
    RK_VOID *pMblk, *pMblk_1080P, *pMblk_576P, *pMblk_G0, *pMblk_G1;
    RK_U32 count;

    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_YUV420SP, 0, &pMblk_1080P);
    Sample_VO_CreateGFXData(720, 480, RK_FMT_YUV420SP, 0, &pMblk_576P);
    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_BGRA5551, 0x1F, &pMblk_G0);
    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_BGRA5551, 0x1F << 10, &pMblk_G1);

    pstVFrame =  reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));

    while (!pCtx->threadExit) {
        if (pCtx->VoVideoLayer >= 0) {
            for (RK_U32 i = 0; i < pCtx->u32VideoWindows; i++) {
                if (pCtx->u32VideoWindows > 10)
                     pMblk = pMblk_576P;
                else if (count % 2)
                    pMblk = pMblk_1080P;
                else
                    pMblk = pMblk_576P;

                pstVFrame->stVFrame.pMbBlk = RK_MPI_VO_CloneBuffer(pMblk);
                RK_MPI_VO_SendFrame(pCtx->VoVideoLayer, i, pstVFrame, 0);
                }
        }
        if (pCtx->VOGfxLayer >= 0) {
            for (RK_U32 i = 0; i < pCtx->u32GFXLayers; i++) {
                if (i == 0)
                    pMblk = pMblk_G0;
                else if (i == 1)
                    pMblk = pMblk_G1;
                else
                    continue;
                pstVFrame->stVFrame.pMbBlk = RK_MPI_VO_CloneBuffer(pMblk);
                RK_MPI_VO_SendFrame(pCtx->VOGfxLayer, i, pstVFrame, 0);
            }
        }
        usleep(1000000);
        count++;
    }
    free(pstVFrame);
    RK_MPI_VO_DestroyGraphicsFrameBuffer(pMblk);
    Sample_Print("%s exit\n", __func__);
    return NULL;
}

static RK_VOID * Sample_VO_SendPicture_Mosaic(RK_VOID *pArg) {
    VoThreadParam *pParam = reinterpret_cast<VoThreadParam *>(pArg);
    VIDEO_FRAME_INFO_S *pstVFrame;
    RK_VOID *pMblk, *pMblk_1080P, *pMblk_576P, *pMblk_G0, *pMblk_G1;
    RK_U32 count;

    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_YUV420SP, 0, &pMblk_1080P);
    Sample_VO_CreateGFXData(720, 480, RK_FMT_YUV420SP, 0, &pMblk_576P);
    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_BGRA5551, 0x1F, &pMblk_G0);
    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_BGRA5551, 0x1F << 10, &pMblk_G1);

    pstVFrame = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));

    while (!pParam->u32Exit) {
        if (pParam->VoVideoLayer >= 0) {
            for (RK_U32 i = 0; i < pParam->u32VideoWindows; i++) {
                if (pParam->u32VideoWindows > 10)
                    pMblk = pMblk_576P;
                else if (count % 2)
                    pMblk = pMblk_1080P;
                else
                    pMblk = pMblk_576P;

                pstVFrame->stVFrame.pMbBlk = RK_MPI_VO_CloneBuffer(pMblk);
                RK_MPI_VO_SendFrame(pParam->VoVideoLayer, i, pstVFrame, 0);
            }
        }
        if (pParam->VOGfxLayer >= 0) {
            for (RK_U32 i = 0; i < pParam->u32GFXLayers; i++) {
                if (i == 0)
                    pMblk = pMblk_G0;
                else if (i == 1)
                    pMblk = pMblk_G1;
                else
                   continue;
                pstVFrame->stVFrame.pMbBlk = RK_MPI_VO_CloneBuffer(pMblk);
                RK_MPI_VO_SendFrame(pParam->VOGfxLayer, i, pstVFrame, 0);
            }
        }
        usleep(1000000);
        count++;
    }
    free(pstVFrame);
    RK_MPI_VO_DestroyGraphicsFrameBuffer(pMblk);
    Sample_Print("%s exit\n", __func__);

    return NULL;
}

static RK_VOID Sample_VO_StartSendPicture_Mosaic(pthread_t *pThreadID, VoThreadParam *pstParam) {
    pthread_create(pThreadID, NULL, Sample_VO_SendPicture_Mosaic, reinterpret_cast<RK_VOID *>(pstParam));
}

static RK_VOID Sample_VO_StopSendPicture_Mosaic(pthread_t tThreadID, VoThreadParam *pstParam) {
    pstParam->u32Exit = 1;
    pthread_join(tThreadID, RK_NULL);
}

static RK_VOID Sample_VO_StartSendPicture(pthread_t *pThreadID, TEST_VO_CTX_S *pstParam) {
    pthread_create(pThreadID, NULL, Sample_VO_SendPicture, reinterpret_cast<RK_VOID *>(pstParam));
}

static RK_VOID Sample_VO_StopSendPicture(pthread_t tThreadID, TEST_VO_CTX_S *pstParam) {
    pstParam->threadExit = RK_TRUE;;
    pthread_join(tThreadID, RK_NULL);
}

static RK_S32 Sample_VO_Demo_Video_Mosaic(TEST_VO_CTX_S *ctx) {
    VO_PUB_ATTR_S            stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S    stLayerAttr;
    RK_U32                   u32DispWidth, u32DispHeight;
    VO_LAYER                 VoLayer;
    VO_DEV                   VoDev;
    RK_S32                   u32Windows;
    RK_U32                   u32Fps, i;
    pthread_t                tThreadID;
    RK_S32                   s32Ret = RK_SUCCESS;
    VO_LAYER_MODE_E          Vo_layer_mode;
    VoThreadParam            stThreadParam;
    PIXEL_FORMAT_E           video_format;

    VoDev = ctx->VoDev;
    u32Windows = ctx->u32Windows;
    u32Fps =  ctx->u32DispFrmRt;
    VoLayer = ctx->VoLayer;
    video_format = Sample_vo_test_fmt_to_rtfmt(ctx->s32PixFormat);

    Sample_Print("%s VoDev %u Windows %u Fps %u\n", __func__, VoDev, u32Windows, u32Fps);

    /* Bind Layer */
    switch (ctx->VoLayerMode) {
        case 0:
            Vo_layer_mode = VO_LAYER_MODE_CURSOR;
            break;
        case 1:
            Vo_layer_mode = VO_LAYER_MODE_GRAPHIC;
            break;
        case 2:
            Vo_layer_mode = VO_LAYER_MODE_VIDEO;
            break;
        default:
            Vo_layer_mode = VO_LAYER_MODE_VIDEO;
    }

    RK_MPI_VO_BindLayer(VoLayer, VoDev, Vo_layer_mode);

    switch (ctx->enIntfType) {
        case DISPLAY_TYPE_VGA:
            stVoPubAttr.enIntfType = VO_INTF_VGA;
            break;
        case DISPLAY_TYPE_HDMI:
            stVoPubAttr.enIntfType = VO_INTF_HDMI;
            break;
        case DISPLAY_TYPE_EDP:
            stVoPubAttr.enIntfType = VO_INTF_EDP;
            break;
        default:
            stVoPubAttr.enIntfType = VO_INTF_HDMI;
            Sample_Print("option not set ,use HDMI default\n");
    }

    for (i = 0; i < ARRAY_LENGTH(test_mode_table); i++) {
        if (ctx->enIntfSync == test_mode_table[i].mode)
            break;
    }

    if (i == ARRAY_LENGTH(test_mode_table)) {
        Sample_Print("%s not found supported mode in test mode_table\n", __func__);
        return RK_FAILURE;
    }

    stVoPubAttr.enIntfSync = test_mode_table[i].enIntfSync;
    Sample_VO_StartDev(VoDev, &stVoPubAttr);

     /* Enable Layer */
    stLayerAttr.enPixFormat                  = video_format;
    stLayerAttr.stDispRect.s32X              = ctx->s32Y;
    stLayerAttr.stDispRect.s32Y              = ctx->s32Y;
    stLayerAttr.stDispRect.u32Width          = ctx->u32DispWidth;
    stLayerAttr.stDispRect.u32Height         = ctx->u32DispHeight;
    stLayerAttr.stImageSize.u32Width         = ctx->u32ImgeWidth;
    stLayerAttr.stImageSize.u32Height        = ctx->u32ImageHeight;
    stLayerAttr.u32DispFrmRt                 = ctx->u32DispFrmRt;
    Sample_VO_StartLayer(VoLayer, &stLayerAttr);
    Sample_VO_MultiWindowns_Start(ctx);
    stThreadParam.u32Exit = 0;
    stThreadParam.u32VideoWindows = u32Windows;
    stThreadParam.VoVideoLayer = VoLayer;
    stThreadParam.VOGfxLayer = -1;
    stThreadParam.u32GFXLayers = -1;

    Sample_Print("Start Send Picture\n");
    Sample_VO_StartSendPicture_Mosaic(&tThreadID, &stThreadParam);

    while (1) {
        Sample_Print("Press 'q' to quit\n");
        if (getchar() == 'q')
            break;
    }

    Sample_VO_StopSendPicture_Mosaic(tThreadID, &stThreadParam);
    Sample_VO_MultiWindowns_Stop(VoLayer, u32Windows);
    Sample_VO_StopLayer(VoLayer);
    Sample_VO_StopDev(VoDev);
 end:
    RK_MPI_VO_UnBindLayer(VoLayer, VoDev);
    RK_MPI_VO_CloseFd();

    return s32Ret;
}

static RK_S32 Sample_VO_Demo_UI(TEST_VO_CTX_S *ctx) {
    VO_PUB_ATTR_S            stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S    stLayerAttr;
    VIDEO_FRAME_INFO_S       stVFrame;
    RK_U32 u32DispWidth, u32DispHeight;
    RK_U32 u32ImgWidth, u32ImgHeight;
    VO_LAYER VoLayer;
    RK_VOID *pMblk = RK_NULL;
    RK_VOID *pMblk_480P = RK_NULL;
    RK_U32 s32Ret = RK_SUCCESS;
    RK_U32 count;

    Sample_VO_CreateGFXData(ctx->u32ImgeWidth, ctx->u32ImageHeight, RK_FMT_YUV420SP, 0, &pMblk);
    Sample_VO_CreateGFXData(720, 480, RK_FMT_YUV420SP, 0, &pMblk_480P);
    /* Bind Layer */
    if (ctx->VoDev== RK356X_VO_DEV_HD0)
        VoLayer = RK356X_VOP_LAYER_ESMART_0;
    else
        VoLayer = RK356X_VOP_LAYER_ESMART_1;

    RK_MPI_VO_BindLayer(VoLayer, ctx->VoDev, VO_LAYER_MODE_GRAPHIC);

    /* Enable VO Device */
    if (ctx->VoDev == RK356X_VO_DEV_HD0) {
        stVoPubAttr.enIntfType = VO_INTF_HDMI;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    } else if (ctx->VoDev == RK356X_VO_DEV_HD1) {
        stVoPubAttr.enIntfType = VO_INTF_EDP;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;
    } else {
        s32Ret = RK_FAILURE;
        goto end;
    }
    Sample_VO_StartDev(ctx->VoDev, &stVoPubAttr);

    /* Enable Layer */
    stLayerAttr.enPixFormat                      = RK_FMT_YUV420SP;
    if ((ctx->s32X == 0) && (ctx->s32Y == 0)) {
        stLayerAttr.stDispRect.s32X              = 0;
        stLayerAttr.stDispRect.s32Y              = 0;
        stLayerAttr.stDispRect.u32Width          = ctx->u32DispWidth;
        stLayerAttr.stDispRect.u32Height         = ctx->u32DispHeight;
        stLayerAttr.stImageSize.u32Width         = ctx->u32ImgeWidth;
        stLayerAttr.stImageSize.u32Height        = ctx->u32ImageHeight;
    } else {
        stLayerAttr.stDispRect.s32X              = (ctx->u32DispWidth/2)*(1%2) - 200;
        stLayerAttr.stDispRect.s32Y              = (ctx->u32DispHeight/2)*(1/2) + 200;
        stLayerAttr.stDispRect.u32Width          = ctx->u32DispWidth/2;
        stLayerAttr.stDispRect.u32Height         = ctx->u32DispHeight/2;
        stLayerAttr.stImageSize.u32Width         = ctx->u32ImgeWidth;
        stLayerAttr.stImageSize.u32Height        = ctx->u32ImageHeight;
    }
    Sample_VO_StartLayer(VoLayer, &stLayerAttr);
    /* Set Layer */
    while (1) {
        sleep(1);
        count++;
        if (count % 2) {
            stVFrame.stVFrame.pMbBlk = pMblk;
            RK_MPI_VO_SendLayerFrame(VoLayer, &stVFrame);
        } else {
            stVFrame.stVFrame.pMbBlk = pMblk_480P;
            RK_MPI_VO_SendLayerFrame(VoLayer, &stVFrame);
        }
    }

    Sample_Print("press 'q' to quit\n");
    while (getchar() != 'q') {
        sleep(1);
    }

    Sample_VO_StopLayer(VoLayer);
    Sample_VO_StopDev(ctx->VoDev);
end:
    RK_MPI_VO_UnBindLayer(VoLayer, ctx->VoDev);
    RK_MPI_VO_DestroyGraphicsFrameBuffer(pMblk);

    RK_MPI_VO_CloseFd();

    return s32Ret;
}

static RK_S32 Sample_VO_Demo_Alpha(TEST_VO_CTX_S *ctx) {
    VO_PUB_ATTR_S                stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S        stLayerAttr;
    VoThreadParam                stThreadParam;
    RK_U32 u32DispWidth, u32DispHeight, u32GfxLayers;
    VO_LAYER VoGfxLayer, VoVideoLayer;
    RK_S32 s32Ret = RK_SUCCESS;
    VO_DEV VoDev;
    RK_U32 u32VideoWindows;
    pthread_t tThreadID_HD0;
    pthread_t tThreadID;

    VoDev = ctx->VoDev;
    u32VideoWindows = ctx->u32Windows;
    /* Bind Layer */
    if (VoDev == RK356X_VO_DEV_HD0) {
        VoGfxLayer = RK356X_VOP_LAYER_ESMART_0;
        VoVideoLayer = RK356X_VOP_LAYER_CLUSTER_0;
    } else {
        return RK_FAILURE;
    }
    RK_MPI_VO_BindLayer(VoGfxLayer, VoDev, VO_LAYER_MODE_GRAPHIC);
    RK_MPI_VO_BindLayer(VoVideoLayer, VoDev, VO_LAYER_MODE_VIDEO);

    /* Enable VO Device */
    if (VoDev == RK356X_VO_DEV_HD0) {
        u32DispWidth = 1920;
        u32DispHeight = 1080;
        stVoPubAttr.enIntfType = VO_INTF_HDMI;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    } else {
         s32Ret = RK_FAILURE;
        goto end;
    }
    Sample_VO_StartDev(VoDev, &stVoPubAttr);

    /* Enable Video Layer */
    if (VoVideoLayer != -1) {
        stLayerAttr.enPixFormat              = RK_FMT_BGR888;
        stLayerAttr.stDispRect.s32X          = 0;
        stLayerAttr.stDispRect.s32Y          = 0;
        stLayerAttr.stDispRect.u32Width      = u32DispWidth;
        stLayerAttr.stDispRect.u32Height     = u32DispHeight;
        stLayerAttr.stImageSize.u32Width     = u32DispWidth;
        stLayerAttr.stImageSize.u32Height    = u32DispHeight;
        stLayerAttr.u32DispFrmRt             = 25;
        Sample_VO_StartLayer(VoVideoLayer, &stLayerAttr);
        ctx->VoLayer = VoVideoLayer;
        Sample_VO_MultiWindowns_Start(ctx);
    }

    /* Enable GFX Layer */
    u32GfxLayers = 2;
    if (VoGfxLayer != -1) {
        stLayerAttr.enPixFormat                  = RK_FMT_BGRA8888;
        stLayerAttr.stDispRect.s32X              = 0;
        stLayerAttr.stDispRect.s32Y              = 0;
        stLayerAttr.stDispRect.u32Width          = u32DispWidth;
        stLayerAttr.stDispRect.u32Height         = u32DispHeight;
        stLayerAttr.stImageSize.u32Width         = u32DispWidth;
        stLayerAttr.stImageSize.u32Height        = u32DispHeight;
        stLayerAttr.u32DispFrmRt                 = 25;
        Sample_VO_StartLayer(VoGfxLayer, &stLayerAttr);
        Sample_VO_MultiGFXLayer_Start(VoGfxLayer, u32GfxLayers);
    }

    Sample_Print("HD0: Start Send Picture\n");
    ctx->threadExit = RK_FALSE;
    ctx->u32VideoWindows = u32VideoWindows;
    ctx->VoVideoLayer = VoVideoLayer;
    ctx->u32GFXLayers = u32GfxLayers;
    ctx->VOGfxLayer = VoGfxLayer;
    Sample_VO_StartSendPicture(&tThreadID_HD0, ctx);

    Sample_Print("Start HD1\n");
    VoDev = RK356X_VO_DEV_HD1;
    VoGfxLayer = RK356X_VOP_LAYER_ESMART_1;
    VoVideoLayer = RK356X_VOP_LAYER_CLUSTER_1;
    RK_MPI_VO_BindLayer(VoGfxLayer, VoDev, VO_LAYER_MODE_GRAPHIC);
    RK_MPI_VO_BindLayer(VoVideoLayer, VoDev, VO_LAYER_MODE_VIDEO);

    u32DispWidth = 1024;
    u32DispHeight = 768;
    stVoPubAttr.enIntfType = VO_INTF_EDP;
    stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;

    Sample_VO_StartDev(VoDev, &stVoPubAttr);

    /* Enable Video Layer */
    if (VoVideoLayer != -1) {
        stLayerAttr.enPixFormat                  = RK_FMT_BGR888;
        stLayerAttr.stDispRect.s32X              = 0;
        stLayerAttr.stDispRect.s32Y              = 0;
        stLayerAttr.stDispRect.u32Width          = u32DispWidth;
        stLayerAttr.stDispRect.u32Height         = u32DispHeight;
        stLayerAttr.stImageSize.u32Width         = u32DispWidth;
        stLayerAttr.stImageSize.u32Height        = u32DispHeight;
        stLayerAttr.u32DispFrmRt                 = 25;
        Sample_VO_StartLayer(VoVideoLayer, &stLayerAttr);
        ctx->VoLayer = VoVideoLayer;
        Sample_VO_MultiWindowns_Start(ctx);
    }

    /* Enable GFX Layer */
    u32GfxLayers = 2;
    if (VoGfxLayer != -1) {
        stLayerAttr.enPixFormat                  = RK_FMT_BGRA8888;
        stLayerAttr.stDispRect.s32X              = 0;
        stLayerAttr.stDispRect.s32Y              = 0;
        stLayerAttr.stDispRect.u32Width          = u32DispWidth;
        stLayerAttr.stDispRect.u32Height         = u32DispHeight;
        stLayerAttr.stImageSize.u32Width         = u32DispWidth;
        stLayerAttr.stImageSize.u32Height        = u32DispHeight;
        stLayerAttr.u32DispFrmRt                 = 25;
        Sample_VO_StartLayer(VoGfxLayer, &stLayerAttr);
        Sample_VO_MultiGFXLayer_Start(VoGfxLayer, u32GfxLayers);
    }

    Sample_Print("HD1: Start Send Picture\n");
    stThreadParam.u32Exit = 0;
    stThreadParam.u32VideoWindows = u32VideoWindows;
    stThreadParam.VoVideoLayer = VoVideoLayer;
    stThreadParam.u32GFXLayers = u32GfxLayers;
    stThreadParam.VOGfxLayer = VoGfxLayer;
    Sample_VO_StartSendPicture_Mosaic(&tThreadID, &stThreadParam);

    while (getchar() != 'q') {
        sleep(1);
    }

    Sample_VO_StopSendPicture(tThreadID_HD0, ctx);
    Sample_VO_StopSendPicture_Mosaic(tThreadID, &stThreadParam);
    if (VoGfxLayer != -1) {
        Sample_VO_MultiWindowns_Stop(VoGfxLayer, u32GfxLayers);
        Sample_VO_StopLayer(VoGfxLayer);
        VoGfxLayer = RK356X_VOP_LAYER_ESMART_0;
        VoVideoLayer = RK356X_VOP_LAYER_CLUSTER_0;
        Sample_VO_MultiWindowns_Stop(VoGfxLayer, u32GfxLayers);
        Sample_VO_StopLayer(VoGfxLayer);
    }
    if (VoVideoLayer != -1) {
        Sample_VO_MultiWindowns_Stop(VoVideoLayer, u32VideoWindows);
        Sample_VO_StopLayer(VoVideoLayer);
        VoGfxLayer = RK356X_VOP_LAYER_ESMART_0;
        VoVideoLayer = RK356X_VOP_LAYER_CLUSTER_0;
        Sample_VO_MultiWindowns_Stop(VoVideoLayer, u32VideoWindows);
        Sample_VO_StopLayer(VoVideoLayer);
    }

    Sample_VO_StopDev(RK356X_VO_DEV_HD1);
    Sample_VO_StopDev(RK356X_VO_DEV_HD0);

end:
    RK_MPI_VO_UnBindLayer(VoGfxLayer, RK356X_VO_DEV_HD0);
    RK_MPI_VO_UnBindLayer(VoVideoLayer, RK356X_VO_DEV_HD0);

    VoGfxLayer = RK356X_VOP_LAYER_ESMART_1;
    VoVideoLayer = RK356X_VOP_LAYER_CLUSTER_1;
    RK_MPI_VO_UnBindLayer(VoGfxLayer, RK356X_VO_DEV_HD1);
    RK_MPI_VO_UnBindLayer(VoVideoLayer, RK356X_VO_DEV_HD1);

    RK_MPI_VO_CloseFd();

    return s32Ret;
}

static RK_S32 Sample_VO_Video_Homologous(TEST_VO_CTX_S *ctx) {
    VO_PUB_ATTR_S            stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S    stLayerAttr;
    VIDEO_FRAME_INFO_S       *pFrame;
    VO_WBC_SOURCE_S          stWbcSource;
    VO_WBC_ATTR_S            stWbcAttr;
    VoThreadParam            stThreadParam;
    RK_U32 u32DispWidth, u32DispHeight;
    RK_U32 VoDev, VoLayer, VoWbc;
    RK_U32 u32ImgWidth, u32ImgHeight;
    RK_U32 u32Windows;
    pthread_t tThreadID;
    RK_VOID *pMblk = RK_NULL;
    RK_U32 s32Ret = RK_SUCCESS;
    PIXEL_FORMAT_E  wbc_format;
    TEST_VO_CTX_S   stVoCtx;
    u32ImgWidth = 1920;
    u32ImgHeight = 1080;
    Sample_VO_CreateGFXData(u32ImgWidth, u32ImgHeight, RK_FMT_YUV420SP, 0, &pMblk);

    Sample_Print("Start HD0\n");
    /* Start HD0 Device */
    VoDev = RK356X_VO_DEV_HD0;
    /* Bind Layer */
    VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
    RK_MPI_VO_BindLayer(VoLayer, VoDev, VO_LAYER_MODE_VIDEO);

    /* Enable VO Device */
    if (VoDev == RK356X_VO_DEV_HD0) {
        u32DispWidth = 1920;
        u32DispHeight = 1080;
        stVoPubAttr.enIntfType = VO_INTF_HDMI;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P50;
    } else if (VoDev == RK356X_VO_DEV_HD1) {
        u32DispWidth = 1024;
        u32DispHeight = 768;
        stVoPubAttr.enIntfType = VO_INTF_EDP;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;
    } else {
        s32Ret = RK_FAILURE;
    }
    Sample_VO_StartDev(VoDev, &stVoPubAttr);

    /* Enable Layer */
    stLayerAttr.enPixFormat              = RK_FMT_BGR888;
    stLayerAttr.stDispRect.s32X          = 0;
    stLayerAttr.stDispRect.s32Y          = 0;
    stLayerAttr.stDispRect.u32Width      = u32DispWidth;
    stLayerAttr.stDispRect.u32Height     = u32DispHeight;
    stLayerAttr.stImageSize.u32Width     = u32DispWidth;
    stLayerAttr.stImageSize.u32Height    = u32DispHeight;
    stLayerAttr.u32DispFrmRt             = 25;
    Sample_VO_StartLayer(VoLayer, &stLayerAttr);

    u32Windows = ctx->u32Windows;
    Sample_VO_MultiWindowns_Start(ctx);
    memcpy(&(stVoCtx), ctx, sizeof(TEST_VO_CTX_S));
    wbc_format = Sample_vo_test_fmt_to_rtfmt(stVoCtx.u32WbcPixFormat);

    Sample_Print("Start Send Picture\n");
    ctx->threadExit = RK_FALSE;
    ctx->u32VideoWindows = u32Windows;
    ctx->VoVideoLayer = VoLayer;
    ctx->u32GFXLayers = 0;
    ctx->VOGfxLayer = -1;
    Sample_VO_StartSendPicture(&tThreadID, ctx);

    Sample_Print("Start HD1\n");
    /* Start HD1 Device */
    VoDev = RK356X_VO_DEV_HD1;
    /* Bind Layer */
    VoLayer = RK356X_VOP_LAYER_CLUSTER_1;
    RK_MPI_VO_BindLayer(VoLayer, VoDev, VO_LAYER_MODE_VIDEO);

    /* Enable VO Device */
    if (VoDev == RK356X_VO_DEV_HD0) {
        u32DispWidth = 1920;
        u32DispHeight = 1080;
        stVoPubAttr.enIntfType = VO_INTF_HDMI;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P50;
    } else if (VoDev == RK356X_VO_DEV_HD1) {
        u32DispWidth = 1024;
        u32DispHeight = 768;
        stVoPubAttr.enIntfType = VO_INTF_EDP;
        stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;
    } else {
        s32Ret = RK_FAILURE;
       // goto end;
    }
    Sample_VO_StartDev(VoDev, &stVoPubAttr);

    /* Enable Layer */
    stLayerAttr.enPixFormat              = RK_FMT_BGR888;
    stLayerAttr.stDispRect.s32X          = 0;
    stLayerAttr.stDispRect.s32Y          = 0;
    stLayerAttr.stDispRect.u32Width      = u32DispWidth;
    stLayerAttr.stDispRect.u32Height     = u32DispHeight;
    stLayerAttr.stImageSize.u32Width     = u32DispWidth;
    stLayerAttr.stImageSize.u32Height    = u32DispHeight;
    stLayerAttr.u32DispFrmRt             = 25;
    Sample_VO_StartLayer(VoLayer, &stLayerAttr);

    ctx->VoLayer = RK356X_VOP_LAYER_CLUSTER_1;
    ctx->u32Windows = 1;
    Sample_VO_MultiWindowns_Start(ctx);

    Sample_Print("Start WBC\n");

    /* Start WBC and bind to HD0 */
    VoWbc = 0;
    /* WBC bind source */
    switch (stVoCtx.u32WbcSourceType) {
        case WBC_SOURCE_DEV:
            stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
            break;
        case WBC_SOURCE_VIDEO:
            stWbcSource.enSourceType = VO_WBC_SOURCE_VIDEO;
            break;
        default:
            stWbcSource.enSourceType = VO_WBC_SOURCE_VIDEO;
    }
    stWbcSource.u32SourceId = stVoCtx.u32WbcSourceId;
    RK_MPI_VO_SetWbcSource(VoWbc, &stWbcSource);

    /* Start WBC */
    Sample_Print("wbc format is %d\n", stWbcAttr.enPixelFormat);
    stWbcAttr.enPixelFormat = wbc_format;
    stWbcAttr.stTargetSize.u32Width = stVoCtx.u32WbcWidth;
    stWbcAttr.stTargetSize.u32Height = stVoCtx.u32WbcHeight;
    stWbcAttr.enCompressMode = COMPRESS_MODE_NONE;
    stWbcAttr.u32FrameRate = 25;
    RK_MPI_VO_SetWbcAttr(VoWbc, &stWbcAttr);
    RK_MPI_VO_EnableWbc(VoWbc);

    Sample_Print("--------------\nWBC Enabled\n---------------\n");

    pFrame = reinterpret_cast<VIDEO_FRAME_INFO_S*>(malloc(sizeof(VIDEO_FRAME_INFO_S)));

    if (ctx->wbc_auto == RK_TRUE) {
        /* Bind WBC to HD1 */
        Sample_VO_BindVoWbc(VoWbc, VoLayer, 0);
        while (1) {
            Sample_Print("Press 'q' to quit\n");
            if (getchar() == 'q')
                break;
        }
        Sample_VO_StopSendPicture(tThreadID, ctx);
        Sample_Print("Stop WBC\n");
        Sample_VO_UnBindVoWbc(VoWbc, VoLayer, 0);
        RK_MPI_VO_DisableWbc(VoWbc);
        Sample_Print("Stop HD1\n");
        Sample_VO_MultiWindowns_Stop(VoLayer, 1);
        Sample_VO_StopLayer(VoLayer);
        Sample_VO_StopDev(VoDev);
        RK_MPI_VO_UnBindLayer(VoLayer, VoDev);
        Sample_Print("Stop HD0\n");
        VoDev = RK356X_VO_DEV_HD0;
        VoLayer = RK356X_VOP_LAYER_CLUSTER_0;

        Sample_VO_MultiWindowns_Stop(VoLayer, u32Windows);
        Sample_VO_StopLayer(VoLayer);
        Sample_VO_StopDev(VoDev);
        RK_MPI_VO_DestroyGraphicsFrameBuffer(pMblk);
        RK_MPI_VO_CloseFd();
    } else {
        while (1) {
            Sample_Print("wbc manul test \n");
            RK_MPI_VO_GetWbcFrame(VoWbc, pFrame, 0);
            RK_MPI_VO_SendLayerFrame(RK356X_VOP_LAYER_CLUSTER_1, pFrame);
            usleep(1000llu * 1000);
            RK_MPI_VO_ReleaseWbcFrame(VoWbc, pFrame);
        }
    }

    return s32Ret;
}

static RK_S32 check_options(const TEST_VO_CTX_S *ctx) {
    if (ctx->VoDev > VO_MAX_DEV_NUM) {
        RK_LOGE("illegal param, pls enter 0 or 1 max Vodev is %d!", VO_MAX_DEV_NUM);
        goto __FAILED;
    }
    if ((ctx->VoLayerMode == 2) && (ctx->VoLayer > VO_MAX_LAYER_NUM)) {
        RK_LOGE("illegal param, pls enter 0 or 2 max VoLyer is %d!", VO_MAX_LAYER_NUM);
        goto __FAILED;
    }
    if ((ctx->VoLayerMode == 1) && (ctx->VoLayer > VO_MAX_LAYER_NUM)) {
        RK_LOGE("illegal param, pls enter 4 or 5 max VoLyer is %d!", VO_MAX_LAYER_NUM);
        goto __FAILED;
    }
    if (ctx->u32Windows <= 0 || ctx->u32Windows >= MAX_WINDOWS_NUM) {
        RK_LOGE("illegal param, windos rang [1,63] max u32Windows is %d!", MAX_WINDOWS_NUM);
        goto __FAILED;
    }
    if (ctx->u32WbcPixFormat >= MAX_VO_FORMAT_RGB_NUM) {
        RK_LOGE("illegal param, WBC PixFormat rang [0,3]");
        goto __FAILED;
    }
    if (ctx->s32PixFormat > MAX_VO_FORMAT_RGB_NUM) {
        RK_LOGE("illegal param, Vidoe PixFormat rang [0,4]");
        goto __FAILED;
    }

    return RK_SUCCESS;

__FAILED:
    return RK_FAILURE;
}

static const char *const usages[] = {
    "./rk_mpi_vo_test [-i SRC_PATH] [--imag_width IMG_WIDTH]"
    "[--imag_height IMG_HEIGHT] [--dis_width DIS_WIDTH] [--dis_height DIS_HEIGHT]...",
    NULL,
};

static void  mpi_vo_test_show_options(const TEST_VO_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("vop device id                   : %d\n", ctx->VoDev);
    RK_PRINT("vop layer id                    : %d\n", ctx->VoLayer);
    RK_PRINT("window size                     : %d\n", ctx->u32Windows);
    RK_PRINT("connector type                  : %d\n", ctx->enIntfType);
    RK_PRINT("display mode                    : %d\n", ctx->enIntfSync);
    RK_PRINT("layer mode                      : %d\n", ctx->VoLayerMode);
    RK_PRINT("display resolution rect X       : %d\n", ctx->s32X);
    RK_PRINT("display resolution rect Y       : %d\n", ctx->s32Y);
    RK_PRINT("display pixel format            : %d\n", ctx->s32PixFormat);
    RK_PRINT("display resolution width        : %d\n", ctx->u32DispWidth);
    RK_PRINT("display resolution height       : %d\n", ctx->u32DispHeight);
    RK_PRINT("display rate                    : %d\n", ctx->u32DispFrmRt);
    RK_PRINT("display rate ratio              : %d\n", ctx->u32DispFrmRtRatio);
    RK_PRINT("chn display mode                : %d\n", ctx->u32ChnDismode);
    RK_PRINT("chn Border lpx                  : %d\n", ctx->stBorderCfg.u32LeftWidth);
    RK_PRINT("chn Border rpx                  : %d\n", ctx->stBorderCfg.u32RightWidth);
    RK_PRINT("chn Border tpx                  : %d\n", ctx->stBorderCfg.u32TopWidth);
    RK_PRINT("chn Border bpx                  : %d\n", ctx->stBorderCfg.u32BottomWidth);
    RK_PRINT("video aspect ratio mode         : %d\n", ctx->uEnMode);
    RK_PRINT("input ImgWidth                  : %d\n", ctx->u32ImgeWidth);
    RK_PRINT("input ImgHeight                 : %d\n", ctx->u32ImageHeight);
    RK_PRINT("ui                              : %d\n", ctx->ui);
    RK_PRINT("ui alpha                        : %d\n", ctx->ui_alpha);
    RK_PRINT("wbc enable                      : %d\n", ctx->wbc_enable);
    RK_PRINT("wbc width                       : %d\n", ctx->u32WbcWidth);
    RK_PRINT("wbc height                      : %d\n", ctx->u32WbcHeight);
    RK_PRINT("wbc compress mode               : %d\n", ctx->u32WbcCompressMode);
    RK_PRINT("wbc pixel format                : %d\n", ctx->u32WbcPixFormat);
    RK_PRINT("wbc source type                 : %d\n", ctx->u32WbcSourceType);
    RK_PRINT("wbc souce id                    : %d\n", ctx->u32WbcSourceId);
}

void init_cfg(TEST_VO_CTX_S *ctx) {
    RK_S32 i = 0;

    ctx->u32Windows = 4;
    ctx->enIntfType = 0;  /*0: HDMI 1: EDP 2: VGA*/
    ctx->enIntfSync = 18; /*1080P60*/

    ctx->VoDev = RK356X_VO_DEV_HD0;
    ctx->VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
    ctx->VoLayerMode = 1; /* CURSOR = 0,GRAPHIC = 1,VIDEO = 2,*/

    ctx->u32ImgeWidth = 1920;
    ctx->u32ImageHeight = 1080;

    ctx->s32X = 0;
    ctx->s32Y = 0;
    ctx->u32DispWidth  = 1920;
    ctx->u32DispHeight = 1080;
    ctx->s32PixFormat = TEST_VO_FORMAT_RGB888;
    ctx->u32DispFrmRt = 25;
    ctx->u32DispFrmRtRatio = 1;
    ctx->uEnMode = 1;

    ctx->wbc_auto = RK_TRUE;
    ctx->u32WbcWidth = 1024;
    ctx->u32WbcHeight = 768;
    ctx->u32WbcPixFormat = TEST_VO_FORMAT_RGB888;
    ctx->u32WbcCompressMode = COMPRESS_MODE_NONE;
    ctx->u32WbcSourceType = WBC_SOURCE_VIDEO;
    ctx->u32WbcSourceId = RK356X_VO_DEV_HD0;

    ctx->bVoPlay = RK_FALSE;
    ctx->u32Screen0Chn = 16;
    ctx->u32Screen1Chn = 4;
    ctx->bEnWbc = RK_FALSE;
    ctx->bChnPriority = RK_FALSE;
    ctx->bEnWbcToVO = RK_TRUE;
    ctx->s32LoopCount = -1;
    ctx->u32ChnDismode = VO_CHANNEL_PLAY_NORMAL;

    ctx->stVoCfg.u32Screen0VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
    ctx->stVoCfg.u32Screen1VoLayer = RK356X_VOP_LAYER_CLUSTER_1;
    ctx->stVoCfg.u32Screen0Rows = 4;
    ctx->stVoCfg.u32Screen1Rows = 3;
    ctx->stVoCfg.bDoubleScreen  = RK_TRUE;

    ctx->stVdecCfg.u32FrameBufferCnt = MAX_FRAME_QUEUE;
    ctx->stVdecCfg.enCompressMode = COMPRESS_AFBC_16x16;

    ctx->stWbcCfg.stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
    ctx->stWbcCfg.stWbcSource.u32SourceId = VO_WBC_SOURCE_VIDEO;
    ctx->stWbcCfg.stWbcAttr.enPixelFormat = RK_FMT_RGB888;
    ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Width = 1280;
    ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Height = 720;
    ctx->stWbcCfg.stWbcAttr.u32FrameRate = 25;
    ctx->stWbcCfg.stWbcAttr.enCompressMode = COMPRESS_MODE_NONE;
    ctx->stWbcCfg.s32ChnId = 0;
    ctx->stWbcCfg.s32VdecChnId = -1;

    ctx->stBorderCfg.u32BottomWidth = 2;
    ctx->stBorderCfg.u32TopWidth = 2;
    ctx->stBorderCfg.u32LeftWidth = 2;
    ctx->stBorderCfg.u32RightWidth = 2;
}

int main(int argc, const char **argv) {
    RK_S32  s32Ret;
    TEST_VO_CTX_S ctx;

    memset(&ctx, 0, sizeof(TEST_VO_CTX_S));

    init_cfg(&ctx);

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input",  &(ctx.cfgFileUri),
                   "input config file. <required>", NULL, 0, 0),
        OPT_INTEGER('d', "device_id", &(ctx.VoDev),
                     "Vop id. e.g.(0/1). default(0).", NULL, 0, 0),
        OPT_INTEGER('l', "layer_id", &(ctx.VoLayer),
                     "Layer id. e.g.(0/2/4/6) default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_enable", &(ctx.wbc_enable),
                     "wbc_enalbe. e.g.(0) default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "ui", &(ctx.ui),
                     "ui. e.g.(0) default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "ui_alpha", &(ctx.ui_alpha),
                     "ui_alpha. e.g.(0) default(0).", NULL, 0, 0),
        OPT_INTEGER('w', "Windows", &(ctx.u32Windows),
                     "windows num. e.g [1-64] default(4),max is 63.", NULL, 0, 0),
        OPT_INTEGER('\0', "ConnectorType", &(ctx.enIntfType),
                     "Connctor Type. e.g.(0: HDMI 1: EDP 2: VGA). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "layer_mode", &(ctx.VoLayerMode),
                     "Layer type. e.g.(0: CURSOR 1: UI 2: Video). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "display_mode", &(ctx.enIntfSync),
                     "dispaly. e.g.(12/14) default (12. 12 is 1080P60). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "disp_frmrt", &(ctx.u32DispFrmRt),
                     "disp_frmrt. default(25).", NULL, 0, 0),
        OPT_INTEGER('\0', "disp_frmrt_ratio", &(ctx.u32DispFrmRtRatio),
                     "disp_frmrt_ratio. e.g.(32, 16, 8, 4, 2 ,1). default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "aspect_mode", &(ctx.uEnMode),
                     "video aspect ratio."
                     "e.g.(1: ratio no change 2: ratio manual set). default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "border_lpx", &(ctx.stBorderCfg.u32LeftWidth),
                     "chn Border lpx. default(2).", NULL, 0, 0),
        OPT_INTEGER('\0', "border_rpx", &(ctx.stBorderCfg.u32RightWidth),
                      "chn Border rpx. default(2).", NULL, 0, 0),
        OPT_INTEGER('\0', "border_tpx", &(ctx.stBorderCfg.u32TopWidth),
                     "chn Border tpx. default(2).", NULL, 0, 0),
        OPT_INTEGER('\0', "border_bpx", &(ctx.stBorderCfg.u32BottomWidth),
                     "chn Border bpx. default(2).", NULL, 0, 0),
        OPT_INTEGER('\0', "disp_x", &(ctx.s32X),
                     "disp_x. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "disp_y", &(ctx.s32Y),
                    "disp_y. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "video_format", &(ctx.s32PixFormat),
                     "video pixel format."
                     "e.g.(0: ARGB8888 1: ABGR888 2: RGB888 3: BGR888 4: RK_FMT_YUV420SP)."
                     "default(4. 4 is RGB888).", NULL, 0, 0),
        OPT_INTEGER('\0', "disp_width", &(ctx.u32DispWidth),
                     "dst width. e.g.(1920). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "disp_height", &(ctx.u32DispHeight),
                     "dst height. e.g.(1080). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "image_width", &(ctx.u32ImgeWidth),
                     "dst width. e.g.(1920). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "image_height", &(ctx.u32ImageHeight),
                     "dst height. e.g.(1080). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_width", &(ctx.u32WbcWidth),
                     "dst width. e.g.(1920). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_height", &(ctx.u32WbcHeight),
                     "dst height. e.g.(1080). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_compress", &(ctx.u32WbcCompressMode),
                     "wbc compress mode. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_format", &(ctx.u32WbcPixFormat),
                     "wbc pixel format."
                     "e.g.(0: ARGB8888 1: ABGR888 2: RGB888 3: BGR888 4:ARGB1555 5.ABGR1555)."
                     "default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_type", &(ctx.u32WbcSourceType),
                     "wbc souce type. e.g.(0: dev 1: video default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_id", &(ctx.u32WbcSourceId),
                     "wbc souce id. default(0).", NULL, 0, 0),
        OPT_STRING('\0', "voplay",  &(ctx.bVoPlay),
                   "play video test, default(0): 0: RK_FALSE, 1: RK_TRUE", NULL, 0, 0),
        OPT_STRING('\0', "wbc_auto",  &(ctx.wbc_auto),
                   "wbc auto bind, default(1): 0: RK_FALSE, 1: RK_TRUE", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_chn", &(ctx.u32Screen0Chn),
                    "the channel num of screen0. default(16)", NULL, 0, 0),
        OPT_INTEGER('\0', "chn_display", &(ctx.u32ChnDismode),
                     "the chn dispaly mode."
                     "e.g.(0: normol 1: pause 2: step 3: speed).default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_chn", &(ctx.u32Screen1Chn),
                    "the channel num of screen1 default(4)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_rows", &(ctx.stVoCfg.u32Screen0Rows),
                    "the rows/cols of screen0 display. default(4: 4x4)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_rows", &(ctx.stVoCfg.u32Screen1Rows),
                    "the rows/cols of screen1 display.default(3 : 3x3)", NULL, 0, 0),
        OPT_INTEGER('\0', "en_wbc", &(ctx.bEnWbc),
                    "enable wbc. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "en_chnPriority", &(ctx.bChnPriority),
                    "enable Chn Priority. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_src",  &(ctx.stWbcCfg.stWbcSource.u32SourceId),
                   "the source of wbc, default(1)", NULL, 0, 0),
        OPT_STRING('\0', "double_screen",  &(ctx.stVoCfg.bDoubleScreen),
                   "double screen or not, default(1): 0: RK_FALSE, 1: RK_TRUE", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);

    mpi_vo_test_show_options(&ctx);

    if (check_options(&ctx)) {
        RK_LOGE("illegal input parameters");
        argparse_usage(&argparse);
        goto __FAILED;
    }

    if (ctx.wbc_enable) {
        Sample_VO_Video_Homologous(&ctx);
    } else if (ctx.ui_alpha) {
        Sample_VO_Demo_Alpha(&ctx);
    } else if (ctx.ui) {
        Sample_VO_Demo_UI(&ctx);
    } else if (ctx.bVoPlay) {
         unit_mpi_voplay_test(&ctx);
    } else {
        Sample_VO_Demo_Video_Mosaic(&ctx);
    }

    Sample_Print("test running ok.");

    return RK_SUCCESS;
__FAILED:
    RK_LOGE("test running failed!");
    return RK_FAILURE;
}

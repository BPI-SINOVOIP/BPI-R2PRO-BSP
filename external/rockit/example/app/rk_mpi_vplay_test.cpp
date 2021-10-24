/*
 * Copyright 2018 Rockchip Electronics Co. LTD
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

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>

#include "rk_debug.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_sys.h"
#include "rk_comm_mb.h"
#include "rk_comm_vpss.h"
#include "rk_common.h"
#include "rk_comm_adec.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_vpss.h"
#include "rk_mpi_ao.h"
#include "rk_mpi_adec.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_rgn.h"
#include "mpi_test_utils.h"
#include "argparse.h"
#include "loadbmp.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavformat/version.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
}

#define MAX_FRAME_QUEUE              8
#define MAX_TIME_OUT_MS              300

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

typedef struct rkRGN_CFG_S {
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stRgnChnAttr;
    char *srcFileBmpUri;
} RGN_CFG_S;

typedef struct rkVENC_CFG_S {
    VENC_CHN_ATTR_S stAttr;
    const char *dstFilePath;
    RK_S32 s32ChnId;
} VENC_CFG_S;

typedef struct rkWBC_CFG_S {
    VO_WBC_SOURCE_S stWbcSource;
    VO_WBC_ATTR_S stWbcAttr;
    RK_S32 s32ChnId;
    RK_S32 s32VdecChnId;
} WBC_CFG_S;

typedef struct rkVDEC_CFG_S {
    RK_U32 u32FrameBufferCnt;
    COMPRESS_MODE_E enCompressMode;
} VDEC_CFG_S;

typedef struct rkVO_Timing_S {
    RK_S32  enIntfSync;
    RK_U32  u32Width;
    RK_U32  u32Height;
} VO_Timing_S;

typedef enum rkVO_VIDEO_UI_MODE_E {
    VIDEO_UI_MODE_NONE,
    VIDEO_UI_MODE_MERGE,
    VIDEO_UI_MODE_SPLIT,
    VIDEO_UI_MODE_BUTT
} VO_VIDEO_UI_MODE_E;

typedef struct rkVO_CFG_S {
    RK_U32 u32Screen0VoLayer;
    RK_U32 u32Screen1VoLayer;

    RK_U32 u32Screen0DisplayWidth;
    RK_U32 u32Screen0DisplayHeight;

    RK_U32 u32Screen1DisplayWidth;
    RK_U32 u32Screen1DisplayHeight;

    RK_U32 u32Screen0Rows;
    RK_U32 u32Screen1Rows;
    RK_BOOL bDoubleScreen;

    RK_BOOL bEnUI;
    VO_VIDEO_UI_MODE_E enUIMode;
    RK_U32 u32UIVoLayer;
    RK_U32 u32UIChn;
} VO_CFG_S;

typedef struct rkVPSS_CFG_S {
    const char *dstFilePath;
    RK_S32 s32DevId;
    RK_S32 s32ChnId;
    RK_U32 u32VpssChnCnt;
    VPSS_GRP_ATTR_S stGrpVpssAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_CHN_NUM];
} VPSS_CFG_S;

typedef struct rkPARSER_CFG_S {
    char *srcFileUri[VDEC_MAX_CHN_NUM];
    RK_CODEC_ID_E enCodecId;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_U32 u32StreamIndex;
    AVFormatContext *pstAvfc;
} PARSER_CFG_S;

typedef struct _rkMpiCtx {
    const char *cfgFileUri;
    RK_S32 s32LoopCount;
    RK_U32 u32ChnIndex;
    RK_U32 u32Screen0Resolution;
    RK_U32 u32Screen1Resolution;
    RK_U32 u32Screen0Chn;
    RK_U32 u32Screen1Chn;
    RK_BOOL bThreadExit;

    RK_BOOL bEnVpss;
    RK_BOOL bEnWbc;
    RK_BOOL bEnRgn;
    RK_BOOL bEnWbcToVO;

    PARSER_CFG_S stParserCfg;
    VDEC_CFG_S stVdecCfg;
    VO_CFG_S stVoCfg;
    WBC_CFG_S stWbcCfg;
    VENC_CFG_S stVencCfg;
    VPSS_CFG_S stVpssCfg;
    RGN_CFG_S stRgnCfg;
} MPI_CTX_S;

//-------------------------------------------------------------------
#define RK356X_VO_DEV_HD0      0
#define RK356X_VO_DEV_HD1      1
// #define VO_TEST_ROCKIT_LOCAL_BUFFFER
// #define VO_TEST_ROCKIT_PRIORITY

enum {
    RK356X_VOP_LAYER_CLUSTER0,
    RK356X_VOP_LAYER_CLUSTER0_1,
    RK356X_VOP_LAYER_CLUSTER1,
    RK356X_VOP_LAYER_CLUSTER1_1,
    RK356X_VOP_LAYER_ESMART_0,
    RK356X_VOP_LAYER_ESMART_1,
    RK356X_VOP_LAYER_SMART0,
    RK356X_VOP_LAYER_SMART1,
};

#ifdef VO_TEST_ROCKIT_LOCAL_BUFFFER
static VO_DEV VoLayer = RK356X_VOP_LAYER_ESMART_0;  // hdmi
static VO_DEV VoLayer_second = RK356X_VOP_LAYER_ESMART_1;  // vga
#else
static VO_DEV VoLayer = RK356X_VOP_LAYER_CLUSTER0;  // hdmi
static VO_DEV VoLayer_second = RK356X_VOP_LAYER_CLUSTER1;  // vga
static VO_LAYER VoGfxLayer = RK356X_VOP_LAYER_ESMART_0;
static VO_LAYER VoGfxLayer_second = RK356X_VOP_LAYER_ESMART_1;
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])
#endif

static VO_Timing_S stVoTimings[] = {
    {VO_OUTPUT_640x480_60, 640, 480},
    {VO_OUTPUT_800x600_60, 800, 600},
    {VO_OUTPUT_1024x768_60, 1024, 768},
    {VO_OUTPUT_1280x1024_60, 1280, 1024},
    {VO_OUTPUT_1366x768_60, 1366, 768},
    {VO_OUTPUT_1440x900_60, 1440, 900},
    {VO_OUTPUT_1280x800_60, 1280, 800},
    {VO_OUTPUT_1600x1200_60, 1600, 1200},
    {VO_OUTPUT_1680x1050_60, 1680, 1050},
    {VO_OUTPUT_1920x1200_60, 1920, 1200},
};

static void Sample_VO_GetDisplaySize(RK_S32 enIntfSync, RK_U32 *s32W, RK_U32 *s32H) {
    switch (enIntfSync) {
        case VO_OUTPUT_PAL:
        case VO_OUTPUT_576P50:
            *s32W = 720;
            *s32H = 576;
            break;
        case VO_OUTPUT_NTSC:
        case VO_OUTPUT_480P60:
            *s32W = 720;
            *s32H = 480;
            break;
        case VO_OUTPUT_720P50:
        case VO_OUTPUT_720P60:
            *s32W = 1280;
            *s32H = 720;
            break;
        case VO_OUTPUT_1080P24:
        case VO_OUTPUT_1080P25:
        case VO_OUTPUT_1080P30:
        case VO_OUTPUT_1080I50:
        case VO_OUTPUT_1080I60:
        case VO_OUTPUT_1080P50:
        case VO_OUTPUT_1080P60:
            *s32W = 1920;
            *s32H = 1080;
            break;
        case VO_OUTPUT_3840x2160_24:
        case VO_OUTPUT_3840x2160_25:
        case VO_OUTPUT_3840x2160_30:
        case VO_OUTPUT_3840x2160_50:
        case VO_OUTPUT_3840x2160_60:
            *s32W = 3840;
            *s32H = 2160;
            break;
        case VO_OUTPUT_4096x2160_24:
        case VO_OUTPUT_4096x2160_25:
        case VO_OUTPUT_4096x2160_30:
        case VO_OUTPUT_4096x2160_50:
        case VO_OUTPUT_4096x2160_60:
            *s32W = 4096;
            *s32H = 2160;
            break;
        default:
            for (RK_S32 i = 0; i < ARRAY_LENGTH(stVoTimings); i++) {
                if (stVoTimings[i].enIntfSync == enIntfSync) {
                    *s32W = stVoTimings[i].u32Width;
                    *s32H = stVoTimings[i].u32Height;
                    break;
                }
            }
            break;
    }
}

static void* vo_test_thread_func(void *pArgs) {
    // hide,show
    RK_MPI_VO_HideChn(VoLayer, 4);
    usleep(1000llu * 5000);
    RK_MPI_VO_ShowChn(VoLayer, 4);

    // pause, resume
    RK_MPI_VO_PauseChn(VoLayer_second, 1);
    usleep(1000llu * 5000);
    RK_MPI_VO_ResumeChn(VoLayer_second, 1);

    // step, resume
    for (RK_S32 i = 0; i < 50; i++) {
        RK_MPI_VO_StepChn(VoLayer, 5);
        usleep(1000llu * 1000);
    }
    RK_MPI_VO_ResumeChn(VoLayer, 5);

    return RK_NULL;
}

static void Sample_VO_GetPictureData(VO_FRAME_INFO_S *pstFrameInfo, const char *pFileName) {
    FILE *fp = RK_NULL;
    RK_U32 u32Size, u32Bpp, u32Stride;
    RK_S8 *pBuffer = RK_NULL;

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
        RK_LOGE("%s Unsupported Format %d\n", __func__, pstFrameInfo->enPixelFormat);
        return;
    }

    fp = fopen(pFileName, "r");
    if (RK_NULL == fp) {
        RK_LOGE("cat not find file\n");
        return;
    }
    u32Size = fseek(fp, 0, SEEK_END);
    fseek(fp, 0, SEEK_SET);

    RK_LOGE("%dx%d %dx%d\n",
              pstFrameInfo->u32Width, pstFrameInfo->u32Height,
              pstFrameInfo->u32VirWidth, pstFrameInfo->u32VirHeight);

    if (pstFrameInfo->u32Width != pstFrameInfo->u32VirWidth) {
        u32Stride = pstFrameInfo->u32VirWidth * u32Bpp;

        for (RK_S32 i = 0; i < pstFrameInfo->u32Height; i++) {
            pBuffer = reinterpret_cast<RK_S8 *>(pstFrameInfo->pData) + u32Stride * i;
            fread(pBuffer, 1, pstFrameInfo->u32Width * u32Bpp, fp);
        }

        if (pstFrameInfo->enPixelFormat == RK_FMT_YUV420SP ||
            pstFrameInfo->enPixelFormat == RK_FMT_YUV420SP_VU) {
            for (RK_S32 i = 0; i < pstFrameInfo->u32Height / 2; i++) {
                pBuffer = reinterpret_cast<RK_S8 *>(pstFrameInfo->pData)
                         + u32Stride * pstFrameInfo->u32VirHeight + i * u32Stride;
                fread(pBuffer, 1, pstFrameInfo->u32Width, fp);
            }
        }
    } else {
        fread(pstFrameInfo->pData, 1, u32Size, fp);
    }

    fclose(fp);
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

static RK_S32 Sample_VO_CreateGFXData(RK_U32 u32Width, RK_U32 u32Height, RK_U32 foramt, RK_U32 value, RK_VOID **pMblk) {
    VO_FRAME_INFO_S stFrameInfo;
    RK_U32 u32BuffSize;

    u32BuffSize = RK_MPI_VO_CreateGraphicsFrameBuffer(u32Width, u32Height, foramt, pMblk);
    if (u32BuffSize == 0) {
        RK_LOGE("can not create gfx buffer\n");
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
        RK_LOGE("[%s] Get Layer Attr failed\n", __func__);
        return RK_FAILURE;
    }

    for (i = 0; i < u32Layers; i++) {
        stChnAttr.stRect.s32X = 0;
        stChnAttr.stRect.s32Y = 0;
        stChnAttr.stRect.u32Width = stLayerAttr.stImageSize.u32Width;
        stChnAttr.stRect.u32Height = stLayerAttr.stImageSize.u32Height;
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
            RK_LOGE("[%s] set chn Attr failed\n", __func__);
            return RK_FAILURE;
        }

        s32Ret = RK_MPI_VO_EnableChn(VoLayer, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("[%s] Enalbe chn failed\n", __func__);
            return RK_FAILURE;
        }
    }

    RK_LOGE("[%s] success\n", __func__);
    return s32Ret;
}

static RK_S32 mpi_vo_init_sample(const MPI_CTX_S *ctx, RK_S32 primRows , RK_S32 secondRows) {
    VO_PUB_ATTR_S VoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CSC_S  VideoCSC;
    VO_CHN_ATTR_S VoChnAttr[64];
    VO_BORDER_S border;
    RK_S32  i;
    RK_U32 count = 0;
    RK_U32 u32DispBufLen;
    RK_U32 s32Ret;
    RK_U32 u32DispWidth = ctx->stVoCfg.u32Screen0DisplayWidth;
    RK_U32 u32DispHeight = ctx->stVoCfg.u32Screen0DisplayHeight;
    RK_U32 u32ImageWidth = u32DispWidth;
    RK_U32 u32ImageHeight = u32DispHeight;

    memset(&VoPubAttr, 0, sizeof(VO_PUB_ATTR_S));
    memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
    memset(&VideoCSC, 0, sizeof(VO_CSC_S));
    memset(&VoChnAttr, 0, sizeof(VoChnAttr));
    memset(&border, 0, sizeof(VO_BORDER_S));

    VoPubAttr.enIntfType = VO_INTF_HDMI;
    VoPubAttr.enIntfSync = (VO_INTF_SYNC_E)ctx->u32Screen0Resolution;

    if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_MERGE) {
        VO_FRAME_INFO_S stVFrame;
        RK_MPI_VO_SetGFxMode(VO_MODE_GFX_PRE_CREATED);
        /* Get 1st GFX buffer */
        stVFrame.enPixelFormat = RK_FMT_BGRA5551;
        stVFrame.u32FgAlpha = 128;
        stVFrame.u32BgAlpha = 0;
        stVFrame.u32Width = u32DispWidth;
        stVFrame.u32Height = u32DispHeight;
        RK_MPI_VO_GetGfxFrameBuffer(VoLayer, 127, &stVFrame);
        /* Draw 1st GFX */
        memset(stVFrame.pData, 0xff, stVFrame.u32Size);

        /* Get 2nd GFX buffer */
        stVFrame.u32FgAlpha = 128;
        stVFrame.u32BgAlpha = 0;
        RK_MPI_VO_GetGfxFrameBuffer(VoLayer, 126, &stVFrame);

        /* Draw 2nd GFX */
        memset(stVFrame.pData, 0, stVFrame.u32Size);
    }

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
        VoPubAttr1.enIntfSync = (VO_INTF_SYNC_E)ctx->u32Screen1Resolution;

        if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_MERGE) {
            VO_FRAME_INFO_S stVFrame;
            stVFrame.enPixelFormat = RK_FMT_BGRA5551;
            stVFrame.u32FgAlpha = 128;
            stVFrame.u32BgAlpha = 0;
            stVFrame.u32Width = u32DispWidth;
            stVFrame.u32Height = u32DispHeight;
            RK_MPI_VO_GetGfxFrameBuffer(VoLayer_second, 127, &stVFrame);
            /* Draw 1st GFX */
            memset(stVFrame.pData, 0xff, stVFrame.u32Size);

            /* Get 2nd GFX buffer */
            stVFrame.u32FgAlpha = 128;
            stVFrame.u32BgAlpha = 0;
            RK_MPI_VO_GetGfxFrameBuffer(VoLayer_second, 126, &stVFrame);

            /* Draw 2nd GFX */
            memset(stVFrame.pData, 0, stVFrame.u32Size);
        }

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

    Sample_VO_GetDisplaySize(ctx->u32Screen0Resolution, &u32DispWidth, &u32DispHeight);
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stDispRect.u32Width = u32DispWidth;
    stLayerAttr.stDispRect.u32Height = u32DispHeight;
    stLayerAttr.stImageSize.u32Width = u32ImageWidth;
    stLayerAttr.stImageSize.u32Height = u32ImageHeight;

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
        VoChnAttr[i].stRect.s32X = (u32ImageWidth/primRows)*(i%primRows);
        VoChnAttr[i].stRect.s32Y = (u32ImageHeight/primRows)*(i/primRows);
        VoChnAttr[i].stRect.u32Width = u32ImageWidth/primRows;
        VoChnAttr[i].stRect.u32Width = VoChnAttr[i].stRect.u32Width;
        VoChnAttr[i].stRect.u32Height = u32ImageHeight/primRows;
        VoChnAttr[i].stRect.u32Height = VoChnAttr[i].stRect.u32Height;
    }

#ifdef VO_TEST_ROCKIT_PRIORITY
    VoChnAttr[0].u32Priority = 15;

    VoChnAttr[1].u32Priority = 10;
    VoChnAttr[1].stRect.s32X = (u32DispWidth/3)*(1%3) - 100;
    VoChnAttr[1].stRect.s32Y = (u32DispHeight/3)*(1/3) + 100;
    VoChnAttr[1].stRect.u32Width = u32DispWidth/3 + 200;
        VoChnAttr[1].stRect.u32Width = VoChnAttr[1].stRect.u32Width;
    VoChnAttr[1].stRect.u32Height = u32DispHeight/3 + 200;
        VoChnAttr[1].stRect.u32Height = VoChnAttr[1].stRect.u32Height;

    VoChnAttr[2].u32Priority = 5;
#endif

    for (i = 0; i < totalCH; i++) {
        // set attribute of vo chn
        RK_S32 s32ret = RK_MPI_VO_SetChnAttr(VoLayer, i, &VoChnAttr[i]);
        if (RK_SUCCESS != s32ret) {
            RK_LOGE("vo set dev %d chn %d attr failed! \n", VoLayer, i);
        }
        // set border
        border.bBorderEn = RK_TRUE;
        // Navy blue #000080
        // Midnight Blue #191970
        border.stBorder.u32Color = 0x191970;
        border.stBorder.u32LeftWidth = 2;
        border.stBorder.u32RightWidth = 2;
        border.stBorder.u32TopWidth = 2;
        border.stBorder.u32BottomWidth = 2;
        s32Ret = RK_MPI_VO_SetChnBorder(VoLayer, i, &border);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_SPLIT) {
        /* Enable GFX Layer */
        stLayerAttr.enPixFormat = RK_FMT_BGRA8888;
        stLayerAttr.stDispRect.s32X = 0;
        stLayerAttr.stDispRect.s32Y = 0;

        if (u32ImageWidth > 1920) {
            stLayerAttr.stImageSize.u32Width = 1920;
        } else {
            stLayerAttr.stImageSize.u32Width = u32ImageWidth;
        }

        if (u32ImageHeight > 1080) {
            stLayerAttr.stImageSize.u32Height  = 1080;
        } else {
            stLayerAttr.stImageSize.u32Height  = u32ImageHeight;
        }
        stLayerAttr.u32DispFrmRt = 25;
        Sample_VO_StartLayer(VoGfxLayer, &stLayerAttr);
        Sample_VO_MultiGFXLayer_Start(VoGfxLayer, ctx->stVoCfg.u32UIChn);
    }

    if (ctx->stVoCfg.bDoubleScreen) {
        u32DispWidth = ctx->stVoCfg.u32Screen1DisplayWidth;
        u32DispHeight = ctx->stVoCfg.u32Screen1DisplayHeight;
        u32ImageWidth = u32DispWidth;
        u32ImageHeight = u32DispHeight;

        Sample_VO_GetDisplaySize(ctx->u32Screen1Resolution, &u32DispWidth, &u32DispHeight);
        stLayerAttr.stDispRect.s32X = 0;
        stLayerAttr.stDispRect.s32Y = 0;
        stLayerAttr.stDispRect.u32Width = u32DispWidth;
        stLayerAttr.stDispRect.u32Height = u32DispHeight;
        stLayerAttr.stImageSize.u32Width = u32ImageWidth;
        stLayerAttr.stImageSize.u32Height = u32ImageHeight;

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
            VoChnAttr[i].stRect.s32X = (u32ImageWidth/secondRows)*(i%secondRows);
            VoChnAttr[i].stRect.s32Y = (u32ImageHeight/secondRows)*(i/secondRows);
            VoChnAttr[i].stRect.u32Width = u32ImageWidth/secondRows;
            VoChnAttr[i].stRect.u32Width = VoChnAttr[i].stRect.u32Width;
            VoChnAttr[i].stRect.u32Height = u32ImageHeight/secondRows;
            VoChnAttr[i].stRect.u32Height = VoChnAttr[i].stRect.u32Height;
        }

        for (i = 0; i < totalCH; i++) {
            // set attribute of vo chn
            RK_S32 s32ret = RK_MPI_VO_SetChnAttr(VoLayer_second, i, &VoChnAttr[i]);
            if (RK_SUCCESS != s32ret) {
                RK_LOGE("vo set dev %d chn %d attr failed! \n", VoLayer_second, i);
            }
            // set border
            border.bBorderEn = RK_TRUE;
            // Navy blue #000080
            // Midnight Blue #191970
            border.stBorder.u32Color = 0x191970;
            border.stBorder.u32LeftWidth = 2;
            border.stBorder.u32RightWidth = 2;
            border.stBorder.u32TopWidth = 2;
            border.stBorder.u32BottomWidth = 2;
            s32Ret = RK_MPI_VO_SetChnBorder(VoLayer_second, i, &border);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
        }

        if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_SPLIT) {
            /* Enable GFX Layer */
            stLayerAttr.enPixFormat = RK_FMT_BGRA8888;
            stLayerAttr.stDispRect.s32X = 0;
            stLayerAttr.stDispRect.s32Y = 0;
            if (u32ImageWidth > 1920) {
                stLayerAttr.stImageSize.u32Width = 1920;
            } else {
                stLayerAttr.stImageSize.u32Width = u32ImageWidth;
            }

            if (u32ImageHeight > 1080) {
                stLayerAttr.stImageSize.u32Height  = 1080;
            } else {
                stLayerAttr.stImageSize.u32Height  = u32ImageHeight;
            }

            stLayerAttr.u32DispFrmRt = 25;
            Sample_VO_StartLayer(VoGfxLayer_second, &stLayerAttr);
            Sample_VO_MultiGFXLayer_Start(VoGfxLayer_second, ctx->stVoCfg.u32UIChn);
        }
    }

    return RK_SUCCESS;
}

static RK_S32 mpi_vo_deinit_sample(const MPI_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;

    if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_SPLIT) {
        // disable vo gfx chn
        for (RK_S32 i = 0; i < ctx->stVoCfg.u32UIChn; i++) {
            RK_MPI_VO_DisableChn(VoGfxLayer, i);
        }
        // disable vo gfx layer
        s32Ret = RK_MPI_VO_DisableLayer(VoGfxLayer);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        if (ctx->stVoCfg.bDoubleScreen) {
            for (RK_S32 i = 0; i < ctx->stVoCfg.u32UIChn; i++) {
                RK_MPI_VO_DisableChn(VoGfxLayer_second, i);
            }
            s32Ret = RK_MPI_VO_DisableLayer(VoGfxLayer_second);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
        }
    }

    // disable vo layer
    s32Ret = RK_MPI_VO_DisableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    if (ctx->stVoCfg.bDoubleScreen) {
        s32Ret = RK_MPI_VO_DisableLayer(VoLayer_second);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    // disable vo dev
    s32Ret = RK_MPI_VO_Disable(RK356X_VO_DEV_HD0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    if (ctx->stVoCfg.bDoubleScreen) {
        s32Ret = RK_MPI_VO_Disable(RK356X_VO_DEV_HD1);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    return s32Ret;
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

static RK_S32 open_parser(MPI_CTX_S *ctx, RK_U32 u32Ch) {
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

static RK_S32 open_config(MPI_CTX_S *ctx) {
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
        RK_S32 len = strlen(line);
        pBuffer = reinterpret_cast<char *>(malloc(len));
        snprintf(pBuffer, len, "%s", line);
        while (len) {
            if (pBuffer[len] == '\r') {
                pBuffer[len] = '\0';
                break;
            }
            len--;
        }
        ctx->stParserCfg.srcFileUri[u32Count] = pBuffer;

        RK_LOGI("url %s", ctx->stParserCfg.srcFileUri[u32Count]);
        u32Count++;

        if (u32Count >= VDEC_MAX_CHN_NUM)
            break;
    }

    rk_safe_free(line);
    fclose(fp);
    return RK_SUCCESS;
}

static RK_S32 close_config(MPI_CTX_S *ctx) {
    RK_S32 i;

    for (i = 0; i < VDEC_MAX_CHN_NUM; i++) {
        rk_safe_free(ctx->stParserCfg.srcFileUri[i]);
    }
    return RK_SUCCESS;
}

RK_S32 load_bmp(const RK_CHAR *filename, BITMAP_S *pstBitmap) {
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if (get_bmp_info(filename, &bmpFileHeader, &bmpInfo) < 0) {
        RK_LOGE("GetBmpInfo err!\n");
        return RK_FAILURE;
    }

    Surface.enColorFmt = OSD_COLOR_FMT_BGRA5551;

    pstBitmap->pData = malloc(4 * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight));

    if (RK_NULL == pstBitmap->pData) {
        RK_LOGE("malloc osd memroy err!");
        return RK_FAILURE;
    }

    create_surface_by_bitmap(filename, &Surface,  reinterpret_cast<RK_U8 *>(pstBitmap->pData));

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = RK_FMT_BGRA5551;

    return RK_SUCCESS;
}

static RK_S32 create_rgn(MPI_CTX_S *ctx) {
    RK_S32 i;
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_HANDLE RgnHandle = 0;

    s32Ret = RK_MPI_RGN_Create(RgnHandle, &ctx->stRgnCfg.stRgnAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_Create (%d) failed with %#x!", RgnHandle, s32Ret);
        RK_MPI_RGN_Destroy(RgnHandle);
        return RK_FAILURE;
    }
    RK_LOGI("The handle: %d, create success!", RgnHandle);

    BITMAP_S stBitmap;
    RK_LOGE("srcFileBmpUri %s", ctx->stRgnCfg.srcFileBmpUri);
    s32Ret = load_bmp(ctx->stRgnCfg.srcFileBmpUri, &stBitmap);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("Load bmp failed with %#x!", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_RGN_SetBitMap(RgnHandle, &stBitmap);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_RGN_SetBitMap failed with %#x!", s32Ret);
        return RK_FAILURE;
    }

    if (RK_NULL != stBitmap.pData) {
        free(stBitmap.pData);
        stBitmap.pData = NULL;
    }

    RK_LOGI("The bmp handle: %d, create success!", RgnHandle);
    return RK_SUCCESS;
}

static RK_S32 destory_rgn(MPI_CTX_S *ctx) {
    RK_S32 i;
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_HANDLE RgnHandle = 0;

    s32Ret = RK_MPI_RGN_Destroy(RgnHandle);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_Destroy (%d) failed with %#x!", RgnHandle, s32Ret);
    }
    return RK_SUCCESS;
}

static RK_S32 create_vpss(MPI_CTX_S *ctx, RK_S32 s32Grp, RK_S32 s32OutChnNum) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CHN VpssChn[VPSS_MAX_CHN_NUM] = { VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3 };
    VPSS_CFG_S *pstVpssCfg = &ctx->stVpssCfg;
    VPSS_CROP_INFO_S stCropInfo;

    s32Ret = RK_MPI_VPSS_CreateGrp(s32Grp, &ctx->stVpssCfg.stGrpVpssAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    for (RK_S32 i = 0; i < s32OutChnNum; i++) {
        s32Ret = RK_MPI_VPSS_SetChnAttr(s32Grp, VpssChn[i], &pstVpssCfg->stVpssChnAttr[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VPSS_EnableChn(s32Grp, VpssChn[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VPSS_EnableBackupFrame(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VPSS_StartGrp(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return  RK_SUCCESS;
}

static RK_S32 destory_vpss(MPI_CTX_S *ctx, RK_S32 s32Grp, RK_S32 s32OutChnNum) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CHN VpssChn[VPSS_MAX_CHN_NUM] = { VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3 };

    s32Ret = RK_MPI_VPSS_StopGrp(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    for (RK_S32 i = 0; i < s32OutChnNum; i++) {
        s32Ret = RK_MPI_VPSS_DisableChn(s32Grp, VpssChn[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VPSS_DisableBackupFrame(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VPSS_DestroyGrp(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return  RK_SUCCESS;
}

static RK_S32 creat_wbc(MPI_CTX_S *ctx, RK_U32 u32Ch) {
    RK_S32 s32Ret = RK_SUCCESS;
    VO_WBC VoWbc = u32Ch;

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

static RK_S32 creat_venc(MPI_CTX_S *ctx, RK_U32 u32Ch) {
    VENC_RECV_PIC_PARAM_S stRecvParam;

    memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));

    RK_MPI_VENC_CreateChn(u32Ch, &ctx->stVencCfg.stAttr);
    RK_MPI_VENC_StartRecvFrame(u32Ch, &stRecvParam);

    return RK_SUCCESS;
}

static RK_S32 creat_wbc_vdec(MPI_CTX_S *ctx, RK_U32 u32Ch) {
    RK_S32 s32Ret = RK_SUCCESS;
    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;

    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stVdecParam, 0, sizeof(VDEC_CHN_PARAM_S));

    stAttr.enMode = VIDEO_MODE_FRAME;
    stAttr.enType =  ctx->stVencCfg.stAttr.stVencAttr.enType;
    stAttr.u32PicWidth = ctx->stVencCfg.stAttr.stVencAttr.u32PicWidth;
    stAttr.u32PicHeight = ctx->stVencCfg.stAttr.stVencAttr.u32PicHeight;
    stAttr.u32FrameBufCnt = ctx->stVdecCfg.u32FrameBufferCnt;

    RK_LOGD("stAttr.enType %d u32PicWidth %d u32PicHeight %d u32Ch %d",
                stAttr.enType, stAttr.u32PicWidth, stAttr.u32PicHeight, u32Ch);

    s32Ret = RK_MPI_VDEC_CreateChn(u32Ch, &stAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed! ", u32Ch);
        RK_LOGE(R"(enMode %d enType %d u32PicWidth %d u32PicHeight %d \
            u32PicVirWidth %d u32PicVirHeight %d u32FrameBufCnt %d)", \
            stAttr.enMode, stAttr.enType, stAttr.u32PicWidth, stAttr.u32PicHeight, \
            stAttr.u32PicVirWidth, stAttr.u32PicVirHeight, stAttr.u32FrameBufCnt);
        return s32Ret;
    }

    stVdecParam.stVdecVideoParam.enCompressMode = COMPRESS_MODE_NONE;
    s32Ret = RK_MPI_VDEC_SetChnParam(u32Ch, &stVdecParam);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VDEC_StartRecvStream(u32Ch);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vdec %d start failed");
        return s32Ret;
    }

    RK_LOGD("create wbc vdec %d success", u32Ch);
    return RK_SUCCESS;
}

static RK_S32 create_vdec(MPI_CTX_S *ctx, RK_U32 u32Ch) {
    RK_S32 s32Ret = RK_SUCCESS;
    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;

    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stVdecParam, 0, sizeof(VDEC_CHN_PARAM_S));

    stAttr.enMode = VIDEO_MODE_FRAME;
    stAttr.enType = ctx->stParserCfg.enCodecId;
    stAttr.u32PicWidth = ctx->stParserCfg.u32SrcWidth;
    stAttr.u32PicHeight = ctx->stParserCfg.u32SrcHeight;
    stAttr.u32FrameBufCnt = ctx->stVdecCfg.u32FrameBufferCnt;

    s32Ret = RK_MPI_VDEC_CreateChn(u32Ch, &stAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed! ", u32Ch);
        RK_LOGE(R"(enMode %d enType %d u32PicWidth %d u32PicHeight %d \
            u32PicVirWidth %d u32PicVirHeight %d u32FrameBufCnt %d)", \
            stAttr.enMode, stAttr.enType, stAttr.u32PicWidth, stAttr.u32PicHeight, \
            stAttr.u32PicVirWidth, stAttr.u32PicVirHeight, stAttr.u32FrameBufCnt);
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

static RK_S32 wbc_send_stream(RK_U32 u32Ch, MB_BLK pMbBlk, RK_U32 u32Size, RK_U64 u64PTS) {
    RK_S32 s32Ret = 0;
    RK_U8 *data = RK_NULL;
    VDEC_STREAM_S stStream;
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    stStream.u64PTS = u64PTS;
    stStream.pMbBlk = pMbBlk;
    stStream.u32Len = u32Size;
    stStream.bEndOfStream = RK_FALSE;
    stStream.bEndOfFrame = RK_FALSE;
    stStream.bBypassMbBlk = RK_TRUE;
__RETRY:
    s32Ret = RK_MPI_VDEC_SendStream(u32Ch, &stStream, MAX_TIME_OUT_MS);
    if (s32Ret != RK_SUCCESS) {
        usleep(1000llu);
        goto  __RETRY;
    }

    RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
    return RK_SUCCESS;
}


static RK_S32 screen_init(MPI_CTX_S *ctx, RK_U32 u32VoLayer, RK_U32 u32Index, RK_U32 u32VoChn) {
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stVdecChn, stVpssChn, stVoChn;

    s32Ret = open_parser(ctx, u32Index);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d ch parser failed!", u32Index);
        return s32Ret;
    }

    s32Ret = create_vdec(ctx, u32Index);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d ch vdec failed!", u32Index);
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_EnableChn(u32VoLayer, u32VoChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d layer %d ch vo failed!", u32VoLayer, u32VoChn);
        return s32Ret;
    }
    RK_LOGD("create vo layer %d ch %d", u32VoLayer,  u32VoChn);

    if (ctx->bEnVpss) {
        s32Ret = create_vpss(ctx, u32Index, ctx->stVpssCfg.u32VpssChnCnt);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("creat %d grp vpss failed!", u32Index);
            return s32Ret;;
        }

        stVdecChn.enModId = RK_ID_VDEC;
        stVdecChn.s32DevId = 0;
        stVdecChn.s32ChnId = u32Index;
        RK_LOGD("vdec ch %d", stVdecChn.s32ChnId);
        stVpssChn.enModId = RK_ID_VPSS;
        stVpssChn.s32DevId = u32Index;
        stVpssChn.s32ChnId = 0;
        RK_LOGD("vdec to vpss ch %d vpss group %d", stVpssChn.s32ChnId , stVpssChn.s32DevId);
        s32Ret = RK_MPI_SYS_Bind(&stVdecChn, &stVpssChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vdec and vpss bind error ");
            return s32Ret;;
        }

        stVpssChn.enModId = RK_ID_VPSS;
        stVpssChn.s32DevId = u32Index;
        stVpssChn.s32ChnId = 0;

        RK_LOGD("vpss  Ch %d", stVpssChn.s32ChnId);
        stVoChn.enModId = RK_ID_VO;
        stVoChn.s32DevId = u32VoLayer;
        stVoChn.s32ChnId = u32VoChn;

        RK_LOGD("VoLayer %d, voCh %d", u32VoLayer,  u32VoChn);

        s32Ret = RK_MPI_SYS_Bind(&stVpssChn, &stVoChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vpss and vo bind error");
            return s32Ret;
        }
    } else {
        stVdecChn.enModId = RK_ID_VDEC;
        stVdecChn.s32DevId = 0;
        stVdecChn.s32ChnId = u32Index;
        RK_LOGD("vdec ch %d", stVdecChn.s32ChnId);

        stVoChn.enModId = RK_ID_VO;
        stVoChn.s32DevId = u32VoLayer;
        stVoChn.s32ChnId = u32VoChn;

        RK_LOGD("VoLayer %d, voCh %d", u32VoLayer, u32VoChn);

        s32Ret = RK_MPI_SYS_Bind(&stVdecChn, &stVoChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vdec and vo bind error");
            return s32Ret;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 screen_deinit(MPI_CTX_S *ctx, RK_U32 u32VoLayer, RK_U32 u32Index, RK_U32 u32VoChn) {
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stVdecChn, stVpssChn, stVoChn;

    if (ctx->bEnVpss) {
        stVdecChn.enModId = RK_ID_VDEC;
        stVdecChn.s32DevId = 0;
        stVdecChn.s32ChnId = u32Index;

        stVpssChn.enModId = RK_ID_VPSS;
        stVpssChn.s32DevId = u32Index;
        stVpssChn.s32ChnId = 0;

        s32Ret = RK_MPI_SYS_UnBind(&stVdecChn, &stVpssChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("unbind vdec and vpss bind error ");
            return s32Ret;;
        }

        stVpssChn.enModId = RK_ID_VPSS;
        stVpssChn.s32DevId = u32Index;
        stVpssChn.s32ChnId = 0;

        stVoChn.enModId = RK_ID_VO;
        stVoChn.s32DevId = u32VoLayer;
        stVoChn.s32ChnId = u32VoChn;

        s32Ret = RK_MPI_SYS_UnBind(&stVpssChn, &stVoChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("unbind vpss and vo bind error");
            return s32Ret;
        }

        s32Ret = destory_vpss(ctx, u32Index, ctx->stVpssCfg.u32VpssChnCnt);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("destory vpss error");
            return s32Ret;
        }
    } else {
        stVdecChn.enModId = RK_ID_VDEC;
        stVdecChn.s32DevId = 0;
        stVdecChn.s32ChnId = u32Index;

        stVoChn.enModId = RK_ID_VO;
        stVoChn.s32DevId = u32VoLayer;
        stVoChn.s32ChnId = u32VoChn;

        s32Ret = RK_MPI_SYS_UnBind(&stVdecChn, &stVoChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("unbind vdec and vo bind error");
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VDEC_StopRecvStream(u32Index);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VDEC_DestroyChn(u32Index);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_DisableChn(u32VoLayer, u32VoChn);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return RK_SUCCESS;
}

static RK_S32 rgn_init(MPI_CTX_S *ctx, RK_U32 u32VoLayer, RK_U32 u32Chn, RGN_HANDLE RgnHandle) {
    RK_S32 i;
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stMppChn;

    stMppChn.enModId = RK_ID_VO;
    stMppChn.s32DevId = u32VoLayer;
    stMppChn.s32ChnId = u32Chn;

    RK_LOGD("u32Screen0Chn %d s32DevId %d s32ChnId %d xpos %d ypos %d",
        ctx->u32Screen0Chn, stMppChn.s32DevId, stMppChn.s32ChnId,
        ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X,
        ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);

    s32Ret = RK_MPI_RGN_AttachToChn(RgnHandle, &stMppChn, &ctx->stRgnCfg.stRgnChnAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_AttachToChn (%d) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

static RK_S32 rgn_deinit(MPI_CTX_S *ctx, RK_U32 u32VoLayer, RK_U32 u32Chn, RGN_HANDLE RgnHandle) {
    RK_S32 i;
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stMppChn;

    stMppChn.enModId = RK_ID_VO;
    stMppChn.s32DevId = u32VoLayer;
    stMppChn.s32ChnId = u32Chn;

    s32Ret = RK_MPI_RGN_DetachFromChn(RgnHandle, &stMppChn);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_DetachFromChn (%d) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}


static RK_S32 wbc_init(MPI_CTX_S *ctx) {
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
        stDstChn.s32ChnId  = 0;

        s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("creat wbc bind vo failed!");
            return RK_FAILURE;
        }
    } else {
        s32Ret = creat_venc(ctx, ctx->stVencCfg.s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("create %d ch venc failed", ctx->stVencCfg.s32ChnId);
            return s32Ret;
        }
        stDstChn.enModId = RK_ID_VENC;
        stDstChn.s32DevId = 0;
        stDstChn.s32ChnId = ctx->stVencCfg.s32ChnId;

        s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE(" wbc bind venc failed");
            return s32Ret;
        }

        if (!ctx->stVencCfg.dstFilePath) {
            RK_S32 wbcVolayer = 0;
            RK_S32 wbcVoCh = 0;
            RK_S32 wbcVdecCh = 0;

            wbcVdecCh = ctx->u32Screen0Chn + ctx->u32Screen1Chn;
            if (ctx->stWbcCfg.stWbcSource.u32SourceId == RK356X_VO_DEV_HD1) {
                wbcVolayer =  ctx->stVoCfg.u32Screen0VoLayer;
                wbcVoCh = ctx->u32Screen0Chn;
            } else {
                wbcVolayer =  ctx->stVoCfg.u32Screen1VoLayer;
                wbcVoCh = ctx->u32Screen1Chn;
            }

            s32Ret = creat_wbc_vdec(ctx, wbcVdecCh);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("create %d ch wbc vdec failed", wbcVdecCh);
                return s32Ret;
            }
            ctx->stWbcCfg.s32VdecChnId = wbcVdecCh;
            RK_LOGD("create wbc vdec ch %d", wbcVdecCh);

            s32Ret = RK_MPI_VO_EnableChn(wbcVolayer, wbcVoCh);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("creat %d layer %d ch vo failed!", wbcVolayer, wbcVoCh);
                return s32Ret;
            }

            stSrcChn.enModId = RK_ID_VDEC;
            stSrcChn.s32DevId = 0;
            stSrcChn.s32ChnId = wbcVdecCh;
            RK_LOGD("wbc vdec ch %d", stSrcChn.s32ChnId);

            stDstChn.enModId = RK_ID_VO;
            stDstChn.s32DevId = wbcVolayer;
            stDstChn.s32ChnId = wbcVoCh;
            RK_LOGD("wbc vdec to vo layer %d ch %d ", stDstChn.s32DevId, stDstChn.s32ChnId);
            s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE(" wbc vdec bind vo failed");
                return s32Ret;
            }
        }
    }

    return RK_SUCCESS;
}

static RK_S32 wbc_deinit(MPI_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    MPP_CHN_S stSrcChn, stDstChn;

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
    } else {
        stSrcChn.enModId    = RK_ID_WBC;
        stSrcChn.s32DevId   = 0;
        stSrcChn.s32ChnId   = 0;

        stDstChn.enModId   = RK_ID_VENC;
        stDstChn.s32DevId  = 0;
        stDstChn.s32ChnId  = 0;

        s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = destory_wbc(0);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }

        s32Ret = RK_MPI_VENC_StopRecvFrame(0);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }

        s32Ret = RK_MPI_VENC_DestroyChn(0);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    return RK_SUCCESS;
}

static void* venc_get_stream_thread(void *pArgs) {
    prctl(PR_SET_NAME, "venc_get");

    MPI_CTX_S *ctx = reinterpret_cast<MPI_CTX_S *>(pArgs);
    RK_S32 s32Ret = RK_SUCCESS;
    VENC_STREAM_S stFrame;
    void *pData = RK_NULL;
    FILE *fp = RK_NULL;
    char name[256]  = {0};
    RK_U32 u32Ch = ctx->u32ChnIndex;

    memset(&stFrame, 0, sizeof(VENC_STREAM_S));

    if (ctx->stVencCfg.dstFilePath != RK_NULL) {
        mkdir(ctx->stVencCfg.dstFilePath, 0777);

        switch (ctx->stVencCfg.stAttr.stVencAttr.enType) {
            case RK_VIDEO_ID_AVC:
                snprintf(name, sizeof(name), "%s/venc_%d.h264", ctx->stVencCfg.dstFilePath, ctx->u32ChnIndex);
                break;
            case RK_VIDEO_ID_HEVC:
                snprintf(name, sizeof(name), "%s/venc_%d.h265", ctx->stVencCfg.dstFilePath, ctx->u32ChnIndex);
                break;
            default:
                snprintf(name, sizeof(name), "%s/venc_%d.bin", ctx->stVencCfg.dstFilePath, ctx->u32ChnIndex);
                break;
        }

        fp = fopen(name, "wb");
        if (fp == RK_NULL) {
            RK_LOGE("chn %d can't open file %s in get picture thread!\n", u32Ch, name);
            return RK_NULL;
        }
    }
    stFrame.pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));

    while (!ctx->bThreadExit) {
        s32Ret = RK_MPI_VENC_GetStream(u32Ch, &stFrame, -1);
        if (s32Ret >= 0) {
            // RK_LOGI("get one stream");
            if (ctx->stVencCfg.dstFilePath != RK_NULL) {
                pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
                fwrite(pData, 1, stFrame.pstPack->u32Len, fp);
                fflush(fp);
                RK_MPI_VENC_ReleaseStream(u32Ch, &stFrame);
            } else {
                wbc_send_stream(ctx->stWbcCfg.s32VdecChnId, stFrame.pstPack->pMbBlk,
                                stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS);
            }
        } else {
             if (ctx->bThreadExit) {
                break;
             }

             usleep(1000llu);
        }
    }

    if (stFrame.pstPack)
        free(stFrame.pstPack);

    if (fp)
        fclose(fp);

    return RK_NULL;
}

void* vo_ui_thread(void *pArgs) {
    prctl(PR_SET_NAME, "vo_ui");
    MPI_CTX_S *ctx = reinterpret_cast<MPI_CTX_S *>(pArgs);
    RK_S32 s32Ret = 0;

    VIDEO_FRAME_INFO_S *pstVFrame;
    RK_VOID *pMblk, *pMblk_G0, *pMblk_G1;
    RK_U32 count;

    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_BGRA5551, 0x1F, &pMblk_G0);
    Sample_VO_CreateGFXData(1920, 1080, RK_FMT_BGRA5551, 0x1F << 10, &pMblk_G1);

    pstVFrame = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));

    while (!ctx->bThreadExit) {
        for (RK_U32 i = 0; i < ctx->stVoCfg.u32UIChn; i++) {
            if (i == 0)
                pMblk = pMblk_G0;
            else if (i == 1)
                pMblk = pMblk_G1;
            else
                continue;
            pstVFrame->stVFrame.pMbBlk = RK_MPI_VO_CloneBuffer(pMblk);
            RK_MPI_VO_SendFrame(ctx->stVoCfg.u32UIVoLayer, i, pstVFrame, 0);
            usleep(33000llu);
        }
    }
    free(pstVFrame);
    RK_MPI_VO_DestroyGraphicsFrameBuffer(pMblk_G0);
    RK_MPI_VO_DestroyGraphicsFrameBuffer(pMblk_G1);

    return RK_NULL;
}

void* vdec_send_stream_thread(void *pArgs) {
    prctl(PR_SET_NAME, "vdec_send");

    MPI_CTX_S *ctx = reinterpret_cast<MPI_CTX_S *>(pArgs);
    RK_S32 s32Ret = 0;

    VDEC_STREAM_S stStream;
    MB_BLK buffer = RK_NULL;
    MB_EXT_CONFIG_S stMbExtConfig;
    AVPacket *avPacket = RK_NULL;
    RK_S32 s32LoopCount = ctx->s32LoopCount;
    RK_BOOL bEos = RK_FALSE;
    RK_BOOL bFindKeyFrame = RK_FALSE;

    RK_LOGD("%s in\n", __FUNCTION__);
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));
    memset(&stMbExtConfig, 0, sizeof(MB_EXT_CONFIG_S));

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

            bEos = RK_TRUE;
            bFindKeyFrame = RK_FALSE;
            continue;
        }

       if (ctx->stParserCfg.u32StreamIndex == avPacket->stream_index) {
            if (!bFindKeyFrame) {
                if (avPacket->flags & AV_PKT_FLAG_KEY == AV_PKT_FLAG_KEY) {
                    bFindKeyFrame = RK_TRUE;
                } else {
                    mpi_ffmpeg_free(avPacket);
                    continue;
                }
            }

            if (bEos) {
                RK_LOGD("entern send %d\n", ctx->u32ChnIndex);
                bEos = RK_FALSE;
            }

            stMbExtConfig.pFreeCB = mpi_ffmpeg_free;
            stMbExtConfig.pOpaque = avPacket;
            stMbExtConfig.pu8VirAddr = avPacket->data;
            stMbExtConfig.u64Size = avPacket->size;

            RK_MPI_SYS_CreateMB(&buffer, &stMbExtConfig);

            stStream.u64PTS = avPacket->pts;
            stStream.pMbBlk = buffer;
            stStream.u32Len = avPacket->size;
            stStream.bEndOfStream = RK_FALSE;
            stStream.bEndOfFrame = RK_FALSE;
            stStream.bBypassMbBlk = RK_TRUE;

__RETRY:
            s32Ret = RK_MPI_VDEC_SendStream(ctx->u32ChnIndex, &stStream, MAX_TIME_OUT_MS);
            if (s32Ret != RK_SUCCESS) {
                if (ctx->bThreadExit) {
                     RK_LOGE("failed to RK_MPI_VDEC_SendStream");
                     mpi_ffmpeg_free(avPacket);
                     RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
                     break;
                }
                usleep(10000llu);
                goto  __RETRY;
            }

            RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
        } else {
            mpi_ffmpeg_free(avPacket);
        }
    }


      if (ctx->stParserCfg.pstAvfc)
        avformat_close_input(&(ctx->stParserCfg.pstAvfc));

__FAILED:

    RK_LOGD("%s out\n", __FUNCTION__);
    return RK_NULL;
}


RK_S32 unit_mpi_vplay_test(MPI_CTX_S *ctx) {
    RK_U32  i, k;

    RK_S32 s32Ret = RK_SUCCESS;
    RK_U32 u32MaxCh = 0;
    RK_U32 u32step = 0;
    RK_S32 u32VpssGrap = 0;

    MPP_CHN_S stSrcChn, stDstChn;
    VPSS_CHN VpssChn[VPSS_MAX_CHN_NUM] = { VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3 };

    MPI_CTX_S mpiVdecCtx[VDEC_MAX_CHN_NUM];
    MPI_CTX_S mpiVencCtx[VENC_MAX_CHN_NUM];

    pthread_t voThread[2];
    MPI_CTX_S mpiVoUiCtx[2];

    pthread_t vdecSendThread[VDEC_MAX_CHN_NUM];
    pthread_t vdecGetThread[VDEC_MAX_CHN_NUM];

    MPI_CTX_S mpiVpssCtx;
    pthread_t VpssGetThread;

    pthread_t vencThread;
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

    if (ctx->bEnRgn) {
        s32Ret = create_rgn(ctx);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("create rgn failed!");
        }
    }

    for (i = 0; i < ctx->u32Screen0Chn; i++) {
        s32Ret = screen_init(ctx, ctx->stVoCfg.u32Screen0VoLayer, i, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("screen0_process failed!", i);
            continue;
        }

        if (i < 4 && ctx->bEnRgn)
            rgn_init(ctx, ctx->stVoCfg.u32Screen0VoLayer, i, 0);

        ctx->u32ChnIndex = i;
        memcpy(&(mpiVdecCtx[i]), ctx, sizeof(MPI_CTX_S));
        pthread_create(&vdecSendThread[u32VdecSendThreadNum], 0,
            vdec_send_stream_thread, reinterpret_cast<void *>(&mpiVdecCtx[i]));
        u32VdecSendThreadNum++;
    }

    for (i = ctx->u32Screen0Chn; i < u32MaxCh; i++) {
        s32Ret = screen_init(ctx, ctx->stVoCfg.u32Screen1VoLayer, i, i - ctx->u32Screen0Chn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vpss screen_init failed!", i);
            continue;
        }
        ctx->u32ChnIndex = i;
        memcpy(&(mpiVdecCtx[i]), ctx, sizeof(MPI_CTX_S));
        pthread_create(&vdecSendThread[u32VdecSendThreadNum], 0,
            vdec_send_stream_thread, reinterpret_cast<void *>(&mpiVdecCtx[i]));
        u32VdecSendThreadNum++;
    }

    if (ctx->bEnWbc) {
        s32Ret = wbc_init(ctx);
        if (s32Ret == RK_SUCCESS) {
            ctx->u32ChnIndex = 0;
            memcpy(&(mpiVencCtx[0]), ctx, sizeof(MPI_CTX_S));
            pthread_create(&vencThread, 0, venc_get_stream_thread, reinterpret_cast<void *>(&mpiVencCtx[0]));
        } else {
            RK_LOGE("wb process failed");
        }
    }

    if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_SPLIT) {
        ctx->u32ChnIndex = 0;
        ctx->stVoCfg.u32UIVoLayer = VoGfxLayer;
        memcpy(&(mpiVoUiCtx[0]), ctx, sizeof(MPI_CTX_S));
        pthread_create(&voThread[0], 0, vo_ui_thread, reinterpret_cast<void *>(&mpiVoUiCtx[0]));

        ctx->u32ChnIndex = 0;
        ctx->stVoCfg.u32UIVoLayer = VoGfxLayer_second;
        memcpy(&(mpiVoUiCtx[1]), ctx, sizeof(MPI_CTX_S));
        pthread_create(&voThread[1], 0, vo_ui_thread, reinterpret_cast<void *>(&mpiVoUiCtx[1]));
    }

    u32VdecSendThreadNum = 0;
    for (i = 0; i< ctx->u32Screen0Chn; i++) {
        pthread_join(vdecSendThread[u32VdecSendThreadNum], RK_NULL);
        u32VdecSendThreadNum++;

        if (i < 4 && ctx->bEnRgn)
            rgn_deinit(ctx, ctx->stVoCfg.u32Screen0VoLayer, i, 0);

        s32Ret = screen_deinit(ctx, ctx->stVoCfg.u32Screen0VoLayer, i, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("screen init failed! vplay %d chn %d", ctx->stVoCfg.u32Screen0VoLayer, i);
            continue;
        }
    }

    for (i = ctx->u32Screen0Chn; i < u32MaxCh; i++) {
        pthread_join(vdecSendThread[u32VdecSendThreadNum], RK_NULL);
        u32VdecSendThreadNum++;

        s32Ret = screen_deinit(ctx, ctx->stVoCfg.u32Screen1VoLayer, i, i - ctx->u32Screen0Chn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("screen init failed! vplay %d chn %d", ctx->stVoCfg.u32Screen0VoLayer, i);
            continue;
        }
    }

    if (ctx->bEnWbc) {
       mpiVencCtx[0].bThreadExit = RK_TRUE;
       wbc_deinit(ctx);
       pthread_join(vencThread, RK_NULL);
    }

    if (ctx->stVoCfg.enUIMode == VIDEO_UI_MODE_SPLIT) {
         mpiVoUiCtx[0].bThreadExit = RK_TRUE;
         pthread_join(voThread[0], RK_NULL);

         mpiVoUiCtx[1].bThreadExit = RK_TRUE;
         pthread_join(voThread[1], RK_NULL);
    }

    if (ctx->bEnRgn) {
        destory_rgn(ctx);
    }

    mpi_vo_deinit_sample(ctx);
    close_config(ctx);
    return RK_SUCCESS;
}

static RK_S32 check_options(const MPI_CTX_S *ctx) {
    if (ctx->cfgFileUri == RK_NULL) {
        RK_LOGE("illegal cfg file!");
        goto __FAILED;
    }

    if ((ctx->u32Screen0Chn + ctx->u32Screen1Chn) > VDEC_MAX_CHN_NUM) {
        RK_LOGE("illegal param, max ch is %d!", VDEC_MAX_CHN_NUM);
        goto __FAILED;
    }

    if (ctx->bEnVpss && ctx->stVpssCfg.u32VpssChnCnt > VPSS_MAX_CHN_NUM) {
         RK_LOGE("vpss illegal param, max grap %d, max output ch is %d!",
            VPSS_MAX_GRP_NUM, VPSS_MAX_CHN_NUM);
        goto __FAILED;
    }

    return RK_SUCCESS;

__FAILED:
    return RK_FAILURE;
}

static const char *const usages[] = {
    "./rk_mpi_vplay_test [-i CFG_PATH]...",
    RK_NULL,
};

void init_cfg(MPI_CTX_S *ctx) {
    RK_S32 i = 0;

    ctx->u32Screen0Resolution = VO_OUTPUT_1080P60;
    ctx->u32Screen1Resolution =  VO_OUTPUT_1024x768_60;
    ctx->u32Screen0Chn = 16;
    ctx->u32Screen1Chn = 4;
    ctx->bEnVpss = RK_FALSE;
    ctx->bEnWbc = RK_FALSE;
    ctx->bEnRgn = RK_FALSE;
    ctx->bEnWbcToVO = RK_FALSE;
    ctx->s32LoopCount = -1;

    ctx->stVoCfg.u32Screen0VoLayer = RK356X_VOP_LAYER_CLUSTER0;
    ctx->stVoCfg.u32Screen1VoLayer = RK356X_VOP_LAYER_CLUSTER1;
    ctx->stVoCfg.u32Screen0DisplayWidth = 1920;
    ctx->stVoCfg.u32Screen0DisplayHeight = 1080;
    ctx->stVoCfg.u32Screen1DisplayWidth = 1024;
    ctx->stVoCfg.u32Screen1DisplayHeight = 768;
    ctx->stVoCfg.u32Screen0Rows = 4;
    ctx->stVoCfg.u32Screen1Rows = 3;
    ctx->stVoCfg.bDoubleScreen  = RK_TRUE;
    ctx->stVoCfg.bEnUI = RK_FALSE;
    ctx->stVoCfg.enUIMode = VIDEO_UI_MODE_NONE;
    ctx->stVoCfg.u32UIChn = 2;

    ctx->stVdecCfg.u32FrameBufferCnt = MAX_FRAME_QUEUE;
    ctx->stVdecCfg.enCompressMode = COMPRESS_AFBC_16x16;

    ctx->stVpssCfg.u32VpssChnCnt = 1;
    ctx->stVpssCfg.stGrpVpssAttr.u32MaxW = 4096;
    ctx->stVpssCfg.stGrpVpssAttr.u32MaxH = 4096;
    ctx->stVpssCfg.stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;
    ctx->stVpssCfg.stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;
    ctx->stVpssCfg.stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;
    ctx->stVpssCfg.stGrpVpssAttr.enCompressMode = COMPRESS_AFBC_16x16;

    for (i = 0; i < VPSS_MAX_CHN_NUM; i ++) {
        ctx->stVpssCfg.stVpssChnAttr[i].enChnMode = VPSS_CHN_MODE_PASSTHROUGH;
        ctx->stVpssCfg.stVpssChnAttr[i].enDynamicRange = DYNAMIC_RANGE_SDR8;
        ctx->stVpssCfg.stVpssChnAttr[i].enPixelFormat = RK_FMT_YUV420SP;
        ctx->stVpssCfg.stVpssChnAttr[i].stFrameRate.s32SrcFrameRate = -1;
        ctx->stVpssCfg.stVpssChnAttr[i].stFrameRate.s32DstFrameRate = -1;
        ctx->stVpssCfg.stVpssChnAttr[i].u32Width = 704;
        ctx->stVpssCfg.stVpssChnAttr[i].u32Height = 576;
        ctx->stVpssCfg.stVpssChnAttr[i].enCompressMode = COMPRESS_MODE_NONE;
    }

    ctx->stWbcCfg.stWbcSource.enSourceType = VO_WBC_SOURCE_VIDEO;
    ctx->stWbcCfg.stWbcSource.u32SourceId = RK356X_VO_DEV_HD1;
    ctx->stWbcCfg.stWbcAttr.enPixelFormat = RK_FMT_RGB888;
    ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Width = 1280;
    ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Height = 720;
    ctx->stWbcCfg.stWbcAttr.u32FrameRate = 25;
    ctx->stWbcCfg.stWbcAttr.enCompressMode = COMPRESS_MODE_NONE;
    ctx->stWbcCfg.s32ChnId = 0;
    ctx->stWbcCfg.s32VdecChnId = -1;

    ctx->stVencCfg.stAttr.stVencAttr.enType = RK_VIDEO_ID_AVC;
    ctx->stVencCfg.s32ChnId = 0;
    ctx->stVencCfg.stAttr.stVencAttr.enPixelFormat = ctx->stWbcCfg.stWbcAttr.enPixelFormat;
    ctx->stVencCfg.stAttr.stVencAttr.u32PicWidth = ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Width;
    ctx->stVencCfg.stAttr.stVencAttr.u32PicHeight = ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Height;
    ctx->stVencCfg.stAttr.stVencAttr.u32VirWidth = ctx->stVencCfg.stAttr.stVencAttr.u32PicWidth;
    ctx->stVencCfg.stAttr.stVencAttr.u32VirHeight = ctx->stVencCfg.stAttr.stVencAttr.u32PicHeight;
    ctx->stVencCfg.stAttr.stVencAttr.u32BufSize =
        ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Width * ctx->stWbcCfg.stWbcAttr.stTargetSize.u32Height;

    ctx->stRgnCfg.stRgnAttr.enType = OVERLAY_RGN;
    ctx->stRgnCfg.stRgnAttr.unAttr.stOverlay.enPixelFmt = RK_FMT_BGRA5551;
    ctx->stRgnCfg.stRgnAttr.unAttr.stOverlay.stSize.u32Width = 128;
    ctx->stRgnCfg.stRgnAttr.unAttr.stOverlay.stSize.u32Height = 96;

    ctx->stRgnCfg.stRgnChnAttr.bShow = RK_TRUE;
    ctx->stRgnCfg.stRgnChnAttr.enType = OVERLAY_RGN;
    ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
    ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
    ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 128;
    ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
    ctx->stRgnCfg.stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
}

RK_S32 main(RK_S32 argc, const char **argv) {
    MPI_CTX_S ctx;
    memset(&ctx, 0, sizeof(MPI_CTX_S));

    init_cfg(&ctx);

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input",  &(ctx.cfgFileUri),
                   "input config file. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_resolution", &(ctx.u32Screen0Resolution),
                    "screen0 resolution. default(10)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_resolution", &(ctx.u32Screen1Resolution),
                    "screen1 resolution default(14)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_disWidth", &(ctx.stVoCfg.u32Screen0DisplayWidth),
                    "screen0 display width", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_disHeight", &(ctx.stVoCfg.u32Screen0DisplayHeight),
                    "screen0 display height", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_disWidth", &(ctx.stVoCfg.u32Screen1DisplayWidth),
                    "screen1 display width", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_disHeight", &(ctx.stVoCfg.u32Screen1DisplayHeight),
                    "screen1 display height", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_chn", &(ctx.u32Screen0Chn),
                    "the channel num of screen0. default(16)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_chn", &(ctx.u32Screen1Chn),
                    "the channel num of screen1 default(4)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen0_rows", &(ctx.stVoCfg.u32Screen0Rows),
                    "the rows/cols of screen0 display. default(4: 4x4)", NULL, 0, 0),
        OPT_INTEGER('\0', "screen1_rows", &(ctx.stVoCfg.u32Screen1Rows),
                    "the rows/cols of screen1 display.default(3 : 3x3)", NULL, 0, 0),
        OPT_INTEGER('\0', "loop_count", &(ctx.s32LoopCount),
                    "loop running count. default(-1:infinite)", NULL, 0, 0),
        OPT_INTEGER('\0', "compress_mode", &(ctx.stVdecCfg.enCompressMode),
                     "vdec compress mode. default(1); 0: NONE, 1: AFBC_16X16", NULL, 0, 0),
        OPT_INTEGER('\0', "dec_buf_cnt", &(ctx.stVdecCfg.u32FrameBufferCnt),
                    "vdec decode buffer count. default(8)", NULL, 0, 0),
        OPT_INTEGER('\0', "en_vpss", &(ctx.bEnVpss),
                    "enable vpss. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "vpss_chn_cnt", &(ctx.stVpssCfg.u32VpssChnCnt),
                    "enable vpss channel count. default(1)", NULL, 0, 0),
        OPT_INTEGER('\0', "en_wbc", &(ctx.bEnWbc),
                    "enable wbc. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "wbc_src",  &(ctx.stWbcCfg.stWbcSource.u32SourceId),
                   "the source of wbc, default(1)", NULL, 0, 0),
        OPT_STRING('\0', "venc_output",  &(ctx.stVencCfg.dstFilePath),
                   "the directory of encoder output", NULL, 0, 0),
        OPT_INTEGER('\0', "double_screen",  &(ctx.stVoCfg.bDoubleScreen),
                   "double screen or not, default(1): 0: RK_FALSE, 1: RK_TRUE", NULL, 0, 0),
        OPT_INTEGER('\0', "en_rgn", &(ctx.bEnRgn),
                    "enable rgn. default(0)", NULL, 0, 0),
        OPT_STRING('\0', "input_bmp",  &(ctx.stRgnCfg.srcFileBmpUri),
                   "rgn input file", NULL, 0, 0),
        OPT_INTEGER('\0', "en_ui", &(ctx.stVoCfg.bEnUI),
                   "enable ui. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "en_ui_mode", &(ctx.stVoCfg.enUIMode),
                   "enable ui mode. default(0): 1: VIDEO_UI_MERGE, 2: VIDEO_UI_SPLIT", NULL, 0, 0),
        OPT_INTEGER('\0', "en_wbctovo", &(ctx.bEnWbcToVO),
                   "enable wbc to vo. default(0)", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    if (check_options(&ctx)) {
        RK_LOGE("illegal input parameters");
        argparse_usage(&argparse);
        goto __FAILED;
    }

    // Compatible with old switch options
    if (ctx.stVoCfg.bEnUI && ctx.stVoCfg.enUIMode == VIDEO_UI_MODE_NONE) {
        ctx.stVoCfg.enUIMode = VIDEO_UI_MODE_MERGE;
    }

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        goto __FAILED;
    }

    if (unit_mpi_vplay_test(&ctx) < 0) {
        goto __FAILED;
    }

    if (RK_MPI_SYS_Exit() != RK_SUCCESS) {
        goto __FAILED;
    }

    RK_LOGE("test running success!");
    return RK_SUCCESS;
__FAILED:
    RK_LOGE("test running failed!");
    return RK_FAILURE;
}

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include "rk_defines.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_vdec.h"
#include "rk_type.h"
#include "rk_mpi_rgn.h"
#include "rk_mpi_cal.h"
#include "rk_comm_vo.h"

//VI_CHN_ATTR_S stChnAttr;

#define TEST_VENC_MAX 2
#define MAX_FRAME_QUEUE 8
#define MAX_TIME_OUT_MS 20

typedef struct _rkWbcCfg
{
    VO_WBC_SOURCE_S stWbcSource;
    VO_WBC_ATTR_S stWbcAttr;
    RK_S32 s32ChnId;
    RK_S32 s32VdecChnId;
} WBC_CFG;

typedef struct _rkVdecCfg
{
    RK_U32 u32FrameBufferCnt;
    COMPRESS_MODE_E enCompressMode;
} VDEC_CFG;

typedef struct _rkVOCfg
{
    RK_U32 u32Screen0VoLayer;
    RK_U32 u32Screen1VoLayer;

    RK_U32 u32Screen0Rows;
    RK_U32 u32Screen1Rows;
    RK_BOOL bDoubleScreen;
} VO_CFG;
/*
typedef struct _rkParserCfg {
    char *srcFileUri[VDEC_MAX_CHN_NUM];
    RK_CODEC_ID_E enCodecId;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_U32 u32StreamIndex;
    AVFormatContext *pstAvfc;
} PARSER_CFG;
*/
typedef struct _rkBorderCfg
{
    RK_U32 u32LeftWidth;
    RK_U32 u32RightWidth;
    RK_U32 u32TopWidth;
    RK_U32 u32BottomWidth;
} Border_CFG;

#define RK356X_VOP_LAYER_CLUSTER_0 0
#define RK356X_VOP_LAYER_CLUSTER_1 2
#define RK356X_VOP_LAYER_ESMART_0 4
#define RK356X_VOP_LAYER_ESMART_1 5
#define RK356X_VOP_LAYER_SMART_0 6
#define RK356X_VOP_LAYER_SMART_1 7

#define RK356X_VO_DEV_HD0 0
#define RK356X_VO_DEV_HD1 1

#define DISPLAY_TYPE_HDMI 0
#define DISPLAY_TYPE_EDP 1
#define DISPLAY_TYPE_VGA 2

#define MAX_VO_FORMAT_RGB_NUM 4
#define TEST_VO_FORMAT_ARGB8888 0
#define TEST_VO_FORMAT_ABGR8888 1
#define TEST_VO_FORMAT_RGB888 2
#define TEST_VO_FORMAT_BGR888 3
#define TEST_VO_FORMAT_ARGB1555 4
#define TEST_VO_FORMAT_ABGR1555 5
#define TEST_VO_FORMAT_NV12 6
#define TEST_VO_FORMAT_NV21 7

#define VO_CHANNEL_PLAY_NORMAL 0
#define VO_CHANNEL_PLAY_PAUSE 1
#define VO_CHANNEL_PLAY_STEP 2
#define VO_CHANNEL_PLAY_SPEED 3

#define WBC_SOURCE_DEV 0
#define WBC_SOURCE_VIDEO 1
#define WBC_SOURCE_GRAPHIC 2

#define MAX_WINDOWS_NUM 64
#define MAX_STEP_FRAME_NUM 50

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a)[0])

typedef struct _TEST_MODE
{
    RK_U32 mode;
    VO_INTF_SYNC_E enIntfSync;
} TEST_MODE_S;

//static VO_DEV VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
//static VO_DEV VoLayer_second = RK356X_VOP_LAYER_CLUSTER_1;

TEST_MODE_S test_mode_table[] = {
    {0, VO_OUTPUT_640x480_60},
    {1, VO_OUTPUT_NTSC},
    {2, VO_OUTPUT_PAL},
    {3, VO_OUTPUT_480P60},
    {4, VO_OUTPUT_576P50},
    {5, VO_OUTPUT_800x600_60},
    {6, VO_OUTPUT_1024x768_60},
    {7, VO_OUTPUT_720P50},
    {8, VO_OUTPUT_720P60},
    {9, VO_OUTPUT_1280x800_60},
    {10, VO_OUTPUT_1280x1024_60},
    {11, VO_OUTPUT_1366x768_60},
    {12, VO_OUTPUT_1440x900_60},
    {13, VO_OUTPUT_1600x1200_60},
    {14, VO_OUTPUT_1680x1050_60},
    {15, VO_OUTPUT_1080I50},
    {16, VO_OUTPUT_1080I60},
    {17, VO_OUTPUT_1080P50},
    {18, VO_OUTPUT_1080P60},
    {19, VO_OUTPUT_1920x1200_60},
    {20, VO_OUTPUT_3840x2160_24},
    {21, VO_OUTPUT_3840x2160_25},
    {22, VO_OUTPUT_3840x2160_30},
    {23, VO_OUTPUT_3840x2160_50},
    {24, VO_OUTPUT_3840x2160_60},
};

#define Sample_Print(format, ...) RK_LOGI(format, ##__VA_ARGS__)

typedef enum rkTestVIMODE_E
{
    TEST_VI_MODE_VI_ONLY = 0,
    TEST_VI_MODE_BIND_VENC = 1,
    TEST_VI_MODE_BIND_VENC_MULTI = 2,
} TEST_VI_MODE_E;

typedef struct _rkTestVencCfg
{
    RK_BOOL bOutDebugCfg;
    VENC_CHN_ATTR_S stAttr;
    RK_CHAR dstFilePath[128];
    RK_CHAR dstFileName[128];
    RK_S32 s32ChnId;
    FILE *fp;
} TEST_VENC_CFG;

typedef struct _rkMpiVICtx
{
    RK_S32 width;
    RK_S32 height;
    RK_S32 devId;
    RK_S32 pipeId;
    RK_S32 channelId;
    RK_S32 loopCountSet;
    VI_DEV_ATTR_S stDevAttr;
    VI_DEV_BIND_PIPE_S stBindPipe;
    VI_CHN_ATTR_S stChnAttr;
    VI_SAVE_FILE_INFO_S stDebugFile;
    VI_FRAME_S stViFrame;
    VI_CHN_STATUS_S stChnStatus;
    TEST_VI_MODE_E enMode;
    const char *aEntityName;
    // for venc
    // TEST_VENC_CFG stVencCfg[TEST_VENC_MAX];
    // VENC_STREAM_S stFrame[TEST_VENC_MAX];
} TEST_VI_CTX_S;

typedef struct _rkMpiVOCtx
{
    RK_S32 VoDev;
    RK_S32 VoLayer;
    RK_S32 VoLayerMode;

    RK_S32 u32Windows;
    RK_U32 enIntfType; /* 0 HDMI 1 edp */
    RK_U32 enIntfSync; /* VO_OUTPUT_1080P50 */
    RK_U32 s32X;
    RK_U32 s32Y;
    RK_U32 u32DispWidth;
    RK_U32 u32DispHeight;
    RK_U32 u32ImgeWidth;
    RK_U32 u32ImageHeight;
    RK_S32 s32PixFormat;
    RK_U32 u32DispFrmRt;
    RK_U32 u32DispFrmRtRatio;
    RK_U32 uEnMode;

    RK_U32 wbc_enable;
    RK_BOOL wbc_auto;
    RK_U32 ui;
    RK_U32 ui_alpha;
    RK_U32 u32WbcWidth;
    RK_S32 u32WbcHeight;
    COMPRESS_MODE_E u32WbcCompressMode;
    RK_S32 u32WbcPixFormat;
    RK_U32 u32WbcSourceType;
    RK_U32 u32WbcSourceId;

    VO_LAYER VoVideoLayer;
    RK_U32 u32VideoWindows;
    RK_U32 u32GFXLayers;
    VO_LAYER VOGfxLayer;
    RK_BOOL threadExit;

    RK_BOOL bVoPlay;
    const char *cfgFileUri;
    RK_S32 s32LoopCount;
    RK_U32 u32ChnIndex;
    RK_U32 u32Screen0Chn;
    RK_U32 u32ChnDismode;
    RK_U32 u32Screen1Chn;
    RK_BOOL bThreadExit;

    RK_BOOL bEnWbc;
    RK_BOOL bEnWbcToVO;
    RK_BOOL bChnPriority;

    //   PARSER_CFG  stParserCfg;
    VDEC_CFG stVdecCfg;
    VO_CFG stVoCfg;
    WBC_CFG stWbcCfg;
    Border_CFG stBorderCfg;
} TEST_VO_CTX_S;

static PIXEL_FORMAT_E Sample_vo_test_fmt_to_rtfmt(RK_S32 format)
{
    switch (format)
    {
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

static RK_S32 Sample_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, pstLayerAttr);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    return s32Ret;
}

static RK_S32 test_vi_init(TEST_VI_CTX_S *ctx)
{

    MPP_CHN_S stSrcChn, stDestChn;
    RK_S32 loopCount = 0;
    void *pData = RK_NULL;
    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 i;

    s32Ret = RK_MPI_VI_GetDevAttr(ctx->devId, &ctx->stDevAttr);
    if (s32Ret == RK_ERR_VI_NOT_CONFIG)
    {
        // 0-1.config dev
        s32Ret = RK_MPI_VI_SetDevAttr(ctx->devId, &ctx->stDevAttr);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_VI_SetDevAttr %x", s32Ret);
        }
        else
        {
            RK_LOGE("RK_MPI_VI_SetDevAttr good");
        }
    }
    else
    {
        RK_LOGE("RK_MPI_VI_SetDevAttr already");
    }

    s32Ret = RK_MPI_VI_GetDevIsEnable(ctx->devId);
    if (s32Ret != RK_SUCCESS)
    {
        // 1-2.enable dev
        s32Ret = RK_MPI_VI_EnableDev(ctx->devId);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_VI_EnableDev %x", s32Ret);
        }
        else
        {
            RK_LOGE("RK_MPI_VI_EnableDev already");
        }
        // 1-3.bind dev/pipe
        ctx->stBindPipe.u32Num = ctx->pipeId;
        ctx->stBindPipe.PipeId[0] = ctx->pipeId;
        s32Ret = RK_MPI_VI_SetDevBindPipe(ctx->devId, &ctx->stBindPipe);
        if (s32Ret != RK_SUCCESS)
        {
            RK_LOGE("RK_MPI_VI_SetDevBindPipe %x", s32Ret);
            goto __FAILED;
        }
        else
        {
            RK_LOGE("RK_MPI_VI_SetDevBindPipe good");
        }
    }
    else
    {
        RK_LOGE("RK_MPI_VI_EnableDev already");
    }

    ctx->stChnAttr.stSize.u32Width = ctx->width;
    ctx->stChnAttr.stSize.u32Height = ctx->height;
    s32Ret = RK_MPI_VI_SetChnAttr(ctx->pipeId, ctx->channelId, &ctx->stChnAttr);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VI_SetChnAttr %x", s32Ret);
        goto __FAILED;
    }
    else
    {
        RK_LOGE("RK_MPI_VI_SetChnAttr good");
    }
    // 3.enable channel
    s32Ret = RK_MPI_VI_EnableChn(ctx->pipeId, ctx->channelId);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("RK_MPI_VI_EnableChn %x", s32Ret);
        goto __FAILED;
    }
    else
    {
        RK_LOGE("RK_MPI_VI_EnableChn good");
    }

    return s32Ret;

__FAILED:
    RK_MPI_VI_DisableDev(ctx->devId);
}

static RK_S32 VO_Disable_test(VO_LAYER VoLayer, VO_DEV VoDev)
{
}

static RK_S32 VO_Enable_test()
{
    VO_PUB_ATTR_S VoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    RK_U32 s32Ret;
    VO_CHN_ATTR_S stChnAttr;

    VO_LAYER VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
    VO_DEV VoDev = RK356X_VO_DEV_HD0;

    // unint first
    RK_MPI_VO_DisableLayer(VoLayer);
    RK_MPI_VO_DisableLayer(2);
    RK_MPI_VO_DisableLayer(4);
    RK_MPI_VO_DisableLayer(5);
    RK_MPI_VO_DisableLayer(6);
    RK_MPI_VO_Disable(VoDev);
    RK_LOGE("[%s] unint VO config first\n", __func__);

    memset(&VoPubAttr, 0, sizeof(VO_PUB_ATTR_S));
    memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));

    //stLayerAttr.enPixFormat                  = RK_FMT_BGRA8888;
    stLayerAttr.enPixFormat = RK_FMT_YUV420SP;
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stDispRect.u32Width = 1920;
    stLayerAttr.stDispRect.u32Height = 1080;
    stLayerAttr.stImageSize.u32Width = 1920;
    stLayerAttr.stImageSize.u32Height = 1080;
    stLayerAttr.u32DispFrmRt = 30;

    s32Ret = RK_MPI_VO_GetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    VoPubAttr.enIntfType = VO_INTF_HDMI;
    VoPubAttr.enIntfSync = VO_OUTPUT_1080P60;

    s32Ret = RK_MPI_VO_SetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_Enable(VoDev);
    if (s32Ret != RK_SUCCESS)
    {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;
    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS)
        return RK_FAILURE;

    stChnAttr.stRect.s32X = 0;
    stChnAttr.stRect.s32Y = 0;
    stChnAttr.stRect.u32Width = stLayerAttr.stImageSize.u32Width;
    stChnAttr.stRect.u32Height = stLayerAttr.stImageSize.u32Height;
    stChnAttr.u32Priority = 0;
    if (1)
    {
        stChnAttr.u32FgAlpha = 128;
        stChnAttr.u32BgAlpha = 0;
    }
    else
    {
        stChnAttr.u32FgAlpha = 0;
        stChnAttr.u32BgAlpha = 128;
    }

    s32Ret = RK_MPI_VO_SetChnAttr(VoLayer, 0, &stChnAttr);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("[%s] set chn Attr failed,s32Ret:%d\n", __func__, s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_EnableChn(VoLayer, 0);
    if (s32Ret != RK_SUCCESS)
    {
        RK_LOGE("[%s] Enalbe chn failed,s32Ret:%d\n", __func__, s32Ret);
        return RK_FAILURE;
    }
}

int main()
{
    MPP_CHN_S stSrcChn, stDstChn;
    RK_U32 s32Ret;

    TEST_VI_CTX_S *ctx;
    TEST_VO_CTX_S voctx;
    memset(&voctx, 0, sizeof(TEST_VO_CTX_S));
    ctx = (TEST_VI_CTX_S *)(malloc(sizeof(TEST_VI_CTX_S)));
    memset(ctx, 0, sizeof(TEST_VI_CTX_S));

    ctx->width = 1920;
    ctx->height = 1080;
    ctx->devId = 0;
    ctx->pipeId = ctx->devId;
    ctx->channelId = 0;
    ctx->loopCountSet = 100;
    //    ctx->enMode = TEST_VI_MODE_BIND_VENC;
    ctx->stChnAttr.stIspOpt.u32BufCount = 3;
    ctx->stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
    ctx->aEntityName = "/dev/video1";

    if (ctx->aEntityName != RK_NULL)
        memcpy(ctx->stChnAttr.stIspOpt.aEntityName, ctx->aEntityName, strlen(ctx->aEntityName));
    //  stChnAttr.stSize.u32Width = 1920;
    //  stChnAttr.stSize.u32Height = 1080;
    printf("sys init\n");
    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS)
    {
        printf("sys init failed\n");
        return s32Ret;
    }
    else
    {
        printf("RK MPI INIT init succed\n");
    }

    test_vi_init(ctx);

    VO_Enable_test();

    stSrcChn.enModId = RK_ID_VI;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDstChn.enModId = RK_ID_VO;
    stDstChn.s32DevId = 0;
    stDstChn.s32ChnId = 0;

    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
    if (s32Ret != RK_SUCCESS)
    {
        printf("RK_MPI_SYS_Bind failed %x", s32Ret);
        goto __FAILED2;
    }

    printf(" initial finish\n");
    while (getchar() != 'q')
    {
        usleep(1000 * 1000);
    }
end:
    RK_MPI_VO_DisableChn(RK356X_VOP_LAYER_CLUSTER_0, 0);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_CLUSTER_0);
    RK_MPI_VO_Disable(RK356X_VO_DEV_HD0);
    RK_MPI_VO_CloseFd();

__FAILED2:
    RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
__FAILED1:
    RK_MPI_VI_DisableChn(0, 1);
    return 0;
}

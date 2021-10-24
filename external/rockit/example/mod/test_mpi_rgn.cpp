/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "rk_debug.h"
#include "rk_type.h"
#include "rk_mpi_rgn.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_cal.h"

#include "loadbmp.h"
#include "argparse.h"
#include "mpi_test_utils.h"

#define MAX_TIME_OUT_MS              20

typedef struct _rkTestRGNCtx {
    const char *srcFileRawName;
    const char *srcFileBmpName;
    const char *dstFilePath;
    RK_S32      s32Operation;
    RK_S32      s32LoopCount;
    RK_S32      s32RgnCount;
    RECT_S      stRegion;
    RK_U32      u32RawWidth;
    RK_U32      u32RawHeight;
    RK_U32      u32BmpFormat;
    pthread_t   vencSendFrameTid;
    pthread_t   vencGetStreamTid;
} TEST_RGN_CTX_S;

void* venc_get_stream(void *pArgs) {
    TEST_RGN_CTX_S  *ctx        = reinterpret_cast<TEST_RGN_CTX_S *>(pArgs);
    void            *pData      = RK_NULL;
    RK_S32           s32Ret     = RK_SUCCESS;
    FILE            *fp         = RK_NULL;
    char             name[256]  = {0};
    RK_S32           s32StreamCnt = 0;
    VENC_STREAM_S    stFrame;

    if (ctx->dstFilePath) {
        snprintf(name, sizeof(name), "%srgn_venc_out.h264", ctx->dstFilePath);
        fp = fopen(name, "wb");
        if (fp == RK_NULL) {
            RK_LOGE("chn %d can't open file %s in get picture thread!\n", 0, name);
            return RK_NULL;
        }
    }
    stFrame.pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));

    while (true) {
        s32Ret = RK_MPI_VENC_GetStream(0, &stFrame, -1);
        if (s32Ret >= 0) {
            if (stFrame.pstPack->bStreamEnd == RK_TRUE) {
                RK_LOGI("chn %d reach EOS stream", 0);
                RK_MPI_VENC_ReleaseStream(0, &stFrame);
                break;
            }
            s32StreamCnt++;
            RK_LOGI("get chn %d stream %d", 0, s32StreamCnt);
            if (ctx->dstFilePath != RK_NULL) {
                pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
                fwrite(pData, 1, stFrame.pstPack->u32Len, fp);
                fflush(fp);
            }
            RK_MPI_VENC_ReleaseStream(0, &stFrame);
        } else {
             usleep(1000llu);
        }
    }

    if (stFrame.pstPack)
        free(stFrame.pstPack);

    if (fp)
        fclose(fp);

    return RK_NULL;
}

void* venc_send_frame(void *pArgs) {
    TEST_RGN_CTX_S      *ctx            = reinterpret_cast<TEST_RGN_CTX_S *>(pArgs);
    RK_S32               s32Ret         = RK_SUCCESS;
    RK_S8               *pData          = RK_NULL;
    RK_S8               *pVirAddr       = RK_NULL;
    RK_U32               u32SrcSize     = 0;
    FILE                *fp             = RK_NULL;
    MB_BLK               blk            = RK_NULL;
    MB_POOL              pool           = MB_INVALID_POOLID;
    RK_U32               u32BufferSize  = 0;
    RK_S32               s32FrameCount  = 0;
    RK_S32               s32ReachEOS    = 0;
    VIDEO_FRAME_INFO_S   stFrame;
    MB_EXT_CONFIG_S      stMbExtConfig;
    MB_POOL_CONFIG_S     stMbPoolCfg;
    PIC_BUF_ATTR_S       stPicBufAttr;
    MB_PIC_CAL_S         stMbPicCalResult;

    fp = fopen(ctx->srcFileRawName, "r");
    if (fp == RK_NULL) {
        RK_LOGE("chn %d can't open file %s!\n", 0, ctx->srcFileRawName);
        return RK_NULL;
    }

    stPicBufAttr.u32Width = ctx->u32RawWidth;
    stPicBufAttr.u32Height = ctx->u32RawHeight;
    stPicBufAttr.enCompMode = COMPRESS_MODE_NONE;
    stPicBufAttr.enPixelFormat = RK_FMT_YUV420SP;
    s32Ret = RK_MPI_CAL_COMM_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
        return RK_NULL;
    }
    u32BufferSize = stMbPicCalResult.u32MBSize;

    memset(&stMbPoolCfg, 0, sizeof(MB_POOL_CONFIG_S));
    stMbPoolCfg.u64MBSize = u32BufferSize;
    stMbPoolCfg.u32MBCnt  = 10;
    stMbPoolCfg.enAllocType = MB_ALLOC_TYPE_DMA;

    pool = RK_MPI_MB_CreatePool(&stMbPoolCfg);

    usleep(500 * 1000);

    while (true) {
        pData = reinterpret_cast<RK_S8 *>(calloc(1, u32BufferSize));

        u32SrcSize = fread(pData, 1, u32BufferSize, fp);
        if (u32SrcSize <= 0) {
            s32ReachEOS = 1;
        }
        blk = RK_MPI_MB_GetMB(pool, u32SrcSize, RK_TRUE);
        pVirAddr = reinterpret_cast<RK_S8 *>(RK_MPI_MB_Handle2VirAddr(blk));

        memcpy(pVirAddr, pData, u32SrcSize);
        stFrame.stVFrame.pMbBlk = blk;
        stFrame.stVFrame.u32Width = ctx->u32RawWidth;
        stFrame.stVFrame.u32Height = ctx->u32RawHeight;
        stFrame.stVFrame.u32VirWidth = ctx->u32RawWidth;
        stFrame.stVFrame.u32VirHeight = ctx->u32RawHeight;
        stFrame.stVFrame.enPixelFormat = RK_FMT_YUV420SP;
        stFrame.stVFrame.u32FrameFlag |= s32ReachEOS ? FRAME_FLAG_SNAP_END : 0;
__RETRY:
        s32Ret = RK_MPI_VENC_SendFrame(0, &stFrame, MAX_TIME_OUT_MS);
        if (s32Ret < 0) {
            usleep(50000llu);
            goto  __RETRY;
        } else {
            RK_MPI_MB_ReleaseMB(blk);
            s32FrameCount++;
            RK_LOGI("chn %d frame %d", 0, s32FrameCount);
        }
        usleep(50000llu);
        if (pData) {
            free(pData);
        }
        if (s32ReachEOS) {
            RK_LOGI("chn %d reach EOS.", 0);
            break;
        }
    }

    if (fp)
        fclose(fp);

    RK_MPI_MB_DestroyPool(pool);

    return RK_NULL;
}

static RK_S32 test_venc_start(VENC_CHN VencChn, TEST_RGN_CTX_S *pstCtx) {
    RK_S32                  s32Ret = RK_SUCCESS;
    VENC_CHN_ATTR_S         stAttr;
    VENC_RECV_PIC_PARAM_S   stRecvParam;

    memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));
    memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));

    stAttr.stVencAttr.enType = RK_VIDEO_ID_AVC;
    stAttr.stVencAttr.u32Profile = H264E_PROFILE_HIGH;
    stAttr.stVencAttr.u32PicWidth = pstCtx->u32RawWidth;
    stAttr.stVencAttr.u32PicHeight = pstCtx->u32RawHeight;
    stAttr.stVencAttr.u32VirWidth = pstCtx->u32RawWidth;
    stAttr.stVencAttr.u32VirHeight = pstCtx->u32RawHeight;
    stAttr.stVencAttr.u32StreamBufCnt = 10;
    stAttr.stVencAttr.u32BufSize = pstCtx->u32RawWidth * pstCtx->u32RawHeight * 2;
    s32Ret = RK_MPI_VENC_CreateChn(VencChn, &stAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VENC_StartRecvFrame(VencChn, &stRecvParam);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    pthread_create(&pstCtx->vencSendFrameTid, 0, venc_send_frame, reinterpret_cast<void *>(pstCtx));
    pthread_create(&pstCtx->vencGetStreamTid, 0, venc_get_stream, reinterpret_cast<void *>(pstCtx));

    return s32Ret;
}

static RK_S32 test_venc_stop(VENC_CHN VencChn, TEST_RGN_CTX_S *pstCtx) {
    RK_S32 s32Ret = RK_SUCCESS;

    pthread_join(pstCtx->vencSendFrameTid, RK_NULL);
    RK_MPI_VENC_StopRecvFrame(VencChn);
    RK_MPI_VENC_DestroyChn(VencChn);
    pthread_join(pstCtx->vencGetStreamTid, RK_NULL);

    return s32Ret;
}

RK_S32 test_rgn_chg_position(RGN_HANDLE RgnHandle, VENC_CHN VencChn, POINT_S *pstPoint) {
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RK_S32 s32Ret;

    stChn.enModId = RK_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VencChn;

    if (NULL == pstPoint) {
        RK_LOGE("input parameter is null. it is invaild!");
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_RGN_GetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstPoint->s32X;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstPoint->s32Y;
    s32Ret = RK_MPI_RGN_SetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

/******************************************************************************
 * funciton : osd region show or hide
 ******************************************************************************/
RK_S32 test_rgn_show_or_hide(RGN_HANDLE RgnHandle, VENC_CHN VencChn, RK_BOOL bShow) {
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RK_S32 s32Ret;

    stChn.enModId = RK_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VencChn;

    s32Ret = RK_MPI_RGN_GetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    stChnAttr.bShow = bShow;

    s32Ret = RK_MPI_RGN_SetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 test_rgn_load_bmp(const RK_CHAR *filename, BITMAP_S *pstBitmap, TEST_RGN_CTX_S *pstRgnCtx) {
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if (get_bmp_info(filename, &bmpFileHeader, &bmpInfo) < 0) {
        RK_LOGE("GetBmpInfo err!\n");
        return RK_FAILURE;
    }

    switch (pstRgnCtx->u32BmpFormat) {
      case RK_FMT_ARGB8888:
        Surface.enColorFmt = OSD_COLOR_FMT_ARGB8888;
      break;
      case RK_FMT_BGRA8888:
        Surface.enColorFmt = OSD_COLOR_FMT_BGRA8888;
      break;
      case RK_FMT_ARGB1555:
        Surface.enColorFmt = OSD_COLOR_FMT_ARGB1555;
      break;
      case RK_FMT_BGRA5551:
        Surface.enColorFmt = OSD_COLOR_FMT_BGRA5551;
      break;
      default:
        return RK_FAILURE;
    }

    pstBitmap->pData = malloc(4 * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight));

    if (RK_NULL == pstBitmap->pData) {
        RK_LOGE("malloc osd memroy err!");
        return RK_FAILURE;
    }

    create_surface_by_bitmap(filename, &Surface, reinterpret_cast<RK_U8 *>(pstBitmap->pData));

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = (PIXEL_FORMAT_E)pstRgnCtx->u32BmpFormat;

    return RK_SUCCESS;
}

RK_S32 test_rgn_overlay_process(TEST_RGN_CTX_S *pstRgnCtx) {
    RK_S32          s32Ret       = RK_SUCCESS;
    RGN_HANDLE      RgnHandle    = 0;
    VENC_CHN        VencChn      = 0;
    POINT_S         stPoint      = {0};
    BITMAP_S        stBitmap;
    MPP_CHN_S       stMppChn;
    RGN_ATTR_S      stRgnAttr;
    RGN_CHN_ATTR_S  stRgnChnAttr;
    VENC_ATTR_S     stVencAttr;
    RK_BOOL         bShow;

    VencChn = 0;
    s32Ret = test_venc_start(VencChn, pstRgnCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    /****************************************
     step 1: create overlay regions
    ****************************************/
    for (RK_S32 i = 0; i< pstRgnCtx->s32RgnCount; i++) {
        stRgnAttr.enType = OVERLAY_RGN;
        stRgnAttr.unAttr.stOverlay.enPixelFmt = (PIXEL_FORMAT_E)pstRgnCtx->u32BmpFormat;
        stRgnAttr.unAttr.stOverlay.stSize.u32Width  = pstRgnCtx->stRegion.u32Width;
        stRgnAttr.unAttr.stOverlay.stSize.u32Height = pstRgnCtx->stRegion.u32Height;

        RgnHandle = i;

        s32Ret = RK_MPI_RGN_Create(RgnHandle, &stRgnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_Create (%d) failed with %#x!", RgnHandle, s32Ret);
            RK_MPI_RGN_Destroy(RgnHandle);
            return RK_FAILURE;
        }
        RK_LOGI("The handle: %d, create success!", RgnHandle);
    }

    /*********************************************
     step 2: display overlay regions to venc groups
    *********************************************/
    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        RgnHandle = i;
        VencChn = 0;
        stMppChn.enModId = RK_ID_VENC;
        stMppChn.s32DevId = 0;
        stMppChn.s32ChnId = VencChn;

        memset(&stRgnChnAttr, 0, sizeof(stRgnChnAttr));
        stRgnChnAttr.bShow = RK_TRUE;
        stRgnChnAttr.enType = OVERLAY_RGN;
        stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstRgnCtx->stRegion.s32X + 48 * i;
        stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstRgnCtx->stRegion.s32Y + 48 * i;
        stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 128;
        stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
        stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = i;
        s32Ret = RK_MPI_RGN_AttachToChn(RgnHandle, &stMppChn, &stRgnChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_AttachToChn (%d) failed with %#x!", RgnHandle, s32Ret);
            return RK_FAILURE;
        }
    }

    RK_LOGI("Display region to chn success!");
    usleep(5000);

    /*********************************************
     step 3: show bitmap
    *********************************************/
    RK_S64 s64ShowBmpStart = mpi_test_utils_get_now_us();
    s32Ret = test_rgn_load_bmp(pstRgnCtx->srcFileBmpName, &stBitmap, pstRgnCtx);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("Load bmp failed with %#x!", s32Ret);
        return RK_FAILURE;
    }

    for (RK_S32 i = 0; i< pstRgnCtx->s32RgnCount; i++) {
        RgnHandle = i;

        s32Ret = RK_MPI_RGN_SetBitMap(RgnHandle, &stBitmap);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_RGN_SetBitMap failed with %#x!", s32Ret);
            return RK_FAILURE;
        }
    }

    if (RK_NULL != stBitmap.pData) {
        free(stBitmap.pData);
        stBitmap.pData = NULL;
    }
    RK_S64 s64ShowBmpEnd = mpi_test_utils_get_now_us();

    RK_LOGI("Handle:%d, space time %lld us, load bmp success!", RgnHandle, s64ShowBmpEnd - s64ShowBmpStart);
    usleep(1000000);

    /*********************************************
     step 4: change overlay regions' position
    *********************************************/
    RK_S64 s64ChgPosStart = mpi_test_utils_get_now_us();
    RgnHandle = 0;

    VencChn = 0;
    stPoint.s32X = 64;
    stPoint.s32Y = 0;
    s32Ret = test_rgn_chg_position(RgnHandle, VencChn, &stPoint);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("Change region(%d) position failed with %#x!", RgnHandle, s32Ret);
        return RK_FAILURE;
    }

    RK_S64 s64ChgPosEnd = mpi_test_utils_get_now_us();

    RK_LOGI("Handle:%d, space time %lld us, change point success,new point(x:%d,y:%d)!",
        RgnHandle, s64ChgPosEnd - s64ChgPosStart, stPoint.s32X, stPoint.s32Y);
    usleep(5000);

    /*********************************************
     step 5: show or hide overlay regions
    *********************************************/
    RgnHandle = 0;
    bShow = RK_FALSE;

    for (RK_S32 i = 0; i < 4; i++) {
        VencChn = 0;
        s32Ret = test_rgn_show_or_hide(RgnHandle, VencChn, bShow);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("Region(%d) show failed with %#x!", RgnHandle, s32Ret);
            return RK_FAILURE;
        }

        bShow = bShow ? RK_FALSE : RK_TRUE;

        usleep(300000);
    }

    RK_LOGI("Handle:%d, show or hide osd success", RgnHandle);

    /*********************************************
     step 6: Detach osd from chn
    *********************************************/
    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        RgnHandle = i;
        VencChn = 0;
        stMppChn.enModId = RK_ID_VENC;
        stMppChn.s32DevId = 0;
        stMppChn.s32ChnId = VencChn;

        s32Ret = RK_MPI_RGN_DetachFromChn(RgnHandle, &stMppChn);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_DetachFrmChn (%d) failed with %#x!", RgnHandle, s32Ret);
            return RK_FAILURE;
        }

        RK_LOGI("Detach handle:%d from chn success", RgnHandle);
        usleep(5000);
    }

    /*********************************************
     step 7: destory region
    *********************************************/
    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        RgnHandle = i;
        s32Ret = RK_MPI_RGN_Destroy(RgnHandle);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_Destroy [%d] failed with %#x", RgnHandle, s32Ret);
        }
        RK_LOGI("Destory handle:%d success", RgnHandle);
    }

    VencChn = 0;
    s32Ret = test_venc_stop(VencChn, pstRgnCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 test_rgn_cover_process(TEST_RGN_CTX_S *pstRgnCtx) {
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_HANDLE coverHandle;
    RGN_ATTR_S stCoverAttr;
    MPP_CHN_S stCoverChn;
    RGN_CHN_ATTR_S stCoverChnAttr;
    VENC_CHN vencChn;

    memset(&stCoverAttr, 0, sizeof(stCoverAttr));
    memset(&stCoverChnAttr, 0, sizeof(RGN_CHN_ATTR_S));

    vencChn = 0;
    s32Ret = test_venc_start(vencChn, pstRgnCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        coverHandle = i;
        stCoverAttr.enType = COVER_RGN;
        s32Ret = RK_MPI_RGN_Create(coverHandle, &stCoverAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            return RK_FAILURE;
        }

        stCoverChn.enModId = RK_ID_VENC;
        stCoverChn.s32ChnId = 0;
        stCoverChn.s32DevId = vencChn;

        stCoverChnAttr.bShow = RK_TRUE;
        stCoverChnAttr.enType = COVER_RGN;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = pstRgnCtx->stRegion.s32X;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = pstRgnCtx->stRegion.s32Y;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = pstRgnCtx->stRegion.u32Width;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = pstRgnCtx->stRegion.u32Height;
        stCoverChnAttr.unChnAttr.stCoverChn.u32Color = 0xffffff;
        stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = 0;
        s32Ret = RK_MPI_RGN_AttachToChn(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto AttachCover_failed;
        }
    }

    RK_LOGI("create an cover region and attach it to venc chn0");
    usleep(1000000);

    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        coverHandle = i;
        s32Ret = RK_MPI_RGN_GetDisplayAttr(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto exit;
        }
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = 64 * i;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 64 * i;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 256;
        stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 256;
        stCoverChnAttr.unChnAttr.stCoverChn.u32Color = 0x00f800;
        stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = 1;
        s32Ret = RK_MPI_RGN_SetDisplayAttr(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto exit;
        }

        RK_LOGI("resize the cover region %d to <%d, %d, %d, %d>, color<0x%x>",
            i,
            stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X,
            stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y,
            stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width,
            stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height,
            stCoverChnAttr.unChnAttr.stCoverChn.u32Color);
    }
    usleep(500000);

    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        coverHandle = i;
        s32Ret = RK_MPI_RGN_GetDisplayAttr(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto exit;
        }
        stCoverChnAttr.bShow = RK_FALSE;
        s32Ret = RK_MPI_RGN_SetDisplayAttr(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto exit;
        }
    }
    RK_LOGI("hide the cover and the overlay success");
    usleep(500000);

    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        coverHandle = i;
        s32Ret = RK_MPI_RGN_GetDisplayAttr(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto exit;
        }
        stCoverChnAttr.bShow = RK_TRUE;
        s32Ret = RK_MPI_RGN_SetDisplayAttr(coverHandle, &stCoverChn, &stCoverChnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("failed with %#x!", s32Ret);
            goto exit;
        }
    }

    RK_LOGI("show the cover and the overlay success");
    usleep(500000);

    s32Ret = test_venc_stop(vencChn, pstRgnCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

exit:
    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        coverHandle = i;
        RK_MPI_RGN_DetachFromChn(coverHandle, &stCoverChn);
    }
AttachCover_failed:
    for (RK_S32 i = 0; i < pstRgnCtx->s32RgnCount; i++) {
        coverHandle = i;
        RK_MPI_RGN_Destroy(coverHandle);
    }
    return s32Ret;
}

RK_S32 unit_test_mpi_rgn_venc(TEST_RGN_CTX_S *pstRgnCtx) {
    RK_S32 s32Ret = RK_SUCCESS;

    switch (pstRgnCtx->s32Operation) {
      case OVERLAY_RGN: {
        s32Ret = test_rgn_overlay_process(pstRgnCtx);
      } break;
      case COVER_RGN: {
        s32Ret = test_rgn_cover_process(pstRgnCtx);
      } break;
      default:
        RK_LOGE("unsupport operation %d.", pstRgnCtx->s32Operation);
        s32Ret = RK_FAILURE;
    }
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

static void mpi_rgn_test_show_options(const TEST_RGN_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("rgn input raw file name   : %s\n", ctx->srcFileRawName);
    RK_PRINT("rgn input bmp file name   : %s\n", ctx->srcFileBmpName);
    RK_PRINT("rgn output file path      : %s\n", ctx->dstFilePath);
    RK_PRINT("rgn count                 : %d\n", ctx->s32RgnCount);
    RK_PRINT("rgn operation             : %d\n", ctx->s32Operation);
    RK_PRINT("rgn region pos x          : %d\n", ctx->stRegion.s32X);
    RK_PRINT("rgn region pos y          : %d\n", ctx->stRegion.s32Y);
    RK_PRINT("rgn canvas width          : %d\n", ctx->stRegion.u32Width);
    RK_PRINT("rgn canvas height         : %d\n", ctx->stRegion.u32Height);
    RK_PRINT("rgn raw width             : %d\n", ctx->u32RawWidth);
    RK_PRINT("rgn raw height            : %d\n", ctx->u32RawHeight);
}

static const char *const usages[] = {
    "./rk_mpi_rgn_test [-w RECT_W] [-h RECT_H]...",
    NULL,
};

int main(int argc, const char **argv) {
    RK_S32 s32Ret = RK_SUCCESS;
    TEST_RGN_CTX_S stRgnCtx;

    memset(&stRgnCtx, 0, sizeof(TEST_RGN_CTX_S));
    stRgnCtx.s32Operation   = OVERLAY_RGN;
    stRgnCtx.s32LoopCount   = 1;
    stRgnCtx.s32RgnCount    = 1;
    stRgnCtx.u32BmpFormat   = RK_FMT_BGRA5551;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input_raw_name", &(stRgnCtx.srcFileRawName),
                    "input raw data file name. <required>", NULL, 0, 0),
        OPT_STRING('\0', "input_bmp_name", &(stRgnCtx.srcFileBmpName),
                    "input bmp data file name. <required>", NULL, 0, 0),
        OPT_STRING('o', "output_path", &(stRgnCtx.dstFilePath),
                    "output stream file path. default(RK_NULL).", NULL, 0, 0),
        OPT_INTEGER('r', "rgn_count", &(stRgnCtx.s32RgnCount),
                    "the number of rgn handle. default(1).", NULL, 0, 0),
        OPT_INTEGER('p', "operation", &(stRgnCtx.s32Operation),
                    "RGN operation. default(0). 0: overlay. 1: cover.", NULL, 0, 0),
        OPT_INTEGER('x', "rect_x", &(stRgnCtx.stRegion.s32X),
                    "RGN region pos x. default(0).", NULL, 0, 0),
        OPT_INTEGER('y', "rect_y", &(stRgnCtx.stRegion.s32Y),
                    "RGN region pos x. default(0).", NULL, 0, 0),
        OPT_INTEGER('w', "bmp_w", &(stRgnCtx.stRegion.u32Width),
                    "bmp width. default(0). <required>", NULL, 0, 0),
        OPT_INTEGER('h', "bmp_h", &(stRgnCtx.stRegion.u32Height),
                    "bmp height. default(0). <required>", NULL, 0, 0),
        OPT_INTEGER('W', "raw_w", &(stRgnCtx.u32RawWidth),
                    "raw width. default(0). <required>", NULL, 0, 0),
        OPT_INTEGER('H', "raw_h", &(stRgnCtx.u32RawHeight),
                    "raw height. default(0). <required>", NULL, 0, 0),
        OPT_INTEGER('f', "format", &(stRgnCtx.u32BmpFormat),
                    "bmp pixel format. default(65557). 65546: ARGB1555, 65557: BGRA5551", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_rgn_test_show_options(&stRgnCtx);

    if (stRgnCtx.stRegion.u32Width <= 0 ||
          stRgnCtx.stRegion.u32Height <= 0 ||
          stRgnCtx.u32RawWidth <= 0 ||
          stRgnCtx.u32RawHeight <= 0 ||
          stRgnCtx.srcFileRawName == RK_NULL ||
          stRgnCtx.srcFileBmpName == RK_NULL) {
        argparse_usage(&argparse);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = unit_test_mpi_rgn_venc(&stRgnCtx);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    s32Ret = RK_MPI_SYS_Exit();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    RK_LOGI("test running ok.");
    return RK_SUCCESS;

__FAILED:
    RK_MPI_SYS_Exit();
    RK_LOGE("test running failed!");
    return s32Ret;
}


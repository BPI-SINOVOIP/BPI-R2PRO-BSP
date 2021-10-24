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
#undef DBG_MOD_ID
#define DBG_MOD_ID       RK_ID_VPSS

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "rk_debug.h"
#include "rk_mpi_vpss.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_cal.h"
#include "argparse.h"
#include "mpi_test_utils.h"

typedef struct _rkMpiVPSSCtx {
    const char *srcFileName;
    const char *dstFilePath;
    RK_S32  s32LoopCount;
    RK_S32  s32GrpNum;
    RK_S32  s32ChnNum;
    RK_BOOL bGrpCropEn;
    RK_BOOL bChnCropEn;
    RK_S32  s32GrpCropRatio;
    RK_S32  s32ChnCropRatio;

    RK_S32  s32SrcWidth;
    RK_S32  s32SrcHeight;
    RK_S32  s32SrcCompressMode;
    RK_S32  s32SrcPixFormat;

    RK_S32  s32DstWidth;
    RK_S32  s32DstHeight;
    RK_S32  s32DstCompressMode;
    RK_S32  s32DstPixFormat;

    RK_S32  s32Rotation;
    RK_S32  s32RotationEx;

    RK_BOOL bBackupFrame;
    RK_BOOL bAttachPool;
    RK_S32  s32ChnMode;

    RK_S32  s32GrpIndex;
} TEST_VPSS_CTX_S;

RK_S32 mpi_vpss_test_create_grp(VPSS_GRP VpssGrp, const TEST_VPSS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_GRP_ATTR_S stGrpVpssAttr;

    memset(&stGrpVpssAttr, 0, sizeof(VPSS_GRP_ATTR_S));
    stGrpVpssAttr.u32MaxW = 4096;
    stGrpVpssAttr.u32MaxH = 4096;
    stGrpVpssAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32SrcPixFormat;
    stGrpVpssAttr.enCompressMode = (COMPRESS_MODE_E)ctx->s32SrcCompressMode;
    stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;
    stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;
    s32Ret = RK_MPI_VPSS_CreateGrp(VpssGrp, &stGrpVpssAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 mpi_vpss_test_set_grp_crop(VPSS_GRP VpssGrp, const TEST_VPSS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CROP_INFO_S stCropInfo;

    memset(&stCropInfo, 0, sizeof(VPSS_CROP_INFO_S));
    stCropInfo.bEnable = ctx->bGrpCropEn;
    stCropInfo.enCropCoordinate = VPSS_CROP_RATIO_COOR;
    stCropInfo.stCropRect.s32X = 500 - ctx->s32GrpCropRatio / 2;
    stCropInfo.stCropRect.s32Y = 500 - ctx->s32GrpCropRatio / 2;
    stCropInfo.stCropRect.u32Width = ctx->s32GrpCropRatio;
    stCropInfo.stCropRect.u32Height = ctx->s32GrpCropRatio;
    s32Ret = RK_MPI_VPSS_SetGrpCrop(VpssGrp, &stCropInfo);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VPSS_GetGrpCrop(VpssGrp, &stCropInfo);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 mpi_vpss_test_chn_set_rotation(
        VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation) {
    RK_S32 s32Ret = RK_SUCCESS;
    ROTATION_E rotation = ROTATION_0;
    s32Ret = RK_MPI_VPSS_SetChnRotation(VpssGrp, VpssChn, enRotation);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VPSS_GetChnRotation(VpssGrp, VpssChn, &rotation);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    if (rotation != enRotation) {
        s32Ret = RK_FAILURE;
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 mpi_vpss_test_chn_set_rotation_ex(
        VPSS_GRP VpssGrp, VPSS_CHN VpssChn, RK_S32 rotationEx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_ROTATION_EX_ATTR_S stRotationEx;

    stRotationEx.bEnable = RK_TRUE;
    stRotationEx.stRotationEx.u32Angle = rotationEx;
    s32Ret = RK_MPI_VPSS_SetChnRotationEx(VpssGrp, VpssChn, &stRotationEx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VPSS_GetChnRotationEx(VpssGrp, VpssChn, &stRotationEx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    if (stRotationEx.bEnable != RK_TRUE
            || stRotationEx.stRotationEx.u32Angle != rotationEx) {
        s32Ret = RK_FAILURE;
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 mpi_vpss_test_chn_set_crop(
        VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const TEST_VPSS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CROP_INFO_S stChnCropInfo;

    stChnCropInfo.bEnable = ctx->bChnCropEn;
    stChnCropInfo.enCropCoordinate = VPSS_CROP_RATIO_COOR;
    stChnCropInfo.stCropRect.s32X = 500 - ctx->s32ChnCropRatio / 2;
    stChnCropInfo.stCropRect.s32Y = 500 - ctx->s32ChnCropRatio / 2;
    stChnCropInfo.stCropRect.u32Width = ctx->s32ChnCropRatio;
    stChnCropInfo.stCropRect.u32Height = ctx->s32ChnCropRatio;
    s32Ret = RK_MPI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stChnCropInfo);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VPSS_GetChnCrop(VpssGrp, VpssChn, &stChnCropInfo);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 mpi_vpss_test_enable_chn(
        VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const TEST_VPSS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CHN_ATTR_S stVpssChnAttr;

    memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));
    stVpssChnAttr.enChnMode = (VPSS_CHN_MODE_E)ctx->s32ChnMode;
    stVpssChnAttr.enCompressMode = (COMPRESS_MODE_E)ctx->s32DstCompressMode;
    stVpssChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssChnAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32DstPixFormat;
    stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssChnAttr.u32Width = ctx->s32DstWidth;
    stVpssChnAttr.u32Height = ctx->s32DstHeight;

    s32Ret = RK_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

RK_S32 mpi_vpss_test_write_file(const char *path, MB_BLK blk, RK_S32 size) {
    FILE *fdW = fopen(path, "wb");
    if (fdW == RK_NULL) {
        RK_LOGE("open %s failed, error: %s", path, strerror(errno));
        return RK_ERR_VPSS_BUSY;
    }
    fwrite(RK_MPI_MB_Handle2VirAddr(blk), 1, size, fdW);
    fflush(fdW);
    fclose(fdW);

    return RK_SUCCESS;
}

RK_S32 mpi_vpss_test_process(
        VPSS_GRP VpssGrp, const TEST_VPSS_CTX_S *ctx, MB_BLK blk, RK_S32 size) {
    RK_S32 s32Ret = RK_SUCCESS;
    VIDEO_FRAME_INFO_S frameIn = {0};
    VIDEO_FRAME_INFO_S frameOut = {0};

    frameIn.stVFrame.pMbBlk = blk;
    frameIn.stVFrame.u32Width = ctx->s32SrcWidth;
    frameIn.stVFrame.u32Height = ctx->s32SrcHeight;
    frameIn.stVFrame.u32VirWidth = RK_ALIGN(ctx->s32SrcWidth, 16);
    frameIn.stVFrame.u32VirHeight = RK_ALIGN(ctx->s32SrcHeight, 16);
    frameIn.stVFrame.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32SrcPixFormat;
    frameIn.stVFrame.enCompressMode = (COMPRESS_MODE_E)ctx->s32SrcCompressMode;

    RK_MPI_VPSS_SendFrame(VpssGrp, 0, &frameIn, -1);
    RK_MPI_MB_ReleaseMB(blk);

    for (RK_S32 i = 0; i < ctx->s32ChnNum; i++) {
        s32Ret = RK_MPI_VPSS_GetChnFrame(VpssGrp, i, &frameOut, -1);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        RK_LOGI("get chn[%d] frame %p", i, frameOut.stVFrame.pMbBlk);
        if (ctx->dstFilePath != RK_NULL) {
            char cWritePath[128] = {0};
            PIC_BUF_ATTR_S stPicBufAttr;
            MB_PIC_CAL_S stMbPicCalResult;
            RK_S32 frameWidth = frameOut.stVFrame.u32VirWidth;
            RK_S32 frameHeight = frameOut.stVFrame.u32VirHeight;

            stPicBufAttr.u32Width = frameWidth;
            stPicBufAttr.u32Height = frameHeight;
            stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32DstPixFormat;
            stPicBufAttr.enCompMode = (COMPRESS_MODE_E)ctx->s32DstCompressMode;
            s32Ret = RK_MPI_CAL_VGS_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
                return s32Ret;
            }
            RK_S32 frameLength = stMbPicCalResult.u32MBSize;
            snprintf(cWritePath, sizeof(cWritePath), "%schn_out_%dx%d_%d_%d.bin",
                     ctx->dstFilePath, frameWidth, frameHeight, VpssGrp, i);
            s32Ret = mpi_vpss_test_write_file(cWritePath,
                                     frameOut.stVFrame.pMbBlk,
                                     frameLength);
            if (s32Ret != RK_SUCCESS) {
                return s32Ret;
            }
        }
        s32Ret = RK_MPI_VPSS_ReleaseChnFrame(VpssGrp, i, &frameOut);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }
    if (ctx->bBackupFrame) {
        frameOut = {0};
        s32Ret = RK_MPI_VPSS_GetGrpFrame(VpssGrp, 0, &frameOut);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        char cWritePath[128] = {0};
        PIC_BUF_ATTR_S stPicBufAttr;
        MB_PIC_CAL_S stMbPicCalResult;

        stPicBufAttr.u32Width = frameOut.stVFrame.u32VirWidth;
        stPicBufAttr.u32Height = frameOut.stVFrame.u32VirHeight;
        stPicBufAttr.enCompMode = (COMPRESS_MODE_E)frameOut.stVFrame.enCompressMode;
        stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)frameOut.stVFrame.enPixelFormat;
        s32Ret = RK_MPI_CAL_VGS_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
            return RK_NULL;
        }
        snprintf(cWritePath, sizeof(cWritePath), "%sgrp_out_%dx%d_%d.bin",
                 ctx->dstFilePath, stMbPicCalResult.u32VirWidth, stMbPicCalResult.u32VirHeight, VpssGrp);
        RK_LOGI("get group[%d] frame %p w %d h %d size %d", VpssGrp,
                    frameOut.stVFrame.pMbBlk, stMbPicCalResult.u32VirWidth,
                    stMbPicCalResult.u32VirHeight, stMbPicCalResult.u32MBSize);
        s32Ret = mpi_vpss_test_write_file(cWritePath,
                         frameOut.stVFrame.pMbBlk,
                         stMbPicCalResult.u32MBSize);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VPSS_ReleaseGrpFrame(VpssGrp, 0, &frameOut);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    return RK_SUCCESS;
}

static void *test_mpi_vpss_single(void *arg) {
    RK_S32           s32Ret = RK_SUCCESS;
    TEST_VPSS_CTX_S *ctx = reinterpret_cast<TEST_VPSS_CTX_S *>(arg);
    MB_POOL          pool = MB_INVALID_POOLID;
    MB_POOL_CONFIG_S pstMbPoolCfg;
    FILE            *fdR = RK_NULL;
    MB_BLK           srcBlk = MB_INVALID_HANDLE;
    RK_S32           readLen = 0;
    PIC_BUF_ATTR_S stPicBufAttr;
    MB_PIC_CAL_S stMbPicCalResult;

    stPicBufAttr.u32Width = ctx->s32SrcWidth;
    stPicBufAttr.u32Height = ctx->s32SrcHeight;
    stPicBufAttr.enCompMode = (COMPRESS_MODE_E)ctx->s32SrcCompressMode;
    stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32SrcPixFormat;
    s32Ret = RK_MPI_CAL_VGS_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
        return RK_NULL;
    }

    memset(&pstMbPoolCfg, 0, sizeof(MB_POOL_CONFIG_S));
    pstMbPoolCfg.u64MBSize = stMbPicCalResult.u32MBSize;
    pstMbPoolCfg.u32MBCnt  = 10;
    pstMbPoolCfg.enRemapMode = MB_REMAP_MODE_CACHED;
    pstMbPoolCfg.bPreAlloc = RK_TRUE;
    pool = RK_MPI_MB_CreatePool(&pstMbPoolCfg);
    if (pool == MB_INVALID_POOLID) {
        RK_LOGE("create pool failed!");
        s32Ret = RK_FAILURE;
        goto __FAILED;
    }

    s32Ret = mpi_vpss_test_create_grp(ctx->s32GrpIndex, ctx);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    s32Ret = RK_MPI_VPSS_ResetGrp(ctx->s32GrpIndex);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    s32Ret = mpi_vpss_test_set_grp_crop(ctx->s32GrpIndex, ctx);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    for (RK_S32 chnIndex = 0; chnIndex < ctx->s32ChnNum; chnIndex++) {
        s32Ret = mpi_vpss_test_chn_set_crop(ctx->s32GrpIndex, chnIndex, ctx);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
        if (ctx->s32Rotation > 0) {
            s32Ret = mpi_vpss_test_chn_set_rotation(ctx->s32GrpIndex, chnIndex, (ROTATION_E)ctx->s32Rotation);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        }
        if (ctx->s32RotationEx > 0) {
            s32Ret = mpi_vpss_test_chn_set_rotation_ex(ctx->s32GrpIndex, chnIndex, ctx->s32RotationEx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        }
        s32Ret = mpi_vpss_test_enable_chn(ctx->s32GrpIndex, chnIndex, ctx);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
        if (ctx->bAttachPool) {
            s32Ret = RK_MPI_VPSS_AttachMbPool(ctx->s32GrpIndex, chnIndex, pool);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        }
    }
    s32Ret = RK_MPI_VPSS_StartGrp(ctx->s32GrpIndex);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    if (ctx->bBackupFrame) {
        s32Ret = RK_MPI_VPSS_EnableBackupFrame(ctx->s32GrpIndex);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
    }

    for (RK_S32 loopCount = 0; loopCount < ctx->s32LoopCount; loopCount++) {
        fdR = fopen(ctx->srcFileName, "rb");
        if (fdR == RK_NULL) {
            RK_LOGE("open %s failed, error: %s", ctx->srcFileName, strerror(errno));
            s32Ret = RK_FAILURE;
            goto __FAILED;
        }
        srcBlk = RK_MPI_MB_GetMB(pool, pstMbPoolCfg.u64MBSize, RK_TRUE);
        readLen = fread(RK_MPI_MB_Handle2VirAddr(srcBlk), 1, pstMbPoolCfg.u64MBSize, fdR);
        if (readLen != pstMbPoolCfg.u64MBSize) {
            RK_LOGE("read size is not enough. read %d, request %d", readLen, pstMbPoolCfg.u64MBSize);
            s32Ret = RK_FAILURE;
            goto __FAILED;
        }
        s32Ret = mpi_vpss_test_process(ctx->s32GrpIndex, ctx, srcBlk, pstMbPoolCfg.u64MBSize);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("test process failed.");
            goto __FAILED;
        }
        if (fdR != RK_NULL) {
            fclose(fdR);
        }
    }

    for (RK_S32 i = 0; i < ctx->s32ChnNum; i++) {
        if (ctx->bAttachPool) {
            s32Ret = RK_MPI_VPSS_DetachMbPool(ctx->s32GrpIndex, i);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        }
        s32Ret = RK_MPI_VPSS_DisableChn(ctx->s32GrpIndex, i);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
    }

    s32Ret = RK_MPI_VPSS_StopGrp(ctx->s32GrpIndex);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    s32Ret = RK_MPI_VPSS_DestroyGrp(ctx->s32GrpIndex);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    s32Ret = RK_MPI_MB_DestroyPool(pool);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    return arg;
__FAILED:
    if (fdR != RK_NULL) {
        fclose(fdR);
    }
    RK_LOGE("single vpss test %d running failed.", ctx->s32GrpIndex);
    return RK_NULL;
}

RK_S32 unit_test_mpi_vpss(TEST_VPSS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    pthread_t tids[VPSS_MAX_GRP_NUM];
    TEST_VPSS_CTX_S tmpCtx[VPSS_MAX_GRP_NUM];

    for (RK_S32 grpIndex = 0; grpIndex < ctx->s32GrpNum; grpIndex++) {
        memcpy(&(tmpCtx[grpIndex]), ctx, sizeof(TEST_VPSS_CTX_S));
        pthread_create(&tids[grpIndex], 0, test_mpi_vpss_single, reinterpret_cast<void *>(&tmpCtx[grpIndex]));
        ctx->s32GrpIndex++;
    }

    for (RK_S32 grpIndex = 0; grpIndex < ctx->s32GrpNum; grpIndex++) {
        void *retval = RK_NULL;
        pthread_join(tids[grpIndex], &retval);
        if (retval == RK_NULL) {
            RK_LOGE("Has error test.");
            s32Ret = RK_FAILURE;
        }
    }

    return s32Ret;
}

static const char *const usages[] = {
    "./rk_mpi_vpss_test [-i SRC_PATH] [-w SRC_WIDTH] [-h SRC_HEIGHT] [-W DST_WIDTH] [-H DST_HEIGHT]...",
    NULL,
};

static void mpi_vpss_test_show_options(const TEST_VPSS_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name       : %s\n", ctx->srcFileName);
    RK_PRINT("output file name       : %s\n", ctx->dstFilePath);
    RK_PRINT("loop count             : %d\n", ctx->s32LoopCount);
    RK_PRINT("group number           : %d\n", ctx->s32GrpNum);
    RK_PRINT("channel number         : %d\n", ctx->s32ChnNum);
    RK_PRINT("group crop enabled     : %d\n", ctx->bGrpCropEn ? 1 : 0);
    RK_PRINT("chn crop enabled       : %d\n", ctx->bChnCropEn ? 1 : 0);
    RK_PRINT("group crop ratio       : %d\n", ctx->s32GrpCropRatio);
    RK_PRINT("chn crop ratio         : %d\n", ctx->s32ChnCropRatio);
    RK_PRINT("input width            : %d\n", ctx->s32SrcWidth);
    RK_PRINT("input height           : %d\n", ctx->s32SrcHeight);
    RK_PRINT("input compress mode    : %d\n", ctx->s32SrcCompressMode);
    RK_PRINT("input pixel format     : %d\n", ctx->s32SrcPixFormat);
    RK_PRINT("output width           : %d\n", ctx->s32DstWidth);
    RK_PRINT("output height          : %d\n", ctx->s32DstHeight);
    RK_PRINT("output compress mode   : %d\n", ctx->s32DstCompressMode);
    RK_PRINT("output pixel format    : %d\n", ctx->s32DstPixFormat);
    RK_PRINT("fixed rotation         : %d\n", ctx->s32Rotation);
    RK_PRINT("any rotation angle     : %d\n", ctx->s32RotationEx);
    RK_PRINT("enable backup frame    : %d\n", ctx->bBackupFrame);
    RK_PRINT("enable attach mb pool  : %d\n", ctx->bAttachPool);
}

RK_S32 main(int argc, const char **argv) {
    RK_S32          s32Ret;
    TEST_VPSS_CTX_S ctx;

    memset(&ctx, 0, sizeof(TEST_VPSS_CTX_S));

    //  set default params.
    ctx.dstFilePath     = RK_NULL;
    ctx.s32LoopCount    = 1;
    ctx.s32GrpNum       = 1;
    ctx.s32ChnNum       = 1;
    ctx.bGrpCropEn      = RK_FALSE;
    ctx.bChnCropEn      = RK_FALSE;
    ctx.s32GrpCropRatio = 1000;
    ctx.s32ChnCropRatio = 1000;
    ctx.s32SrcCompressMode  = 0;
    ctx.s32SrcPixFormat     = RK_FMT_YUV420SP;
    ctx.s32DstCompressMode  = 0;
    ctx.s32DstPixFormat     = RK_FMT_YUV420SP;
    ctx.s32GrpIndex         = 0;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input",  &(ctx.srcFileName),
                   "input file name. e.g.(/userdata/1080p.nv12). <required>", NULL, 0, 0),
        OPT_STRING('o', "output", &(ctx.dstFilePath),
                    "output file path. e.g.(/userdata/vpss/). default(NULL).", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx.s32LoopCount),
                    "loop running count. default(1)", NULL, 0, 0),
        OPT_INTEGER('g', "group_count", &(ctx.s32GrpNum),
                    "the count of vpss group. default(1).", NULL, 0, 0),
        OPT_INTEGER('c', "channel_count", &(ctx.s32ChnNum),
                    "the count of vpss channel. default(1).", NULL, 0, 0),
        OPT_BOOLEAN('\0', "group_crop_en", &(ctx.bGrpCropEn),
                    "vpss group crop is enabled. default(0).", NULL, 0, 0),
        OPT_BOOLEAN('\0', "channel_crop_en", &(ctx.bChnCropEn),
                    "vpss channel crop is enabled. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "group_crop_ratio", &(ctx.s32GrpCropRatio),
                    "vpss group crop ratio. range(1,1000). default(1000)", NULL, 0, 0),
        OPT_INTEGER('\0', "channel_crop_ratio", &(ctx.s32ChnCropRatio),
                    "vpss channel crop ratio. range(1,1000). default(1000)", NULL, 0, 0),
        OPT_INTEGER('w', "src_width", &(ctx.s32SrcWidth),
                    "src width. e.g.(1920). <required>", NULL, 0, 0),
        OPT_INTEGER('h', "src_height", &(ctx.s32SrcHeight),
                    "src height. e.g.(1080). <required>", NULL, 0, 0),
        OPT_INTEGER('m', "src_compress", &(ctx.s32SrcCompressMode),
                    "src compress mode. default(0).", NULL, 0, 0),
        OPT_INTEGER('f', "src_format", &(ctx.s32SrcPixFormat),
                    "src pixel format. default(0. 0 is NV12).", NULL, 0, 0),
        OPT_INTEGER('W', "dst_width", &(ctx.s32DstWidth),
                    "dst width. e.g.(1920). <required>", NULL, 0, 0),
        OPT_INTEGER('H', "dst_height", &(ctx.s32DstHeight),
                    "dst height. e.g.(1080). <required>", NULL, 0, 0),
        OPT_INTEGER('M', "dst_compress", &(ctx.s32DstCompressMode),
                    "dst compress mode. default(0).", NULL, 0, 0),
        OPT_INTEGER('F', "dst_format", &(ctx.s32DstPixFormat),
                    "dst pixel format. default(0. 0 is NV12).", NULL, 0, 0),
        OPT_INTEGER('r', "rotation", &(ctx.s32Rotation),
                    "fixed rotation angle. default(0). 0: 0. 1: 90. 2: 180. 3: 270", NULL, 0, 0),
        OPT_INTEGER('R', "rotation_ex", &(ctx.s32RotationEx),
                    "any rotation angle. default(0). ", NULL, 0, 0),
        OPT_INTEGER('b', "backup_frame", &(ctx.bBackupFrame),
                    "enable backup frame or not, default(0), 0: RK_FALSE, 1: RK_TRUE", NULL, 0, 0),
        OPT_INTEGER('a', "attach_mb_pool", &(ctx.bAttachPool),
                    "enable attach mb pool or not, default(0), 0: RK_FALSE, 1: RK_TRUE", NULL, 0, 0),
        OPT_INTEGER('\0', "chn_mode", &(ctx.s32ChnMode),
                    "channel mode, default(0), 0: USER, 1: AUTO, 2: PASS-THOUGH", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_vpss_test_show_options(&ctx);

    if (ctx.srcFileName == RK_NULL
        || ctx.s32SrcWidth <= 0
        || ctx.s32SrcHeight <= 0
        || ctx.s32DstWidth <= 0
        || ctx.s32DstHeight <= 0) {
        argparse_usage(&argparse);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = unit_test_mpi_vpss(&ctx);
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

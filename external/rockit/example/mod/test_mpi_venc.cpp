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

#include "rk_debug.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_cal.h"
#include "argparse.h"
#include "mpi_test_utils.h"

#define MAX_TIME_OUT_MS          20
#define TEST_RC_MODE             0

typedef struct _rkMpiVENCCtx {
    const char     *srcFileUri;
    const char     *dstFilePath;
    RK_U32          u32SrcWidth;
    RK_U32          u32SrcHeight;
    RK_U32          u32srcVirWidth;
    RK_U32          u32srcVirHeight;
    RK_S32          s32LoopCount;
    RK_U32          u32ChnIndex;
    RK_U32          u32ChNum;
    RK_U32          u32SrcPixFormat;
    RK_U32          u32DstCodec;
    RK_U32          u32BufferSize;
    RK_U32          u32StreamBufCnt;
    RK_BOOL         threadExit;
    MB_POOL         vencPool;
} TEST_VENC_CTX_S;

static RK_S32 read_with_pixel_width(RK_U8 *pBuf, RK_U32 u32Width, RK_U32 u32Height,
                                     RK_U32 u32VirWidth, RK_U32 u32PixWidth, FILE *fp) {
    RK_U32 u32Row;
    RK_S32 s32ReadSize;

    for (u32Row = 0; u32Row < u32Height; u32Row++) {
        s32ReadSize = fread(pBuf + u32Row * u32VirWidth * u32PixWidth, 1, u32Width * u32PixWidth, fp);
        if (s32ReadSize != u32Width * u32PixWidth) {
            RK_LOGE("read file failed expect %d vs %d\n",
                      u32Width * u32PixWidth, s32ReadSize);
            return RK_FAILURE;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 read_image(RK_U8 *pVirAddr, RK_U32 u32Width, RK_U32 u32Height,
                                  RK_U32 u32VirWidth, RK_U32 u32VirHeight, RK_U32 u32PixFormat, FILE *fp) {
    RK_U32 u32Row = 0;
    RK_U32 u32ReadSize = 0;
    RK_S32 s32Ret = RK_SUCCESS;

    RK_U8 *pBufy = pVirAddr;
    RK_U8 *pBufu = pBufy + u32VirWidth * u32VirHeight;
    RK_U8 *pBufv = pBufu + u32VirWidth * u32VirHeight / 4;

    switch (u32PixFormat) {
        case RK_FMT_YUV420SP: {
            for (u32Row = 0; u32Row < u32Height; u32Row++) {
                u32ReadSize = fread(pBufy + u32Row * u32VirWidth, 1, u32Width, fp);
                if (u32ReadSize != u32Width) {
                     return RK_FAILURE;
                }
            }

            for (u32Row = 0; u32Row < u32Height / 2; u32Row++) {
                u32ReadSize = fread(pBufu + u32Row * u32VirWidth, 1, u32Width, fp);
                if (u32ReadSize != u32Width) {
                    return RK_FAILURE;
                }
            }
        } break;
        case RK_FMT_RGB888:
        case RK_FMT_BGR888: {
            s32Ret = read_with_pixel_width(pBufy, u32Width, u32Height, u32VirWidth, 3, fp);
        } break;
        default : {
            RK_LOGE("read image do not support fmt %d\n", u32PixFormat);
            return RK_FAILURE;
        } break;
    }

    return s32Ret;
}

static RK_S32 check_options(const TEST_VENC_CTX_S *ctx) {
    if (ctx->srcFileUri == RK_NULL) {
        goto __FAILED;
    }

    if (ctx->u32SrcPixFormat == RK_FMT_BUTT ||
        ctx->u32DstCodec <= RK_VIDEO_ID_Unused ||
        ctx->u32SrcWidth <= 0 ||
        ctx->u32SrcHeight <= 0) {
        goto __FAILED;
    }

    return RK_SUCCESS;

__FAILED:
    return RK_ERR_VENC_ILLEGAL_PARAM;
}

void* venc_get_stream(void *pArgs) {
    TEST_VENC_CTX_S *pstCtx     = reinterpret_cast<TEST_VENC_CTX_S *>(pArgs);
    void            *pData      = RK_NULL;
    RK_S32           s32Ret     = RK_SUCCESS;
    FILE            *fp         = RK_NULL;
    char             name[256]  = {0};
    RK_U32           u32Ch      = pstCtx->u32ChnIndex;
    RK_S32           s32StreamCnt = 0;
    VENC_STREAM_S    stFrame;

    if (pstCtx->dstFilePath != RK_NULL) {
        mkdir(pstCtx->dstFilePath, 0777);

        snprintf(name, sizeof(name), "%s/test_%d.bin",
            pstCtx->dstFilePath, pstCtx->u32ChnIndex);

        fp = fopen(name, "wb");
        if (fp == RK_NULL) {
            RK_LOGE("chn %d can't open file %s in get picture thread!\n", u32Ch, name);
            return RK_NULL;
        }
    }
    stFrame.pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));

    while (!pstCtx->threadExit) {
        s32Ret = RK_MPI_VENC_GetStream(u32Ch, &stFrame, -1);
        if (s32Ret >= 0) {
            if (stFrame.pstPack->bStreamEnd == RK_TRUE) {
                RK_LOGI("chn %d reach EOS stream", u32Ch);
                RK_MPI_VENC_ReleaseStream(u32Ch, &stFrame);
                break;
            }
            s32StreamCnt++;
            RK_LOGI("get chn %d stream %d", u32Ch, s32StreamCnt);
            if (pstCtx->dstFilePath != RK_NULL) {
                pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
                fwrite(pData, 1, stFrame.pstPack->u32Len, fp);
                fflush(fp);
            }
            RK_MPI_VENC_ReleaseStream(u32Ch, &stFrame);
        } else {
             if (pstCtx->threadExit) {
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

void* venc_send_frame(void *pArgs) {
    TEST_VENC_CTX_S     *pstCtx        = reinterpret_cast<TEST_VENC_CTX_S *>(pArgs);
    RK_S32               s32Ret         = RK_SUCCESS;
    RK_U8               *pVirAddr       = RK_NULL;
    FILE                *fp             = RK_NULL;
    MB_BLK               blk            = RK_NULL;
    RK_S32               s32LoopCount   = pstCtx->s32LoopCount;
    MB_POOL              pool           = pstCtx->vencPool;
    RK_U32               u32Ch          = pstCtx->u32ChnIndex;
    RK_S32               s32FrameCount  = 0;
    RK_S32               s32ReachEOS    = 0;
    VIDEO_FRAME_INFO_S   stFrame;
    MB_EXT_CONFIG_S      stMbExtConfig;

    fp = fopen(pstCtx->srcFileUri, "r");
    if (fp == RK_NULL) {
        RK_LOGE("chn %d can't open file %s!\n", u32Ch, pstCtx->srcFileUri);
        return RK_NULL;
    }

    while (!pstCtx->threadExit) {
        blk = RK_MPI_MB_GetMB(pool, pstCtx->u32BufferSize, RK_TRUE);

        if (RK_NULL == blk) {
            usleep(2000llu);
            continue;
        }
        pVirAddr = reinterpret_cast<RK_U8 *>(RK_MPI_MB_Handle2VirAddr(blk));
        s32Ret = read_image(pVirAddr, pstCtx->u32SrcWidth, pstCtx->u32SrcHeight,
                  pstCtx->u32srcVirWidth, pstCtx->u32srcVirHeight, pstCtx->u32SrcPixFormat, fp);
        if (s32Ret != RK_SUCCESS) {
             s32ReachEOS = 1;
             if (s32LoopCount > 0) {
                s32LoopCount--;
                RK_LOGI("finish venc count %d\n", pstCtx->s32LoopCount - s32LoopCount);
                if (s32LoopCount > 0) {
                    s32ReachEOS = 0;
                    RK_MPI_MB_ReleaseMB(blk);

                    fseek(fp, 0L, SEEK_SET);
                    RK_LOGI("seek finish ch %d", u32Ch);
                    continue;
                }
             }
        }

        RK_MPI_SYS_MmzFlushCache(blk, RK_FALSE);

        stFrame.stVFrame.pMbBlk = blk;
        stFrame.stVFrame.u32Width = pstCtx->u32SrcWidth;
        stFrame.stVFrame.u32Height = pstCtx->u32SrcHeight;
        stFrame.stVFrame.u32VirWidth = pstCtx->u32srcVirWidth;
        stFrame.stVFrame.u32VirHeight = pstCtx->u32srcVirHeight;
        stFrame.stVFrame.enPixelFormat = (PIXEL_FORMAT_E)pstCtx->u32SrcPixFormat;
        stFrame.stVFrame.u32FrameFlag |= s32ReachEOS ? FRAME_FLAG_SNAP_END : 0;
__RETRY:
        s32Ret = RK_MPI_VENC_SendFrame(u32Ch, &stFrame, -1);
        if (s32Ret < 0) {
            if (pstCtx->threadExit) {
                RK_MPI_MB_ReleaseMB(blk);
                break;
            }

            usleep(10000llu);
            goto  __RETRY;
        } else {
            RK_MPI_MB_ReleaseMB(blk);
            s32FrameCount++;
            RK_LOGI("chn %d frame %d", u32Ch, s32FrameCount);
        }
        if (s32ReachEOS) {
            RK_LOGI("chn %d reach EOS.", u32Ch);
            break;
        }
    }

    if (fp)
        fclose(fp);

    return RK_NULL;
}

RK_S32 unit_test_mpi_venc(TEST_VENC_CTX_S *ctx) {
    RK_S32                  s32Ret = RK_SUCCESS;
    RK_U32                  u32Ch = 0;
    VENC_CHN_ATTR_S         stAttr;
    VENC_RECV_PIC_PARAM_S   stRecvParam;
    VENC_RC_PARAM_S         stRcParam;
    MB_POOL_CONFIG_S        stMbPoolCfg;
    TEST_VENC_CTX_S         stVencCtx[VENC_MAX_CHN_NUM];
    pthread_t               vencThread[VENC_MAX_CHN_NUM];
    pthread_t               getStreamThread[VENC_MAX_CHN_NUM];

    memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));
    memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));
    memset(&stRcParam, 0, sizeof(VENC_RC_PARAM_S));

    if (ctx->u32BufferSize <= 0) {
        PIC_BUF_ATTR_S stPicBufAttr;
        MB_PIC_CAL_S stMbPicCalResult;

        stPicBufAttr.u32Width = ctx->u32SrcWidth;
        stPicBufAttr.u32Height = ctx->u32SrcHeight;
        stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->u32SrcPixFormat;
        stPicBufAttr.enCompMode = COMPRESS_MODE_NONE;
        s32Ret = RK_MPI_CAL_COMM_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
            return s32Ret;
        }
        ctx->u32BufferSize = stMbPicCalResult.u32MBSize;
        RK_LOGD("calc picture size: %d", ctx->u32BufferSize);
    }

    if (ctx->u32BufferSize > 32 * 1024 * 1024) {
        RK_LOGE("too large picture size: %d", ctx->u32BufferSize);
        return RK_FAILURE;
    }

    for (u32Ch = 0; u32Ch < ctx->u32ChNum; u32Ch++) {
        if (ctx->u32ChNum >= 1) {
            ctx->u32ChnIndex = u32Ch;
        }
#if TEST_RC_MODE
        stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        stAttr.stRcAttr.stH264Cbr.u32Gop = 77;
        stAttr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
        stAttr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
        stAttr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
        stAttr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
        stAttr.stRcAttr.stH264Cbr.u32BitRate = 314400;
#endif
        stAttr.stVencAttr.enType = (RK_CODEC_ID_E)ctx->u32DstCodec;
        stAttr.stVencAttr.u32Profile = H264E_PROFILE_HIGH;
        stAttr.stVencAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->u32SrcPixFormat;
        stAttr.stVencAttr.u32PicWidth = ctx->u32SrcWidth;
        stAttr.stVencAttr.u32PicHeight = ctx->u32SrcHeight;

        if (ctx->u32srcVirWidth <= 0) {
            ctx->u32srcVirWidth = ctx->u32SrcWidth;
        }
        stAttr.stVencAttr.u32VirWidth = ctx->u32srcVirWidth;

        if (ctx->u32srcVirHeight <= 0) {
            ctx->u32srcVirHeight = ctx->u32SrcHeight;
        }
        stAttr.stVencAttr.u32VirHeight = ctx->u32srcVirHeight;
        stAttr.stVencAttr.u32StreamBufCnt = ctx->u32StreamBufCnt;
        stAttr.stVencAttr.u32BufSize = ctx->u32BufferSize;

        RK_MPI_VENC_CreateChn(u32Ch, &stAttr);

#if TEST_RC_MODE
        stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
        stAttr.stRcAttr.stH264Avbr.u32Gop = 99;
        stAttr.stRcAttr.stH264Avbr.u32SrcFrameRateNum = 25;
        stAttr.stRcAttr.stH264Avbr.u32SrcFrameRateDen = 1;
        stAttr.stRcAttr.stH264Avbr.fr32DstFrameRateNum = 25;
        stAttr.stRcAttr.stH264Avbr.fr32DstFrameRateDen = 1;
        stAttr.stRcAttr.stH264Avbr.u32BitRate = 614400;
        RK_MPI_VENC_SetChnAttr(u32Ch, &stAttr);
#endif

        RK_MPI_VENC_StartRecvFrame(u32Ch, &stRecvParam);

#if TEST_RC_MODE
        stRcParam.s32FirstFrameStartQp = 25;
        stRcParam.stParamH264.u32StepQp = 4;
        stRcParam.stParamH264.u32MaxQp = 40;
        stRcParam.stParamH264.u32MinQp = 20;
        RK_MPI_VENC_SetRcParam(u32Ch, &stRcParam);
#endif

        memset(&stMbPoolCfg, 0, sizeof(MB_POOL_CONFIG_S));
        stMbPoolCfg.u64MBSize = ctx->u32BufferSize;
        stMbPoolCfg.u32MBCnt  = 10;
        stMbPoolCfg.enAllocType = MB_ALLOC_TYPE_DMA;

        ctx->vencPool = RK_MPI_MB_CreatePool(&stMbPoolCfg);

        memcpy(&(stVencCtx[u32Ch]), ctx, sizeof(TEST_VENC_CTX_S));
        pthread_create(&vencThread[u32Ch], 0, venc_send_frame, reinterpret_cast<void *>(&stVencCtx[u32Ch]));
        pthread_create(&getStreamThread[u32Ch], 0, venc_get_stream, reinterpret_cast<void *>(&stVencCtx[u32Ch]));
    }

    for (u32Ch = 0; u32Ch < ctx->u32ChNum; u32Ch++) {
        pthread_join(vencThread[u32Ch], RK_NULL);
        pthread_join(getStreamThread[u32Ch], RK_NULL);

        stVencCtx[u32Ch].threadExit = RK_TRUE;
        RK_MPI_VENC_StopRecvFrame(u32Ch);

        RK_MPI_VENC_DestroyChn(u32Ch);
        RK_MPI_MB_DestroyPool(stVencCtx[u32Ch].vencPool);
    }

    return RK_SUCCESS;
}

static const char *const usages[] = {
    "./rk_mpi_venc_test [-i SRC_PATH] [-w SRC_WIDTH] [-h SRC_HEIGHT]",
    NULL,
};

static void mpi_venc_test_show_options(const TEST_VENC_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name       : %s\n", ctx->srcFileUri);
    RK_PRINT("output file name       : %s\n", ctx->dstFilePath);
    RK_PRINT("src width              : %d\n", ctx->u32SrcWidth);
    RK_PRINT("src height             : %d\n", ctx->u32SrcHeight);
    RK_PRINT("src virWidth           : %d\n", ctx->u32srcVirWidth);
    RK_PRINT("src virHeight          : %d\n", ctx->u32srcVirHeight);
    RK_PRINT("src pixel format       : %d\n", ctx->u32SrcPixFormat);
    RK_PRINT("encode codec type      : %d\n", ctx->u32DstCodec);
    RK_PRINT("loop count             : %d\n", ctx->s32LoopCount);
    RK_PRINT("channel index          : %d\n", ctx->u32ChnIndex);
    RK_PRINT("channel num            : %d\n", ctx->u32ChNum);
    RK_PRINT("output buffer count    : %d\n", ctx->u32StreamBufCnt);
    RK_PRINT("one picture size       : %d\n", ctx->u32BufferSize);
    return;
}

int main(int argc, const char **argv) {
    RK_S32 s32Ret = RK_SUCCESS;
    TEST_VENC_CTX_S ctx;
    memset(&ctx, 0, sizeof(TEST_VENC_CTX_S));

    ctx.s32LoopCount    = 1;
    ctx.u32StreamBufCnt = 8;
    ctx.u32ChNum        = 1;
    ctx.u32SrcPixFormat = RK_FMT_YUV420SP;
    ctx.u32DstCodec     = RK_VIDEO_ID_AVC;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input",  &(ctx.srcFileUri),
                   "input file name. <required>", NULL, 0, 0),
        OPT_STRING('o', "output", &(ctx.dstFilePath),
                   "the directory of encoder output", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx.s32LoopCount),
                    "loop running count. default(1)", NULL, 0, 0),
        OPT_INTEGER('w', "width", &(ctx.u32SrcWidth),
                    "input source width. <required>", NULL, 0, 0),
        OPT_INTEGER('h', "height", &(ctx.u32SrcHeight),
                    "input source height. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "vir_width", &(ctx.u32srcVirWidth),
                    "input source virWidth.", NULL, 0, 0),
        OPT_INTEGER('\0', "vir_height", &(ctx.u32srcVirHeight),
                    "input source virHeight.", NULL, 0, 0),
        OPT_INTEGER('f', "pixel_format", &(ctx.u32SrcPixFormat),
                    "input source pixel format. default(0: NV12).", NULL, 0, 0),
        OPT_INTEGER('C', "codec", &(ctx.u32DstCodec),
                     "venc encode codec(8:h264, 9:mjpeg, 12:h265,...). default(8)", NULL, 0, 0),
        OPT_INTEGER('c', "channel_count", &(ctx.u32ChNum),
                     "venc channel count. default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "channel_index", &(ctx.u32ChnIndex),
                     "venc channel index. default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "enc_buf_cnt", &(ctx.u32StreamBufCnt),
                     "venc encode output buffer count, default(8)", NULL, 0, 0),
        OPT_INTEGER('\0', "src_pic_size", &(ctx.u32BufferSize),
                     "the size of input single picture", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_venc_test_show_options(&ctx);

    if (check_options(&ctx)) {
        argparse_usage(&argparse);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    if (unit_test_mpi_venc(&ctx) < 0) {
        goto __FAILED;
    }

    s32Ret = RK_MPI_SYS_Exit();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    RK_LOGE("test running success!");
    return RK_SUCCESS;
__FAILED:
    RK_MPI_SYS_Exit();
    RK_LOGE("test running failed!");
    return s32Ret;
}

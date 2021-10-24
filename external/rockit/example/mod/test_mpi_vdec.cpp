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
#include "rk_mpi_vdec.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"
#include "argparse.h"
#include "mpi_test_utils.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavformat/version.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
}

#define MAX_STREAM_CNT               8
#define MAX_TIME_OUT_MS              20

#ifndef VDEC_INT64_MIN
#define VDEC_INT64_MIN               (-0x7fffffffffffffffLL-1)
#endif

#ifndef VDEC_INT64_MAX
#define VDEC_INT64_MAX               INT64_C(9223372036854775807)
#endif

typedef struct _rkMpiVDECCtx {
    const char *srcFileUri;
    const char *dstFilePath;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_S32 s32LoopCount;
    RK_U32 u32ChnIndex;
    RK_U32 u32ChNum;
    RK_U32 u32InputMode;
    RK_CODEC_ID_E enCodecId;
    RK_U32 u32CompressMode;
    RK_U32 u32FrameBufferCnt;
    pthread_t pVdecThread;
    pthread_t pGetPicThread;
    AVFormatContext *avfc;
    RK_U32 u32StreamIndex;
    RK_U32 u32ReadSize;
    RK_BOOL threadExit;
} TEST_VDEC_CTX_S;

static RK_S32 check_options(const TEST_VDEC_CTX_S *ctx) {
    if (ctx->srcFileUri == RK_NULL) {
        goto __FAILED;
    }

    if (ctx->u32InputMode == VIDEO_MODE_STREAM) {
        if (ctx->enCodecId <= RK_VIDEO_ID_Unused ||
            ctx->u32SrcWidth <= 0 ||
            ctx->u32SrcHeight <= 0) {
            goto __FAILED;
        }
    }
    return RK_SUCCESS;

__FAILED:
    return RK_FAILURE;
}

RK_S32 open_parser(TEST_VDEC_CTX_S *ctx) {
    RK_S32 s32Ret = 0;
    RK_U32 u32RetryNum = 5;
    RK_BOOL bFindStream = RK_FALSE;
    AVFormatContext *avfc = RK_NULL;
    const AVStream* stream = RK_NULL;

    avformat_network_init();

    avfc = avformat_alloc_context();

__RETRY:
    s32Ret = avformat_open_input(&(avfc), ctx->srcFileUri, NULL, NULL);
    if (s32Ret < 0) {
        if (s32Ret == -110 && u32RetryNum >= 0) {
            RK_LOGE("AGAIN");
            u32RetryNum--;
            goto __RETRY;
        } else {
            RK_LOGE("open input failed!");
            goto __FAILED;
        }
    }

    if (avformat_find_stream_info(avfc, NULL) < 0) {
        goto __FAILED;
    }

    RK_LOGD("found stream num: %d", avfc->nb_streams);
    for (RK_S32 i = 0; i < avfc->nb_streams; i++) {
        stream = avfc->streams[i];

        if (RK_NULL != stream && stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ctx->enCodecId = (RK_CODEC_ID_E)utils_av_to_rk_codec(stream->codecpar->codec_id);
            ctx->u32SrcWidth = stream->codecpar->width;
            ctx->u32SrcHeight = stream->codecpar->height;
            ctx->u32StreamIndex = stream->index;
            RK_LOGI("found video stream width %d height %d", ctx->u32SrcWidth, ctx->u32SrcHeight);
            bFindStream = RK_TRUE;
            break;
        }
    }

    if (!bFindStream)
        goto __FAILED;

    RK_LOGI("open success %d", bFindStream);
    ctx->avfc = avfc;
    return RK_SUCCESS;

__FAILED:
    if (avfc)
        avformat_close_input(&(avfc));
    return RK_ERR_VDEC_ILLEGAL_PARAM;
}

void* mpi_get_pic(void *pArgs) {
    TEST_VDEC_CTX_S *ctx = reinterpret_cast<TEST_VDEC_CTX_S *>(pArgs);
    FILE *fp = RK_NULL;
    void *data = RK_NULL;
    VIDEO_FRAME_INFO_S sFrame;
    RK_S32 s32Ret;
    char name[256] = {0};
    RK_S32 s32FrameCount = 0;

    memset(&sFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

    if (ctx->dstFilePath != RK_NULL) {
        mkdir(ctx->dstFilePath, 0777);
        snprintf(name, sizeof(name), "%stest_%d.yuv", ctx->dstFilePath, ctx->u32ChnIndex);

        fp = fopen(name, "wb");
        if (fp == RK_NULL) {
            RK_LOGE("can't open output file %s\n", name);
            return NULL;
        }
    }

    while (!ctx->threadExit) {
        s32Ret = RK_MPI_VDEC_GetFrame(ctx->u32ChnIndex, &sFrame, MAX_TIME_OUT_MS);

        if (s32Ret >= 0) {
            s32FrameCount++;
            RK_LOGI("get chn %d frame %d", ctx->u32ChnIndex, s32FrameCount);
            if ((sFrame.stVFrame.u32FrameFlag & FRAME_FLAG_SNAP_END) == FRAME_FLAG_SNAP_END) {
                RK_MPI_VDEC_ReleaseFrame(ctx->u32ChnIndex, &sFrame);
                RK_LOGI("chn %d reach eos frame.", ctx->u32ChnIndex);
                break;
            }
            data = RK_MPI_MB_Handle2VirAddr(sFrame.stVFrame.pMbBlk);
            if (fp) {
                RK_MPI_SYS_MmzFlushCache(sFrame.stVFrame.pMbBlk, RK_TRUE);
                fwrite(data, 1, sFrame.stVFrame.u32VirWidth * sFrame.stVFrame.u32VirHeight * 3 /2, fp);
                fflush(fp);
            }

            RK_MPI_VDEC_ReleaseFrame(ctx->u32ChnIndex, &sFrame);
        } else {
            if (ctx->threadExit)
                break;

            usleep(1000llu);
        }
    }

    if (fp)
        fclose(fp);

    return RK_NULL;
}


static RK_S32 mpi_vdec_free(void *opaque) {
    if (opaque)
        free(opaque);
    return 0;
}

static void* mpi_send_stream(void *pArgs) {
    TEST_VDEC_CTX_S *ctx = reinterpret_cast<TEST_VDEC_CTX_S *>(pArgs);
    RK_S32 s32Size = 0;
    RK_S32 s32Ret = 0;
    RK_U8 *data = RK_NULL;
    FILE *fp = RK_NULL;
    MB_BLK buffer = RK_NULL;
    MB_EXT_CONFIG_S pstMbExtConfig;
    VDEC_CHN_STATUS_S staus;
    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;
    VDEC_STREAM_S stStream;
    RK_S32 s32LoopCount = ctx->s32LoopCount;
    RK_S32 s32PacketCount = 0;
    RK_S32 s32ReachEOS = 0;

    fp = fopen(ctx->srcFileUri, "r");
    if (fp == RK_NULL) {
        RK_LOGE("open file %s failed", ctx->srcFileUri);
        return RK_NULL;
    }

    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stVdecParam, 0, sizeof(VDEC_CHN_PARAM_S));
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    if (ctx->enCodecId == RK_VIDEO_ID_MJPEG) {
        stAttr.enMode = VIDEO_MODE_FRAME;
    } else {
        stAttr.enMode = VIDEO_MODE_STREAM;
    }
    stAttr.enType = ctx->enCodecId;
    stAttr.u32PicWidth = ctx->u32SrcWidth;
    stAttr.u32PicHeight = ctx->u32SrcHeight;
    stAttr.u32PicVirWidth = ctx->u32SrcWidth;
    stAttr.u32PicVirHeight = ctx->u32SrcHeight;
    stAttr.u32FrameBufCnt = ctx->u32FrameBufferCnt;
    stAttr.u32StreamBufCnt = MAX_STREAM_CNT;

    s32Ret = RK_MPI_VDEC_CreateChn(ctx->u32ChnIndex, &stAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed! ", ctx->u32ChnIndex);
        return RK_NULL;
    }
    stVdecParam.stVdecVideoParam.enCompressMode = (COMPRESS_MODE_E) ctx->u32CompressMode;
    RK_MPI_VDEC_SetChnParam(ctx->u32ChnIndex, &stVdecParam);
    RK_MPI_VDEC_StartRecvStream(ctx->u32ChnIndex);

    while (!ctx->threadExit) {
        data = reinterpret_cast<RK_U8 *>(calloc(ctx->u32ReadSize, sizeof(RK_U8)));
        memset(data, 0, ctx->u32ReadSize);
        s32Size = fread(data, 1, ctx->u32ReadSize, fp);
        if (s32Size <= 0) {
            s32ReachEOS = 1;
            if (s32LoopCount > 0) {
                s32LoopCount--;
                RK_LOGI("finish dec count %d\n", ctx->s32LoopCount - s32LoopCount);
                if (s32LoopCount > 0) {
                    mpi_vdec_free(data);
                    s32ReachEOS = 0;
                    fseek(fp, 0, SEEK_SET);
                    continue;
                }
            }
        }

        pstMbExtConfig.pFreeCB = mpi_vdec_free;
        pstMbExtConfig.pOpaque = data;
        pstMbExtConfig.pu8VirAddr = data;
        pstMbExtConfig.u64Size = s32Size;

        RK_MPI_SYS_CreateMB(&buffer, &pstMbExtConfig);

        stStream.u64PTS = 0;
        stStream.pMbBlk = buffer;
        stStream.u32Len = s32Size;
        stStream.bEndOfStream = s32ReachEOS ? RK_TRUE : RK_FALSE;
        stStream.bEndOfFrame = s32ReachEOS ? RK_TRUE : RK_FALSE;
        stStream.bBypassMbBlk = RK_TRUE;
__RETRY:
        s32Ret = RK_MPI_VDEC_SendStream(ctx->u32ChnIndex, &stStream, MAX_TIME_OUT_MS);
        if (s32Ret < 0) {
            if (ctx->threadExit) {
                mpi_vdec_free(data);
                RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
                break;
            }
            usleep(1000llu);
            goto  __RETRY;
        } else {
            s32PacketCount++;
            RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
            RK_LOGI("send chn %d packet %d", ctx->u32ChnIndex, s32PacketCount);
        }
        if (s32ReachEOS) {
            RK_LOGI("chn %d input reach EOS", ctx->u32ChnIndex);
            break;
        }
    }

    if (fp)
        fclose(fp);

    RK_LOGI("%s out\n", __FUNCTION__);
    return RK_NULL;
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

void* mpi_send_frame(void *pArgs) {
    TEST_VDEC_CTX_S *ctx = reinterpret_cast<TEST_VDEC_CTX_S *>(pArgs);
    RK_S32 s32Size = 0;
    RK_S32 s32Ret = 0;

    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;
    VDEC_STREAM_S stStream;
    MB_BLK buffer = RK_NULL;
    MB_EXT_CONFIG_S pstMbExtConfig;
    AVPacket *avPacket = RK_NULL;
    RK_BOOL bFindKeyFrame = RK_FALSE;
    RK_S32 s32LoopCount = ctx->s32LoopCount;
    RK_S32 s32PacketCount = 0;
    RK_S32 s32ReachEOS = 0;

    if (open_parser(ctx) < 0)
        goto __FAILED;

    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stVdecParam, 0, sizeof(VDEC_CHN_PARAM_S));
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    stAttr.enMode = VIDEO_MODE_FRAME;
    stAttr.enType = ctx->enCodecId;
    stAttr.u32PicWidth = ctx->u32SrcWidth;
    stAttr.u32PicHeight = ctx->u32SrcHeight;
    stAttr.u32FrameBufCnt = ctx->u32FrameBufferCnt;
    stAttr.u32StreamBufCnt = MAX_STREAM_CNT;

    s32Ret = RK_MPI_VDEC_CreateChn(ctx->u32ChnIndex, &stAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed! ", ctx->u32ChnIndex);
        return RK_NULL;
    }
    stVdecParam.stVdecVideoParam.enCompressMode = (COMPRESS_MODE_E) ctx->u32CompressMode;
    RK_MPI_VDEC_SetChnParam(ctx->u32ChnIndex, &stVdecParam);
    RK_MPI_VDEC_StartRecvStream(ctx->u32ChnIndex);

    while (!ctx->threadExit) {
        avPacket = av_packet_alloc();
        av_init_packet(avPacket);
        s32Ret = av_read_frame(ctx->avfc, avPacket);
        if (s32Ret == AVERROR_EOF || s32Ret == AVERROR_EXIT) {
            s32ReachEOS = 1;

            if (s32LoopCount > 0) {
                s32LoopCount--;
                RK_LOGI("finish dec count %d\n", ctx->s32LoopCount - s32LoopCount);
                if (s32LoopCount > 0) {
                    mpi_ffmpeg_free(avPacket);
                    s32ReachEOS = 0;

                    avformat_seek_file(ctx->avfc, ctx->u32StreamIndex, VDEC_INT64_MIN,
                                        0, VDEC_INT64_MAX, AVSEEK_FLAG_BYTE);
                    bFindKeyFrame = RK_FALSE;
                    RK_LOGE("seek finish chn %d", ctx->u32ChnIndex);
                    continue;
                }
            }
        }

       if (ctx->u32StreamIndex == avPacket->stream_index) {
            if (!bFindKeyFrame) {
                if (avPacket->flags & AV_PKT_FLAG_KEY == AV_PKT_FLAG_KEY) {
                    bFindKeyFrame = RK_TRUE;
                } else {
                    mpi_ffmpeg_free(avPacket);
                    continue;
                }
            }
            pstMbExtConfig.pFreeCB = mpi_ffmpeg_free;
            pstMbExtConfig.pOpaque = avPacket;
            pstMbExtConfig.pu8VirAddr = avPacket->data;
            pstMbExtConfig.u64Size = avPacket->size;

            RK_MPI_SYS_CreateMB(&buffer, &pstMbExtConfig);

            stStream.u64PTS = avPacket->pts;
            stStream.pMbBlk = buffer;
            stStream.u32Len = avPacket->size;
            stStream.bEndOfStream = s32ReachEOS ? RK_TRUE : RK_FALSE;
            stStream.bEndOfFrame = s32ReachEOS ? RK_TRUE : RK_FALSE;
            stStream.bBypassMbBlk = RK_TRUE;
__RETRY:
            s32Ret = RK_MPI_VDEC_SendStream(ctx->u32ChnIndex, &stStream, MAX_TIME_OUT_MS);
            if (s32Ret < 0) {
                if (ctx->threadExit) {
                     mpi_ffmpeg_free(avPacket);
                     RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
                     break;
                }
                usleep(1000llu);
                goto  __RETRY;
            } else {
                RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
                s32PacketCount++;
                RK_LOGI("send chn %d packet %d", ctx->u32ChnIndex, s32PacketCount);
            }
            if (s32ReachEOS) {
                break;
            }
        } else {
            mpi_ffmpeg_free(avPacket);
        }
    }

    if (ctx->avfc)
        avformat_close_input(&(ctx->avfc));

__FAILED:
    RK_LOGD("%s out\n", __FUNCTION__);
    return RK_SUCCESS;
}

RK_S32 unit_test_mpi_vdec(TEST_VDEC_CTX_S *ctx) {
    RK_U32 u32Ch = 0;
    TEST_VDEC_CTX_S vdecCtx[VDEC_MAX_CHN_NUM];
    pthread_t vdecThread[VDEC_MAX_CHN_NUM];
    pthread_t getPicThread[VDEC_MAX_CHN_NUM];

    for (u32Ch = 0; u32Ch < ctx->u32ChNum; u32Ch++) {
        if (ctx->u32ChNum > 1) {
            ctx->u32ChnIndex = u32Ch;
        }
        memcpy(&(vdecCtx[u32Ch]), ctx, sizeof(TEST_VDEC_CTX_S));
        if (ctx->u32InputMode == VIDEO_MODE_STREAM || ctx->enCodecId == RK_VIDEO_ID_MJPEG) {
            // only support single jpeg picture in file
            if (vdecCtx[u32Ch].enCodecId == RK_VIDEO_ID_MJPEG) {
                vdecCtx[u32Ch].u32ReadSize = ctx->u32SrcWidth * ctx->u32SrcHeight;
            }
            pthread_create(&vdecThread[u32Ch], 0, mpi_send_stream, reinterpret_cast<void *>(&vdecCtx[u32Ch]));
        } else  {
            pthread_create(&vdecThread[u32Ch], 0, mpi_send_frame, reinterpret_cast<void *>(&vdecCtx[u32Ch]));
        }

         pthread_create(&getPicThread[u32Ch], 0, mpi_get_pic, reinterpret_cast<void *>(&vdecCtx[u32Ch]));
    }

    for (u32Ch = 0; u32Ch < ctx->u32ChNum; u32Ch++) {
        pthread_join(vdecThread[u32Ch], RK_NULL);
        pthread_join(getPicThread[u32Ch], RK_NULL);

        vdecCtx[u32Ch].threadExit = RK_TRUE;
        RK_MPI_VDEC_StopRecvStream(u32Ch);
        RK_MPI_VDEC_DestroyChn(u32Ch);
    }

    return RK_SUCCESS;
}

static void mpi_vdec_test_show_options(const TEST_VDEC_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input file name        : %s\n", ctx->srcFileUri);
    RK_PRINT("output path            : %s\n", ctx->dstFilePath);
    RK_PRINT("input width            : %d\n", ctx->u32SrcWidth);
    RK_PRINT("input height           : %d\n", ctx->u32SrcHeight);
    RK_PRINT("input codecId          : %d\n", ctx->enCodecId);
    RK_PRINT("loop count             : %d\n", ctx->s32LoopCount);
    RK_PRINT("channel index          : %d\n", ctx->u32ChnIndex);
    RK_PRINT("channel number         : %d\n", ctx->u32ChNum);
    RK_PRINT("output compress mode   : %d\n", ctx->u32CompressMode);
    RK_PRINT("input mode             : %d\n", ctx->u32InputMode);
    return;
}

static const char *const usages[] = {
    "./rk_mpi_vdec_test [-i SRC_PATH] [-o OUTPUT_PATH]...",
    NULL,
};

int main(int argc, const char **argv) {
    TEST_VDEC_CTX_S ctx;
    memset(&ctx, 0, sizeof(TEST_VDEC_CTX_S));

    ctx.u32InputMode = VIDEO_MODE_FRAME;
    ctx.s32LoopCount = 1;
    ctx.u32CompressMode = COMPRESS_MODE_NONE;  // Suggest::COMPRESS_AFBC_16x16;
    ctx.u32FrameBufferCnt = 8;
    ctx.u32ReadSize = 1024;
    ctx.u32ChNum = 1;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input", &(ctx.srcFileUri),
                   "input file name. <required>", NULL, 0, 0),
        OPT_STRING('o', "output", &(ctx.dstFilePath),
                   "the directory of decoder output", NULL, 0, 0),
        OPT_INTEGER('C', "codec", &(ctx.enCodecId),
                   "input stream codec(8:h264, 9:mjpeg, 12:h265,...) <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx.s32LoopCount),
                    "loop running count. default(1)", NULL, 0, 0),
        OPT_INTEGER('w', "width", &(ctx.u32SrcWidth),
                    "input source width <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('h', "height", &(ctx.u32SrcHeight),
                    "input source height <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('\0', "channel_index", &(ctx.u32ChnIndex),
                    "vdec channel index. default(0).", NULL, 0, 0),
        OPT_INTEGER('c', "channel_count", &(ctx.u32ChNum),
                    "vdec channel count. default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "dec_mode", &(ctx.u32InputMode),
                    "vdec decode mode. range(0:StreamMode, 1:FrameMode). default(1)", NULL, 0, 0),
        OPT_INTEGER('\0', "dec_buf_cnt", &(ctx.u32FrameBufferCnt),
                    "vdec decode output buffer count, default(8)", NULL, 0, 0),
        OPT_INTEGER('\0', "compress_mode", &(ctx.u32CompressMode),
                    "vdec compress mode, default(0); 0: NONE, 1: AFBC_16X16", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_vdec_test_show_options(&ctx);

    if (check_options(&ctx)) {
        RK_LOGE("illegal input parameters");
        argparse_usage(&argparse);
        goto __FAILED;
    }

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        goto __FAILED;
    }

    if (unit_test_mpi_vdec(&ctx) < 0) {
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


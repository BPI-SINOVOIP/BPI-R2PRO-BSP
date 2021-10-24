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
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include "rk_debug.h"
#include "rk_mpi_adec.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_ao.h"
#include "rk_mpi_sys.h"
#include "argparse.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavformat/version.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
}

typedef struct _rkAplayCtx {
    const char *srcFilePath;
    RK_S32      s32LoopCount;
    RK_S32      s32ChnNum;
    RK_S32      s32ChnIndex;
    RK_S32      s32DecMode;
    RK_S32      s32InputChannel;
    RK_S32      s32DeviceChannel;
    RK_S32      s32BitWidth;
    RK_S32      s32DevId;
    RK_S32      s32DeviceSampleRate;
    RK_S32      s32InputSampleRate;
    RK_S32      s32PeriodCount;
    RK_S32      s32PeriodSize;
    char       *chCardName;
    char       *chCodecId;
    RK_BOOL     bBlock;

    AVFormatContext *avformat;
} TEST_APLAY_CTX_S;

static RK_S32 test_free_aplay(void *opaque) {
    if (opaque != RK_NULL) {
        free(opaque);
        opaque = RK_NULL;
    }
    return 0;
}

void aplay_dump_stream_info(AVCodecParameters *cpar) {
    RK_LOGI("cpar->bits_per_coded_sample = %d", cpar->bits_per_coded_sample);
    RK_LOGI("cpar->channels = %d", cpar->channels);
    RK_LOGI("cpar->sample_rate = %d", cpar->sample_rate);
    RK_LOGI("cpar->frame_size = %d", cpar->frame_size);
    RK_LOGI("cpar->trailing_padding = %d", cpar->trailing_padding);
    RK_LOGI("cpar->initial_padding = %d", cpar->initial_padding);
    RK_LOGI("cpar->bits_per_raw_sample = %d", cpar->bits_per_raw_sample);
    RK_LOGI("cpar->channel_layout = %d", cpar->channel_layout);
    RK_LOGI("cpar->block_align = %d", cpar->block_align);
}

void aplay_ffmpeg_format_close(AVFormatContext *avformat) {
    if (avformat) {
        avformat_close_input(&(avformat));
    }
}

AVFormatContext *aplay_ffmpeg_format_open(TEST_APLAY_CTX_S *params) {
    RK_S32 ret = 0;
    RK_S32 err = 0;
    RK_S32 retryNum = 5;
    avformat_network_init();
    AVFormatContext *avformat = avformat_alloc_context();
    AVInputFormat *iformat = RK_NULL;

__RETRY:
    if (strstr(params->chCodecId, "g726")) {
        iformat = av_find_input_format("g726");
    } else if (strstr(params->chCodecId, "g711a")) {
        iformat = av_find_input_format("alaw");
    } else if (strstr(params->chCodecId, "g711u")) {
        iformat = av_find_input_format("mulaw");
    } else if (strstr(params->chCodecId, "g722")) {
        iformat = av_find_input_format("g722");
    }

    err = avformat_open_input(&(avformat), params->srcFilePath, iformat, RK_NULL);
    if (err < 0) {
        if (err == -110 && retryNum >= 0) {
            RK_LOGD("AGAIN");
            retryNum--;
            goto __RETRY;
        } else {
            RK_LOGI("avformat_open_input failed, err = %d", err);
            goto __FAILED;
        }
    }

    return avformat;
__FAILED:

    aplay_ffmpeg_format_close(avformat);

    return RK_NULL;
}

static RK_S32 aplay_ffmpeg_free(void *opaque) {
    AVPacket* avPkt = reinterpret_cast<AVPacket*>(opaque);
    if (RK_NULL != avPkt) {
        av_packet_unref(avPkt);
        av_packet_free(&avPkt);
    }
    avPkt = RK_NULL;
    return 0;
}

static AUDIO_SOUND_MODE_E aplay_find_sound_mode(RK_S32 ch) {
    AUDIO_SOUND_MODE_E channel = AUDIO_SOUND_MODE_BUTT;
    switch (ch) {
      case 1:
        channel = AUDIO_SOUND_MODE_MONO;
        break;
      case 2:
        channel = AUDIO_SOUND_MODE_STEREO;
        break;
      default:
        RK_LOGE("channel = %d not support", ch);
        return AUDIO_SOUND_MODE_BUTT;
    }

    return channel;
}

static AUDIO_BIT_WIDTH_E aplay_find_bit_width(RK_S32 bit) {
    AUDIO_BIT_WIDTH_E bitWidth = AUDIO_BIT_WIDTH_BUTT;
    switch (bit) {
      case 8:
        bitWidth = AUDIO_BIT_WIDTH_8;
        break;
      case 16:
        bitWidth = AUDIO_BIT_WIDTH_16;
        break;
      case 24:
        bitWidth = AUDIO_BIT_WIDTH_24;
        break;
      default:
        RK_LOGE("bitWidth(%d) not support", bit);
        return AUDIO_BIT_WIDTH_BUTT;
    }

    return bitWidth;
}

static RK_U32 aplay_find_audio_codec_id(char *format) {
    if (strstr(format, "mp3")) {
        return RK_AUDIO_ID_MP3;
    } else if (strstr(format, "aac")) {
        return RK_AUDIO_ID_AAC;
    } else if (strstr(format, "flac")) {
        return RK_AUDIO_ID_FLAC;
    } else if (strstr(format, "mp2")) {
        return RK_AUDIO_ID_MP2;
    } else if (strstr(format, "g722")) {
        return RK_AUDIO_ID_ADPCM_G722;
    } else if (strstr(format, "g726")) {
        return RK_AUDIO_ID_ADPCM_G726;
    } else if (strstr(format, "g711a")) {
        return RK_AUDIO_ID_PCM_ALAW;
    } else if (strstr(format, "g711u")) {
        return RK_AUDIO_ID_PCM_MULAW;
    } else if (strstr(format, "opus")) {
        return RK_AUDIO_ID_OPUS;
    }

    RK_LOGE("test not find codec id : %s", format);
    return -1;
}

RK_S32 test_open_device_aplay(TEST_APLAY_CTX_S *ctx) {
    AUDIO_DEV aoDevId = ctx->s32DevId;
    AUDIO_SOUND_MODE_E soundMode;

    AIO_ATTR_S aoAttr;
    memset(&aoAttr, 0, sizeof(AIO_ATTR_S));

    if (ctx->chCardName) {
        snprintf(reinterpret_cast<char *>(aoAttr.u8CardName),
                 sizeof(aoAttr.u8CardName), "%s", ctx->chCardName);
    }

    aoAttr.soundCard.channels = ctx->s32DeviceChannel;
    aoAttr.soundCard.sampleRate = ctx->s32DeviceSampleRate;
    aoAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;

    AUDIO_BIT_WIDTH_E bitWidth = aplay_find_bit_width(ctx->s32BitWidth);
    if (bitWidth == AUDIO_BIT_WIDTH_BUTT) {
        goto __FAILED;
    }
    aoAttr.enBitwidth = bitWidth;
    aoAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)ctx->s32InputSampleRate;
    soundMode = aplay_find_sound_mode(ctx->s32InputChannel);
    if (soundMode == AUDIO_SOUND_MODE_BUTT) {
        goto __FAILED;
    }
    aoAttr.enSoundmode = soundMode;
    aoAttr.u32FrmNum = ctx->s32PeriodCount;
    aoAttr.u32PtNumPerFrm = ctx->s32PeriodSize;

    aoAttr.u32EXFlag = 0;
    aoAttr.u32ChnCnt = 2;

    RK_MPI_AO_SetPubAttr(aoDevId, &aoAttr);

    RK_MPI_AO_Enable(aoDevId);

    return RK_SUCCESS;
__FAILED:
    return RK_FAILURE;
}

RK_S32 test_init_aplay(TEST_APLAY_CTX_S *ctx) {
    RK_S32 i = 0;
    RK_S32 result;
    ADEC_CHN_ATTR_S stAdecAttr;
    ADEC_CHN AdChn = (ADEC_CHN)(ctx->s32ChnIndex);
    AVFormatContext *avfmt = RK_NULL;

    memset(&stAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));

    avfmt = aplay_ffmpeg_format_open(ctx);
    if (!avfmt) {
        RK_LOGE("failed to ffmpeg format open file: %s", ctx->srcFilePath);
        return RK_FAILURE;
    }
    ctx->avformat = avfmt;

    for (i = 0; i < avfmt->nb_streams; i++) {
        if (avfmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            AVCodecParameters* cpar = avfmt->streams[i]->codecpar;
            aplay_dump_stream_info(cpar);
            stAdecAttr.stAdecCodec.u32BitPerCodedSample = cpar->bits_per_coded_sample;
            stAdecAttr.stAdecCodec.u32Channels = cpar->channels;
            stAdecAttr.stAdecCodec.u32SampleRate = cpar->sample_rate;
            break;
        }
    }

    if (stAdecAttr.stAdecCodec.u32Channels == 0) {
        stAdecAttr.stAdecCodec.u32SampleRate = ctx->s32InputSampleRate;
        stAdecAttr.stAdecCodec.u32Channels = ctx->s32InputChannel;
    }

    RK_U32 codecId = aplay_find_audio_codec_id(ctx->chCodecId);
    if (codecId == -1) {
        return RK_FAILURE;
    }

    stAdecAttr.enType    = (RK_CODEC_ID_E)codecId;
    stAdecAttr.enMode    = (ADEC_MODE_E)ctx->s32DecMode;
    stAdecAttr.u32BufCount = 4;
    stAdecAttr.extraDataSize = 0;
    stAdecAttr.extraData = RK_NULL;

    result = RK_MPI_ADEC_CreateChn(ctx->s32ChnIndex, &stAdecAttr);
    if (result) {
        RK_LOGE("create adec chn %d err:0x%x\n", AdChn, result);
        return RK_FAILURE;
    }

    result =  RK_MPI_AO_EnableChn(ctx->s32DevId, ctx->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ao enable channel fail, aoChn = %d, reason = %x", ctx->s32ChnIndex, result);
        return RK_FAILURE;
    }

    // set sample rate of input data
    result = RK_MPI_AO_EnableReSmp(ctx->s32DevId, ctx->s32ChnIndex,
                                  (AUDIO_SAMPLE_RATE_E)ctx->s32InputSampleRate);
    if (result != 0) {
        RK_LOGE("ao enable channel fail, reason = %x, aoChn = %d", result, ctx->s32ChnIndex);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 deinit_mpi_aplay(TEST_APLAY_CTX_S *ctx) {
    MPP_CHN_S stSrcChn, stDstChn;
    stSrcChn.enModId = RK_ID_ADEC;
    stSrcChn.s32DevId = ctx->s32DevId;
    stSrcChn.s32ChnId = ctx->s32ChnIndex;

    stDstChn.enModId = RK_ID_AO;
    stDstChn.s32DevId = ctx->s32DevId;
    stDstChn.s32ChnId = ctx->s32ChnIndex;

    RK_MPI_AO_WaitEos(ctx->s32DevId, ctx->s32ChnIndex, -1);

    RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);

    RK_MPI_ADEC_DestroyChn(ctx->s32ChnIndex);

    RK_S32 result = RK_MPI_AO_DisableReSmp(ctx->s32DevId, ctx->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ao disable resample fail, reason = %d", result);
        return RK_FAILURE;
    }

    result = RK_MPI_AO_DisableChn(ctx->s32DevId, ctx->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ao disable channel fail, reason = %d", result);
        return RK_FAILURE;
    }

    result =  RK_MPI_AO_Disable(ctx->s32DevId);
    if (result != 0) {
        RK_LOGE("ao disable  fail, reason = %d", result);
        return RK_FAILURE;
    }

    aplay_ffmpeg_format_close(ctx->avformat);
    return RK_SUCCESS;
}

RK_S32 aplay_adec_bind_ao(TEST_APLAY_CTX_S *ctx) {
    /* bind ADEC to AO channel */
    MPP_CHN_S stSrcChn, stDstChn;
    stSrcChn.enModId = RK_ID_ADEC;
    stSrcChn.s32DevId = ctx->s32DevId;
    stSrcChn.s32ChnId = ctx->s32ChnIndex;

    stDstChn.enModId = RK_ID_AO;
    stDstChn.s32DevId = ctx->s32DevId;
    stDstChn.s32ChnId = ctx->s32ChnIndex;
    RK_MPI_SYS_Bind(&stSrcChn, &stDstChn);
    return RK_SUCCESS;
}

static void *aplay_send_pack_thread(void *arg) {
    RK_S32 s32ret = 0;
    TEST_APLAY_CTX_S *params = reinterpret_cast<TEST_APLAY_CTX_S *>(arg);
    RK_U8 *srcData = RK_NULL;
    RK_S32 srcSize = 0;
    RK_S32 err = 0;
    RK_S32 pktEos = 0;
    AUDIO_STREAM_S stAudioStream;
    RK_BOOL bBlock = params->bBlock;
    ADEC_CHN AdChn = (ADEC_CHN)(params->s32ChnIndex);
    RK_U64 timeStamp = 0;
    RK_S32 count = 0;
    AVPacket *avPacket = RK_NULL;
    AVStream *stream = RK_NULL;

    aplay_adec_bind_ao(params);

    while (1) {
__RETRY:
        avPacket = av_packet_alloc();
        av_init_packet(avPacket);
        err = av_read_frame(params->avformat, avPacket);
        if (err == AVERROR_EOF || err == AVERROR_EXIT) {
            srcData = RK_NULL;
            srcSize = 0;
        } else {
            stream  = params->avformat->streams[avPacket->stream_index];
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                srcData = avPacket->data;
                srcSize = avPacket->size;
            } else {
                srcData = RK_NULL;
                srcSize = 0;
                aplay_ffmpeg_free(avPacket);
                goto __RETRY;
            }
        }

        RK_LOGI("srcData = %p, srcSize = %d", srcData, srcSize);
        if (srcSize == 0 || srcData == RK_NULL) {
            RK_LOGI("read eos packet, now send eos packet!");
            pktEos = 1;
        }

        if (pktEos) {
            RK_MPI_ADEC_SendEndOfStream(AdChn, RK_FALSE);
            break;
        } else {
            stAudioStream.u32Len = srcSize;
            stAudioStream.u64TimeStamp = timeStamp;
            stAudioStream.u32Seq = ++count;
            stAudioStream.bBypassMbBlk = RK_TRUE;
            MB_EXT_CONFIG_S extConfig = {0};
            extConfig.pFreeCB =  aplay_ffmpeg_free;
            extConfig.pOpaque = avPacket;
            extConfig.pu8VirAddr = srcData;
            extConfig.u64Size    = srcSize;
            RK_MPI_SYS_CreateMB(&(stAudioStream.pMbBlk), &extConfig);
__RETRYSEND:
            s32ret = RK_MPI_ADEC_SendStream(AdChn, &stAudioStream, bBlock);
            if (s32ret != RK_SUCCESS) {
                RK_LOGE("fail to send adec stream.");
                goto __RETRYSEND;
            }
            RK_MPI_MB_ReleaseMB(stAudioStream.pMbBlk);
        }
        timeStamp++;
    }

__FAILED:

    return RK_NULL;
}

static void *aplay_send_stream_thread(void *arg) {
    RK_S32 s32ret = 0;
    TEST_APLAY_CTX_S *params = reinterpret_cast<TEST_APLAY_CTX_S *>(arg);
    RK_U8 *srcData = RK_NULL;
    RK_S32 srcSize = 0;
    FILE  *file = RK_NULL;
    RK_S32 err = 0;
    RK_S32 pktEos = 0;
    AUDIO_STREAM_S stAudioStream;
    RK_BOOL bBlock = params->bBlock;
    ADEC_CHN AdChn = (ADEC_CHN)(params->s32ChnIndex);
    RK_U64 timeStamp = 0;
    RK_S32 count = 0;
    RK_U32 codecId = 0;

    aplay_adec_bind_ao(params);

    file = fopen(params->srcFilePath, "rb");
    if (file == RK_NULL) {
        RK_LOGE("failed to open output file %s, error: %s", params->srcFilePath, strerror(errno));
        goto __FAILED;
    }

    codecId = aplay_find_audio_codec_id(params->chCodecId);
    if (codecId == RK_AUDIO_ID_MP3 && params->avformat->pb) {
        RK_U32 dataOffset = avio_tell(params->avformat->pb);
        if (dataOffset > 0) {
            fseek(file, dataOffset, 0);
        }
        RK_LOGI("header data offset(%ld)", dataOffset);
    }

    while (1) {
        srcData = reinterpret_cast<RK_U8 *>(calloc(512, sizeof(RK_U8)));
        memset(srcData, 0, 512);
        srcSize = fread(srcData, 1, 512, file);

        RK_LOGI("srcData = %p, srcSize = %d", srcData, srcSize);
        if (srcSize == 0 || srcData == RK_NULL) {
            RK_LOGI("read eos packet, now send eos packet!");
            pktEos = 1;
        }

        if (pktEos) {
            RK_MPI_ADEC_SendEndOfStream(AdChn, RK_FALSE);
            break;
        } else {
            stAudioStream.u32Len = srcSize;
            stAudioStream.u64TimeStamp = timeStamp;
            stAudioStream.u32Seq = ++count;
            stAudioStream.bBypassMbBlk = RK_TRUE;
            MB_EXT_CONFIG_S extConfig = {0};
            extConfig.pFreeCB = test_free_aplay;
            extConfig.pOpaque = srcData;
            extConfig.pu8VirAddr = srcData;
            extConfig.u64Size    = srcSize;
            RK_MPI_SYS_CreateMB(&(stAudioStream.pMbBlk), &extConfig);
__RETRY:
            s32ret = RK_MPI_ADEC_SendStream(AdChn, &stAudioStream, bBlock);
            if (s32ret != RK_SUCCESS) {
                RK_LOGE("fail to send adec stream.");
                goto __RETRY;
            }
            RK_MPI_MB_ReleaseMB(stAudioStream.pMbBlk);
        }
        timeStamp++;
    }

__FAILED:
    if (file) {
        fclose(file);
        file = RK_NULL;
    }
    return RK_NULL;
}

RK_S32 unit_test_adec_bind_ao(TEST_APLAY_CTX_S *ctx) {
    RK_S32 i = 0;
    TEST_APLAY_CTX_S params[AO_MAX_CHN_NUM];
    pthread_t tidSend[AO_MAX_CHN_NUM];

    if (test_open_device_aplay(ctx) != RK_SUCCESS) {
        goto __FAILED;
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        memcpy(&(params[i]), ctx, sizeof(TEST_APLAY_CTX_S));
        params[i].s32ChnIndex = i;

        if (test_init_aplay(&params[i]) != RK_SUCCESS) {
            goto __FAILED;
        }

        if (ctx->s32DecMode == ADEC_MODE_STREAM) {
            pthread_create(&tidSend[i], RK_NULL, aplay_send_stream_thread, reinterpret_cast<void *>(&params[i]));
        } else {
            pthread_create(&tidSend[i], RK_NULL, aplay_send_pack_thread, reinterpret_cast<void *>(&params[i]));
        }
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        pthread_join(tidSend[i], RK_NULL);
        deinit_mpi_aplay(&params[i]);
    }

    return RK_SUCCESS;
__FAILED:
    return RK_FAILURE;
}

static const char *const usages[] = {
    "./rk_mpi_aplay_test [-i src_path] [-C name] [--input_ch ch]",
    "[--device_ch ch] [--input_rate rate] [--device_rate rate]...",
    NULL,
};

static void mpi_aplay_test_show_options(const TEST_APLAY_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name      : %s\n", ctx->srcFilePath);
    RK_PRINT("loop count            : %d\n", ctx->s32LoopCount);
    RK_PRINT("channel number        : %d\n", ctx->s32ChnNum);
    RK_PRINT("codec name            : %s\n", ctx->chCodecId);
    RK_PRINT("open device rate      : %d\n", ctx->s32DeviceSampleRate);
    RK_PRINT("input stream rate     : %d\n", ctx->s32InputSampleRate);
    RK_PRINT("open device channel   : %d\n", ctx->s32DeviceChannel);
    RK_PRINT("input channel         : %d\n", ctx->s32InputChannel);
    RK_PRINT("bit_width             : %d\n", ctx->s32BitWidth);
    RK_PRINT("period_count          : %d\n", ctx->s32PeriodCount);
    RK_PRINT("period_size           : %d\n", ctx->s32PeriodSize);
    RK_PRINT("sound card name       : %s\n", ctx->chCardName);
    RK_PRINT("device id             : %d\n", ctx->s32DevId);
}

int main(int argc, const char **argv) {
    RK_S32            i;
    RK_S32            s32Ret;
    TEST_APLAY_CTX_S  *ctx;

    ctx = reinterpret_cast<TEST_APLAY_CTX_S *>(malloc(sizeof(TEST_APLAY_CTX_S)));
    memset(ctx, 0, sizeof(TEST_APLAY_CTX_S));

    ctx->srcFilePath     = RK_NULL;
    ctx->s32LoopCount    = 1;
    ctx->s32ChnNum       = 1;
    ctx->s32DecMode      = 0;
    ctx->chCodecId       = RK_NULL;
    ctx->s32BitWidth     = 16;
    ctx->s32PeriodCount  = 4;
    ctx->s32PeriodSize   = 1024;
    ctx->chCardName      = RK_NULL;
    ctx->s32DevId        = 0;
    ctx->bBlock          = RK_TRUE;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input",  &(ctx->srcFilePath),
                   "input file name , e.g.(./*.pcm). <required>", NULL, 0, 0),
        OPT_STRING('C', "codec", &(ctx->chCodecId),
                    "codec, e.g.(mp3, aac, mp2). <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "input_ch", &(ctx->s32InputChannel),
                    "the number of input stream channels. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "device_ch", &(ctx->s32DeviceChannel),
                    "the number of sound card channels. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "input_rate", &(ctx->s32InputSampleRate),
                    "the sample rate of input stream. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "device_rate", &(ctx->s32DeviceSampleRate),
                    "the sample rate of open sound card. <required>", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx->s32LoopCount),
                    "loop running count. default(1)", NULL, 0, 0),
        OPT_INTEGER('c', "channel_count", &(ctx->s32ChnNum),
                    "the count of adec channel. default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "bit_width", &(ctx->s32BitWidth),
                    "the bit width of open sound card, range(8, 16, 24), default(16)", NULL, 0, 0),
        OPT_INTEGER('\0', "period_size", &(ctx->s32PeriodSize),
                    "the period size for open sound card, default(1024)", NULL, 0, 0),
        OPT_INTEGER('\0', "period_count", &(ctx->s32PeriodCount),
                    "the period count for open sound card, default(4)", NULL, 0, 0),
        OPT_STRING('\0', "sound_card_name", &(ctx->chCardName),
                    "the sound name for open sound card, default(NULL)", NULL, 0, 0),
        OPT_INTEGER('\0', "dec_mode", &(ctx->s32DecMode),
                    "the audio stream decode mode, range(0: pack mode, 1: stream mode), default(0)", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_aplay_test_show_options(ctx);

    if (ctx->srcFilePath == RK_NULL
        || ctx->s32InputChannel <= 0
        || ctx->s32DeviceChannel <= 0
        || ctx->s32DeviceSampleRate <= 0
        || ctx->s32InputSampleRate <= 0
        || ctx->chCodecId == RK_NULL) {
        argparse_usage(&argparse);
        goto __FAILED;
    }

    for (i = 0; i < ctx->s32LoopCount; i++) {
        s32Ret = unit_test_adec_bind_ao(ctx);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
    }

__FAILED:
    if (ctx) {
        free(ctx);
        ctx = RK_NULL;
    }

    return 0;
}


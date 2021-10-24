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
#include "rk_mpi_aenc.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_ai.h"
#include "rk_mpi_sys.h"
#include "argparse.h"

typedef struct _rkACaptureCtx {
    const char *dstFilePath;
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
    RK_S32      s32InputFormat;
    RK_S32      s32PeriodCount;
    RK_S32      s32PeriodSize;
    RK_S32      s32MilliSec;
    char       *chCardName;
    char       *chCodecId;
} TEST_ACAPTURE_CTX_S;

static AUDIO_SOUND_MODE_E acapture_find_sound_mode(RK_S32 ch) {
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

static AUDIO_BIT_WIDTH_E acapture_find_bit_width(RK_S32 bit) {
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

static RK_U32 acapture_find_audio_codec_id(char *format) {
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

RK_S32 test_open_device_acapture(TEST_ACAPTURE_CTX_S *ctx) {
    AUDIO_DEV aiDevId = ctx->s32DevId;
    AUDIO_SOUND_MODE_E soundMode;

    AIO_ATTR_S aiAttr;
    RK_S32 result;
    memset(&aiAttr, 0, sizeof(AIO_ATTR_S));

    if (ctx->chCardName) {
        snprintf(reinterpret_cast<char *>(aiAttr.u8CardName),
                 sizeof(aiAttr.u8CardName), "%s", ctx->chCardName);
    }

    aiAttr.soundCard.channels = ctx->s32DeviceChannel;
    aiAttr.soundCard.sampleRate = ctx->s32DeviceSampleRate;
    aiAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;

    AUDIO_BIT_WIDTH_E bitWidth = acapture_find_bit_width(ctx->s32BitWidth);
    if (bitWidth == AUDIO_BIT_WIDTH_BUTT) {
        goto __FAILED;
    }
    aiAttr.enBitwidth = bitWidth;
    aiAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)ctx->s32InputSampleRate;
    soundMode = acapture_find_sound_mode(ctx->s32InputChannel);
    if (soundMode == AUDIO_SOUND_MODE_BUTT) {
        goto __FAILED;
    }
    aiAttr.enSoundmode = soundMode;
    aiAttr.u32FrmNum = ctx->s32PeriodCount;
    aiAttr.u32PtNumPerFrm = ctx->s32PeriodSize;

    aiAttr.u32EXFlag = 0;
    aiAttr.u32ChnCnt = 2;

    RK_MPI_AI_SetPubAttr(aiDevId, &aiAttr);
    if (result != 0) {
        RK_LOGE("ai set attr fail, reason = %d", result);
        goto __FAILED;
    }

    RK_MPI_AI_Enable(aiDevId);
    if (result != 0) {
        RK_LOGE("ai enable fail, reason = %d", result);
        goto __FAILED;
    }

    return RK_SUCCESS;
__FAILED:
    return RK_FAILURE;
}

RK_S32 test_init_acapture(TEST_ACAPTURE_CTX_S *ctx) {
    RK_S32 i = 0;
    RK_S32 result;
    AENC_CHN_ATTR_S stAencAttr;
    AENC_CHN AdChn = (AENC_CHN)(ctx->s32ChnIndex);

    memset(&stAencAttr, 0, sizeof(AENC_CHN_ATTR_S));

    RK_U32 codecId = acapture_find_audio_codec_id(ctx->chCodecId);
    if (codecId == -1) {
        return RK_FAILURE;
    }

    RK_U32 format = acapture_find_bit_width(ctx->s32InputFormat);
    if (codecId == -1) {
        return RK_FAILURE;
    }

    stAencAttr.enType = (RK_CODEC_ID_E)codecId;
    stAencAttr.stAencCodec.u32Channels = ctx->s32InputChannel;
    stAencAttr.stAencCodec.u32SampleRate = ctx->s32InputSampleRate;
    stAencAttr.stAencCodec.enBitwidth = (AUDIO_BIT_WIDTH_E)format;
    stAencAttr.u32BufCount = 4;
    stAencAttr.extraDataSize = 0;
    stAencAttr.extraData = RK_NULL;

    result = RK_MPI_AENC_CreateChn(AdChn, &stAencAttr);
    if (result) {
        RK_LOGE("create aenc chn %d err:0x%x\n", AdChn, result);
        goto __FAILED;
    }

    result =  RK_MPI_AI_EnableChn(ctx->s32DevId, ctx->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ao enable channel fail, aoChn = %d, reason = %x", ctx->s32ChnIndex, result);
        goto __FAILED;
    }

    // set sample rate of input data
    result = RK_MPI_AI_EnableReSmp(ctx->s32DevId, ctx->s32ChnIndex,
                                  (AUDIO_SAMPLE_RATE_E)ctx->s32InputSampleRate);
    if (result != 0) {
        RK_LOGE("ao enable channel fail, reason = %x, aoChn = %d", result, ctx->s32ChnIndex);
        goto __FAILED;
    }

    return RK_SUCCESS;
__FAILED:
    return RK_FAILURE;
}

RK_S32 deinit_mpi_acapture(TEST_ACAPTURE_CTX_S *ctx) {
    RK_S32 result;
    MPP_CHN_S stSrcChn, stDstChn;
    stSrcChn.enModId = RK_ID_AI;
    stSrcChn.s32DevId = ctx->s32DevId;
    stSrcChn.s32ChnId = ctx->s32ChnIndex;

    stDstChn.enModId = RK_ID_AENC;
    stDstChn.s32DevId = ctx->s32DevId;
    stDstChn.s32ChnId = ctx->s32ChnIndex;

    result = RK_MPI_SYS_UnBind(&stSrcChn, &stDstChn);
    if (result != RK_SUCCESS) {
        RK_LOGE("ai unbind aenc fail, reason = %d", result);
        return RK_FAILURE;
    }

    result = RK_MPI_AENC_DestroyChn(ctx->s32ChnIndex);
    if (result != RK_SUCCESS) {
        RK_LOGE("aenc destroy fail, reason = %d", result);
        return RK_FAILURE;
    }

    result = RK_MPI_AI_DisableReSmp(ctx->s32DevId, ctx->s32ChnIndex);
    if (result != RK_SUCCESS) {
        RK_LOGE("ai disable resample fail, reason = %d", result);
        return RK_FAILURE;
    }

    result = RK_MPI_AI_DisableChn(ctx->s32DevId, ctx->s32ChnIndex);
    if (result != RK_SUCCESS) {
        RK_LOGE("ai disable channel fail, reason = %d", result);
        return RK_FAILURE;
    }

    result =  RK_MPI_AI_Disable(ctx->s32DevId);
    if (result != RK_SUCCESS) {
        RK_LOGE("ai disable fail, reason = %d", result);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 acapture_ai_bind_aenc(TEST_ACAPTURE_CTX_S *ctx) {
    /* bind AI to AENC channel */
    MPP_CHN_S stSrcChn, stDstChn;
    stSrcChn.enModId = RK_ID_AI;
    stSrcChn.s32DevId = ctx->s32DevId;
    stSrcChn.s32ChnId = ctx->s32ChnIndex;

    stDstChn.enModId = RK_ID_AENC;
    stDstChn.s32DevId = ctx->s32DevId;
    stDstChn.s32ChnId = ctx->s32ChnIndex;

    if (RK_MPI_SYS_Bind(&stSrcChn, &stDstChn) != RK_SUCCESS) {
        RK_LOGE("ai bind aenc fail");
        return RK_FAILURE;
    }
    return RK_SUCCESS;
}

static void *acapture_receive_stream_thread(void *arg) {
    RK_S32 s32ret = 0;
    FILE *file = RK_NULL;
    TEST_ACAPTURE_CTX_S *params = reinterpret_cast<TEST_ACAPTURE_CTX_S *>(arg);
    AUDIO_STREAM_S pstStream;
    AENC_CHN AdChn = (AENC_CHN)(params->s32ChnIndex);
    RK_S32 eos = 0;
    RK_S32 count = 0;

    s32ret = acapture_ai_bind_aenc(params);
    if (s32ret != RK_SUCCESS) {
        RK_LOGE("ai and aenc bind error");
        goto __FAILED;
    }

    if (params->dstFilePath) {
        file = fopen(params->dstFilePath, "wb+");
        if (file == RK_NULL) {
            RK_LOGE("failed to open output file %s, error: %s.", params->dstFilePath, strerror(errno));
            goto __FAILED;
        }
    }

    while (1) {
        s32ret = RK_MPI_AENC_GetStream(AdChn, &pstStream, params->s32MilliSec);
        if (s32ret == RK_SUCCESS) {
            MB_BLK bBlk = pstStream.pMbBlk;
            RK_VOID *pstFrame = RK_MPI_MB_Handle2VirAddr(bBlk);
            RK_S32 frameSize = pstStream.u32Len;
            eos = (frameSize <= 0) ? 1 : 0;
            if (pstFrame) {
                RK_LOGI("get frame data = %p, size = %d", pstFrame, frameSize);
                if (file) {
                    fwrite(pstFrame, frameSize, 1, file);
                    fflush(file);
                }
                RK_MPI_AENC_ReleaseStream(AdChn, &pstStream);
                count++;
            }
        } else {
            RK_LOGE("fail to get aenc frame.");
        }

        if (eos) {
            RK_LOGI("get eos stream.");
            break;
        }
    }

__FAILED:
    if (file) {
        fclose(file);
        file = RK_NULL;
    }
    return RK_NULL;
}

RK_S32 unit_test_ai_bind_aenc(TEST_ACAPTURE_CTX_S *ctx) {
    RK_S32 i = 0;
    TEST_ACAPTURE_CTX_S params[AI_MAX_CHN_NUM];
    pthread_t tidcap[AI_MAX_CHN_NUM];

    if (ctx->s32ChnNum > AENC_MAX_CHN_NUM) {
        RK_LOGE("aenc chn(%d) > max_chn(%d)", ctx->s32ChnNum, AENC_MAX_CHN_NUM);
        goto __FAILED;
    }

    if (test_open_device_acapture(ctx) != RK_SUCCESS) {
        goto __FAILED;
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        memcpy(&(params[i]), ctx, sizeof(TEST_ACAPTURE_CTX_S));
        params[i].s32ChnIndex = i;
        params[i].s32MilliSec = -1;

        if (test_init_acapture(&params[i]) != RK_SUCCESS) {
            goto __FAILED;
        }

        pthread_create(&tidcap[i], RK_NULL, acapture_receive_stream_thread, reinterpret_cast<void *>(&params[i]));
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        pthread_join(tidcap[i], RK_NULL);
        deinit_mpi_acapture(&params[i]);
    }

    return RK_SUCCESS;
__FAILED:
    return RK_FAILURE;
}

static const char *const usages[] = {
    "./rk_mpi_acapture_test [-i src_path] [-C name] [--input_ch ch]",
    "[--device_ch ch] [--input_rate rate] [--device_rate rate]...",
    NULL,
};

static void mpi_acapture_test_show_options(const TEST_ACAPTURE_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name      : %s\n", ctx->dstFilePath);
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
    RK_S32 i;
    RK_S32 s32Ret;
    TEST_ACAPTURE_CTX_S *ctx;

    ctx = reinterpret_cast<TEST_ACAPTURE_CTX_S *>(malloc(sizeof(TEST_ACAPTURE_CTX_S)));
    memset(ctx, 0, sizeof(TEST_ACAPTURE_CTX_S));

    ctx->dstFilePath     = RK_NULL;
    ctx->s32LoopCount    = 1;
    ctx->s32ChnNum       = 1;
    ctx->s32DecMode      = 0;
    ctx->chCodecId       = RK_NULL;
    ctx->s32InputFormat  = 16;
    ctx->s32BitWidth     = 16;
    ctx->s32PeriodCount  = 4;
    ctx->s32PeriodSize   = 1024;
    ctx->chCardName      = RK_NULL;
    ctx->s32DevId        = 0;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('o', "output",  &(ctx->dstFilePath),
                   "input file name , e.g.(./*.pcm). <required>", NULL, 0, 0),
        OPT_STRING('C', "codec", &(ctx->chCodecId),
                    "codec, e.g.(g711u, aac, g711a). <required>", NULL, 0, 0),
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
        OPT_INTEGER('\0', "input_format", &(ctx->s32InputFormat),
                    "the input format of input stream. <required>", NULL, 0, 0),
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
    mpi_acapture_test_show_options(ctx);

    if (ctx->dstFilePath == RK_NULL
        || ctx->s32InputChannel <= 0
        || ctx->s32DeviceChannel <= 0
        || ctx->s32DeviceSampleRate <= 0
        || ctx->s32InputSampleRate <= 0
        || ctx->chCodecId == RK_NULL) {
        argparse_usage(&argparse);
        goto __FAILED;
    }

    for (i = 0; i < ctx->s32LoopCount; i++) {
        s32Ret = unit_test_ai_bind_aenc(ctx);
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


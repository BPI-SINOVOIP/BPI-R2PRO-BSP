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

#ifndef INCLUDE_RT_MPI_RK_COMM_ADEC_H_
#define INCLUDE_RT_MPI_RK_COMM_ADEC_H_

#include "rk_common.h"
#include "rk_comm_mb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum rkADEC_MODE_E {
    /*
     * require input is valid dec pack(a complete frame encode result),
     * e.g.the stream get from AENC is a valid dec pack, the stream know
     * actually pack len from file is also a dec pack.
     * this mode is high-performative*/
    ADEC_MODE_PACK = 0,

    /*
     * input is stream,low-performative, if you couldn't find out whether
     * a stream is vaild dec pack,you could use this mode
     */
    ADEC_MODE_STREAM,

    ADEC_MODE_BUTT
} ADEC_MODE_E;

typedef struct rkADEC_ATTR_CODEC_S {
    RK_U32 u32Channels;
    RK_U32 u32SampleRate;
    RK_U32 u32BitPerCodedSample;     // codewords
} ADEC_ATTR_CODEC_S;

typedef struct rkADEC_CH_ATTR_S {
    RK_CODEC_ID_E     enType;         /* audio codec id */
    ADEC_MODE_E       enMode;         /* decode mode */
    ADEC_ATTR_CODEC_S stAdecCodec;    /* channel count & samplerate */
    RK_U32            u32BufCount;    /* decode buffer count */
    MB_BLK            extraData;      /* decode key parameters */
    RK_U32            extraDataSize;  /* key parameters size */
} ADEC_CHN_ATTR_S;

typedef struct rkADEC_CH_STATE_S {
    RK_BOOL bEndOfStream;             /* EOS flag */
    RK_U32 u32BufferFrmNum;           /* total number of channel buffer */
    RK_U32 u32BufferFreeNum;          /* free number of channel buffer */
    RK_U32 u32BufferBusyNum;          /* busy number of channel buffer */
} ADEC_CHN_STATE_S;

typedef enum rkEN_ADEC_ERR_CODE_E {
    ADEC_ERR_DECODER_ERR     = 64,
    ADEC_ERR_BUF_LACK,
} RK_ADEC_ERR_CODE_E;

/* invlalid device ID */
#define RK_ERR_ADEC_INVALID_DEVID     RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define RK_ERR_ADEC_INVALID_CHNID     RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define RK_ERR_ADEC_ILLEGAL_PARAM     RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
/* channel exists */
#define RK_ERR_ADEC_EXIST             RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_EXIST)
/* channel unexists */
#define RK_ERR_ADEC_UNEXIST           RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)
/* using a NULL point */
#define RK_ERR_ADEC_NULL_PTR          RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define RK_ERR_ADEC_NOT_CONFIG        RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define RK_ERR_ADEC_NOT_SUPPORT       RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define RK_ERR_ADEC_NOT_PERM          RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define RK_ERR_ADEC_NOMEM             RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_NOMEM)
/* failure caused by malloc buffer */
#define RK_ERR_ADEC_NOBUF             RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_NOBUF)
/* no data in buffer */
#define RK_ERR_ADEC_BUF_EMPTY         RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_EMPTY)
/* no buffer for new data */
#define RK_ERR_ADEC_BUF_FULL          RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_FULL)
/* system is not ready,had not initialed or loaded*/
#define RK_ERR_ADEC_SYS_NOTREADY      RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, RK_ERR_SYS_NOTREADY)
/* decoder internal err */
#define RK_ERR_ADEC_DECODER_ERR       RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, ADEC_ERR_DECODER_ERR)
/* input buffer not enough to decode one frame */
#define RK_ERR_ADEC_BUF_LACK          RK_DEF_ERR(RK_ID_ADEC, RK_ERR_LEVEL_ERROR, ADEC_ERR_BUF_LACK)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  INCLUDE_RT_MPI_RK_COMM_ADEC_H_ */


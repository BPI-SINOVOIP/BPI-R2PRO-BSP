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

#ifndef INCLUDE_RT_MPI_RK_COMM_AENC_H_
#define INCLUDE_RT_MPI_RK_COMM_AENC_H_

#include "rk_type.h"
#include "rk_common.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct rkAENC_ATTR_CODEC_S {
    RK_U32 u32Channels;
    RK_U32 u32SampleRate;
    AUDIO_BIT_WIDTH_E enBitwidth;
} AENC_ATTR_CODEC_S;

typedef struct rkAENC_CHN_ATTR_S {
    RK_CODEC_ID_E     enType;         /* audio codec id */
    AENC_ATTR_CODEC_S stAencCodec;    /* channel count & samplerate */
    RK_U32            u32BufCount;     /* encode buffer count */
    MB_BLK            extraData;      /* encode key parameters */
    RK_U32            extraDataSize;  /* key parameters size */
} AENC_CHN_ATTR_S;

typedef enum rkEN_AENC_ERR_CODE_E {
    AENC_ERR_ENCODER_ERR     = 64 ,
    AENC_ERR_VQE_ERR        = 65 ,
} EN_AENC_ERR_CODE_E;

/* invlalid device ID */
#define RK_ERR_AENC_INVALID_DEVID     RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define RK_ERR_AENC_INVALID_CHNID     RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define RK_ERR_AENC_ILLEGAL_PARAM     RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
/* channel exists */
#define RK_ERR_AENC_EXIST             RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_EXIST)
/* channel unexists */
#define RK_ERR_AENC_UNEXIST           RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)
/* using a NULL point */
#define RK_ERR_AENC_NULL_PTR          RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define RK_ERR_AENC_NOT_CONFIG        RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define RK_ERR_AENC_NOT_SUPPORT       RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change static attribute */
#define RK_ERR_AENC_NOT_PERM          RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define RK_ERR_AENC_NOMEM             RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_NOMEM)
/* failure caused by malloc buffer */
#define RK_ERR_AENC_NOBUF             RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_NOBUF)
/* no data in buffer */
#define RK_ERR_AENC_BUF_EMPTY         RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_EMPTY)
/* no buffer for new data */
#define RK_ERR_AENC_BUF_FULL          RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_FULL)
/* system is not ready,had not initialed or loaded*/
#define RK_ERR_AENC_SYS_NOTREADY      RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, RK_ERR_SYS_NOTREADY)
/* encoder internal err */
#define RK_ERR_AENC_ENCODER_ERR       RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, AENC_ERR_ENCODER_ERR)
/* vqe internal err */
#define RK_ERR_AENC_VQE_ERR       RK_DEF_ERR(RK_ID_AENC, RK_ERR_LEVEL_ERROR, AENC_ERR_VQE_ERR)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of #ifndef INCLUDE_RT_MPI_RK_COMM_AENC_H_*/

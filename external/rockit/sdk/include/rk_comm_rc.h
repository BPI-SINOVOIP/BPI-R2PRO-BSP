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

#ifndef INCLUDE_RT_MPI_RK_COMM_RC_H_
#define INCLUDE_RT_MPI_RK_COMM_RC_H_

#include "rk_type.h"
#include "rk_errno.h"
#include "rk_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* rc quality */
typedef enum rkVENC_RC_QUALITY_E {
    VENC_RC_QUALITY_HIGHEST,
    VENC_RC_QUALITY_HIGHER,
    VENC_RC_QUALITY_HIGH,
    VENC_RC_QUALITY_MEDIUM,
    VENC_RC_QUALITY_LOW,
    VENC_RC_QUALITY_LOWER,
    VENC_RC_QUALITY_LOWEST,
    VENC_RC_QUALITY_BUTT,
} VENC_RC_QUALITY_E;
/* rc mode */
typedef enum rkVENC_RC_MODE_E {
    // H264
    VENC_RC_MODE_H264CBR = 1,
    VENC_RC_MODE_H264VBR,
    VENC_RC_MODE_H264AVBR,
    // MJPEG
    VENC_RC_MODE_MJPEGCBR,
    VENC_RC_MODE_MJPEGVBR,
    // H265
    VENC_RC_MODE_H265CBR,
    VENC_RC_MODE_H265VBR,
    VENC_RC_MODE_H265AVBR,
    VENC_RC_MODE_BUTT,
} VENC_RC_MODE_E;

typedef enum rkVENC_NALU_TYPE_E {
    VENC_NALU_BSLICE = 1,
    VENC_NALU_PSLICE,
    VENC_NALU_ISLICE,
    VENC_NALU_IDRSLICE,
    VENC_NALU_SEI,
    VENC_NALU_VPS,
    VENC_NALU_SPS,
    VENC_NALU_PPS,
    VENC_NALU_BUTT
} VENC_NALU_TYPE_E;

/* the attribute of h264e cbr*/
typedef struct rkVENC_H264_CBR_S {
    RK_U32 u32Gop;  // RW; Range:[1, 65536]; the interval of I Frame.
    RK_U32 u32SrcFrameRateNum;
    RK_U32 u32SrcFrameRateDen;
    RK_U32 fr32DstFrameRateNum;
    RK_U32 fr32DstFrameRateDen;
    RK_U32 u32BitRate;  // RW; Range:[2, 614400]; average bitrate
} VENC_H264_CBR_S;

/* the attribute of h264e vbr*/
typedef struct rkVENC_H264_VBR_S {
    RK_U32 u32Gop;  // RW; Range:[1, 65536]; the interval of ISLICE.
    RK_U32 u32SrcFrameRateNum;
    RK_U32 u32SrcFrameRateDen;
    RK_U32 fr32DstFrameRateNum;
    RK_U32 fr32DstFrameRateDen;
    RK_U32 u32BitRate;  // RW; Range:[2, 614400];the max bitrate
} VENC_H264_VBR_S;

/* the attribute of h264e vbr*/
typedef struct rkVENC_H264_AVBR_S {
    RK_U32 u32Gop;  // RW; Range:[1, 65536]; the interval of ISLICE.
    RK_U32 u32SrcFrameRateNum;
    RK_U32 u32SrcFrameRateDen;
    RK_U32 fr32DstFrameRateNum;
    RK_U32 fr32DstFrameRateDen;
    RK_U32 u32BitRate;  // RW; Range:[2, 614400];the max bitrate
} VENC_H264_AVBR_S;

/* the attribute of mjpege cbr*/
typedef struct rkVENC_MJPEG_CBR_S {
    RK_U32 u32SrcFrameRateNum;
    RK_U32 u32SrcFrameRateDen;
    RK_U32 fr32DstFrameRateNum;
    RK_U32 fr32DstFrameRateDen;
    RK_U32 u32BitRate;  // RW; Range:[2000, 98000000]; average bitrate
} VENC_MJPEG_CBR_S;

/* the attribute of mjpege vbr*/
typedef struct rkVENC_MJPEG_VBR_S {
    RK_U32 u32SrcFrameRateNum;
    RK_U32 u32SrcFrameRateDen;
    RK_U32 fr32DstFrameRateNum;
    RK_U32 fr32DstFrameRateDen;
    RK_U32 u32BitRate;  // RW; Range:[2000, 98000000]; average bitrate
} VENC_MJPEG_VBR_S;

typedef struct rkVENC_H264_CBR_S VENC_H265_CBR_S;
typedef struct rkVENC_H264_VBR_S VENC_H265_VBR_S;
typedef struct rkVENC_H264_AVBR_S VENC_H265_AVBR_S;

/* the attribute of rc*/
typedef struct rkVENC_RC_ATTR_S {
    /* RW; the type of rc*/
    VENC_RC_MODE_E enRcMode;
    union {
    VENC_H264_CBR_S stH264Cbr;
    VENC_H264_VBR_S stH264Vbr;
    VENC_H264_AVBR_S stH264Avbr;

    VENC_MJPEG_CBR_S stMjpegCbr;
    VENC_MJPEG_VBR_S stMjpegVbr;

    VENC_H265_CBR_S stH265Cbr;
    VENC_H265_VBR_S stH265Vbr;
    VENC_H265_AVBR_S stH265Avbr;
    };
} VENC_RC_ATTR_S;

/* The param of H264e */
typedef struct rkVENC_PARAM_H264_S {
    RK_U32 u32StepQp;
    RK_U32 u32MaxQp;   // RW; Range:[8, 51];the max QP value
    RK_U32 u32MinQp;   // RW; Range:[0, 48]; the min QP value,can not be larger than
                       // u32MaxQp
    RK_U32 u32MaxIQp;  // RW; max qp for i frame
    RK_U32 u32MinIQp;  // RW; min qp for i frame,can not be larger
                       // than u32MaxIQp
    // RK_S32  s32MaxReEncodeTimes;        /* RW; Range:[0, 3]; Range:max number
    // of re-encode times.*/
} VENC_PARAM_H264_S;

/* The param of h265e */
typedef struct rkVENC_PARAM_H265_S {
    RK_U32 u32StepQp;
    RK_U32 u32MaxQp;   // RW; Range:[8, 51];the max QP value
    RK_U32 u32MinQp;   // RW; Range:[0, 48];the min QP value ,can not be larger than
                       // u32MaxQp
    RK_U32 u32MaxIQp;  // RW; max qp for i frame
    RK_U32 u32MinIQp;  // RW; min qp for i frame,can not be larger than u32MaxIQp

    // RK_S32  s32MaxReEncodeTimes;         /* RW; Range:[0, 3]; Range:max number
    // of re-encode times.*/
    // RK_U32  u32DeltIpQp;
} VENC_PARAM_H265_S;

/* The param of mjpege */
typedef struct rkVENC_PARAM_MJPEG_S {
    RK_U32 u32Qfactor;
} VENC_PARAM_MJPEG_S;

/* The param of rc */
typedef struct rkVENC_RC_PARAM_S {
    RK_U32 s32FirstFrameStartQp;  // RW; Start QP value of the first frame
    union {
        VENC_PARAM_H264_S stParamH264;
        VENC_PARAM_H265_S stParamH265;
        VENC_PARAM_MJPEG_S stParamMjpeg;
    };
} VENC_RC_PARAM_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_COMM_RC_H_ */

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

#ifndef INCLUDE_RT_MPI_RK_COMM_RGN_H__
#define INCLUDE_RT_MPI_RK_COMM_RGN_H__

#include "rk_common.h"
#include "rk_comm_video.h"
#include "rk_errno.h"
#include "rk_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef RK_U32 RGN_HANDLE;

/* type of video regions */
typedef enum rkRGN_TYPE_E {
    OVERLAY_RGN = 0,     /* video overlay region */
    COVER_RGN,
    MOSAIC_RGN,
    RGN_BUTT
} RGN_TYPE_E;

typedef struct rkOVERLAY_ATTR_S {
    /* bitmap pixel format,now only support BGRA5551 or ARGB8888 */
    PIXEL_FORMAT_E enPixelFmt;
    /* region size,W:[16,RGN_OVERLAY_MAX_WIDTH],align:16,H:[16,RGN_OVERLAY_MAX_HEIGHT],align:16 */
    SIZE_S stSize;
} OVERLAY_ATTR_S;

typedef struct rkOVERLAY_CHN_ATTR_S {
    /* X:[0,OVERLAY_MAX_X_VENC],align:16,Y:[0,OVERLAY_MAX_Y_VENC],align:16 */
    POINT_S stPoint;

    /* background an foreground transparence when pixel format is BGRA5551
      * the pixel format is BGRA5551,when the alpha bit is 1 this alpha is value!
      * range:[0,255]
      */
    RK_U32 u32FgAlpha;

    /* background an foreground transparence when pixel format is BGRA5551
      * the pixel format is BGRA5551,when the alpha bit is 0 this alpha is value!
      * range:[0,255]
      */
    RK_U32 u32BgAlpha;

    RK_U32 u32Layer;   /* OVERLAY region layer range:[0,7]*/
} OVERLAY_CHN_ATTR_S;

typedef enum rkRGN_COORDINATE_E {
    RGN_ABS_COOR = 0,   /*Absolute coordinate*/
    RGN_RATIO_COOR      /*Ratio coordinate*/
} RGN_COORDINATE_E;

typedef struct rkCOVER_CHN_ATTR_S {
    RECT_S stRect;                          /* config of rect */
    RK_U32 u32Color;                        /* RGB888 format */
    RK_U32 u32Layer;                        /* COVER region layer */
    RGN_COORDINATE_E enCoordinate;          /* ratio coordiante or abs coordinate */
} COVER_CHN_ATTR_S;

typedef enum rkMOSAIC_BLK_SIZE_E {
    MOSAIC_BLK_SIZE_8 = 0,    /*block size 8*8 of MOSAIC*/
    MOSAIC_BLK_SIZE_16,       /*block size 16*16 of MOSAIC*/
    MOSAIC_BLK_SIZE_32,       /*block size 32*32 of MOSAIC*/
    MOSAIC_BLK_SIZE_64,       /*block size 64*64 of MOSAIC*/
    MOSAIC_BLK_SIZE_BUTT
} MOSAIC_BLK_SIZE_E;

typedef struct rkMOSAIC_CHN_ATTR_S {
    RECT_S stRect;                 /*location of MOSAIC*/
    MOSAIC_BLK_SIZE_E enBlkSize;   /*block size of MOSAIC*/
    RK_U32 u32Layer;               /*MOSAIC region layer range:[0,3] */
} MOSAIC_CHN_ATTR_S;

typedef union rkRGN_ATTR_U {
    OVERLAY_ATTR_S      stOverlay;      /* attribute of overlay region */
} RGN_ATTR_U;

typedef union rkRGN_CHN_ATTR_U {
    OVERLAY_CHN_ATTR_S      stOverlayChn;      /* attribute of overlay region */
    COVER_CHN_ATTR_S        stCoverChn;        /* attribute of cover region */
    MOSAIC_CHN_ATTR_S       stMosaicChn;       /* attribute of mosic region */
} RGN_CHN_ATTR_U;

/* attribute of a region */
typedef struct rkRGN_ATTR_S {
    RGN_TYPE_E enType;  /* region type */
    RGN_ATTR_U unAttr;  /* region attribute */
} RGN_ATTR_S;

/* attribute of a region */
typedef struct rkRGN_CHN_ATTR_S {
    RK_BOOL           bShow;
    RGN_TYPE_E        enType;     /* region type */
    RGN_CHN_ATTR_U    unChnAttr;  /* region attribute */
} RGN_CHN_ATTR_S;

/* PingPong buffer change when set attr, it needs to remap memory in mpi interface */
#define RK_NOTICE_RGN_BUFFER_CHANGE  RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_NOTICE, RK_SUCCESS)

/* invlalid device ID */
#define RK_ERR_RGN_INVALID_DEVID     RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define RK_ERR_RGN_INVALID_CHNID     RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define RK_ERR_RGN_ILLEGAL_PARAM     RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
/* channel exists */
#define RK_ERR_RGN_EXIST             RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_EXIST)
/*UN exist*/
#define RK_ERR_RGN_UNEXIST           RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)
/* using a NULL point */
#define RK_ERR_RGN_NULL_PTR          RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define RK_ERR_RGN_NOT_CONFIG        RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define RK_ERR_RGN_NOT_SUPPORT       RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define RK_ERR_RGN_NOT_PERM          RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define RK_ERR_RGN_NOMEM             RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_NOMEM)
/* failure caused by malloc buffer */
#define RK_ERR_RGN_NOBUF             RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_NOBUF)
/* no data in buffer */
#define RK_ERR_RGN_BUF_EMPTY         RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_EMPTY)
/* no buffer for new data */
#define RK_ERR_RGN_BUF_FULL          RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_FULL)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define RK_ERR_RGN_BADADDR           RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_BADADDR)
/* resource is busy, eg. destroy a venc chn without unregistering it */
#define RK_ERR_RGN_BUSY              RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_BUSY)
/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define RK_ERR_RGN_NOTREADY          RK_DEF_ERR(RK_ID_RGN, RK_ERR_LEVEL_ERROR, RK_ERR_SYS_NOTREADY)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_COMM_RGN_H__ */


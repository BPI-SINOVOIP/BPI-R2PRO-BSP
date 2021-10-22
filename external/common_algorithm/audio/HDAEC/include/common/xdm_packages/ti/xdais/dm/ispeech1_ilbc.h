/* 
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/**
 *  @file       ti/xdais/dm/ispeech1_ilbc.h
 *
 *  @brief      This header defines all types, constants, enums, and functions
 *              that are needed for iLBC
 */
/**
 *  @defgroup   ti_xdais_dm_ISPEECH1_ILBC  ISPEECH1_ILBC - XDM Speech Interface (iLBC)
 *
 *  This is the XDM speech interface shared between the various iLBC codecs.
 */

#ifndef ti_xdais_dm_ISPEECH1_ILBC_
#define ti_xdais_dm_ISPEECH1_ILBC_

#include "ispeech1.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_xdais_dm_ISPEECH1_ILBC */
/*@{*/


/**
 *  @brief      Codec selection.
 *
 *  @remarks    This parameter is used to select 20ms/30ms frame size for iLBC
 */
typedef enum {
    ISPEECH1_ILBC_CODECSELECT_20MS = 0,      /**< 20MS frame size */
    ISPEECH1_ILBC_CODECSELECT_30MS = 1,      /**< 30MS frame size */

    /** Default setting. */
    ISPEECH1_ILBC_CODECSELECT_DEFAULT = ISPEECH1_ILBC_CODECSELECT_20MS
} ISPEECH1_ILBC_CodecSelect;


/*@}*/

#ifdef __cplusplus
}
#endif

#endif
/*
 *  @(#) ti.xdais.dm; 1, 0, 7,1; 10-7-2011 13:03:56; /db/wtree/library/trees/dais/dais-u07/src/ xlibrary

 */


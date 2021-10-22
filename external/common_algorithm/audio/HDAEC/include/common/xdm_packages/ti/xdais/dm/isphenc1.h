/*==============================================================================
*
*            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION
*
*   Property of Texas Instruments
*   For Use by Texas Instruments and/or its Licensees Only.
*   Restricted rights to use, duplicate or disclose this code are
*   granted through contract.
*   Unauthorized reproduction and/or distribution are strictly prohibited.
*   This product is protected under copyright law and trade secret law as an
*   Unpublished work.
*   Created 2011, (C) Copyright 2011 Texas Instruments.  All rights reserved.
*
*   Component  :
*
*   Filename   : 	ti/xdais/dm/isphenc1.h
*
*   Description:	This header defines all types, constants, and functions
*              		shared by all implementations of the speech/voice encoder
*              		algorithms.
*
*              		This is the xDM 1.00 Speech/Voice Encoder Interface.
*
*=============================================================================*/

#ifndef ti_xdais_dm_ISPHENC1_
#define ti_xdais_dm_ISPHENC1_

#include <ti/xdais/ialg.h>
#include <ti/xdais/xdas.h>
#include "xdm.h"
#include "ispeech1.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_xdais_dm_ISPHENC1 */
/*@{*/

#define ISPHENC1_EOK       XDM_EOK             /**< @copydoc XDM_EOK */
#define ISPHENC1_EFAIL     XDM_EFAIL           /**< @copydoc XDM_EFAIL */
#define ISPHENC1_EUNSUPPORTED XDM_EUNSUPPORTED /**< @copydoc XDM_EUNSUPPORTED */

/**
 *  @brief      This value signifies no output being available at encoder,
 *              for example in the case of intermediate calls in 10msec
 *              frame size execution.
 */
#define ISPHENC1_ENOOUTPUT               1

/**
 *  @brief      This must be the first field of all ISPHENC1
 *              instance objects.
 */
typedef struct ISPHENC1_Obj {
    struct ISPHENC1_Fxns *fxns;
} ISPHENC1_Obj;


/**
 *  @brief      Opaque handle to an ISPHENC1 objects.
 */
typedef struct ISPHENC1_Obj  *ISPHENC1_Handle;


/**
 *  @brief      Defines the creation time parameters for
 *              all ISPHENC1 instance objects.
 *
 *  @remarks    Some of the fields in this structure are optional and depend
 *              on the class of speech encoder you're creating.
 *
 *  @extensibleStruct
 */
typedef struct ISPHENC1_Params {
    XDAS_Int16 size;            /**< @sizeField */
    XDAS_Int16 frameSize;       /**< Input frame size in bytes
                                 *   for sample based codecs.
                                 */
    XDAS_Int16 compandingLaw;   /**< Optional, codec-specific companding law.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_PCM_CompandingLaw
                                 *   @sa ISPEECH1_G726_CompandingLaw
                                 */
    XDAS_Int16 packingType;     /**< Optional, codec-specific packing type.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_PackingType
                                 *   @sa ISPEECH1_G726_PackingType
                                 *   @sa ISPEECH1_WBAMR_PackingType
                                 */
    XDAS_Int16 vadSelection;    /**< Optional, codec-specific voice activity
                                 *   detection selection.  See your
                                 *   codec-specific interface documentation
                                 *   options.
                                 *
                                 *   @sa ISPEECH1_AMR_VADSelect
                                 *   @sa ISPEECH1_SMV_VADSelect
                                 */
    XDAS_Int16 codecSelection;  /**< Optional, codec-specific codec selection.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_CodecSelect
                                 *   @sa ISPHENC1_Params.codecSelection
                                 */
    XDAS_Int16 bitRate;         /**< Optional, codec-specific bit rate.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_BitRate
                                 *   @sa ISPEECH1_G723_BitRate
                                 *   @sa ISPEECH1_G726_BitRate
                                 *   @sa ISPEECH1_WBAMR_BitRate
                                 */
    XDAS_Int16 reserved;        /**< Reserved - serves to pad this structure. */
    XDAS_Int8  **tablesPtr;     /**< Optional pointer to the codec's
                                 *   initialization tables.
                                 *
                                 *   @remarks   This parameter specifies a
                                 *              pointer to the array of the
                                 *              codec's table block pointers.
                                 *              If the application requires
                                 *              the codec to select the
                                 *              default table address array
                                 *              pointer, it should set this
                                 *              to @c NULL.
                                 */
} ISPHENC1_Params;


/**
 *  @brief      This structure defines the codec parameters that can be
 *              modified after creation via ISPHENC1_Fxns.control().
 *
 *  @remarks    It is not necessary that a given implementation support all
 *              dynamic parameters to be configurable at run time.  If a
 *              particular algorithm does not support run-time updates to
 *              a parameter that the application is attempting to change
 *              at runtime, it may indicate this as an error.
 *
 *  @extensibleStruct
 */
typedef struct ISPHENC1_DynamicParams {
    XDAS_Int16 size;            /**< @sizeField */
    XDAS_Int16 frameSize;       /**< Input frame size in bytes for sample
                                 *   based codecs.
                                 */
    XDAS_Int16 bitRate;         /**< Optional, codec-specific bit rate.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_BitRate
                                 *   @sa ISPEECH1_G723_BitRate
                                 *   @sa ISPEECH1_WBAMR_BitRate
                                 */
    XDAS_Int16 mode;            /**< Optional, codec-specific mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_EVRC_Mode
                                 *   @sa ISPEECH1_SMV_Mode
                                 */
    XDAS_Int16 vadFlag;         /**< @copydoc ISPEECH1_VADFlag
                                 *   @sa ISPEECH1_VADFlag.
                                 */
    XDAS_Int16 noiseSuppressionMode; /**< Optional, codec-specific noise
                                 *   suppression mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_EVRC_NoisePreProc
                                 *   @sa ISPEECH1_G723_NoisePreProc
                                 *   @sa ISPEECH1_SMV_NoisePreProc
                                 */
    XDAS_Int16 ttyTddMode;      /**< Optional, codec-specific TTY mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_EVRC_TTYMode
                                 *   @sa ISPEECH1_SMV_TTYMode
                                 */
    XDAS_Int16 dtmfMode;        /**< Optional, codec-specific DTMF mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_SMV_DTMFMode
                                 */
    XDAS_Int16 dataTransmit;    /**< Optional, codec-specific data transmit
                                 *   mode. See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_SMV_DataMode
                                 */
    XDAS_Int16 reserved;        /**< Reserved - serves to pad this structure. */
} ISPHENC1_DynamicParams;


/**
 *  @brief      Defines the input arguments for all ISPHENC1 instance
 *              process function.
 *
 *  @extensibleStruct
 */
typedef struct ISPHENC1_InArgs {
    XDAS_Int16 size;            /**< @sizeField */
    XDAS_Int16 nullTrafficChannel; /**< Enable/disable the null traffic
                                 *   channel.
                                 *
                                 *   @remarks   This field is used for
                                 *              enabling/disabling the
                                 *              null traffic channel
                                 *              frames.  When enabled,
                                 *              CDMA encoders produce
                                 *              1/8th rate frames with
                                 *              all bits set to 1
                                 *              without encoding the
                                 *              actual input.
                                 *
                                 *   @sa ISPEECH1_NullTrafficMode
                                 */
    XDM1_SingleBufDesc data;    /**< Raw data input for data
                                 *   transmission.
                                 */
} ISPHENC1_InArgs;


/**
 *  @brief      Defines instance status parameters.
 *
 *  @extensibleStruct
 */
typedef struct ISPHENC1_Status {
    XDAS_Int16 size;            /**< @sizeField */
    XDAS_Int16 frameSize;       /**< Input frame size in bytes for sample
                                 *   based codecs.
                                 */
    XDAS_Int32 extendedError;   /**< @extendedErrorField */
    XDM1_SingleBufDesc data;    /**< Buffer descriptor for data passing.
                                 *
                                 *   @remarks   If this field is not used,
                                 *              the application <b>must</b>
                                 *              set @c data.buf to NULL.
                                 *
                                 *   @remarks   This buffer can be used as
                                 *              either input or output,
                                 *              depending on the command.
                                 *
                                 *   @remarks   The buffer will be provided
                                 *              by the application, and
                                 *              returned to the application
                                 *              upon return of the
                                 *              ISPHENC1_Fxns.control()
                                 *              call.  The algorithm must
                                 *              not retain a pointer to this
                                 *              data.
                                 *
                                 *   @sa        #XDM_GETVERSION
                                 */
    XDAS_Int16 bitRate;         /**< Optional, codec-specific bit rate.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_BitRate
                                 *   @sa ISPEECH1_G723_BitRate
                                 *   @sa ISPEECH1_WBAMR_BitRate
                                 */
    XDAS_Int16 mode;            /**< Optional, codec-specific mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_EVRC_Mode
                                 *   @sa ISPEECH1_SMV_Mode
                                 */
    XDAS_Int16 vadFlag;         /**< @copydoc ISPEECH1_VADFlag
                                 *   @sa ISPEECH1_VADFlag
                                 */
    XDAS_Int16 noiseSuppressionMode; /**< Optional, codec-specific noise
                                 *   suppression mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_EVRC_NoisePreProc
                                 *   @sa ISPEECH1_G723_NoisePreProc
                                 *   @sa ISPEECH1_SMV_NoisePreProc
                                 */
    XDAS_Int16 ttyTddMode;      /**< Optional, codec-specific TTY mode.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_EVRC_TTYMode
                                 *   @sa ISPEECH1_SMV_TTYMode
                                 */
    XDAS_Int16 dataTransmit;    /**< Optional, codec-specific data transmit
                                 *   mode. See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_SMV_DataMode
                                 */
    XDAS_Int16 compandingLaw;   /**< Optional, codec-specific companding law.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_PCM_CompandingLaw
                                 *   @sa ISPEECH1_G726_CompandingLaw
                                 */
    XDAS_Int16 packingType;     /**< Optional, codec-specific packing type.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_PackingType
                                 *   @sa ISPEECH1_G726_PackingType
                                 *   @sa ISPEECH1_WBAMR_PackingType
                                 */
    XDAS_Int16 vadSelection;    /**< Optional, codec-specific voice activity
                                 *   detection selection.  See your
                                 *   codec-specific interface documentation
                                 *   options.
                                 *
                                 *   @sa ISPEECH1_AMR_VADSelect
                                 *   @sa ISPEECH1_SMV_VADSelect
                                 */
    XDAS_Int16 codecSelection;  /**< Optional, codec-specific codec selection.
                                 *   See your codec-specific interface
                                 *   documentation options.
                                 *
                                 *   @sa ISPEECH1_AMR_CodecSelect
                                 *   @sa ISPHENC1_Params.codecSelection
                                 */
    XDM_AlgBufInfo  bufInfo;    /**< Input and output buffer information.
                                 *   @sa XDM_AlgBufInfo
                                 */
}ISPHENC1_Status;


/**
 *  @brief      Encoder frame types.
 *
 *  @enumWarning
 */
typedef enum {
    ISPHENC1_FTYPE_SPEECH = 0,  /**< Speech frame */
    ISPHENC1_FTYPE_SIDFRAME = 1,/**< SID frames for codecs which support DTX. */
    ISPHENC1_FTYPE_NODATA = 2   /**< Untransmitted frame for codecs which
                                 *   support DTX.
                                 */
} ISPHENC1_FrameType;


/**
 *  @brief      Defines the run time output arguments for
 *              all ISPHENC1 instance objects.
 *
 *  @extensibleStruct
 *
 *  @sa         ISPHENC1_Fxns::process()
 */
typedef struct ISPHENC1_OutArgs {
    XDAS_Int16 size;            /**< @sizeField */
    XDAS_Int16 frameType;       /**< Encoder frame types.
                                 *
                                 *   @sa ISPHENC1_FrameType
                                 */
    XDAS_Int32 extendedError;   /**< @extendedErrorField */
} ISPHENC1_OutArgs;


/**
 *  @brief      Defines the control commands for the ISPHENC1 module.
 *
 *  @remarks    This ID can be extended in IMOD interface for
 *              additional control commands.
 *
 *  @sa         XDM_CmdId
 *
 *  @sa         ISPHENC1_Fxns::control()
 */
typedef  IALG_Cmd ISPHENC1_Cmd;


/**
 *  @brief      Defines all of the operations on ISPHENC1 objects.
 */
typedef struct ISPHENC1_Fxns{
    IALG_Fxns   ialg;             /**< xDAIS algorithm interface.
                                   *
                                   *   @sa      IALG_Fxns
                                   */

/**
 *  @brief      Basic speech/voice decoding call.
 *
 *  @param[in]  handle          Handle to an algorithm instance.
 *  @param[in,out] inBuf        Input buffer descriptor.
 *  @param[in,out] outBuf       Output buffer descriptor.
 *  @param[in]  inArgs          Input arguments.  This is a required
 *                              parameter.
 *  @param[out] outArgs         Ouput results.  This is a required parameter.
 *
 *  @remarks    process() is a blocking call.  When process() returns, the
 *              algorithm's processing is complete.
 *
 *  @pre        @c handle must be a valid algorithm instance handle.
 *
 *  @pre        @c inArgs must not be NULL, and must point to a valid
 *              ISPHENC1_InArgs structure.
 *
 *  @pre        @c outArgs must not be NULL, and must point to a valid
 *              ISPHENC1_OutArgs structure.
 *
 *  @pre        @c inBuf must not be NULL, and must point to a valid
 *              XDM1_SingleBufDesc structure.
 *
 *  @pre        @c inBuf.buf must not be NULL, and must point to
 *              a valid buffer of data that is at least
 *              @c inBuf.bufSize bytes in length.
 *
 *  @pre        @c outBuf must not be NULL, and must point to a valid
 *              XDM1_SingleBufDesc structure.
 *
 *  @pre        @c outBuf.buf must not be NULL, and must point to
 *              a valid buffer of data that is at least
 *              @c outBuf.bufSize bytes in length.
 *
 *  @pre        The buffers in @c inBuf and @c outBuf are physically
 *              contiguous and owned by the calling application.
 *
 *  @post       The algorithm <b>must not</b> modify the contents of @c inArgs.
 *
 *  @post       The algorithm <b>must not</b> modify the contents of
 *              @c inBuf, with the exception of @c inBuf.accessMask.
 *              That is, the data and buffers pointed to by these parameters
 *              must be treated as read-only.
 *
 *  @post       The algorithm <b>must</b> appropriately set/clear the
 *              @c XDM1_SingleBufDesc.accessMask field in both @c inBuf
 *              and @c outBuf.
 *              For example, if the algorithm only read from
 *              @c inBuf.buf using the algorithm processor, it
 *              could utilize #XDM_SETACCESSMODE_READ to update the appropriate
 *              @c accessMask fields.
 *              The application <i>may</i> utilize these
 *              returned values to appropriately manage cache.
 *
 *  @post       The buffers in @c inBuf and @c outBuf are
 *              owned by the calling application.
 *
 *  @retval     ISPHENC1_EOK            @copydoc ISPHENC1_EOK
 *  @retval     ISPHENC1_EFAIL          @copydoc ISPHENC1_EFAIL
 *                                      See ISPHENC1_Status#extendedError
 *                                      for more detailed further error
 *                                      conditions.
 *  @retval     ISPHENC1_EUNSUPPORTED   @copydoc ISPHENC1_EUNSUPPORTED
 */
    XDAS_Int32 (*process)(ISPHENC1_Handle handle,
        XDM1_SingleBufDesc *inBuf, XDM1_SingleBufDesc *outBuf,
        ISPHENC1_InArgs *inArgs, ISPHENC1_OutArgs *outArgs);


/**
 *  @brief      Control behavior of an algorithm.
 *
 *  @param[in]  handle          Handle to an algorithm instance.
 *  @param[in]  id              Command id.  See #XDM_CmdId.
 *  @param[in]  params          Dynamic parameters.  This is a required
 *                              parameter.
 *  @param[out] status          Output results.  This is a required parameter.
 *
 *  @pre        @c handle must be a valid algorithm instance handle.
 *
 *  @pre        @c params must not be NULL, and must point to a valid
 *              ISPHENC1_DynamicParams structure.
 *
 *  @pre        @c status must not be NULL, and must point to a valid
 *              ISPHENC1_Status structure.
 *
 *  @pre        If a buffer is provided in the @c status->data field,
 *              it must be physically contiguous and owned by the calling
 *              application.
 *
 *  @post       The algorithm <b>must not</b> modify the contents of @c params.
 *              That is, the data pointed to by this parameter must be
 *              treated as read-only.
 *
 *  @post       If a buffer was provided in the @c status->data field,
 *              it is owned by the calling application.
 *
 *  @retval     ISPHENC1_EOK            @copydoc ISPHENC1_EOK
 *  @retval     ISPHENC1_EFAIL          @copydoc ISPHENC1_EFAIL
 *                                      See ISPHENC1_Status#extendedError
 *                                      for more detailed further error
 *                                      conditions.
 *  @retval     ISPHENC1_EUNSUPPORTED   @copydoc ISPHENC1_EUNSUPPORTED
 */
    XDAS_Int32 (*control)(ISPHENC1_Handle handle, ISPHENC1_Cmd id,
        ISPHENC1_DynamicParams *params, ISPHENC1_Status *status);

} ISPHENC1_Fxns;


/*@}*/

#ifdef __cplusplus
}
#endif

#endif  /* ti_xdais_dm_ISPHENC1_ */
/*
 *  @(#) ti.xdais.dm; 1, 0, 2,100; 3-31-2007 20:39:17; /db/wtree/library/trees/dais-h11x/src/
 */


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
*   Filename   : 	ti/xdais/dm/xdm.h
*
*   Description:	This header defines all types, constants, and functions
*              	  	shared across the various xDM classes of algorithms.
*
*              		This is the xDM interface.
*
*=============================================================================*/


#ifndef ti_xdais_dm_XDM_
#define ti_xdais_dm_XDM_

#include <ti/xdais/ialg.h>
#include <ti/xdais/xdas.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_xdais_dm_XDM */
/*@{*/

#define XDM_EOK                 IALG_EOK    /**< Success. */
#define XDM_EFAIL               IALG_EFAIL  /**< General failure. */
#define XDM_EUNSUPPORTED        -3          /**< Request is unsupported. */


#ifdef XDM_INCLUDE_DOT9_SUPPORT
/**
 *  @brief      General runtime failure.
 *
 *  @deprecated This is only supported on 0.9 xDM.  To use it, you must
 *              define "XDM_INCLUDE_DOT9_SUPPORT".
 *              In xDM 1.00+, it is required that codecs return "EFAIL", as
 *              "ERUNTIME" is not supported.
 */
#define XDM_ERUNTIME            -2
#endif

#define XDM_MAX_IO_BUFFERS      16          /**< Max I/O Buffers */

/**
 *  @brief      Buffer descriptor for multiple buffers.
 *
 *  @dot
 *  digraph example {
 *    rankdir=LR;
 *    node [shape=record];
 *    XDM_BufDesc [ style=filled, fillcolor=gray98, label="<bufs> XDAS_Int8 **bufs | <numBufs> XDAS_Int32 numBufs | <bufSizes> XDAIS_Int32 *bufSizes"];
 *    bufArray [ label="<f0> ptr to buf 0 |<f1> ptr to buf 1|<f2> ptr to buf 2|.\n.\n.\n" ];
 *    buf0 [ label="<f0> data buf 0" ];
 *    buf1 [ label="<f0> data buf 1" ];
 *    buf2 [ label="<f0> data buf 2" ];
 *    bufSizes [ label="<f0> size of data buf 0 | size of data buf 1 | size of data buf 2|.\n.\n.\n" ];
 *    XDM_BufDesc:bufs -> bufArray:f0;
 *    bufArray:f0 -> buf0:f0;
 *    bufArray:f1 -> buf1:f0;
 *    bufArray:f2 -> buf2:f0;
 *    XDM_BufDesc:bufSizes -> bufSizes:f0;
 *  }
 *  @enddot
 *
 *  @pre        @c numBufs can not be larger than #XDM_MAX_IO_BUFFERS.  Related,
 *              @c *bufs and @c *bufSizes will never be indexed beyond
 *              #XDM_MAX_IO_BUFFERS elements.
 *
 *  @remarks    This data type is commonly used to manage input and output
 *              buffers.
 *
 *  @remarks    If @c *bufs is a sparse array, @c *bufSizes will be a similar
 *              sparse array.  The @c NULL indexes in @c bufs will be ignored
 *              in @c bufSizes.
 *
 *  @remarks    @c numBufs describes the number of buffers in this descriptor.
 *              if @c *bufs is a sparse array, @c numBufs describes
 *              the number of non-NULL buffers in this descriptor;
 *              this is not necessarily the maximum index of the last
 *              buffer pointed to by @c *bufs.
 *
 *  @remarks    An example utilizing XDM_BufDesc as a sparse array would be
 *              the following:
 *  @code
 *              XDM_BufDesc outBufs;
 *              XDAS_Int32  bufSizeArray[XDM_MAX_IO_BUFFERS];
 *              XDAS_Int8  *pBuffers[XDM_MAX_IO_BUFFERS];
 *              XDAS_Int8   buffer1[4096];
 *              XDAS_Int8   buffer2[1024];
 *
 *              // ensure all pBuffers and bufSizeArray are initially NULL
 *              memset(pBuffers, 0, sizeof(pBuffers[0]) * XDM_MAX_IO_BUFFERS);
 *              memset(bufSizeArray, 0,
 *                  sizeof(bufSizeArray[0]) * XDM_MAX_IO_BUFFERS);
 *
 *              pBuffers[0] = buffer1;
 *              pBuffers[4] = buffer2;
 *
 *              bufSizeArray[0] = 4096;
 *              bufSizeArray[4] = 1024;
 *
 *              outBufs.bufs = pBuffers;
 *              outBufs.numBufs = 2;
 *              outBufs.bufSizes = bufSizeArray;
 *  @endcode
 *
 *  @remarks    The following diagram describes graphically the example above.
 *
 *  @dot
 *  digraph example {
 *    rankdir=LR;
 *    node [shape=record];
 *    XDM_BufDesc [ style=filled, fillcolor=gray98, label="<bufs> bufs = pBuffers | <numBufs> numBufs = 2 | <bufSizes> bufSizes = bufSizeArray"];
 *    bufArray [ label="<f0> pBuffers[0] |<f1> NULL |<f2> NULL|<f3> NULL|<f4> pBuffers[4]|NULL|NULL|.\n.\n.\n" ];
 *    buf0 [ label="<f0> buffer1" ];
 *    buf4 [ label="<f0> buffer2" ];
 *    bufSizes [ label="<f0> 4096|0|0|0|1024|0|0| .\n.\n.\n" ];
 *    XDM_BufDesc:bufs -> bufArray:f0;
 *    bufArray:f0 -> buf0:f0;
 *    bufArray:f4 -> buf4:f0;
 *    XDM_BufDesc:bufSizes -> bufSizes:f0;
 *  }
 *  @enddot
 *
 */
typedef struct XDM_BufDesc {
    XDAS_Int8   **bufs;     /**< Pointer to an array containing buffer
                             *   addresses.
                             */
    XDAS_Int32   numBufs;   /**< Number of buffers. */
    XDAS_Int32  *bufSizes;  /**< Size of each buffer in 8-bit bytes. */
} XDM_BufDesc;


/**
 *  @brief      Single buffer descriptor.
 */
typedef struct XDM_SingleBufDesc {
    XDAS_Int8   *buf;       /**< Pointer to a buffer address. */
    XDAS_Int32  bufSize;    /**< Size of @c buf in 8-bit bytes. */
} XDM_SingleBufDesc;



/**
 *  @brief      Single buffer descriptor.
 */
typedef struct XDM1_SingleBufDesc {
    XDAS_Int8   *buf;       /**< Pointer to a buffer address. */
    XDAS_Int32  bufSize;    /**< Size of @c buf in 8-bit bytes. */
    XDAS_Int32  accessMask; /**< Mask filled by the algorithm, declaring
                             *   how the buffer was accessed <b>by the
                             *   algorithm processor</b>.
                             *
                             *   @remarks  If the buffer was <b>not</b>
                             *             accessed by the algorithm
                             *             processor (e.g., it was filled
                             *             via DMA or other hardware
                             *             accelerator that <i>doesn't</i>
                             *             write through the algorithm's
                             *             CPU), then no bits in this mask
                             *             should be set.
                             *
                             *   @remarks  It is acceptible (and
                             *             appropriate!)to set several
                             *             bits in this mask if the
                             *             algorithm accessed the buffer
                             *             in several ways.
                             *
                             *   @remarks  This mask is often used by the
                             *             application and/or framework
                             *             to appropriately manage cache
                             *             on cache-based systems.
                             *
                             *   @sa XDM_AccessMode
                             */
} XDM1_SingleBufDesc;




/**
 *  @brief      Buffer descriptor.
 */
typedef struct XDM1_BufDesc {
    XDAS_Int32   numBufs;   /**< Number of buffers. */
    XDM1_SingleBufDesc descs[XDM_MAX_IO_BUFFERS]; /** Array of buffer
                             * descriptors.
                             */
} XDM1_BufDesc;


/**
 *  @brief      Access modes used to declare how the algorithm accessed buffers.
 *
 *  @remarks    This indicates how the algorithm's <b>CPU</b> accessed the
 *              buffer, independent of DMA or other hardware accellerators.
 *              For example, if the buffer was written to with DMA (as
 *              opposed to writing to the buffer with the CPU write
 *              instructions), the algorithm should <b>not</b> set the
 *              XDM_ACCESSMODE_WRITE bit.
 *
 *  @remarks    The value of the enum is the bit offset into a mask.  The value
 *              of the enum is not the value to assign the mask.
 *
 *  @sa         XDM1_SingleBufDesc
 *  @sa         XDM1_BufDesc
 */
typedef enum {
    XDM_ACCESSMODE_READ = 0,      /**< The algorithm <i>read</i> from the
                                   *   buffer using the CPU.
                                   *
                                   *   @sa      XDM_SETACCESSMODE_READ
                                   *   @sa      XDM_ISACCESSMODE_READ
                                   */
    XDM_ACCESSMODE_WRITE = 1      /**< The algorithm <i>wrote</i> to the
                                   *   buffer using the CPU.
                                   *
                                   *   @sa      XDM_SETACCESSMODE_WRITE
                                   *   @sa      XDM_ISACCESSMODE_WRITE
                                   */
} XDM_AccessMode;


/**
 *  @brief      Check an access mask for CPU read access.
 *
 *  @param      x       access mask.
 *
 *  @remarks    This is typically used by an application.
 *
 *  @sa         XDM1_SingleBufDesc::accessMask
 *  @sa         XDM1_BufDesc::accessMask
 */
#define XDM_ISACCESSMODE_READ(x)    (((x) >> XDM_ACCESSMODE_READ) & 0x1)


/**
 *  @brief      Check an access mask for CPU write access.
 *
 *  @param      x       access mask.
 *
 *  @remarks    This is typically used by an application.
 *
 *  @sa         XDM1_SingleBufDesc::accessMask
 *  @sa         XDM1_BufDesc::accessMask
 */
#define XDM_ISACCESSMODE_WRITE(x)   (((x) >> XDM_ACCESSMODE_WRITE) & 0x1)


/**
 *  @brief      Clear the "CPU read access" bit in an access mask.
 *
 *  @param      x       access mask.
 *
 *  @remarks    This is typically used by an algorithm.
 *
 *  @sa         XDM_SETACCESSMODE_READ
 *  @sa         XDM1_SingleBufDesc::accessMask
 *  @sa         XDM1_BufDesc::accessMask
 */
#define XDM_CLEARACCESSMODE_READ(x)   ((x) &= (~(0x1 << XDM_ACCESSMODE_READ)))


/**
 *  @brief      Clear the "CPU write access" bit in an access mask.
 *
 *  @param      x       access mask.
 *
 *  @remarks    This is typically used by an algorithm.
 *
 *  @sa         XDM_SETACCESSMODE_WRITE
 *  @sa         XDM1_SingleBufDesc::accessMask
 *  @sa         XDM1_BufDesc::accessMask
 */
#define XDM_CLEARACCESSMODE_WRITE(x)   ((x) &= (~(0x1 << XDM_ACCESSMODE_WRITE)))


/**
 *  @brief      Set the bit to indicate CPU read access in an access mask.
 *
 *  @param      x       access mask.
 *
 *  @remarks    This is typically used by an algorithm.
 *
 *  @sa         XDM1_SingleBufDesc::accessMask
 *  @sa         XDM1_BufDesc::accessMask
 */
#define XDM_SETACCESSMODE_READ(x)   ((x) |= (0x1 << XDM_ACCESSMODE_READ))


/**
 *  @brief      Set the bit to indicate CPU write access in an access mask.
 *
 *  @param      x       access mask.
 *
 *  @remarks    This is typically used by an algorithm.
 *
 *  @sa         XDM1_SingleBufDesc::accessMask
 *  @sa         XDM1_BufDesc::accessMask
 */
#define XDM_SETACCESSMODE_WRITE(x)  ((x) |= (0x1 << XDM_ACCESSMODE_WRITE))


/**
 *  @brief      Buffer information descriptor for input and output buffers.
 */
typedef struct XDM_AlgBufInfo {
    XDAS_Int32 minNumInBufs;       /**< Minimum number of input buffers. */
    XDAS_Int32 minNumOutBufs;      /**< Minimum number of output buffers. */
    XDAS_Int32 minInBufSize[XDM_MAX_IO_BUFFERS];  /**< Minimum size, in 8-bit
                                    * bytes, required for each input buffer.
                                    */
    XDAS_Int32 minOutBufSize[XDM_MAX_IO_BUFFERS]; /**< Minimum size, in 8-bit
                                    * bytes, required for each output buffer.
                                    */
} XDM_AlgBufInfo;


/**
 *  @brief      Standard control commands that must be implemented by
 *              xDM compliant multimedia algorithms.
 *
 *  @remarks    Any control ID extension in IMOD interface should start
 *              from 256 onward.  The ID range from 0 to 255 is
 *              reserved.
 */
typedef enum {
    XDM_GETSTATUS = 0,      /**< Query algorithm to fill status structure. */
    XDM_SETPARAMS = 1,      /**< Set run time dynamic parameters. */
    XDM_RESET = 2,          /**< Reset the algorithm.  All fields in the
                             *   internal data structures are reset and all
                             *   internal buffers are flushed.
                             */
    XDM_SETDEFAULT = 3,     /**< Initialize all fields in the param
                             *   structure to their default values.  The
                             *   application can change specific
                             *   parameters using XDM_SETPARAMS.
                             */
    XDM_FLUSH = 4,          /**< Handle end of stream conditions.  This
                             *   command forces the algorithm to output
                             *   data without additional input.  The
                             *   recommended sequence is to call the
                             *   control() function (with XDM_FLUSH)
                             *   followed by repeated calls to the
                             *   process() function until it returns an
                             *   error.
                             *
                             *   @remarks        The algorithm should return
                             *                   the appropriate, class-specific
                             *                   "EFAIL" error (e.g.
                             *                   ISPHDEC1_EFAIL, IVIDENC1_EFAIL,
                             *                   etc), when flushing is
                             *                   complete.
                             */
    XDM_GETBUFINFO = 5,     /**< Query algorithm instance regarding its
                             *   properties of input and output
                             *   buffers.
                             */
    XDM_GETVERSION = 6      /**< Query the algorithm's version.  The result
                             *   will be returned in the @c data field of the
                             *   respective _Status structure.
                             *
                             *   @remarks   There is no specific format
                             *              defined for version returned by
                             *              the algorithm.
                             */
} XDM_CmdId;


/**
 *  @brief      Extended error information.
 *
 *  @remarks    When an internal error occurs, the algorithm will return
 *              an error return value (e.g. EFAIL, EUNSUPPORTED)
 *
 *  @remarks    The value of each enum is the bit which is set.
 *
 *  @remarks    Bits 32-16 and bit 8 are reserved.  Bits 7-0 are codec and
 *              implementation specific.
 *
 *  @remarks    The algorithm can set multiple bits to 1 based on conditions.
 *              e.g. it will set bits #XDM_FATALERROR (fatal) and
 *              #XDM_UNSUPPORTEDPARAM (unsupported params) in case
 *              of unsupported run time parameters.
 */
typedef enum {
    XDM_APPLIEDCONCEALMENT = 9,  /**< Bit 9 - Applied concealment.
                                  *
                                  *   @remarks  This error is applicable
                                  *             for decoders.  It is
                                  *             set when the decoder
                                  *             was not able to able
                                  *             to decode the
                                  *             bitstream, and the
                                  *             decoder has concealed
                                  *             the bitstream error
                                  *             and produced the
                                  *             concealed output.
                                  */
    XDM_INSUFFICIENTDATA = 10,   /**< Bit 10 - Insufficient input data.
                                  *
                                  *   @remarks  This error is typically
                                  *             applicable for
                                  *             decoders. This is set
                                  *             when the input data
                                  *             provided is not
                                  *             sufficient to produce
                                  *             of one frame of data.
                                  *             This can be also be
                                  *             set for encoders when
                                  *             the number of valid
                                  *             samples in the input
                                  *             frame is not
                                  *             sufficient to process
                                  *             a frame.
                                  */
    XDM_CORRUPTEDDATA = 11,      /**< Bit 11 - Data problem/corruption.
                                  *
                                  *   @remarks  This error is typically
                                  *             applicable for
                                  *             decoders.  This is set
                                  *             when the bitstream has
                                  *             an error and not
                                  *             compliant to the
                                  *             standard syntax.
                                  */
    XDM_CORRUPTEDHEADER = 12,    /**< Bit 12 - Header problem/corruption.
                                  *
                                  *   @remarks  This error is typically
                                  *             applicable for
                                  *             decoders.  This is set
                                  *             when the header
                                  *             information in the
                                  *             bitstream is
                                  *             incorrect.  For example,
                                  *             it is set when
                                  *             Sequence/Picture/Slice
                                  *             etc. are incorrect in
                                  *             video decoders.
                                  */
    XDM_UNSUPPORTEDINPUT = 13,   /**< Bit 13 - Unsupported feature/parameter
                                  *   in input.
                                  *
                                  *   @remarks  This error is set when the
                                  *             algorithm is not able
                                  *             process a certain
                                  *             input data/bitstream
                                  *             format.  It can also be
                                  *             set when a subset of
                                  *             features in a standard
                                  *             are not supported by
                                  *             the algorithm.
                                  *
                                  *   @remarks  For example, if a video
                                  *             encoder only supports
                                  *             4:2:2 format, it can
                                  *             set this error for any
                                  *             other type of input
                                  *             video format.
                                  */
    XDM_UNSUPPORTEDPARAM = 14,   /**< Bit 14 - Unsupported input parameter or
                                  *   configuration.
                                  *
                                  *   @remarks  This error is set when the
                                  *             algorithm doesn't
                                  *             support certain
                                  *             configurable
                                  *             parameters.  For
                                  *             example, if the video
                                  *             decoder doesn't
                                  *             support the "display
                                  *             width" feature, it
                                  *             shall return
                                  *             XDM_UNSUPPORTEDPARAM
                                  *             when the control
                                  *             function is called for
                                  *             setting the
                                  *             @c displayWidth
                                  *             attribute.

                                  */
    XDM_FATALERROR = 15          /**< Bit 15 - Fatal error (stop the codec).
                                  *   If there is an error and this
                                  *   bit is not set, the error is a
                                  *   recoverable one.
                                  *
                                  *   @remarks  This error is set when the
                                  *             algorithm cannot
                                  *             recover from the
                                  *             current state.  It
                                  *             informs the system not
                                  *             to try the next frame
                                  *             and possibly delete
                                  *             the multimedia
                                  *             algorithm instance.  It
                                  *             implies the codec
                                  *             shall not work when
                                  *             reset.
                                  *
                                  *   @remarks  The user should delete the
                                  *             current instance of
                                  *             the codec.
                                  */
} XDM_ErrorBit;

/** Check for fatal error */
#define XDM_ISFATALERROR(x)         (((x) >> XDM_FATALERROR) & 0x1)
/** Check for unsupported parameter */
#define XDM_ISUNSUPPORTEDPARAM(x)   (((x) >> XDM_UNSUPPORTEDPARAM) & 0x1)
/** Check for unsupported input */
#define XDM_ISUNSUPPORTEDINPUT(x)   (((x) >> XDM_UNSUPPORTEDINPUT) & 0x1)
/** Check for corrupted header */
#define XDM_ISCORRUPTEDHEADER(x)    (((x) >> XDM_CORRUPTEDHEADER) & 0x1)
/** Check for corrupted data */
#define XDM_ISCORRUPTEDDATA(x)      (((x) >> XDM_CORRUPTEDDATA) & 0x1)
/** Check for insufficient data */
#define XDM_ISINSUFFICIENTDATA(x)   (((x) >> XDM_INSUFFICIENTDATA) & 0x1)
/** Check for applied concealment */
#define XDM_ISAPPLIEDCONCEALMENT(x) (((x) >> XDM_APPLIEDCONCEALMENT) & 0x1)

/** Set fatal error bit */
#define XDM_SETFATALERROR(x)         ((x) |= (0x1 << XDM_FATALERROR))
/** Set unsupported parameter bit */
#define XDM_SETUNSUPPORTEDPARAM(x)   ((x) |= (0x1 << XDM_UNSUPPORTEDPARAM))
/** Set unsupported input bit */
#define XDM_SETUNSUPPORTEDINPUT(x)   ((x) |= (0x1 << XDM_UNSUPPORTEDINPUT))
/** Set corrupted header bit */
#define XDM_SETCORRUPTEDHEADER(x)    ((x) |= (0x1 << XDM_CORRUPTEDHEADER))
/** Set corrupted data bit */
#define XDM_SETCORRUPTEDDATA(x)      ((x) |= (0x1 << XDM_CORRUPTEDDATA))
/** Set insufficient data bit */
#define XDM_SETINSUFFICIENTDATA(x)   ((x) |= (0x1 << XDM_INSUFFICIENTDATA))
/** Set applied concealment bit */
#define XDM_SETAPPLIEDCONCEALMENT(x) ((x) |= (0x1 << XDM_APPLIEDCONCEALMENT))


/**
 *  @brief      Endianness of data
 */
typedef enum {
    XDM_BYTE = 1,           /**< Big endian stream. */
    XDM_LE_16 = 2,          /**< 16 bit little endian stream. */
    XDM_LE_32 = 3           /**< 32 bit little endian stream. */
} XDM_DataFormat;


/**
 *  @brief      Encoding presets.
 */
typedef enum {
    XDM_DEFAULT = 0,        /**< Default setting of encoder.  See
                             *   codec specific documentation for its
                             *   encoding behaviour.
                             */
    XDM_HIGH_QUALITY = 1,   /**< High quality encoding. */
    XDM_HIGH_SPEED = 2,     /**< High speed encoding. */
    XDM_USER_DEFINED = 3    /**< User defined configuration, using
                             *   advanced parameters.
                             */
} XDM_EncodingPreset;


/**
 *  @brief      Decode entire access unit or only header.
 */
typedef enum {
    XDM_DECODE_AU = 0,      /**< Decode entire access unit, including all
                             *   the headers.
                             */
    XDM_PARSE_HEADER = 1    /**< Decode only header. */
} XDM_DecMode;


/**
 *  @brief      Encode entire access unit or only header.
 */
typedef enum {
    XDM_ENCODE_AU = 0,      /**< Encode entire access unit, including the
                             *   headers.
                             */
    XDM_GENERATE_HEADER = 1 /**< Encode only header. */
} XDM_EncMode;


/**
 *  @brief      Chroma formats.
 */
typedef enum {
    XDM_CHROMA_NA = -1,     /**< Chroma format not applicable. */
    XDM_YUV_420P = 1,       /**< YUV 4:2:0 planer. */
    XDM_YUV_422P = 2,       /**< YUV 4:2:2 planer. */
    XDM_YUV_422IBE = 3,     /**< YUV 4:2:2 interleaved (big endian). */
    XDM_YUV_422ILE = 4,     /**< YUV 4:2:2 interleaved (little endian). */
    XDM_YUV_444P = 5,       /**< YUV 4:4:4 planer. */
    XDM_YUV_411P = 6,       /**< YUV 4:1:1 planer. */
    XDM_GRAY = 7,           /**< Gray format. */
    XDM_RGB = 8,            /**< RGB color format. */

    /** Default setting. */
    XDM_CHROMAFORMAT_DEFAULT = XDM_YUV_422ILE
} XDM_ChromaFormat;


/*@}*/  /* ingroup */


#ifdef __cplusplus
}
#endif

#endif  /* ti_xdais_dm_XDM_ */
/*
 *  @(#) ti.xdais.dm; 1, 0, 2,100; 3-31-2007 20:39:17; /db/wtree/library/trees/dais-h11x/src/
 */


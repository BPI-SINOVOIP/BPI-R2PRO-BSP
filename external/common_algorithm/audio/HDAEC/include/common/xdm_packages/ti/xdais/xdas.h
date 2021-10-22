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
*   Filename   : 	ti/xdais/xdas.h
*
*   Description:	This header defines all types and constants used in the
*              		XDAS interfaces.
*
*=============================================================================*/

#ifndef ti_xdais_XDAS_
#define ti_xdais_XDAS_

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_xdais_XDAS_ */
/*@{*/


#define XDAS_TRUE       1
#define XDAS_FALSE      0


typedef Void            XDAS_Void;
typedef Uint8           XDAS_Bool;


typedef Int8            XDAS_Int8;      /**< Actual size chip dependent. */
typedef Uint8           XDAS_UInt8;     /**< Actual size chip dependent. */
typedef Int16           XDAS_Int16;     /**< Actual size of type is 16 bits. */
typedef Uint16          XDAS_UInt16;    /**< Actual size of type is 16 bits. */
typedef Int32           XDAS_Int32;     /**< Actual size of type is 32 bits. */
typedef Uint32          XDAS_UInt32;    /**< Actual size of type is 32 bits. */


/*@}*/


#ifdef __cplusplus
}
#endif

#endif  /* ti_xdais_XDAS_ */
/*
 *  @(#) ti.xdais; 1, 2.0, 0,100; 3-31-2007 20:39:11; /db/wtree/library/trees/dais-h11x/src/
 */


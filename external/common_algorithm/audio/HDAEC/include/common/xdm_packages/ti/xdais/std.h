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
*   Filename   : 	std.h
*
*   Description:
*
*=============================================================================*/

#ifndef xdc_std__include
#define xdc_std__include

#include <stdarg.h>
#include <stddef.h>

extern char *__PLAT__;  

/* TitleCase standard types */

#define xdc_Void        void

typedef char            xdc_Char;
typedef unsigned char   xdc_UChar;
typedef short           xdc_Short;
typedef unsigned short  xdc_UShort;
typedef int             xdc_Int;
typedef unsigned int    xdc_UInt;
typedef long            xdc_Long;
typedef unsigned long   xdc_ULong;
typedef float           xdc_Float;
typedef double          xdc_Double;
typedef long double     xdc_LDouble;
typedef size_t          xdc_SizeT;
typedef va_list         xdc_VaList;

/* Generic Extended Types */

typedef unsigned short  xdc_Bool;
typedef int             (*xdc_Fxn)();   /* function pointer */
typedef void            *xdc_Ptr;       /* data pointer */
typedef char            *xdc_String;    /* null terminated string */

/* 
 * Import the target-specific std.h
 * 
 * 'xdc_target__' is defined by build world to reference the
 * target-specific <std.h> file (i.e. <ti/targets/std.h>).
 */
#ifdef xdc_target_types__
#define xdc_target__ <xdc_target_types__>
#endif
#ifdef xdc_target__
#include xdc_target__
#endif

/* Long Long Types */

#ifdef xdc__LONGLONG__
typedef long long               xdc_LLong;
typedef unsigned long long      xdc_ULLong;
#else
typedef long                    xdc_LLong;
typedef unsigned long           xdc_ULLong;
#endif

/*
 *  ======== [U]Int<n> ========
 */
typedef signed char     xdc_Int8;
typedef unsigned char   xdc_UInt8;
typedef short           xdc_Int16;
typedef unsigned short  xdc_UInt16;
#ifdef _TMS320C6X
 #ifdef __TI_32BIT_LONG__
  typedef long			xdc_Int32;
  typedef unsigned long   xdc_UInt32;
 #else
  typedef int            xdc_Int32;
  typedef unsigned int   xdc_UInt32;
 #endif
#else
 typedef long            xdc_Int32;
 typedef unsigned long	 xdc_UInt32;
#endif

/*
 *  ======== Arg ========
 */
typedef long            xdc_Arg;

/* Unprefixed Aliases */

#ifndef xdc__nolocalnames

#define Void xdc_Void

typedef xdc_Char        Char;
typedef xdc_UChar       UChar;
typedef xdc_Short       Short;
typedef xdc_UShort      UShort;
typedef xdc_Int         Int;
typedef xdc_UInt        UInt;
typedef xdc_Long        Long;
typedef xdc_ULong       ULong;
typedef xdc_LLong       LLong;
typedef xdc_ULLong      ULLong;
typedef xdc_Float       Float;
typedef xdc_Double      Double;
typedef xdc_LDouble     LDouble;
typedef xdc_SizeT       SizeT;
typedef xdc_VaList      VaList;

typedef xdc_Arg         Arg;
typedef xdc_Bool        Bool;
typedef xdc_Int8        Int8;
typedef xdc_Int16       Int16;
typedef xdc_Int32       Int32;
typedef xdc_Fxn         Fxn;
typedef xdc_Ptr         Ptr;
typedef xdc_String      String;

typedef xdc_UInt8       UInt8;
typedef xdc_UInt16      UInt16;
typedef xdc_UInt32      UInt32;

/* DEPRECATED Aliases */
#ifndef XDC__STRICTYPES__
#undef  xdc_DEPRECATEDTYPES
#define xdc_DEPRECATEDTYPES 1
#ifndef _TI_STD_TYPES
#define _TI_STD_TYPES
#endif
typedef xdc_UInt8       Uint8;
typedef xdc_UInt16      Uint16;
typedef xdc_UInt32      Uint32;
typedef xdc_UInt        Uns;
#endif

#ifdef xdc__BITS8__
typedef xdc_Bits8       Bits8;
#endif

#ifdef xdc__BITS16__
typedef xdc_Bits16      Bits16;
#endif

#ifdef xdc__BITS32__
typedef xdc_Bits32      Bits32;
#endif

#endif /* xdc__nolocalnames */

/* Standard Constants */

#undef NULL
#define NULL 0

#undef FALSE
#define FALSE 0

#undef TRUE
#define TRUE 1

#endif /* xdc_std__include */
/*
 *  @(#) xdc.rts; 1, 0, 0, 0,157; 6-28-2007 14:45:45; /db/ztree/library/trees/xdc-m74x/src/iliad/packages/
 */


/*___________________________________________________________________________
 |
 |   File: rk_typedef_user.h
 |
 |   This file contains the Adaptive Digital Technologies, Inc. 
 |   word length assignments based on processor type. This file
 |   is intended to be used in conjunction with Adaptive Digital's algorithms
 |   and is included by the algorithm public (user) header files.
 |   
 |   This software is the property of Adaptive Digital Technologies, Inc.
 |   Unauthorized use is prohibited.
 |
 |   Copyright (c) 2000-2011, Adaptive Digital Technologies, Inc.
 |
 |   www.adaptivedigital.com
 |   610-825-0182
 |
 |______________________________________________________________________________
*/
//.S
#ifndef _RK_TYPEDEF_USER
#define _RK_TYPEDEF_USER
#ifdef __cplusplus
   #define RK_FT extern "C"
#else
   #define RK_FT extern
#endif
#ifdef _WIN32
   #ifndef WIN32
      #define WIN32
   #endif
#endif

// RK_API is used on Windows platforms to make symbols available to the DLL user
#ifdef WIN32
# ifdef _USRDLL_RK
#  ifdef __cplusplus
#   define RK_API extern "C" __declspec(dllexport)
#  else
#   define RK_API __declspec(dllexport)
#  endif
# elif defined(_USING_RK_DLL)
#  ifdef __cplusplus
#   define RK_API extern "C" __declspec(dllimport)
#  else
#   define RK_API __declspec(dllimport)
#  endif
# else
#  ifdef __cplusplus
#   define RK_API extern "C"
#  else
#   define RK_API
#  endif
# endif
#else	// Not WIN32
#  ifdef __cplusplus
#   define RK_API extern "C"
#  else
#   define RK_API
# endif
#endif


#ifdef __TMS470__
     #define __arm
#endif 


   #if defined (_TMS320C5XX)
      typedef short int           RK_Bool;    //16b
      typedef signed char         RK_Int8;    //16b
      typedef unsigned int        RK_UInt8;   //16b
      typedef   signed short int  RK_Int16;   //16b
      typedef unsigned short int  RK_UInt16;  //16b
      typedef   signed long int   RK_Int32;   //32b
      typedef unsigned long int   RK_UInt32;  //32b
      typedef float               RK_Float32; //32b
      typedef   signed long int   RK_Int40;   //40b
      typedef unsigned long int   RK_UInt40;  //40b
      typedef   signed long int   RK_Int64;   //40b
      typedef unsigned long int   RK_UInt64;  //40b
      #define DSP_TYPE 54
      #define LowBitFirst 0


   #elif defined (__TMS320C55XX__)
      typedef short int           RK_Bool;    //16b
      typedef signed char         RK_Int8;    //16b
      typedef unsigned char       RK_UInt8;   //16b
      typedef   signed short int  RK_Int16;   //16b
      typedef unsigned short int  RK_UInt16;  //16b
      typedef   signed long int   RK_Int32;   //32b
      typedef unsigned long int   RK_UInt32;  //32b
      typedef float               RK_Float32; //32b
      typedef   signed long long  RK_Int40;   //40b
      typedef unsigned long long  RK_UInt40;  //40b
      typedef   signed long long int  RK_Int64;   //64b
      typedef unsigned long long  RK_UInt64;  //unsigned 64 b

   #elif defined (_TMS320C6X)

      typedef int                 RK_Bool;    //32b
      typedef signed char         RK_Int8;    // 8b
      typedef unsigned char       RK_UInt8;   // 8b
      typedef   signed short int  RK_Int16;   //16b
      typedef unsigned short int  RK_UInt16;  //16b
      typedef   signed int        RK_Int32;   //32b
      typedef unsigned int        RK_UInt32;  //32b
      typedef float               RK_Float32; //32b

#ifdef __TI_INT40_T__
      typedef __int40_t           RK_Int40;   //40b
      typedef unsigned __int40_t  RK_UInt40;  //40b
#else      
      typedef   signed long int   RK_Int40;   //40b
      typedef unsigned long int   RK_UInt40;   //40b
#endif
      
#if (__COMPILER_VERSION__ <= 500)
      typedef double              RK_Int64;   //64b
#else
      typedef   signed long long int   RK_Int64;   //64b
      typedef unsigned long long int   RK_UInt64;   //64b
#endif

   
   #elif defined (WIN32)
      typedef int                 RK_Bool;    //32b
      typedef   signed char       RK_Int8;    // 8b
      typedef unsigned char       RK_UInt8;   // 8b
      typedef   signed short int  RK_Int16;   //16b
      typedef unsigned short int  RK_UInt16;  //16b
      typedef   signed long int   RK_Int32;   //32b
      typedef unsigned long int   RK_UInt32;  //32b
      typedef float               RK_Float32; //32b
      typedef __int64             RK_Int40;   //64b
      typedef __int64             RK_Int64;    // 64b
      typedef unsigned __int64    RK_UInt64;    // 64b

   #elif defined (LINUX) || defined (LINUX32) || defined(__arm)  || defined(__i386) ||  defined(__arm__) || defined (__APPLE_CC__) || defined(__GNUC__)
      typedef int                 RK_Bool;    //32b
      typedef   signed char       RK_Int8;    // 8b
      typedef unsigned char       RK_UInt8;   // 8b
      typedef   signed short int  RK_Int16;   //16b
      typedef unsigned short int  RK_UInt16;  //16b
   #if defined(__LP64__)
      typedef   signed int   RK_Int32;        //32b
      typedef unsigned int        RK_UInt32;  //32b
   #else
      typedef   signed long int   RK_Int32;   //32b
      typedef unsigned long int   RK_UInt32;  //32b
   #endif
      typedef float               RK_Float32; //32b
      //typedef   signed long long int  __int64;     //64b
      typedef   signed long long int RK_Int40;   //64b
      typedef   signed long long int RK_Int64;   //64b
      typedef unsigned long long int RK_UInt64;   //64b

   #endif 



#define RK_TRUE           1
#define RK_FALSE          0


// Definitions relating to the Memory Record data type
// Memory Attributes


typedef enum  RK_MemAttributes_e{
    RK_SCRATCH,           /**< Scratch memory. */
    RK_PERSIST,           /**< Persistent memory. */
    RK_WRITEONCE          /**< Write-once persistent memory. */
} RK_MemAttributes_e;

#define RK_MPROG  0x0008  /**< Program memory space bit. */
#define RK_MXTRN  0x0010  /**< External memory space bit. */

typedef enum  RK_MemSpace_e {
    RK_EPROG =            /**< External program memory */
        RK_MPROG | RK_MXTRN,

    RK_IPROG =            /**< Internal program memory */
        RK_MPROG,

    RK_ESDATA =           /**< Off-chip data memory (accessed sequentially) */
        RK_MXTRN + 0,

    RK_EXTERNAL =         /**< Off-chip data memory (accessed randomly) */
        RK_MXTRN + 1,

    RK_DARAM0 = 0,        /**< Dual access on-chip data memory */
    RK_DARAM1 = 1,        /**< Block 1, if independant blocks required */

    RK_SARAM  = 2,        /**< Single access on-chip data memory */
    RK_SARAM0 = 2,        /**< Block 0, equivalent to RK_SARAM */
    RK_SARAM1 = 3,        /**< Block 1, if independant blocks required */

    RK_DARAM2 = 4,        /**< Block 2, if a 3rd independent block required */
    RK_SARAM2 = 5         /**< Block 2, if a 3rd independent block required */
} RK_MemSpace_e;

typedef struct
{
	RK_UInt32			Size;
	RK_UInt8			Alignment;
	RK_MemSpace_e		MemSpaceType;
	RK_MemAttributes_e	MemAttributes;
	void				*base;
}	RK_MemRec_t;

#define todo(a) 
#endif //_RK_TYPEDEF
//.E

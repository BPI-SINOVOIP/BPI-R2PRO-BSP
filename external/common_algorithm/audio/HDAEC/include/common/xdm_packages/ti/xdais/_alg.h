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
*   Filename   : 	_alg.h
*
*   Description:
*
*=============================================================================*/

#ifndef _ALG_
#define _ALG_

/*
 *  ======== _ALG_allocMemory ========
 */
extern Bool _ALG_allocMemory(IALG_MemRec *memTab, Int n);

/*
 *  ======== _ALG_freeMemory ========
 */
extern Void _ALG_freeMemory(IALG_MemRec *memTab, Int n);

#endif

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
*   Filename   : 	alg.h
*
*   Description:
*
*=============================================================================*/

#ifndef ALG_
#define ALG_

#include <common/xdm_packages/ti/xdais/ialg.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 *  ======== ALG_Handle ========
 *  This handle type is used to reference all ALG instance objects
 */
typedef IALG_Handle ALG_Handle;

/*
 *  ======== ALG_activate ========
 *  Restore all shared persistant data associated with algorithm object
 */
extern Void ALG_activate(ALG_Handle alg);

/*
 *  ======== ALG_control ========
 *  Control algorithm object
 */
extern Int ALG_control(ALG_Handle alg, IALG_Cmd cmd, IALG_Status *sptr);

/*
 *  ======== ALG_create ========
 *  Create algorithm object and initialize its memory
 */
extern ALG_Handle ALG_create(IALG_Fxns *fxns, IALG_Handle p, IALG_Params *prms);

/*
 *  ======== ALG_deactivate ========
 *  Save all shared persistant data associated with algorithm object
 *  to some non-shared persistant memory.
 */
extern Void ALG_deactivate(ALG_Handle alg);

/*
 *  ======== ALG_delete ========
 *  Delete algorithm object and release its memory
 */
extern Void ALG_delete(ALG_Handle alg);

/*
 *  ======== ALG_exit ========
 *  Module finalization
 */
extern Void ALG_exit(Void);

/*
 *  ======== ALG_init ========
 *  Module initialization
 */
extern Void ALG_init(Void);

#ifdef __cplusplus
}
#endif

#endif  /* ALG_ */

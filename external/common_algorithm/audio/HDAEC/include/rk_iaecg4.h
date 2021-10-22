/*
//============================================================================
//
//    FILE NAME : IAECG4.h
//
//    ALGORITHM : AECG4
//
//    VENDOR    : RK
//
//    TARGET DSP: C64x
//
//    PURPOSE   : IAECG4 Interface Header
//
//    Component Wizard for eXpressDSP Version 1.33.00 Auto-Generated Component
//
//    Number of Inputs : 1
//    Number of Outputs: 1
//
//    Creation Date: Thu - 23 October 2008
//    Creation Time: 01:55 PM
//
//============================================================================
*/

#ifndef IAECG4_
#define IAECG4_

#include <common/xdm_packages/ti/xdais/std.h>
#include <common/xdm_packages/ti/xdais/xdas.h>
#include <common/xdm_packages/ti/xdais/ialg.h>
#include <rk_typedef_user.h>

#define SAVE_RESTORE_ACTION_GET_LENGTH 0
#define SAVE_RESTORE_ACTION_SAVE 1
#define SAVE_RESTORE_ACTION_RESTORE 2


// No longer needed
//#define NOISE_REDUCTION_OFF 0
//#define NOISE_REDUCTION_LC 1	/* low complexity noise reduction */
//#define MIN_NOISE_REDUCTION_HC 2	/* high complexity noise reduction 2..31 */

#define CNG_OFF 0
#define CNG_ENABLE_SUBBAND 1
#define CNG_ENABLE_FULLBAND 2

#define MTAB_NRECS 7
/*
// ===========================================================================
// IAECG4_Handle
//
// This handle is used to reference all AECG4 instance objects
*/
typedef struct IAECG4_Obj *IAECG4_Handle;

/*
// ===========================================================================
// IAECG4_Obj
//
// This structure must be the first field of all AECG4 instance objects
*/
typedef struct IAECG4_Obj {
    struct IAECG4_Fxns *fxns;
} IAECG4_Obj;

/*
// ===========================================================================
// IAECG4_Status
//
// Status structure defines the parameters that can be changed or read
// during real-time operation of the alogrithm.
*/
#define MAX_BINS 64

#define AECG4_STATUS_FIELDS \
	Int             size;  /* must be first field of all status structures */ \
    /* Get Time Domain Echo Model Items */\
	XDAS_Int16		*pReturnedModel;		/*Points to location where echo model (time domain coefficients) will be stored */\
	XDAS_Int16		nCoefToReturn;			/*Number of coefficients to return (input to control function) */\
	/* Get Status items */\
	XDAS_Int16      txInPowerdBm10;\
    XDAS_Int16      txOutPowerdBm10;\
    XDAS_Int16      rxInPowerdBm10;\
    XDAS_Int16      rxOutPowerdBm10;\
	XDAS_Int16		residualPowerdBm10;\
    XDAS_Int16      erlDirectdB10;\
    XDAS_Int16      erlIndirectdB10;\
	XDAS_Int16		erldB10BestEstimate;\
	XDAS_Int16		worstPerBinERLdB10BestEstimate;\
	XDAS_Int16		worstPerBinERLdB10BestEstimateConfidence;\
	XDAS_Int16		erledB10;\
	XDAS_Int16		shortTermERLEdB10;\
	XDAS_Int16		instantaneousERLEdB100;\
	XDAS_Int16		dynamicNLPAggressivenessAdjustdB10;\
	XDAS_Int16		shadowERLEdB10;\
	XDAS_Int16		rxVADState;\
	XDAS_Int16		txVADState;\
	XDAS_Int16		rxVADStateLatched;\
	XDAS_Int16		currentBulkDelaySamples;\
	XDAS_Int16		txAttenuationdB10;\
	XDAS_Int16		rxAttenuationdB10;\
	XDAS_Int16		rxOutAttenuationdB10;\
	XDAS_Int16		nlpThresholddB10;\
	XDAS_Int16		nlpSaturateFlag;\
	XDAS_Int16		aecState;\
	XDAS_Int16		sbcngResidualPowerdBm10;\
	XDAS_Int16		sbcngCNGPowerdBm10;\
	XDAS_Int16		rxOutAttendB10;\
	XDAS_Int16		sbMaxAttendB10;\
	XDAS_Int16		sbMaxClipLeveldBm10;\
	XDAS_UInt32		sbInitFlags;\
	XDAS_Int16		txFreqOffsetHz;\
	XDAS_Int16		rxFreqOffsetHz;\
	XDAS_Int16		sbTxVRKotalExceeddBm10;\
	XDAS_Int16		EPLMSResidualSample; \
	XDAS_Int16		EPShadowResidualSample; \
	XDAS_Int16		EPReconvergenceConfidenceFractionQ6; \
	XDAS_Int16		instantaneousERLDirectdB10;\
	XDAS_Int16		estimatedBulkDelaySamples;\
	/* Set Status Items */\
	XDAS_Int16		speakerLevelChangeDeltadB;

typedef struct IAECG4_Status {
	AECG4_STATUS_FIELDS
} IAECG4_Status;

/*
// ===========================================================================
// IAECG4_Cmd
//
// The Cmd enumeration defines the control commands for the AECG4
// control method.
*/
typedef enum IAECG4_Cmd {
  IAECG4_GETSTATUS,
  IAECG4_SETSTATUS,
  IAECG4_PAUSE,
  IAECG4_RESUME,
  IAECG4_GET_TIME_DOMAIN_ECHO_MODEL,
  IAECG4_FREEZE_BG_TO_FG,			/* Reserved */
  IAECG4_UNFREEZE_BG_TO_FG,			/* Reserved */
  IAECG4_FREEZE_FG_TO_SHADOW,		/* Reserved */
  IAECG4_UNFREEZE_FG_TO_SHADOW,		/* Reserved */
  IAECG4_FORCE_USE_SHADOW,			/* Reserved */
  IAECG4_FORCE_USE_FOREGROUND,		/* Reserved */
  IAECG4_RESUME_NORMAL_FILTER_USE,	/* Reserved */
  IAECG4_NOTUSE_TAILSEARCH,
  IAECG4_USE_TAILSEARCH,
  IAECG4_ENABLE_TXAGC_RUN,
  IAECG4_DISABLE_TXAGC_RUN,
  IAECG4_ENABLE_RXAGC_RUN,
  IAECG4_DISABLE_RXAGC_RUN
} IAECG4_Cmd;


/* 
  ==============================================
  Lock Callback Definition - used by AEC to obtain, use, and delete a lock
*/
typedef enum
{
	CREATE_LOCK,
	LOCK,
	UNLOCK,
	DELETE_LOCK
}		LockAction_e;

typedef RK_UInt32 (LockCallback_t)(	//returns 0 if OK, 1 otherwise
		void *LockHandle,		//If LOCK, UNLOCK, DELETE_LOCK: Handle to lock. Null otherwise
		char *Name,				//If CREATE_LOCK, name of lock. If LOCK or UNLOCK, name of calling function
		LockAction_e Action,	
		void **CreatedLock		//Used only if action is CREATE_LOCK
		);

/* Multi-Mic Definitions */
#define MAX_MICS 6	//USERS MUST NOT CHANGE THIS
#define MAX_MIC_GROUPS MAX_MICS //USERS MUST NOT CHANGE THIS

typedef struct {
	
	XDAS_Int8 TxFlag;
	XDAS_Int8 RxFlag;
	IAECG4_Handle hRx;
	IAECG4_Handle hTx[MAX_MICS];
	RK_Int16 MMICFrameSize;
}		IMMICAECG4_Obj;
typedef IMMICAECG4_Obj *IMMICAECG4_Handle; 


/* txrxMode definitions */
#define MODE_RX_B 0
#define MODE_TX_B 1
#define MODE_DMNR_B 2
#define MODE_DMNR_PRIMARY_B 3
#define MODE_MMIC_B 4

#define BIT_TO_MASK(b) (1 << b)
#define TXRX_MODE_NORMAL				(BIT_TO_MASK(MODE_RX_B) |	BIT_TO_MASK(MODE_TX_B))
#define TXRX_MODE_RX					BIT_TO_MASK(MODE_RX_B)
#define TXRX_MODE_TX					BIT_TO_MASK(MODE_TX_B)
#define TXRX_MODE_RX_MMIC				(BIT_TO_MASK(MODE_RX_B)	|																	BIT_TO_MASK(MODE_MMIC_B))
#define TXRX_MODE_TX_MMIC				(BIT_TO_MASK(MODE_TX_B) |																	BIT_TO_MASK(MODE_MMIC_B))
#define TXRX_MODE_RX_DMNR				(BIT_TO_MASK(MODE_RX_B)	|																	BIT_TO_MASK(MODE_MMIC_B))
#define TXRX_MODE_DMICNR_TX_PRIMARY		(BIT_TO_MASK(MODE_TX_B) |	BIT_TO_MASK(MODE_DMNR_B) | BIT_TO_MASK(MODE_DMNR_PRIMARY_B)	|	BIT_TO_MASK(MODE_MMIC_B))
#define TXRX_MODE_DMICNR_TX_SECONDARY	(BIT_TO_MASK(MODE_TX_B) |	BIT_TO_MASK(MODE_DMNR_B)									|	BIT_TO_MASK(MODE_MMIC_B))

#define AECG4_RK_NOT_SINGLE_MIC(mode) (mode & (BIT_TO_MASK(MODE_DMNR_B) | BIT_TO_MASK(MODE_MMIC_B)))
#define AECG4_RK_IS_SINGLE_MIC(mode) (!(AECG4_RK_NOT_SINGLE_MIC(mode)))

#define AECG4_RK_IS_DMNR_MIC(mode) (mode & (BIT_TO_MASK(MODE_DMNR_B)))
#define AECG4_RK_NOT_DMNR_MIC(mode) (!(AECG4_RK_IS_DMNR_MIC(mode)))

/*
// ===========================================================================
// IAECG4_Params
//
// This structure defines the creation parameters for all AECG4 objects
*/
//TAG_PARAMS
#define __AECG4_PARAMS \
	LockCallback_t  *lockCallback;			/*0 */\
	XDAS_UInt16 	frameSize;				/*1 */\
	XDAS_UInt8 		antiHowlEnable;			/*2 */\
	XDAS_Int32	 	samplingRate;			/*3 */\
	XDAS_Int32		maxAudioFreq;			/*4*/\
	XDAS_Int16 		fixedBulkDelayMSec;		/*5*/\
	XDAS_Int16		variableBulkDelayMSec;	/*6*/\
	XDAS_Int16		initialBulkDelayMSec;	/*7*/\
    XDAS_Int16 		activeTailLengthMSec;	/*8*/\
    XDAS_Int16 		totalTailLengthMSec;	/*9*/\
	XDAS_Int16		txNLPAggressiveness;	/*10*/\
    XDAS_Int16      maxTxLossSTdB;			/*11 */\
	XDAS_Int16		maxTxLossDTdB;			/*12*/\
    XDAS_Int16      maxRxLossdB;			/*13*/\
	XDAS_Int16		initialRxOutAttendB;	/*14*/\
    XDAS_Int16      targetResidualLeveldBm;	/*15*/\
    XDAS_Int16      maxRxNoiseLeveldBm;		/*16*/\
	XDAS_Int16		worstExpectedERLdB;		/*17*/\
	XDAS_Int16		rxSaturateLeveldBm;		/*18*/\
	XDAS_Int16		noiseReduction1Setting;	/*19*/\
	XDAS_Int16		noiseReduction2Setting;	/*20*/\
	XDAS_Int16		cngEnable;				/*21*/\
	XDAS_Int8		fixedGaindB10;			/*22*/\
    XDAS_Int8       txAGCEnable;			/*23*/\
    XDAS_Int8       txAGCMaxGaindB;			/*24*/\
    XDAS_Int8       txAGCMaxLossdB;			/*25*/\
    XDAS_Int8       txAGCTargetLeveldBm;	/*26*/\
    XDAS_Int8       txAGCLowSigThreshdBm;	/*27*/\
    XDAS_Int8       rxAGCEnable;			/*28*/\
    XDAS_Int8       rxAGCMaxGaindB;			/*29*/\
    XDAS_Int8       rxAGCMaxLossdB;			/*30*/\
    XDAS_Int8       rxAGCTargetLeveldBm;	/*31*/\
    XDAS_Int8       rxAGCLowSigThreshdBm;	/*32*/\
	XDAS_Int8		rxBypassEnable;			/*33*/\
	XDAS_Int16		maxTrainingTimeMSec;	/*34*/\
	XDAS_Int16		trainingRxNoiseLeveldBm;/*35*/\
    XDAS_Int16 *    pTxEqualizerdB10;		/*36*/\
	XDAS_Int8		mipsMemReductionSetting;/*37*/\
	XDAS_Int8		mipsReductionSetting2;	/*38*/\
	XDAS_Int8		txrxMode; /*39*/	/* reserved - must be set to zero*/

#define N_AECG4_PARAMS 40

typedef struct IAECG4_Params {
    Int size;	  /* must be first field of all params structures */
	__AECG4_PARAMS

} IAECG4_Params;
typedef struct
{
	RK_UInt8 EchoPath;
	RK_UInt8 Update;
}		IAECG4_SoftResetParams_t;

/*
// ===========================================================================
// IAECG4_PARAMS
//
// Default parameter values for AECG4 instance objects
*/
extern const IAECG4_Params IAECG4_PARAMS;

/*
// ===========================================================================
// IAECG4_Fxns
//
// This structure defines all of the operations on AECG4 objects
*/
typedef struct IAECG4_Fxns {
    IALG_Fxns	ialg;    /* IAECG4 extends IALG */
    XDAS_Void (*apply)(IAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut, XDAS_Int16 * ptrTxIn, XDAS_Int16 * ptrTxOut);
    XDAS_Void (*applyTx)(IAECG4_Handle handle, XDAS_Int16 * ptrTxIn, XDAS_Int16 * ptrTxOut);
    XDAS_Void (*applyRx)(IAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut);
    XDAS_Void (*backgroundHandler)(IAECG4_Handle handle);
	XDAS_Int32 (*saveRestoreState) (IAECG4_Handle handle, XDAS_Int8 *pState, XDAS_Int32 Length, XDAS_Int8 Action);
	Int (*reset) (IAECG4_Handle handle, const IAECG4_Params *iAECG4Params);
	XDAS_Void (*softReset) (IAECG4_Handle handle, const IAECG4_SoftResetParams_t *pSoftResetParams);
/* Multi-Mic APIs */
    XDAS_Void (*applyMMIC)(IMMICAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut, XDAS_Int16 * ptrTxIn[], XDAS_Int16 * ptrTxOut[]);
    XDAS_Void (*backgroundHandlerMMIC)(IMMICAECG4_Handle handle);
	XDAS_Int32 (*saveRestoreStateMMIC) (IMMICAECG4_Handle handle, XDAS_Int8 *pState, XDAS_Int32 Length, XDAS_Int8 Action, XDAS_Int8 MicIndex);
	Int (*resetMMIC) (IMMICAECG4_Handle handle, const IAECG4_Params *iAECG4Params);
	XDAS_Void (*softResetMMIC) (IMMICAECG4_Handle handle, const IAECG4_SoftResetParams_t *pSoftResetParams);
	XDAS_Void (*AECG4_RK_controlMMIC)(IMMICAECG4_Handle handle, IAECG4_Cmd cmd, XDAS_Int32 MicNumber, IAECG4_Status * status);

} IAECG4_Fxns;



/* Concrete interface to all AECG4 functions */
RK_API IAECG4_Handle AECG4_RK_create(const IAECG4_Fxns *fxns, const IAECG4_Params *prms);
RK_API IMMICAECG4_Handle AECG4_RK_createMMIC(const IAECG4_Fxns *fxns, const IAECG4_Params *prms, const unsigned char MaxMicrophones, XDAS_UInt8 *pMicGroups);
RK_API IMMICAECG4_Handle AECG4_RK_createDMICNR(const IAECG4_Fxns *fxns, const IAECG4_Params *prms);
RK_API IAECG4_Handle AECG4_RK_createStatic(IAECG4_Fxns *fxns, IAECG4_Params *params, IALG_MemRec *memTab);
RK_API void AECG4_RK_staticAllocHelper(const IAECG4_Params *prms);
RK_API Int AECG4_RK_alloc(const IAECG4_Params *prms, struct IAECG4_Fxns **, IALG_MemRec memTab[]);
RK_API Int AECG4_RK_control(IAECG4_Handle handle, IAECG4_Cmd cmd, IAECG4_Status *status);
RK_API Void AECG4_RK_delete(IAECG4_Handle handle);
RK_API Void AECG4_RK_deleteStatic(IAECG4_Handle handle);
RK_API XDAS_Void AECG4_RK_apply(IAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut, XDAS_Int16 * ptrTxIn, XDAS_Int16 * ptrTxOut);
RK_API XDAS_Void AECG4_RK_applyTx(IAECG4_Handle handle, XDAS_Int16 * ptrTxIn, XDAS_Int16 * ptrTxOut);
RK_API XDAS_Void AECG4_RK_applyRx(IAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut);
RK_API XDAS_Void AECG4_RK_backgroundHandler(IAECG4_Handle handle);
RK_API XDAS_Int32 AECG4_RK_saveRestoreState(IAECG4_Handle handle, XDAS_Int8 *pState, XDAS_Int32 Length, XDAS_Int8 Action);
RK_API Int AECG4_RK_reset(IAECG4_Handle handle, const IAECG4_Params *prms);
RK_API XDAS_Void AECG4_RK_softReset (IAECG4_Handle handle, const IAECG4_SoftResetParams_t *pSoftResetParams);
RK_API XDAS_Int16 AECG4_RK_getTxDelaySamples(IAECG4_Handle Handle);
//Multi-Mic APIs

RK_API XDAS_Void AECG4_RK_applyMMIC(IMMICAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut, XDAS_Int16 * ptrTxIn[], XDAS_Int16 * ptrTxOut[]);
RK_API XDAS_Void AECG4_RK_applyMMICTx(IMMICAECG4_Handle handle, XDAS_Int16 * ptrTxIn[], XDAS_Int16 * ptrTxOut[]);
RK_API XDAS_Void AECG4_RK_applyMMICRx(IMMICAECG4_Handle handle, XDAS_Int16 * ptrRxIn, XDAS_Int16 * ptrRxOut);

RK_API XDAS_Void AECG4_RK_backgroundHandlerMMIC(IMMICAECG4_Handle handle);
RK_API XDAS_Int32 AECG4_RK_saveRestoreStateMMIC(IMMICAECG4_Handle handle, XDAS_Int8 *pState, XDAS_Int32 Length, XDAS_Int8 Action, XDAS_Int8 MicIndex);
RK_API Int AECG4_RK_resetMMIC(IMMICAECG4_Handle handle, const IAECG4_Params *iAECG4Params);
RK_API XDAS_Void AECG4_RK_softResetMMIC(IMMICAECG4_Handle handle, const IAECG4_SoftResetParams_t *pSoftResetParams);
RK_API XDAS_Int16 AECG4_RK_getTxDelaySamplesMMIC(IMMICAECG4_Handle Handle);

RK_API XDAS_Void AECG4_RK_controlMMIC(IMMICAECG4_Handle handle, IAECG4_Cmd cmd, XDAS_Int32 MicNumber, IAECG4_Status * status);

RK_API XDAS_Int32 AECG4_RK_addMic(IAECG4_Handle hRxChannel, IAECG4_Handle hMicChannel, XDAS_Int8 MicIndex, XDAS_Int8 MicGroup);

RK_API XDAS_Int32 AECG4_RK_enableMic(IMMICAECG4_Handle mmhandle, XDAS_Int8 MicIndex);
RK_API XDAS_Int32 AECG4_RK_disableMic(IMMICAECG4_Handle mmhandle, XDAS_Int8 MicIndex);
RK_API Void AECG4_RK_deleteMMIC(IMMICAECG4_Handle handle);
RK_API Void AECG4_RK_updateMaxAudioFreqMMIC(IMMICAECG4_Handle handle, XDAS_Int32 maxAudioFreq);
RK_API Void AECG4_RK_updateMaxAudioFreq (IAECG4_Handle handle, XDAS_Int32 maxAudioFreq);
RK_API char *AECG4_RK_getVersionString();
RK_API Void AECG4_RK_updateTxAGC (IAECG4_Handle handle, XDAS_Int8 TxAGCSetting);
RK_API Void AECG4_RK_updateRxAGC (IAECG4_Handle handle, XDAS_Int8 RxAGCSetting);
RK_API Void AECG4_RK_updateTxAGCMMIC (IMMICAECG4_Handle mmhandle, XDAS_Int8 MicIndex, XDAS_Int8 TxAGCRuntimeEnable);
RK_API Void AECG4_RK_updateRxAGCMMIC (IMMICAECG4_Handle mmhandle, XDAS_Int8 RxAGCRuntimeEnable);
RK_API void AECG4_RK_gainBlock(
	RK_Int16 GainValue,	// gain value (units of .1 dBm)
	RK_Int16 *pInput,		// pointer to Input buffer
	RK_Int16 *pOutput,		// pointer to Output buffer
	RK_Int16 Length		// Length of Array to be processed
	);
// These are not per-instance functions
typedef struct 
{
	RK_UInt8 NR2;
	RK_UInt8 Mobile;
	RK_UInt8 MultiMic;
	RK_UInt8 DualMicNoiseReduction;
}		AECG4_BuildInfo_t;
RK_API char *AECG4_RK_getBuildInfoString();
RK_API AECG4_BuildInfo_t *AECG4_RK_getBuildInfo();

RK_API XDAS_Int16 AECG4_RK_getParamCount();
RK_API XDAS_Int16 AECG4_RK_getParamNames(char *pParamNameTable[], RK_Int16 TableSize);

// HD AEC - Runtime Configurable Parameters
#define AECG4_RK_ANTI_HOWL_ENABLE 2

#define AECG4_RK_MAX_AUDIO_FREQ_HZ 4

#define AECG4_RK_TX_NLP_AGGRESSIVENESS 10
#define AECG4_RK_MAX_TX_LOSS_ST_DB 11
#define AECG4_RK_MAX_TX_LOSS_DT_DB 12
#define AECG4_RK_MAX_RX_LOSS_DB 13

#define AECG4_RK_RX_SATURATE_LEVEL_DBM 18
#define AECG4_RK_NOISE_REDUCTION1_SETTING 19
#define AECG4_RK_NOISE_REDUCTION2_SETTING 20
#define AECG4_RK_CNG_ENABLE 21
#define AECG4_RK_FIXED_GAIN_DB10 22
#define AECG4_RK_TXAGC_ENABLE 23
#define AECG4_RK_TXAGC_MAX_GAIN_DB 24				
#define AECG4_RK_TXAGC_MAX_LOSS_DB 25			
#define AECG4_RK_TXAGC_TARGETPOWER_DBM 26
#define AECG4_RK_TXAGC_LOW_SIG_THRESHOLD_DBM 27
#define AECG4_RK_RXAGC_ENABLE 28
#define AECG4_RK_RXAGC_MAX_GAIN_DB 29				
#define AECG4_RK_RXAGC_MAX_LOSS_DB 30		
#define AECG4_RK_RXAGC_TARGETPOWER_DBM 31
#define AECG4_RK_RXAGC_LOW_SIG_THRESHOLD_DBM 32
#define AECG4_RK_RX_BYPASS_ENABLE 33

RK_API XDAS_Int8 AECG4_RK_getParamValue(IAECG4_Handle handle, RK_UInt8 ParamID, RK_Int32 *pValue, char *pStringValue);
RK_API XDAS_Int8 AECG4_RK_setParamValue(IAECG4_Handle handle, RK_UInt8 ParamID, RK_Int32 Value);

RK_API XDAS_Int8 AECG4_RK_getParamValueMMIC(IMMICAECG4_Handle handle, RK_UInt8 ParamID, RK_Int32 *pValue, char *pStringValue);
RK_API XDAS_Int8 AECG4_RK_setParamValueMMIC(IMMICAECG4_Handle handle, RK_UInt8 ParamID, RK_Int32 Value);

#endif	/* IAECG4_ */

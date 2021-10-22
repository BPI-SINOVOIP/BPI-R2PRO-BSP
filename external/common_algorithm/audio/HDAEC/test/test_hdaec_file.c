
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rk_iaecg4.h>

#define TIME_DEBUG(FUNC) {\
	struct timeval tv1, tv2;\
	gettimeofday(&tv1, NULL); \
	FUNC; \
	gettimeofday(&tv2, NULL); \
	printf("elapse %ld ms\n", (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000); \
}

//#define USER_MEM_ALLOC

#define SAMPLING_RATE 16000
#define FRAME_SIZE_MSEC 20 // 12ms/frame
#define FRAME_SIZE (FRAME_SIZE_MSEC* (SAMPLING_RATE/1000))// 64
//#define FRAME_SIZE 64
//#define FRAME_SIZE_MSEC 4

short int READ_WORDS(short int Buffer[], short int Count,FILE *File);
short int WRITE_WORDS(short int Buffer[], short int Count, FILE *File);

// AEC Parameters
IAECG4_Params MyAECParams = {
	// Base Parameters
	sizeof(IAECG4_Params),
	0, // lockCallback
	FRAME_SIZE, // frameSize (samples)
	0, // antiHowlEnable
	SAMPLING_RATE, // samplingRate
	SAMPLING_RATE/2, // maxAudioFreq
	2*FRAME_SIZE_MSEC, // fixedBulkDelayMSec
	0, // variableBulkDelayMSec
	0, // initialBulkDelayMSec
	32, // activeTailLengthMSec
	128, //totalTailLengthMSec
	6, //txNLPAggressiveness
	30, // MaxTxLossSTdB
	20, // MaxTxLossDTdB
	6, // MaxRxLossdB;
	0, // initialRxOutAttendB
	-85, // targetResidualLeveldBm;
	-90, // maxRxNoiseLeveldBm;
	-12, // worstExpectedERLdB
	3, // rxSaturateLeveldBm
	1, // noiseReduction1Setting
	0, // noiseReduction2Setting
	1, // cngEnable
	0, // fixedGaindB
	// txAGC Parameters
	0, // RK_Int8 txAGCEnable;
	10, // RK_Int8 txAGCMaxGaindB;
	0, //RK_Int8 txAGCMaxLossdB;
	-10, // RK_Int8txAGCTargetLeveldBm;
	-50, //RK_Int8 txAGCLowSigThreshdBm;
	// rxAGC Parameters
	0, // RK_Int8 rxAGCEnable;
	10, //rxAGCMaxGaindB;
	15, //rxAGCMaxLossdB;
	-10, //rxAGCTargetLeveldBm;
	-40, //rxAGCLowSigThreshdBm;
	1, //rxBypassEnable
	0, //maxTrainingTimeMSec
	-40, //trainingRxNoiseLeveldBm
	0, //RK_Int16 pTxEqualizer
	0, //mipsMemReductionSetting
	0, //mipsReductionSetting2
	0 //reserved
};
extern IAECG4_Fxns AECG4_ADT_IAECG4;

#ifdef USER_MEM_ALLOC
IALG_MemRec MyMemTab[MTAB_NRECS];
long int TotalSize = 0;
long int Size, Alignment, Offset;
#endif
//#define SINGLE_API	//Define this if both AEC directions can be executed at the same point in the code
//#define RUN_BACKGROUND_IN_FOREGROUND // In a live system, this would NOT be defined	

int Done;
IAECG4_Handle hAEC = 0;
long framecount=0;
static short int RxIn[FRAME_SIZE],TxIn[FRAME_SIZE],RxOut[FRAME_SIZE],TxOut[FRAME_SIZE];

void main(int argc, char *argv[])
{
	IAECG4_Handle hAEC;
	FILE *pTxIn, *pRxIn, *pTxOut, *pRxOut;
	short i, currentBulkDelaySamples = 0;
	IAECG4_Status Status;
	Status.size = sizeof(Status);

	if (argc < 5) {
		printf("Error: Wrong input parameters! A example is as following: 16K pcm\n");
		printf("%s near.pcm far.pcm near_out.pcm far_out.pcm\n", argv[0]);
		exit(-1);
	}

	pTxIn = fopen(argv[1], "rb"); // TxIn_16K.pcm", "rb");
	pRxIn = fopen(argv[2], "rb"); // RxIn_16K.pcm", "rb");

	pTxOut = fopen(argv[3], "wb");
	pRxOut = fopen(argv[4], "wb");

	if ((pTxIn == 0) || (pRxIn == 0) || (pTxOut == 0) || (pRxOut == 0))
	{
		printf("Error opening file(s)\n");
		exit(0);
	}

#ifdef USER_MEM_ALLOC // compute the memtable to help customer memory alloc
	AECG4_ADT_alloc((const IAECG4_Params *) &MyAECParams, 0, MyMemTab);

	for (i = 0; i < MTAB_NRECS; i++)
	{
		// Next line tries to make extra space so memory is aligned
		MyMemTab[i].size = (MyMemTab[i].size + 7) & ~7;
		
		printf("MyMemTab[%d].size=%d \n", i, MyMemTab[i].size);
		// Customer can use their own allocate memory, ie. malloc or from mem pool
		if(MyMemTab[i].size > 0)
			MyMemTab[i].base = (void *) malloc(MyMemTab[i].size); // dynamic memory allocate	
		
		memset(MyMemTab[i].base, 0, MyMemTab[i].size);
	
		TotalSize += MyMemTab[i].size;
	}
	printf("#include <ialg.h>\n");
	printf("ADT_Int64 AECG4Instance[%ld];\n",TotalSize/sizeof(ADT_Int64));
	printf("IALG_MemRec MyMemTab[MTAB_NRECS] = \n");
	printf("{\n");
	TotalSize = 0;
	for (i = 0; i < MTAB_NRECS; i++)
	{
		Size = MyMemTab[i].size; Alignment = MyMemTab[i].alignment; //Offset = (long int) MyMemTab[i].base;
		TotalSize += MyMemTab[i].size;
		Offset = TotalSize/sizeof(ADT_Int64);
		printf(" {%ld,%ld,IALG_SARAM,IALG_PERSIST,&AECG4Instance[%ld]},\n", Size, Alignment, Offset);

	}
	printf("};\n");
	printf("hAEC = AECG4_createStatic((IALG_Fxns *) &AECG4_ADT_IAECG4, &Params, MyMemTab);\n");
	
	hAEC = AECG4_ADT_createStatic(NULL, &MyAECParams, MyMemTab);
#else
	//AECG4_ADT_staticAllocHelper((IAECG4_Params *)&MyAECParams);
	hAEC = (IAECG4_Handle) AECG4_ADT_create(0, &MyAECParams);
#endif
	if (hAEC == 0)
	{
		//printf("AEC Allocation failed\n");
		exit(0);
	}
	Done = 0;
	while (!Done)
	{
retry:
		// Read FRAME_SIZE samples from far end (network) into RxIn
		Done = (READ_WORDS(RxIn, FRAME_SIZE, pRxIn) != FRAME_SIZE);
		//Read FRAME_SIZE samples from microphone into TxIn
		Done |= (READ_WORDS(TxIn, FRAME_SIZE, pTxIn) != FRAME_SIZE);
		if(Done) {
			fseek(pRxIn, 0L, SEEK_SET);
			fseek(pTxIn, 0L, SEEK_SET);
			goto retry;	
		}
		//memset(RxIn, 0, FRAME_SIZE*sizeof(short int));
#ifdef SINGLE_API
		AECG4_ADT_apply(hAEC,RxIn,RxOut,TxIn,TxOut);
#else
		TIME_DEBUG(AECG4_ADT_applyRx(hAEC, RxIn, RxOut);AECG4_ADT_applyTx(hAEC, TxIn, TxOut););
#endif
		// Send FRAME_SIZE samples from RxOut to the speaker
		WRITE_WORDS(RxOut, FRAME_SIZE, pRxOut);
		// Send FRAME_SIZE samples from TxOut to the far end (network)
		WRITE_WORDS(TxOut, FRAME_SIZE, pTxOut);

		framecount++;
	}
	fclose(pTxIn);
	fclose(pTxOut);
	fclose(pRxIn);
	fclose(pRxOut);

	//Wait for AECG4_ADT_backgroundHandler to complete current execution before calling AECG4_ADT_delete
#ifdef USER_MEM_ALLOC // compute the memtable to help customer memory alloc
	for (i = 0; i < MTAB_NRECS; i++)
	{
	
		if(MyMemTab[i].base != 0)
			free(MyMemTab[i].base);
	}

#else
	AECG4_ADT_delete(hAEC);
#endif
}



short int READ_WORDS(short int Buffer[], short int Count,FILE *File)
{
	short int HB, LB;
	short int i = 0;
	while ((i < Count) && !feof(File))
	{
		fread(&LB, 1, 1, File);
		fread(&HB, 1, 1, File);
		Buffer[i] = (HB << 8) + (LB & 0xff);
		i += 1;
	}
	return(i);
}
short int WRITE_WORDS(short int Buffer[], short int Count, FILE *File)
{
	short int HB,LB;
	short int i = 0;
	for (i = 0; i < Count; i++)
	{
		LB = Buffer[i] >> 8;
		HB = Buffer[i] & 0xff;
		fwrite(&HB, 1, 1, File);
		fwrite(&LB, 1, 1, File);
	}
	return(i);
}

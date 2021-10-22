/************************************************************************************
*				                                                                    *
*	Function:                                                                       *
*		The interfaces of the microphone array frond-end processing, including tdoa,*
*       beamforming, dereverberation, agc, vad and wakeup                           *
*				                                                                    *
*************************************************************************************/

#ifndef SKV_PREPROCESS_H
#define SKV_PREPROCESS_H

typedef unsigned char BYTE;

#ifdef __cplusplus
extern "C" {
#endif
	#define INFO_RIGHT 1				// Succeed initialize SKV preprocessor
	#define INFO_PARAM_ERROR 2			// Cann't support the parameter of audio that you input
	#define INFO_EXCEEDDATE_ERROR 3		// Exceed the limited date
	#define INFO_UNKNOW_ERROR 4         // Unknown error
	#define FRAME_SIZE 256				/* The number of sample in a frame */

	/** Initialize the SKV preprocessor. You MUST initialize the SKV preprocessor before using this speech frond-end processing.
	* @param sampling_rate Sampling rate used for the input, MUST be 16000.
	* @param bits_persample Bites per sample used for the input. MUST be 16
	* @param num_src_channel Number of channel of input audio. MUST >= 1
	* @param num_block_channel Number of block channel. Suggest to be 3
			 Larger block channels means stronger enhancement, but would damage the target speech.
	* @param doDereverberation A flag that decide whether to do dereverberation. 0: without dereverberation; 1: with dereverberation
	* @return a state for the initialization of the SKV preprocessor, 0 means Failed, and 1 means Success .
	*/
	int skv_preprocess_state_init(int sampling_rate, int bits_persample, int num_src_channel, int doDereverberation);

	/** Destroys a SKV preprocessor
	*/
	void skv_preprocess_state_destroy();

	/** Preprocess a buffer of audio data
	* @param handle The handle to the skv_processing
	* @param in Audio data (Short) to be processed in PCM format, its size of MUST be 256 * (num_src_channel + num_ref_channel) * N samples, N = 1, 2, 3, ... n
	* @param in_size Lenght of the array 'in' that you want to process
	* @return return the Number of samples in out
	*/
	int skv_preprocess_short(short * in, short * out, int in_size);

	/** Preprocess a buffer of audio data
	* @param handle The handle to the skv_processing
	* @param in Audio data (Short) to be processed in PCM format, its size of MUST be 256 * (num_src_channel + num_ref_channel) * N samples, N = 1, 2, 3, ... n
	* @param in_size Lenght of the array 'in' that you want to process
	* @return return the Number of samples in out
	*/
	int skv_preprocess_byte(BYTE * in, short * out, int in_size, int bigOrlittle);


	/** Used like the ioctl function to control the asr_engine parameters
	* @param st asr_engine state
	* @param request ioctl-type request (one of the SKV_VAD_* macros)
	* @param ptr Data exchanged to-from function
	* @return 0 if no error, -1 if request in unknown
	*/
	int skv_preprocess_ctl(int request, void* ptr);

	/** Set agc level for AGC gain (float) */
#define SKV_SET_AGC_LEVEL 1

/** Set the first speech thresh for adjusting speech (float) */
#define SKV_SET_AGC_FIRST_SPE_THRESH 2

/** Set the second speech thresh for adjusting speech (float) */
#define SKV_SET_AGC_SECOND_SPE_THRESH 3

	/** Set the thresh of speech activation for tdoa (float) */
#define SKV_SET_BeamDNNTDOA_SPE_THRESH 4

	/** Set the length of the decision window required for the decision of tdoa (int) */
#define SKV_SET_BeamDNNTDOA_DECISION_WIN_LENGTH 5


#ifdef __cplusplus
}
#endif

/** @}*/
#endif

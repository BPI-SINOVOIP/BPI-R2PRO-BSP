#include "wave_reader.h"
#include "wave_writer.h"

#include "skv_preprocess.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sys/time.h>

#ifndef IN_SIZE
#define IN_SIZE 256
#endif

typedef unsigned char BYTE;

#define TIME_DEBUG(FUNC) {\
	struct timeval tv1, tv2;\
	gettimeofday(&tv1, NULL); \
	FUNC; \
	gettimeofday(&tv2, NULL); \
	printf("elapse %ld ms\n", (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000); \
}

int main(int argc, char *argv[])
{

	/* For Debug  */
	argc = 3;
	argv[1] = (char *)"csq_0_2mic_out.wav";
	argv[2] = (char *)"csq_0_2mic_out_beamform.wav";
	/* For Debug  */

	int bs = 2, out_size = 0, in_size = 0, res = 0;
	if (argc < 3)
	{
		printf("Error: Wrong input parameters! A example is as following: \n");
		printf("fosafer_enh test_in.wav test_out.wav 0\n");
		exit(-1);
	}

	char * in_filename = argv[1];
	char * out_filename = argv[2];

	// for wave reader
	wave_reader *wr;
	wave_reader_error rerror;

	// for wave writer
	wave_writer *ww;
	wave_writer_error werror;
	wave_writer_format format;

	wr = wave_reader_open(in_filename, &rerror);
	if (wr)
	{
		printf("filename=%s format=%d num_channels=%d sample_rate=%d sample_bits=%d num_samples=%d\n",
			in_filename,
			wave_reader_get_format(wr),
			wave_reader_get_num_channels(wr),
			wave_reader_get_sample_rate(wr),
			wave_reader_get_sample_bits(wr),
			wave_reader_get_num_samples(wr));

		int mSampleRate = wave_reader_get_sample_rate(wr);
		int mBitPerSample = wave_reader_get_sample_bits(wr);
		int mNumChannel = wave_reader_get_num_channels(wr);
		int num_src_channel = mNumChannel;
		int state = skv_preprocess_state_init(mSampleRate, mBitPerSample, num_src_channel, 1);
		switch (state)
		{
		case INFO_RIGHT:
			printf("Succeed to initialize skv_preprocessor!\n");
			break;
		case INFO_PARAM_ERROR:
			printf("Error: Cann't support your parameter, now only support sampling_rate = 16000, bits_persample = 16, num_src_channel > 0 and num_ref_channel > 0\n");
			break;
		case INFO_EXCEEDDATE_ERROR:
			printf("Error: Your so package has been exceed the limited date!\n");
			break;
		default:
			printf("Error: Unknow errors!\n");
			break;
		}

		int read_size = IN_SIZE * mNumChannel * mBitPerSample / 8;
		BYTE * in = ( BYTE * )malloc(read_size * sizeof(BYTE));
		short * out = ( short * )malloc(IN_SIZE * sizeof(short));
		format.num_channels = 1;
		format.sample_rate = wave_reader_get_sample_rate(wr);
		format.sample_bits = wave_reader_get_sample_bits(wr);
		ww = wave_writer_open(out_filename, &format, &werror);
		if (ww)
		{
			clock_t startTime, endTime;
			double Total_time = 0.0;
			double Total_sample = 0.0;
			float Spe_time = 0.0;

			startTime = clock();
			while (0 < (res = wave_reader_get_samples(wr, IN_SIZE, in)))
			{
				in_size = res * ( mBitPerSample / 8 ) * mNumChannel;
				TIME_DEBUG(out_size = skv_preprocess_byte(in, out, in_size, bs));
				wave_writer_put_samples(ww, out_size, out);
				Total_sample = Total_sample + out_size;
			}
			endTime = clock();

			Total_time = (double)(endTime - startTime) / CLOCKS_PER_SEC;
			Spe_time = Total_sample / mSampleRate;
			printf("Finished, speech_time = %f, cost_time = %f\n", Spe_time, Total_time);
			wave_writer_close(ww, &werror);
		}
		else
		{
			printf("werror=%d\n", werror);
		}
		free(in);
	}
	else
	{
		printf("rerror=%d\n", rerror);
	}
	wave_reader_close(wr);
	skv_preprocess_state_destroy();
	system("pause");
	return 0;
}

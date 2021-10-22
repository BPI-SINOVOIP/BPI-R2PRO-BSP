#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <rk_iaecg4.h>

#define NUM_REF_CHANNEL 1
#define NUM_SRC_CHANNEL 1

#define TIME_DEBUG(FUNC) {\
	struct timeval tv1, tv2;\
	gettimeofday(&tv1, NULL); \
	FUNC; \
	gettimeofday(&tv2, NULL); \
	printf("elapse %ld ms\n", (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000); \
}

int main(int argc, char *argv[])
{
	/* Test Example:
	TestAudio test_in.wav test_out.wav
	*/
	int bs = 2, ssize = 0;
	int swFrmLen, swFs;
	int swFrmNum, swRet;
	int ch = 0;
	char achFile[500] = "RK_VoicePara_dealyoff.bin";						/* 参数文件名 */


	short ashwTxIn[256] = { 0 };
	short ashwTxOut[256] = { 0 };
	short ashwRxIn[256] = { 0 };
	short ashwRxOut[256] = { 0 };

	swFs = 16000;/*采样率*/
	swFrmNum = 0;
	swFrmLen = 256;/*帧长*/

	/* For Debug  */
	//argc = 3;
	//argv[1] = (char*)"./1+1mic/csq_0_1mic.wav";
	//argv[2] = (char*)"./1+1mic/csq_0_1mic_out.wav";
	/* For Debug  */
	if (argc < 3)
	{
		printf("Error: Wrong input parameters! A example is as following: \n");
		printf("TestAudio test_in.wav test_out.wav\n");
		exit(-1);
	}

	char* in_filename = argv[1];
	char* out_filename = argv[2];
	int res;
	// for wave reader
	wave_reader* wr;
	wave_reader_error rerror;
	// for wave writer
	wave_writer* ww;
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
		int num_src_channel = mNumChannel - NUM_REF_CHANNEL;  // the number of channels of source signals
		int num_ref_channel = NUM_REF_CHANNEL;				// the number of channels of reference signals
		printf("mic_channel = %d, ref channel = %d\n", num_src_channel, num_ref_channel);
		if (mSampleRate != 16000 || mBitPerSample != 16 || num_src_channel <= 0 || num_ref_channel <= 0)
		{
			fprintf(stderr, "Error: %d SampleRate, %d bits_persample, %d num_src_channel and %d num_ref_channel cann't be supported", mSampleRate, mBitPerSample, num_src_channel, num_ref_channel);
			return 0;
		}
		/* 初始化 */
		for (ch = 0; ch < num_src_channel; ch++)
		{
			printf("init ch %d\n", ch);
			swRet = RK_VOICE_Init(achFile, swFs, swFrmLen, ch);/*改1*/
			printf("init ch %d succeed\n", ch);
		}
		if (0 != swRet)
		{
			printf("\n Init Error: Error Code %d!\n", swRet);
			exit(0);
		}

		printf("init ok\n");
		int read_size = swFrmLen * mNumChannel * mBitPerSample / 8;//4096
		VOICE_BYTE* in = (VOICE_BYTE*)VOICE_Alloc(read_size * sizeof(VOICE_BYTE));
		short* out = (short*)VOICE_Alloc(swFrmLen * num_src_channel * sizeof(short));
		if (out == NULL) {
			fprintf(stderr, "wrong out malloc");
			return 0;
		}
		printf("alloc ok\n");
		int in_size = 0;
		in_size = swFrmLen * mNumChannel * mBitPerSample / 8;//4096
		int in_short_size = in_size / (mBitPerSample / 8);//2048

		format.num_channels = num_src_channel;
		format.sample_rate = wave_reader_get_sample_rate(wr);
		format.sample_bits = wave_reader_get_sample_bits(wr);
		ww = wave_writer_open(out_filename, &format, &werror);
		short* in_short = (short*)VOICE_Alloc(in_short_size* sizeof(short));
		if (in_short == NULL || in_short_size <0) {
			fprintf(stderr,"in_short wrong malloc");
			return 0;
		}
		if (ww)
		{
			while (0 < (res = wave_reader_get_samples(wr, swFrmLen, in)))
			{
				//ssize = skv_aec_byte(in, out, in_size, bs);
				if (bs != 0 && bs != 1)
				{
					bs = skv_bigorsmall();
				}
				if (in == NULL || in_size < 0) {
					fprintf(stderr, "Input is wrong");
				}
				in_short_size = skv_bytes2shorts(in, in_short, in_size, mBitPerSample, bs);

				//int res = skv_aec_short(st->in_short, out, st->in_short_size);
				int in_samples = in_short_size / mNumChannel;
				int c = 0, i = 0, j = 0;
				int s = 0, e = 0;
				int m = 0, k = 0;
				int tmp = 0;
				//fprintf(stderr, "Ref is \n");
				for (j = 0; j < in_samples; j++)
				{
					m = j * mNumChannel;
					tmp = 0;
					for (c = num_src_channel; c < mNumChannel; c++)
					{
						tmp += in_short[m + c];
					}
					ashwRxIn[j] = tmp / num_ref_channel;
					//fprintf(stderr, "%d\t", ashwRxIn[j]);
				}
				//fprintf(stderr, "Origin is \n");

				for (i = 0; i < num_src_channel; i++)
				{
					for (j = 0; j < in_samples; j++)
					{
						m = j * mNumChannel;
						ashwTxIn[j]  = in_short[m + i];/*3308增益问题，临时改*/
						/*fprintf(stderr, "%d\t", in_short[m + i]);
						if (j == in_samples - 1)
							fprintf(stderr, "next channel is \n");
						if (i == num_src_channel-1 && j == in_samples - 1)
							break;*/
						//if(i == 0)
					}
					RK_VOICE_ProcessTx(ashwTxIn, ashwRxIn, ashwTxOut, swFrmLen, i);/*改2*/
					for (j = 0; j < in_samples; j++) 
					{
						m = j * num_src_channel;
						out[m + i] = ashwTxOut[j];
					}
				}
				wave_writer_put_samples(ww, swFrmLen, out);
			}
			wave_writer_close(ww, &werror);
		}
		else
		{
			printf("werror=%d\n", werror);
		}

		printf("end\n");
		VOICE_Free(in);
		VOICE_Free(out);
		VOICE_Free(in_short);
		/* 3.测试环境清理 */
		for (ch = 0; ch < num_src_channel; ch++)
		{
			RK_VOICE_Destory(ch);
		}
	}
	else
	{
		printf("rerror=%d\n", rerror);
	}
}

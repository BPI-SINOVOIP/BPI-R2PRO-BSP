#include "alsa_reader.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>

static snd_pcm_format_t PCM_FORMAT = SND_PCM_FORMAT_S16_LE;
const static int PCM_CHANNLE = 2;
const static int PCM_RATE = 16000;
const static int PCM_PERIOD_SIZE = 256;//16ms
const static int PCM_BUFFER_SIZE = 256 * 4;
const static int PCM_START_THRESHOLD = 1;

struct alsa_reader_t {
	char device[32];
	snd_pcm_t *handle;
};

static int alsa_open(
	snd_pcm_t** handle,
	char *device,
	bool capture,
	int channels,
	unsigned int rate,
	snd_pcm_format_t format,
	snd_pcm_uframes_t period_size,
	snd_pcm_uframes_t buffer_size,
	snd_pcm_uframes_t start_threshold
);

struct alsa_reader_t *alsa_reader_open(char *device, alsa_reader_error *error) {
	struct alsa_reader_t *wr;

	wr = calloc(sizeof(struct alsa_reader_t), 1);
	assert(wr);
	strncpy(wr->device, device, 32);
	alsa_open(&wr->handle,
		wr->device,
		true,
		PCM_CHANNLE,
		PCM_RATE,
		PCM_FORMAT,
		PCM_PERIOD_SIZE,
		PCM_BUFFER_SIZE,
		PCM_START_THRESHOLD
	);
	return wr;
}

void alsa_reader_close(struct alsa_reader_t *wr) {
	free(wr);
}

int alsa_reader_get_format(struct alsa_reader_t *wr) {
	return PCM_FORMAT;
}

int alsa_reader_get_num_channels(struct alsa_reader_t *wr) {
	return PCM_CHANNLE;
}
int alsa_reader_get_sample_rate(struct alsa_reader_t *wr) {
	return PCM_RATE;
}
int alsa_reader_get_sample_bits(struct alsa_reader_t *wr) {
	return 16;
}
int alsa_reader_get_num_samples(struct alsa_reader_t *wr) {
	return 0x7FFFFFFF;
}

int alsa_reader_get_samples(struct alsa_reader_t *wr, int samples, void *buf) {
	int err;

	if (wr->handle == NULL) {
		alsa_open(&wr->handle,
			wr->device,
			true,
			PCM_CHANNLE,
			PCM_RATE,
			PCM_FORMAT,
			PCM_PERIOD_SIZE,
			PCM_BUFFER_SIZE,
			PCM_START_THRESHOLD
		);
	}
	err = snd_pcm_readi(wr->handle, buf, samples);
	if (err < 0) {
		if (err == -EPIPE)
			printf( "Overrun occurred: %d\n", err);
		err = snd_pcm_recover(wr->handle, err, 0);
		if (err < 0) {
			printf( "Error occured while recover recording: %s\n", snd_strerror(err));
			if (wr->handle) {
				snd_pcm_close(wr->handle);
				wr->handle = NULL;
			}
		}
	}
	return err;
}

static int set_sw_params(snd_pcm_t *pcm,
	snd_pcm_uframes_t buffer_size,
	snd_pcm_uframes_t period_size,
	snd_pcm_uframes_t start_threshold
	) {

	snd_pcm_sw_params_t *params;
	int err;

	snd_pcm_sw_params_alloca(&params);

	if ((err = snd_pcm_sw_params_current(pcm, params)) != 0) {
		printf("Get current params: %s", snd_strerror(err));
		return err;
	}

	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > buffer_size)
		start_threshold = buffer_size;

	if ((err = snd_pcm_sw_params_set_start_threshold(pcm, params, start_threshold)) != 0) {
		printf("Set start threshold: %s: %lu", snd_strerror(err), start_threshold);
		return err;
	}

	printf("set threshold %d\n", start_threshold);

	if ((err = snd_pcm_sw_params(pcm, params)) != 0) {
		printf("%s", snd_strerror(err));
		return err;
	}

	return 0;
}

static int alsa_open(snd_pcm_t** handle,
	char *device,
	bool capture,
	int channels,
	unsigned int rate,
	snd_pcm_format_t format,
	snd_pcm_uframes_t period_size,
	snd_pcm_uframes_t buffer_size,
	snd_pcm_uframes_t start_threshold
	) {
	snd_pcm_hw_params_t *hw_params;
	int err;
	if (capture) {
		if ((err = snd_pcm_open(handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
			fprintf(stderr, "cannot open audio device %s (%s)\n",
				snd_strerror(err));
			exit(1);
		}
	} else {
		if ((err = snd_pcm_open(handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			fprintf(stderr, "cannot open audio device %s (%s)\n",
				snd_strerror(err));
			return -1;
		}
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_any(*handle, hw_params)) < 0) {
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_access(*handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "cannot set access type (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_format(*handle, hw_params, format)) < 0) {
		fprintf(stderr, "cannot set sample format (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	fprintf(stdout, "hw_params format set\n");

	if ((err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &rate, 0)) < 0) {
		fprintf(stderr, "cannot set sample rate (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_channels(*handle, hw_params, channels)) < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	err = snd_pcm_hw_params_set_period_size_near(*handle, hw_params, &period_size, 0);
	if (err)
	{
		printf("Error setting period time (%ld): %s\n", period_size, snd_strerror(err));
		exit(1);
	}
	printf("period_size = %d\n",period_size);

	err = snd_pcm_hw_params_set_buffer_size_near(*handle, hw_params, &buffer_size);
	if (err)
	{
		printf("Error setting buffer size (%ld): %s\n", buffer_size, snd_strerror(err));
		exit(1);
	}
	printf("buffer_size = %d\n",buffer_size);

	if ((err = snd_pcm_hw_params(*handle, hw_params)) < 0) {
		fprintf(stderr, "cannot set parameters (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	snd_pcm_hw_params_free(hw_params);

	set_sw_params(*handle, buffer_size, period_size, start_threshold);
	return 0;
}

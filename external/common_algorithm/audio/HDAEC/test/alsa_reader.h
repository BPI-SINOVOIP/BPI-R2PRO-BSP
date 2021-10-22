#ifndef alsa_reader_H
#define alsa_reader_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WR_NO_ERROR = 0,
    WR_OPEN_ERROR,
    WR_IO_ERROR,
    WR_ALLOC_ERROR,
    WR_BAD_CONTENT,
} alsa_reader_error;

typedef struct alsa_reader_t alsa_reader;

alsa_reader *alsa_reader_open(char *filename, alsa_reader_error *error);
void alsa_reader_close(alsa_reader *wr);
int alsa_reader_get_format(alsa_reader *wr);
int alsa_reader_get_num_channels(alsa_reader *wr);
int alsa_reader_get_sample_rate(alsa_reader *wr);
int alsa_reader_get_sample_bits(alsa_reader *wr);
int alsa_reader_get_num_samples(alsa_reader *wr);
int alsa_reader_get_samples(alsa_reader *wr, int n, void *buf);

#ifdef __cplusplus
}
#endif

#endif//alsa_reader_H


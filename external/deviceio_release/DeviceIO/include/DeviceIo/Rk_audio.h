#ifndef __RK_AUDIO_H__
#define __RK_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif


void RK_audio_set_volume(int vol);
int RK_audio_get_volume(void);
void RK_audio_mute(void);
int RK_audio_unmute(void);
int RK_audio_limit_max_volume(int vol);

#ifdef __cplusplus
}
#endif

#endif

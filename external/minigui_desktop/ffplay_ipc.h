#ifndef FFPLAY_IPC_H_
#define FFPLAY_IPC_H_

void media_play(const char *file_path, HWND hWnd, int start_time);
void media_exit(void);
void media_pause(void);
void media_restore(void);
void media_wait(void);
void media_seek_to(char *value);

#endif // FFPLAY_IPC_H_
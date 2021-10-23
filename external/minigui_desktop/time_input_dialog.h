#ifndef __TIME_INPUT_H__
#define __TIME_INPUT_H__

#define INPUT_DATE              0x10
#define INPUT_TIME_12           0x20
#define INPUT_TIME_24           0x30
#define INPUT_TIME_TIMING_12     0x40
#define INPUT_TIME_TIMING_24     0x50

extern HWND last_hWnd;
extern void creat_time_input_dialog(HWND hWnd, int type, struct tm *tar_time);

#endif


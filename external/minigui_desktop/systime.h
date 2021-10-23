#ifndef __SYSTIME_H__
#define __SYSTIME_H__

extern void time_flush(void);
extern void time_set(struct tm time);
extern void timing_set(struct tm time, int n);

#endif

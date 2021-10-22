#ifndef __RK_TIMER_H__
#define __RK_TIMER_H__

#include <stdint.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RK_Timer {
	uint64_t timer_start;
	uint32_t timer_time;
	uint32_t timer_repeat;
	uint64_t timer_last;
	void (*timer_cb)(int end);
	struct RK_Timer *next;
} RK_Timer_t;

typedef void (*RK_timer_callback)(const int end);

int RK_timer_init(void);
int RK_timer_create(RK_Timer_t *handle, RK_timer_callback cb, const uint32_t time, const uint32_t repeat);
int RK_timer_start(RK_Timer_t *handle);
int RK_timer_stop(RK_Timer_t *handle);
int RK_timer_exit(void);

#ifdef __cplusplus
}
#endif

#endif

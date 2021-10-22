#ifndef __RK_KEY_H__
#define __RK_KEY_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*RK_input_callback)(const int key_code, const int key_value);
typedef int (*RK_input_press_callback)(const int key_code);
typedef int (*RK_input_long_press_callback)(const int key_code, const uint32_t time);
typedef int (*RK_input_long_press_hb_callback)(const int key_code, const int times);
typedef int (*RK_input_compose_press_callback)(const char* compose, const uint32_t time);
typedef int (*RK_input_transaction_press_callback)(const char* trans, const uint32_t time);
typedef int (*RK_input_multiple_press_callback)(const int key_code, const int times);

int RK_input_init(RK_input_callback input_callback_cb);
int RK_input_register_press_callback(RK_input_press_callback cb);
int RK_input_register_long_press_callback(RK_input_long_press_callback cb, const uint32_t time, const int key_code);
int RK_input_register_long_press_hb_callback(RK_input_long_press_hb_callback cb, const uint32_t time, const int key_code);
int RK_input_register_multiple_press_callback(RK_input_multiple_press_callback cb, const int key_code, const int times);
int RK_input_register_compose_press_callback(RK_input_compose_press_callback cb, const uint32_t time, const int key_code, ...);
int RK_input_register_transaction_press_callback(RK_input_transaction_press_callback cb, const uint32_t time, int key_code, ...);
int RK_input_events_print(void);
int RK_input_exit(void);


#ifdef __cplusplus
}
#endif

#endif

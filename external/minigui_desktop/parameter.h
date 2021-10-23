/*
 * Copyright (c) 2018 rockchip
 *
 */
#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <DeviceIo/Rk_wifi.h>

#define USE_12_HOUR_FORMAT 0
#define USE_24_HOUR_FORMAT 1

int parameter_recovery(void);
int get_language(void);
void set_language(int val);
int get_screenoff(void);
void set_screenoff(int val);
int get_screenoff_val(void);
void set_screenoff_val(int val);
int get_eq(void);
void set_eq(int val);
int get_backlight(void);
void set_backlight(int val);
int get_gamedisp(void);
void set_gamedisp(int val);
int get_themestyle(void);
void set_themestyle(int val);
char *get_ui_image_path(void);
int parameter_init(void);
void parameter_deinit(void);
int get_volume(void);
void set_volume(int val);
int get_wifi_state(void);
void set_wifi_state(RK_WIFI_RUNNING_State_e     val);
int add_wifi_date(char *ssid, char *psk);
int del_wifi_date(char *ssid);
char *get_wifi_psk(char *ssid);
void _print_wifi(void);
void reset_wifi_val(void);
void set_time_format(int format);
int get_time_format(void);
void set_if_sync_net_time(int status);
int get_if_sync_net_time(void);

#endif

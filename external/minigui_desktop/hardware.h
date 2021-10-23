/*
 * Copyright (c) 2018 rockchip
 *
 */
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

int set_bl_brightness(uint32_t brightness);
int get_bl_brightness(void);
int ac_is_online(void);
int get_battery_capacity(void);
int get_pot_scaled(void);
#endif

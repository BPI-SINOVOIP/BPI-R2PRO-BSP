/*
 * Copyright (c) 2018 rockchip
 *
 */

#include <stdint.h>
#include "sysfs.h"

#define BL_MAX_BRIGHTNESS   255
#define BL_SYSFS_PATH       "/sys/class/backlight/backlight"
#define AC_SYSFS_PATH       "/sys/class/power_supply/ac"
#define BATTERY_SYSFS_PATH  "/sys/class/power_supply/battery"
#define ADC_SYSFS_PATH      "/sys/devices/platform/adc-pot"

int set_bl_brightness(uint32_t brightness)
{
    printf("set_bl_brightness the brightness is %d\n", brightness);
    if (brightness > BL_MAX_BRIGHTNESS)
        brightness = BL_MAX_BRIGHTNESS;
    return write_sysfs_int_and_verify("brightness", BL_SYSFS_PATH, brightness);
}

int get_bl_brightness(void)
{
    return read_sysfs_posint("brightness", BL_SYSFS_PATH);
}

int ac_is_online(void)
{
#ifdef ENABLE_RK816
    char bat_buf[20];
    read_sysfs_string("status", BATTERY_SYSFS_PATH, bat_buf);
    if (!strcmp("Charging", bat_buf) || !strcmp("Full", bat_buf))
    {
        //printf("Charging...\n");
        return 1;
    }
    if (!strcmp("Discharging", bat_buf))
    {
        //printf("Discharging...\n");
        return 0;
    }
    return 0;
#else
    return read_sysfs_posint("online", AC_SYSFS_PATH);
#endif
}

int get_battery_capacity(void)
{
    return read_sysfs_posint("capacity", BATTERY_SYSFS_PATH);
}

int get_pot_scaled(void)
{
    return read_sysfs_posint("scaled", ADC_SYSFS_PATH);
}

#ifndef __RK_BATTERY_H__
#define __RK_BATTERY_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	RK_BATTERY_STATUS = 0,
	RK_BATTERY_LEVEL
} RK_Battery_Msg_Type_e;

typedef enum {
        RK_BATTERY_STATUS_UNKNOWN = 0,
        RK_BATTERY_STATUS_CHARGING,
        RK_BATTERY_STATUS_DISCHARGING,
        RK_BATTERY_STATUS_FULL,
} RK_Battery_Status_e;

typedef enum
{
	RK_BATTERY_LEVEL_IDEL = 0,
	RK_BATTERY_LEVEL_FULL,
	RK_BATTERY_LEVEL_80,
	RK_BATTERY_LEVEL_50,
	RK_BATTERY_LEVEL_LOW,
	RK_BATTERY_LEVEL_OFF
} RK_Battery_Level_e;

typedef void (*RK_battery_callback)(void* userdata, RK_Battery_Msg_Type_e type, int value);

int RK_battery_init(void);
int RK_battery_register_callback(void *userdata, RK_battery_callback cb);
int RK_battery_get_cur_level();
int RK_battery_get_cur_temp();
RK_Battery_Status_e RK_battery_get_status();

#ifdef __cplusplus
}
#endif

#endif

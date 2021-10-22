#ifndef __RK_LED_H__
#define __RK_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum RK_Led_Effect_type {
	Led_Effect_type_NONE = 0,
	Led_Effect_type_BLINK,
	Led_Effect_type_BREATH
} RK_Led_Effect_type_e;

typedef enum RK_Led_Effect_layer {
	Led_Effect_layer_TEMP = 0,
	Led_Effect_layer_STABLE,
	Led_Effect_layer_REALTIME
} RK_Led_Effect_layer_e;

typedef struct RK_Led_Effect {
	int period;               // 灯效周期，例如呼吸一次为3000ms. <=0 表示周期无限大
	int timeout;              // 超时时间，<=0 表示无限大
	int colors;
	int colors_blink;
	int priority;             // 灯效优先级
	char name[64];
	RK_Led_Effect_type_e type;
	RK_Led_Effect_layer_e layer;
} RK_Led_Effect_t;

int RK_led_init(void);
int RK_set_led_effect(RK_Led_Effect *effect);
int RK_set_led_effect_off(const RK_Led_Effect_layer_e layer, const char *name);
int RK_set_all_led_effect_off(void);
int RK_set_all_led_status(const int Rval, const int Gval, const int Bval);
int RK_set_all_led_off();
int RK_led_exit(void);


#ifdef __cplusplus
}
#endif

#endif

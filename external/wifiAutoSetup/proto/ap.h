#ifndef __AP_H__
#define __AP_H__

#include <easy_setup.h>

#define AP_PARAM_KEY 0x1
#define AP_PARAM_NONCE 0x2
#define AP_PARAM_CHANNEL 0x10
#define AP_PARAM_SSID 0x20
typedef struct {
    uint32 param_set;
    uint8 key[16];
    uint8 nonce[16];
    uint8 key_qqcon[16];
    uint8 nonce_qqcon[16];
    uint8 channel;
    uint8 ssid_len;
    uint8 ssid[32];  /* key string for decoding */
} ap_param_t;

typedef struct {
    easy_setup_result_t es_result;
    ip_address_t host_ip_address;      /* setup client's ip address */
    uint16 host_port;            /* setup client's port */
} ap_result_t;

int ap_set_ssid(const char* ssid, int len);
void ap_get_param(void* p);
void ap_set_result(const void* p);
int ap_get_sender_ip(char buff[], int buff_len);
int ap_get_sender_port(uint16* port);

int ap_set_key(const char* key);
int ap_set_nonce(const char* nonce);

#endif /* __AP_H__ */

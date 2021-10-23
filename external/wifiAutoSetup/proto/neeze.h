#ifndef __NEEZE_H__
#define __NEEZE_H__

#include <easy_setup.h>

#define NEEZE_KEY_LEN (16)
#define NEEZE_NONCE_LEN (13)

#define MCAST_PARAM_KEY 0x1
#define MCAST_PARAM_NONCE 0x2
typedef struct {
    uint32 param_set;
    uint8 key[NEEZE_KEY_LEN];  /* key string for decoding */
    uint8 nonce[NEEZE_NONCE_LEN]; /* random bytes */
    uint8 key_qqcon[NEEZE_KEY_LEN];  /* key string for decoding for qqcon */
    uint8 nonce_qqcon[NEEZE_NONCE_LEN]; /* random bytes for qqcon */
} neeze_param_t;

typedef struct {
    easy_setup_result_t es_result;
    ip_address_t host_ip_address;      /* setup client's ip address */
    uint16 host_port;            /* setup client's port */
} neeze_result_t;

void neeze_get_param(void* p);
void neeze_set_result(const void* p);

int neeze_set_key(const char* key);
int neeze_set_nonce(const char* key);

int neeze_get_sender_ip(char buff[], int buff_len);
int neeze_get_sender_port(uint16* port);

#endif /* __NEEZE_H__ */

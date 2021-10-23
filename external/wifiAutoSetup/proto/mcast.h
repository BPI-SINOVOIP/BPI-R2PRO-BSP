#ifndef __BCAST_H__
#define __BCAST_H__

#include <easy_setup.h>

#define MCAST_KEY_LEN (16)
#define MCAST_NONCE_LEN (13)

#define MCAST_PARAM_KEY 0x1
#define MCAST_PARAM_NONCE 0x2
#define MCAST_PARAM_KEY_QQCON 0x4
#define MCAST_PARAM_NONCE_QQCON 0x8
typedef struct {
    uint32 param_set;
    uint8 key[MCAST_KEY_LEN];  /* key string for decoding */
    uint8 nonce[MCAST_NONCE_LEN]; /* random bytes */
    uint8 key_qqcon[MCAST_KEY_LEN];  /* key string for decoding for qqcon */
    uint8 nonce_qqcon[MCAST_NONCE_LEN]; /* random bytes for qqcon */
} mcast_param_t;

typedef struct {
    easy_setup_result_t es_result;
    ip_address_t   host_ip_address;      /* setup client's ip address */
    uint16         host_port;            /* setup client's port */
} mcast_result_t;

void mcast_get_param(void* p);
void mcast_set_result(const void* p);

int mcast_set_key(const char* key);
int mcast_set_nonce(const char* nonce);

int mcast_get_sender_ip(char buff[], int buff_len);
int mcast_get_sender_port(uint16* port);

#endif /* __BCAST_H__ */

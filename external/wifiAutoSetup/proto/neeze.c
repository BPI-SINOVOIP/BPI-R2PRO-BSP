#include <string.h>

#include <neeze.h>

static neeze_param_t g_neeze_param = {
    .param_set = 0,
};

static neeze_result_t g_neeze_result;

void neeze_get_param(void* p) {
    memcpy(p, &g_neeze_param, sizeof(g_neeze_param));
}

void neeze_set_result(const void* p) {
    memcpy(&g_neeze_result, p, sizeof(g_neeze_result));
}

int neeze_set_key(const char* key) {
    int len = sizeof(g_neeze_param.key);
    if (len > strlen(key))
        len = strlen(key);
    memcpy(g_neeze_param.key, key, len);
    g_neeze_param.param_set |= MCAST_PARAM_KEY;

    return 0;
}

int neeze_set_nonce(const char* nonce) {
    int len = sizeof(g_neeze_param.nonce);
    if (len > strlen(nonce))
        len = strlen(nonce);
    memcpy(g_neeze_param.nonce, nonce, len);
    g_neeze_param.param_set |= MCAST_PARAM_NONCE;

    return 0;
}

int neeze_get_sender_ip(char buff[], int buff_len) {
    char ip_text[16];
    neeze_result_t* r = &g_neeze_result;

    if (g_neeze_result.es_result.state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    if (r->host_ip_address.version != 4) {
        return -1;
    }

    int ip = r->host_ip_address.ip.v4;
    snprintf(ip_text, sizeof(ip_text), "%d.%d.%d.%d",
            (ip>>24)&0xff,
            (ip>>16)&0xff,
            (ip>>8)&0xff,
            (ip>>0)&0xff);
    ip_text[strlen(ip_text)] = 0;

    if ((size_t) buff_len < strlen(ip_text)+1) {
        LOGE("insufficient buffer provided: %d < %d\n", buff_len, strlen(ip_text)+1);
        return -1;
    }

    strcpy(buff, ip_text);

    return 0;
}

int neeze_get_sender_port(uint16* port) {
    if (g_neeze_result.es_result.state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    *port = g_neeze_result.host_port;

    return 0;
}

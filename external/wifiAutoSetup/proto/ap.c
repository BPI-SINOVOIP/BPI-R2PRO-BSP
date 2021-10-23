#include <string.h>

#include <ap.h>

static ap_param_t g_ap_param = {
    .param_set = 0,
};
static ap_result_t g_ap_result;

void ap_get_param(void* p) {
    memcpy(p, &g_ap_param, sizeof(g_ap_param));
}

void ap_set_result(const void* p) {
    memcpy(&g_ap_result, p, sizeof(g_ap_result));
}

int ap_set_ssid(const char* ssid, int len) {
    if (!ssid || len <= 0) return -1;

    g_ap_param.ssid_len = len;
    if (g_ap_param.ssid_len > sizeof(g_ap_param.ssid))
        g_ap_param.ssid_len = sizeof(g_ap_param.ssid);
    memcpy(g_ap_param.ssid, ssid, g_ap_param.ssid_len);
    g_ap_param.param_set |= AP_PARAM_SSID;

    return 0;
}

int ap_get_sender_ip(char buff[], int buff_len) {
    char ip_text[16];
    ap_result_t* r = &g_ap_result;

    if (g_ap_result.es_result.state != EASY_SETUP_STATE_DONE) {
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

int ap_get_sender_port(uint16* port) {
    if (g_ap_result.es_result.state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    *port = g_ap_result.host_port;

    return 0;
}

int ap_set_key(const char* key) {
    int len = sizeof(g_ap_param.key);
    if (len > strlen(key))
        len = strlen(key);
    memcpy(g_ap_param.key, key, len);
    g_ap_param.param_set |= AP_PARAM_KEY;

    return 0;
}

int ap_set_nonce(const char* nonce) {
    int len = sizeof(g_ap_param.nonce);
    if (len > strlen(nonce))
        len = strlen(nonce);
    memcpy(g_ap_param.nonce, nonce, len);
    g_ap_param.param_set |= AP_PARAM_NONCE;

    return 0;
}

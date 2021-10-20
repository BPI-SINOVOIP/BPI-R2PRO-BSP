#ifndef __DB_MONITOR_H
#define __DB_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct NetworkConfig {
    char *hwaddr;
    char *method;
    char *ip;
    char *mask;
    char *gate;
    char *dns1;
    char *dns2;
};

int database_network_config(struct NetworkConfig *config);
struct NtpCfg *database_ntp_get(void);
void *database_networkservice_json_get(char *service);
void *database_networkip_json_get(char *interface);
void *database_networkip_get(char *interface);
void *database_networkservice_get(char *service);
void *database_networkpower_json_get(char *type);
void *database_networkpower_get(char *type);
void database_init(void);
GHashTable *database_hash_network_ip_get(void);
void database_networkservice_remove(char *service);
void database_hash_init(void);

#ifdef __cplusplus
}
#endif

#endif

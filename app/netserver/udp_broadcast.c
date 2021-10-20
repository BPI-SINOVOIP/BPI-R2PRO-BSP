#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>

#include "json-c/json.h"
#include "manage.h"
#include "db_monitor.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "udp_broadcast.c"

#define SEEK_DEVICE          "SeekDevice"
#define NETWORK_CONFIG       "NetworkConfig"

int udp_broadcast_tx(char *msg)
{
    int sock;
    struct sockaddr_in peer_addr;
    const int opt = 1;
    //char msg[100] = "Msg from udp broadcast client!";
    socklen_t peer_addrlen = 0;
    int ret = 0;
 
    bzero(&peer_addr, sizeof(struct sockaddr_in));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(6868);
    peer_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    peer_addrlen = sizeof(struct sockaddr_in);
 
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        LOG_INFO("Ceate sock fail\n");
 
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
    if (ret == -1)
        LOG_INFO("Set sock to broadcast format fail\n");
 
    sendto(sock, msg, strlen(msg), 0,
            (struct sockaddr *)&peer_addr, peer_addrlen);
    //LOG_INFO("Done\n");
}

static void *udp_broadcast_rx_thread(void *arg)
{
    int sock;
    struct sockaddr_in own_addr, peer_addr;
    const int opt = 1;
    char recv_msg[1024] = {0};
    socklen_t peer_addrlen = 0;
    char peer_name[30] = {0};
    int ret = 0;

    bzero(&own_addr, sizeof(struct sockaddr_in));
    bzero(&peer_addr, sizeof(struct sockaddr_in));
    own_addr.sin_family = AF_INET;
    own_addr.sin_port = htons(6868);
    own_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        LOG_INFO("Ceate sock fail\n");

    ret = bind(sock, (struct sockaddr *)&own_addr, sizeof(struct sockaddr_in));
    if (ret == -1)
        LOG_INFO("Bind addr fail\n");
 
    while (1) {
        ret = recvfrom(sock, recv_msg, sizeof(recv_msg), 0,
            (struct sockaddr *)&peer_addr, &peer_addrlen);
        if (ret > 0) {
            inet_ntop(AF_INET, &peer_addr.sin_addr.s_addr,
                    peer_name, sizeof(peer_name));
            //LOG_INFO("Recv from %s, msg[%s]\n", peer_name, recv_msg);
            json_object *j_cfg = json_tokener_parse(recv_msg);
            if (j_cfg) {
                char *sender = (char *)json_object_get_string(json_object_object_get(j_cfg, "Sender"));
                //LOG_INFO("%s,sender = %s\n", __func__, sender);
                if (!strcmp("server", sender)) {
                    char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "Cmd"));
                    if (g_str_equal(cmd, SEEK_DEVICE)) {
                        json_object *j_array = (json_object *)get_networkip_json_array("");
                        json_object *j_rx = json_object_new_object();
                        json_object_object_add(j_rx, "Cmd", json_object_new_string(SEEK_DEVICE));
                        json_object_object_add(j_rx, "Sender", json_object_new_string("client"));
                        json_object_object_add(j_rx, "Data", j_array);
                        udp_broadcast_tx((char *)json_object_to_json_string(j_rx));
                        json_object_put(j_rx);
                    } else if (g_str_equal(cmd, NETWORK_CONFIG)) {
                        struct NetworkConfig config;
                        json_object *j_data = json_object_object_get(j_cfg, "Data");
                        config.hwaddr = (char *)json_object_get_string(json_object_object_get(j_data, "sHWAddr"));
                        config.method = (char *)json_object_get_string(json_object_object_get(j_data, "sMethod"));
                        config.ip = (char *)json_object_get_string(json_object_object_get(j_data, "sV4Address"));
                        config.mask = (char *)json_object_get_string(json_object_object_get(j_data, "sV4Netmask"));
                        config.gate = (char *)json_object_get_string(json_object_object_get(j_data, "sV4Gateway"));
                        config.dns1 = (char *)json_object_get_string(json_object_object_get(j_data, "sDNS1"));
                        config.dns2 = (char *)json_object_get_string(json_object_object_get(j_data, "sDNS2"));
                        if (database_network_config(&config) == 0) {
                            json_object *j_rx = json_object_new_object();
                            json_object_object_add(j_rx, "Cmd", json_object_new_string(NETWORK_CONFIG));
                            json_object_object_add(j_rx, "Sender", json_object_new_string("client"));
                            json_object_object_add(j_rx, "Data", json_object_new_string("success"));
                            udp_broadcast_tx((char *)json_object_to_json_string(j_rx));
                            json_object_put(j_rx);
                        }
                    }
                }
                json_object_put(j_cfg);
            }
        } else
            LOG_INFO("Recv msg err\n");

        bzero(recv_msg, sizeof(recv_msg));
    }

    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

void udp_broadcast_init(void)
{
    pthread_t tid;
    pthread_create(&tid, NULL, udp_broadcast_rx_thread, NULL);
}

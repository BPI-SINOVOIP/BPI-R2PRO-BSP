#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <json-c/json.h>

#define SEEK_DEVICE          "SeekDevice"
#define NETWORK_CONFIG       "NetworkConfig"

int udp_broadcast_tx(char *msg)
{
    int sock;
    struct sockaddr_in peer_addr;
    const int opt = 1;
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
}

static void device_info_printf(char *info)
{
    json_object *j_array = json_tokener_parse(info);

    if (j_array) {
        int num = json_object_array_length(j_array);
        for (int i = 0; i < num; i++) {
            json_object *j_cfg = json_object_array_get_idx(j_array, i);
            json_object *j_link = json_object_object_get(j_cfg, "link");
            json_object *j_ipv4 = json_object_object_get(j_cfg, "ipv4");
            json_object *j_dbconfig = json_object_object_get(j_cfg, "dbconfig");
            char *method = (char *)json_object_get_string(json_object_object_get(j_dbconfig, "sV4Method"));

            LOG_INFO("  Interface:%s\n", (char *)json_object_get_string(json_object_object_get(j_link, "sInterface")));
            LOG_INFO("    HWaddr:%s\n", (char *)json_object_get_string(json_object_object_get(j_link, "sAddress")));
            LOG_INFO("    Method:%s\n", method);
            if (!strcmp(method, "dhcp")) {
                LOG_INFO("    inet addr:%s\n", (char *)json_object_get_string(json_object_object_get(j_ipv4, "sV4Address")));
                LOG_INFO("    Mask:%s\n", (char *)json_object_get_string(json_object_object_get(j_ipv4, "sV4Netmask")));
                LOG_INFO("    Gateway:%s\n", (char *)json_object_get_string(json_object_object_get(j_ipv4, "sV4Gateway")));
                LOG_INFO("    DNS1:%s\n", (char *)json_object_get_string(json_object_object_get(j_link, "sDNS1")));
                LOG_INFO("    DNS2:%s\n", (char *)json_object_get_string(json_object_object_get(j_link, "sDNS2")));
            } else {
                LOG_INFO("    inet addr:%s\n", (char *)json_object_get_string(json_object_object_get(j_dbconfig, "sV4Address")));
                LOG_INFO("    Mask:%s\n", (char *)json_object_get_string(json_object_object_get(j_dbconfig, "sV4Netmask")));
                LOG_INFO("    Gateway:%s\n", (char *)json_object_get_string(json_object_object_get(j_dbconfig, "sV4Gateway")));
                LOG_INFO("    DNS1:%s\n", (char *)json_object_get_string(json_object_object_get(j_dbconfig, "sDNS1")));
                LOG_INFO("    DNS2:%s\n", (char *)json_object_get_string(json_object_object_get(j_dbconfig, "sDNS2")));
            }
        }
        json_object_put(j_array);
    }
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
                if (!strcmp("client", sender)) {
                    char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "Cmd"));
                    if (!strcmp(cmd, SEEK_DEVICE)) {
                        LOG_INFO("\n************************\n");
                        LOG_INFO("device: %s\n", peer_name);
                        device_info_printf((char *)json_object_get_string(json_object_object_get(j_cfg, "Data")));
                        LOG_INFO("************************\n");
                    } else if (!strcmp(cmd, NETWORK_CONFIG)) {
                        LOG_INFO("\n************************\n");
                        char *ret = (char *)json_object_get_string(json_object_object_get(j_cfg, "Data"));
                        LOG_INFO("device: %s, %s,%s\n", peer_name, NETWORK_CONFIG, ret);
                        LOG_INFO("************************\n");
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

void help_printf(void)
{
    LOG_INFO("************************\n");
    LOG_INFO("0.help\n");
    LOG_INFO("1.seek device\n");
    LOG_INFO("2.config device\n");
    LOG_INFO("************************\n");
}

void seek_device(void)
{
    json_object *j_rx = json_object_new_object();
    json_object_object_add(j_rx, "Cmd", json_object_new_string(SEEK_DEVICE));
    json_object_object_add(j_rx, "Sender", json_object_new_string("server"));
    json_object_object_add(j_rx, "Data", json_object_new_string(""));
    udp_broadcast_tx((char *)json_object_to_json_string(j_rx));
    json_object_put(j_rx);
}

int is_hwaddr(char *hwaddr)
{
   char *tmp = hwaddr + 2;

   for (int i = 0; i < 5; i ++) {
       if (tmp[0] != ':')
           return -1;
       tmp = tmp + 3;
   }

   return 0;
}

int is_ipv4(char *ip)
{
    char* ptr;
    int count = 0;
    char str[32];
    memcpy(str, ip, strlen(ip));
    const char *p = str;

    while (*p != '\0') {
        if(*p == '.')
        count++;
        p++;
    }

    if (count != 3)
        goto err;

    count = 0;
    ptr = strtok(str, ".");
    while (ptr != NULL) {   
        count++;
        if (ptr[0] == '0' && isdigit(ptr[1]))
            goto err;

        int a = atoi(ptr);
        if (count == 1 && a == 0)
            goto err;

        if (a < 0 || a > 255)
            goto err;

        ptr = strtok(NULL, ".");
    }

    if(count == 4)
        return 0;
err:

    return -1;
}

void config_device(void)
{
    char hwaddr[32] = {0};
    char method[32] = {0};
    char ip[32] = {0};
    char mask[32] = {0};
    char gate[32] = {0};
    char dns1[32] = {0};
    char dns2[32] = {0};
    LOG_INFO("please enter HWaddr:");
    scanf("%s", hwaddr);
    LOG_INFO("please enter Method(dhcp/manual):");
    scanf("%s", method);
    LOG_INFO("please enter inet addr:");
    scanf("%s", ip);
    LOG_INFO("please enter Mask:");
    scanf("%s", mask);
    LOG_INFO("please enter Gateway:");
    scanf("%s", gate);
    LOG_INFO("please enter DNS1:");
    scanf("%s", dns1);
    LOG_INFO("please enter DNS2:");
    scanf("%s", dns2);

    if (is_ipv4(ip) != 0)
        ip[0] = 0;

    if (is_ipv4(mask) != 0)
        mask[0] = 0;

    if (is_ipv4(gate) != 0)
        gate[0] = 0;

    if (is_ipv4(dns1) != 0)
        dns1[0] = 0;

    if (is_ipv4(dns2) != 0)
        dns2[0] = 0;

    LOG_INFO("  HWaddr:%s\n", hwaddr);
    LOG_INFO("  Method:%s\n", method);
    LOG_INFO("  inet addr:%s\n", ip);
    LOG_INFO("  Mask:%s\n", mask);
    LOG_INFO("  Gateway:%s\n", gate);
    LOG_INFO("  DNS1:%s\n", dns1);
    LOG_INFO("  DNS2:%s\n", dns2);
    
    if (is_hwaddr(hwaddr) != 0) {
        LOG_INFO("HWaddr is err,\n");
        return;
    }

    if (strcmp(method, "dhcp") && strcmp(method, "manual")) {
        LOG_INFO("Method is err,\n");
        return;
    }

    if (!strcmp(method, "manual")) {
        if (ip[0] == 0) {
            LOG_INFO("inet addr is err,\n");
            return;
        }
        if (mask[0] == 0) {
            LOG_INFO("Mask is err,\n");
            return;
        }
        if (gate[0] == 0) {
            LOG_INFO("Gateway is err,\n");
            return;
        }
    }

    LOG_INFO("Are you sure you want to configure?(y/n):");
    char yn;
    scanf("%c", &yn);
    if (yn == 0xa)
        scanf("%c", &yn);
    if (yn == 'y' || yn == 'Y') {
        json_object *j_rx = json_object_new_object();
        json_object_object_add(j_rx, "Cmd", json_object_new_string(NETWORK_CONFIG));
        json_object_object_add(j_rx, "Sender", json_object_new_string("server"));

        json_object *j_cfg = json_object_new_object();
        json_object_object_add(j_cfg, "sHWAddr", json_object_new_string(hwaddr));
        json_object_object_add(j_cfg, "sMethod", json_object_new_string(method));
        json_object_object_add(j_cfg, "sV4Address", json_object_new_string(ip));
        json_object_object_add(j_cfg, "sV4Netmask", json_object_new_string(mask));
        json_object_object_add(j_cfg, "sV4Gateway", json_object_new_string(gate));
        json_object_object_add(j_cfg, "sDNS1", json_object_new_string(dns1));
        json_object_object_add(j_cfg, "sDNS2", json_object_new_string(dns2));

        json_object_object_add(j_rx, "Data", j_cfg);
        udp_broadcast_tx((char *)json_object_to_json_string(j_rx));
        json_object_put(j_rx);
    } else {
        LOG_INFO("don't config\n");
    }
}

int main(void)
{
    udp_broadcast_init();

    help_printf();
    while (1) {
        char cmd = 0;
        LOG_INFO("please enter:");
again:
        scanf("%c", &cmd);
        switch(cmd) {
            case '0':
                help_printf();
                break;
            case '1':
                seek_device();
                break;
            case '2':
                config_device();
                break;
            case 0xa:
                continue;
                break;
        }
        goto again;
    }
}
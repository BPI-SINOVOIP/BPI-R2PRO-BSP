#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <arpa/inet.h>

#include <glib.h>

#include <pthread.h>
#include <gdbus.h>

#include "json-c/json.h"
#include "network_func.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "network_func.c"

#define SIOCETHTOOL     0x8946
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define ETHTOOL_GSET        0x00000001 /* Get settings. */
#define ETHTOOL_SSET        0x00000002 /* Set settings. */

/* Indicates what features are supported by the interface. */
#define SUPPORTED_10baseT_Half       (1 << 0)
#define SUPPORTED_10baseT_Full       (1 << 1)
#define SUPPORTED_100baseT_Half      (1 << 2)
#define SUPPORTED_100baseT_Full      (1 << 3)
#define SUPPORTED_1000baseT_Half     (1 << 4)
#define SUPPORTED_1000baseT_Full     (1 << 5)
#define SUPPORTED_Autoneg            (1 << 6)
#define SUPPORTED_TP                 (1 << 7)
#define SUPPORTED_AUI                (1 << 8)
#define SUPPORTED_MII                (1 << 9)
#define SUPPORTED_FIBRE              (1 << 10)
#define SUPPORTED_BNC                (1 << 11)
#define SUPPORTED_10000baseT_Full    (1 << 12)
#define SUPPORTED_Pause              (1 << 13)
#define SUPPORTED_Asym_Pause         (1 << 14)
#define SUPPORTED_2500baseX_Full     (1 << 15)
#define SUPPORTED_Backplane          (1 << 16)
#define SUPPORTED_1000baseKX_Full    (1 << 17)
#define SUPPORTED_10000baseKX4_Full  (1 << 18)
#define SUPPORTED_10000baseKR_Full   (1 << 19)
#define SUPPORTED_10000baseR_FEC     (1 << 20)

#define ADVERTISED_10baseT_Half          (1 << 0)
#define ADVERTISED_10baseT_Full          (1 << 1)
#define ADVERTISED_100baseT_Half     (1 << 2)
#define ADVERTISED_100baseT_Full     (1 << 3)
#define ADVERTISED_1000baseT_Half     (1 << 4)
#define ADVERTISED_1000baseT_Full     (1 << 5)
#define ADVERTISED_Autoneg          (1 << 6)
#define ADVERTISED_TP               (1 << 7)
#define ADVERTISED_AUI               (1 << 8)
#define ADVERTISED_MII               (1 << 9)
#define ADVERTISED_FIBRE          (1 << 10)
#define ADVERTISED_BNC               (1 << 11)
#define ADVERTISED_10000baseT_Full     (1 << 12)
#define ADVERTISED_Pause          (1 << 13)
#define ADVERTISED_Asym_Pause          (1 << 14)
#define ADVERTISED_2500baseX_Full     (1 << 15)
#define ADVERTISED_Backplane          (1 << 16)
#define ADVERTISED_1000baseKX_Full     (1 << 17)
#define ADVERTISED_10000baseKX4_Full     (1 << 18)
#define ADVERTISED_10000baseKR_Full     (1 << 19)
#define ADVERTISED_10000baseR_FEC     (1 << 20)

/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */
#define SPEED_10          10
#define SPEED_100          100
#define SPEED_1000          1000
#define SPEED_2500          2500
#define SPEED_10000          10000

/* Duplex, half or full. */
#define DUPLEX_HALF          0x00
#define DUPLEX_FULL          0x01

/* Which connector port. */
#define PORT_TP               0x00 // 双绞线
#define PORT_AUI          0x01
#define PORT_MII          0x02
#define PORT_FIBRE          0x03
#define PORT_BNC          0x04
#define PORT_DA               0x05
#define PORT_NONE          0xef
#define PORT_OTHER          0xff

/* Enable or disable autonegotiation.  If this is set to enable,
* the forced link modes above are completely ignored.
*/
#define AUTONEG_DISABLE          0x00
#define AUTONEG_ENABLE          0x01

void get_ethernet_speed(char *speed, struct ethtool_cmd *ep)
{
    if (ep->autoneg)
        sprintf(speed, "Auto");
    else
        sprintf(speed, "%d%s", ep->speed, ep->duplex ? "baseT/Full" : "baseT/Half");
}

void get_ethernet_speedsupport(char *speedsupport, struct ethtool_cmd *ep)
{
    if (ep->supported & SUPPORTED_Autoneg)
        strcat(speedsupport, "Auto ");
    if (ep->supported & SUPPORTED_10baseT_Half)
        strcat(speedsupport, "10baseT/Half ");
    if (ep->supported & SUPPORTED_10baseT_Full)
        strcat(speedsupport, "10baseT/Full ");
    if (ep->supported & SUPPORTED_100baseT_Half)
        strcat(speedsupport, "100baseT/Half ");
    if (ep->supported & SUPPORTED_100baseT_Full)
        strcat(speedsupport, "100baseT/Full ");
    if (ep->supported & SUPPORTED_1000baseT_Half)
        strcat(speedsupport, "1000baseT/Half ");
    if (ep->supported & SUPPORTED_1000baseT_Full)
        strcat(speedsupport, "1000baseT/Full ");
}

int get_ethernet_tool(char *interface, struct ethtool_cmd *ep)
{
    struct ifreq ifr, *ifrp;
    int fd;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Cannot get control socket");
        return -1;
    }

    int err;

    ep->cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)ep;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    close(fd);

    if (err != 0)
        return -1;

    return 0;
}

int get_ethernet_tool_speed_set(char *interface, char *speed)
{
    int speed_wanted = -1;
    int duplex_wanted = -1;
    int autoneg_wanted = AUTONEG_ENABLE;
    int advertising_wanted = -1;
    struct ethtool_cmd ecmd;
    struct ifreq ifr;
    int fd = 0;
    int err = -1;
    int needupdate = 0;

    if (interface == NULL) {
        LOG_INFO("interface emtpy...\n");
        return -2;
    }

    strcpy(ifr.ifr_name, interface);
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("ethtool_sset Cannot get control socket");
        return -1;
    }

    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err < 0) {
        perror("Cannot get current device settings");
        close(fd);
        return -1;
    }

    if (g_str_equal(speed, "Auto")) {
        if (ecmd.autoneg != AUTONEG_ENABLE) {
            autoneg_wanted = AUTONEG_ENABLE;
            needupdate = 1;
        }
    } else if (g_str_equal(speed, "10baseT/Half")) {
        if (ecmd.speed != SPEED_10 || ecmd.duplex != DUPLEX_HALF || ecmd.autoneg != AUTONEG_DISABLE) {
            speed_wanted = SPEED_10;
            duplex_wanted = DUPLEX_HALF;
            autoneg_wanted = AUTONEG_DISABLE;
            needupdate = 1;
        }
    } else if (g_str_equal(speed, "10baseT/Full")) {
        if (ecmd.speed != SPEED_10 || ecmd.duplex != DUPLEX_FULL || ecmd.autoneg != AUTONEG_DISABLE) {
            speed_wanted = SPEED_10;
            duplex_wanted = DUPLEX_FULL;
            autoneg_wanted = AUTONEG_DISABLE;
            needupdate = 1;
        }
    } else if (g_str_equal(speed, "100baseT/Half")) {
        if (ecmd.speed != SPEED_100 || ecmd.duplex != DUPLEX_HALF || ecmd.autoneg != AUTONEG_DISABLE) {
            speed_wanted = SPEED_100;
            duplex_wanted = DUPLEX_HALF;
            autoneg_wanted = AUTONEG_DISABLE;
            needupdate = 1;
        }
    } else if (g_str_equal(speed, "100baseT/Full")) {
        if (ecmd.speed != SPEED_100 || ecmd.duplex != DUPLEX_FULL || ecmd.autoneg != AUTONEG_DISABLE) {
            speed_wanted = SPEED_100;
            duplex_wanted = DUPLEX_FULL;
            autoneg_wanted = AUTONEG_DISABLE;
            needupdate = 1;
        }
    } else if (g_str_equal(speed, "1000baseT/Half")) {
        if (ecmd.speed != SPEED_1000 || ecmd.duplex != DUPLEX_HALF || ecmd.autoneg != AUTONEG_DISABLE) {
            speed_wanted = SPEED_1000;
            duplex_wanted = DUPLEX_HALF;
            autoneg_wanted = AUTONEG_DISABLE;
            needupdate = 1;
        }
    } else if (g_str_equal(speed, "1000baseT/Full")) {
        if (ecmd.speed != SPEED_1000 || ecmd.duplex != DUPLEX_FULL || ecmd.autoneg != AUTONEG_DISABLE) {
            speed_wanted = SPEED_1000;
            duplex_wanted = DUPLEX_FULL;
            autoneg_wanted = AUTONEG_DISABLE;
            needupdate = 1;
        }
    }

    if (needupdate == 0)
        goto out;

    if (speed_wanted != -1)
        ecmd.speed = speed_wanted;
    if (duplex_wanted != -1)
        ecmd.duplex = duplex_wanted;
    if (autoneg_wanted != -1)
        ecmd.autoneg = autoneg_wanted;

    if ((autoneg_wanted == AUTONEG_ENABLE) && (advertising_wanted < 0)) {
        if (speed_wanted == SPEED_10 && duplex_wanted == DUPLEX_HALF)
            advertising_wanted = ADVERTISED_10baseT_Half;
        else if (speed_wanted == SPEED_10 &&
                 duplex_wanted == DUPLEX_FULL)
            advertising_wanted = ADVERTISED_10baseT_Full;
        else if (speed_wanted == SPEED_100 &&
                 duplex_wanted == DUPLEX_HALF)
            advertising_wanted = ADVERTISED_100baseT_Half;
        else if (speed_wanted == SPEED_100 &&
                 duplex_wanted == DUPLEX_FULL)
            advertising_wanted = ADVERTISED_100baseT_Full;
        else if (speed_wanted == SPEED_1000 &&
                 duplex_wanted == DUPLEX_HALF)
            advertising_wanted = ADVERTISED_1000baseT_Half;
        else if (speed_wanted == SPEED_1000 &&
                 duplex_wanted == DUPLEX_FULL)
            advertising_wanted = ADVERTISED_1000baseT_Full;
        else if (speed_wanted == SPEED_2500 &&
                 duplex_wanted == DUPLEX_FULL)
            advertising_wanted = ADVERTISED_2500baseX_Full;
        else if (speed_wanted == SPEED_10000 &&
                 duplex_wanted == DUPLEX_FULL)
            advertising_wanted = ADVERTISED_10000baseT_Full;
        else
            advertising_wanted = 0;
    }

    if (advertising_wanted != -1) {
        if (advertising_wanted == 0)
            ecmd.advertising = ecmd.supported &
                               (ADVERTISED_10baseT_Half |
                                ADVERTISED_10baseT_Full |
                                ADVERTISED_100baseT_Half |
                                ADVERTISED_100baseT_Full |
                                ADVERTISED_1000baseT_Half |
                                ADVERTISED_1000baseT_Full |
                                ADVERTISED_2500baseX_Full |
                                ADVERTISED_10000baseT_Full);
        else
            ecmd.advertising = advertising_wanted;
    }

    ecmd.cmd = ETHTOOL_SSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    err = ioctl(fd, SIOCETHTOOL, &ifr);

    if (err < 0)
        perror("Cannot set new settings");

out:
    close(fd);

    return 0;
}

char *get_local_mac(char *interface)
{
    char *mac = NULL;
    struct ifreq ifr;
    int sd;

    bzero(&ifr, sizeof(struct ifreq));
    if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_INFO("get %s mac address socket creat error\n", interface);
        return mac;
    }

    strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name) - 1);

    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0) {
        //LOG_INFO("get %s mac address error\n", interface);
        close(sd);
        return mac;
    }

    mac = g_strdup_printf("%02x:%02x:%02x:%02x:%02x:%02x",
                          (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                          (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                          (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                          (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                          (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                          (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    close(sd);

    return mac;
}

char *get_local_ip(char *interface)
{
    char *ip = NULL;
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd) {
        LOG_INFO("socket error: %s\n", strerror(errno));
        return ip;
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
        LOG_INFO("ioctl error: %s\n", strerror(errno));
        close(sd);
        return ip;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    ip = g_strdup_printf("%s", inet_ntoa(sin.sin_addr));

    close(sd);

    return ip;
}

char *get_local_netmask(char *interface)
{
    char *ip = NULL;
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd) {
        LOG_INFO("socket error: %s\n", strerror(errno));
        return ip;
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if (ioctl(sd, SIOCGIFNETMASK, &ifr) < 0) {
        LOG_INFO("ioctl error: %s\n", strerror(errno));
        close(sd);
        return ip;
    }

    memcpy(&sin, &ifr.ifr_netmask, sizeof(sin));
    ip = g_strdup_printf("%s", inet_ntoa(sin.sin_addr));

    close(sd);

    return ip;
}

char *get_gateway(char *interface)
{
    FILE *fp;
    char buf[512];
    char gateway[30];

    fp = popen("ip route", "r");
    if (NULL == fp) {
        perror("popen error");
        return NULL;
    }

    char *cmp = g_strdup_printf("dev %s scope link", interface);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, cmp) && !strstr(buf, "/")) {
            sscanf(buf, "%s", gateway);
            pclose(fp);
            g_free(cmp);

            return g_strdup(gateway);
        }
    }

    pclose(fp);
    g_free(cmp);

    return NULL;
}

int is_ipv4(char *ip)
{
    char* ptr;
    int count = 0;
    char *str = g_strdup(ip);
    const char *p = str;

    while (*p != '\0') {
        if (*p == '.')
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

    if (count == 4)
        return 0;
err:
    g_free(str);

    return -1;
}

int get_dns(char **dns1, char **dns2)
{
    FILE *fp;
    char buf[512];
    char dns[30];
    int i = 1;

    fp = popen("cat /etc/resolv.conf | grep \"nameserver\"", "r");
    if (NULL == fp) {
        perror("popen error");
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        memset(dns, 0, sizeof(dns));
        sscanf(buf, "%*s%s", dns);

        if (is_ipv4(dns) != 0)
            continue;

        if (i == 1)
            *dns1 = g_strdup(dns);
        else if (i == 2) {
            *dns2 = g_strdup(dns);
            break;
        }
        i++;

    }

    pclose(fp);

    return 0;
}

int net_detect(char* net_name)
{
    int ret = -1;
    int skfd = 0;
    struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0) {
        LOG_INFO("%s:%d Open socket error!\n", __FILE__, __LINE__);
        return ret;
    }

    strcpy(ifr.ifr_name, net_name);

    if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 ) {
        LOG_INFO("Maybe inferface %s is not valid!\n", ifr.ifr_name);
        close(skfd);
        return ret;
    }

    if(ifr.ifr_flags & IFF_UP)
        ret = 1;
    else
        ret = 0;
    close(skfd);

    return ret;
}

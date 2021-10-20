#ifndef __NETWORK_FUNC_H__
#define __NETWORK_FUNC_H__

#include <linux/types.h>

/* This should work for both 32 and 64 bit userland. */

struct ethtool_cmd {
	__u32	cmd;
	__u32	supported;
	__u32	advertising;
	__u16	speed;
	__u8	duplex;
	__u8	port;
	__u8	phy_address;
	__u8	transceiver;
	__u8	autoneg;
	__u8	mdio_support;
	__u32	maxtxpkt;
	__u32	maxrxpkt;
	__u16	speed_hi;
	__u8	eth_tp_mdix;
	__u8	eth_tp_mdix_ctrl;
	__u32	lp_advertising;
	__u32	reserved[2];
};

int get_ethernet_tool(char *interface, struct ethtool_cmd *ep);
char *get_local_mac(char *interface);
char *get_local_ip(char *interface);
char *get_local_netmask(char *interface);
char *get_gateway(char *interface);
int get_dns(char **dns1, char **dns2);
int is_ipv4(char *ip);
void get_ethernet_speed(char *speed, struct ethtool_cmd *ep);
void get_ethernet_speedsupport(char *speedsupport, struct ethtool_cmd *ep);
int get_ethernet_tool_speed_set(char *interface, char *speed);
int net_detect(char* net_name);

#endif
/*
 * Copyright (c) 2017 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef NETLINKWRAPPER_H_
#define NETLINKWRAPPER_H_

#include <mutex>
#include <netinet/in.h>
#include "DeviceIo/WifiManager.h"

namespace DeviceIOFramework {

struct IcmpEchoReply {
	int icmpSeq;
	int icmpLen;
	int ipTtl;
	double rtt;
	std::string fromAddr;
	bool isReply;
};

struct PingResult {
	int dataLen;
	int nsend;
	int nreceived;
	char ip[32];
	std::string error;
	std::vector<IcmpEchoReply> icmpEchoReplys;
};

//config status
enum notify_network_status_type {
	ENetworkNone = 11,
	ENetworkConfigExited,
	ENetworkConfigStarted,
	ENetworkDeviceConnected,
	ENetworkConfigIng,
	ENetworkConfigRouteFailed,
	ENetworkLinkSucceed,
	ENetworkLinkFailed,
	ENetworkRecoveryStart,
	ENetworkRecoverySucceed = 20,
	ENetworkRecoveryFailed,
	ENetworkWifiSucceed,
	ENetworkWifiFailed,
	ENetworkWifiWrongKeyFailed
};
//network status
enum InternetConnectivity {
	UNAVAILABLE = 0,
	AVAILABLE,
	UNKNOW
};
//operation type
enum operation_type {
	EOperationStart = 10,
	EAutoEnd,
	EAutoConfig,
	EManualConfig
};

#define NETLINK_WPA_CONFIG_FILE "/data/cfg/wpa_supplicant.conf"
#define NETLINK_SSID_PREFIX_ROCKCHIP      "Rockchip-Echo-"

#define NETLINK_AUTO_CONFIG_TIMEOUT 20*60//10*60
#define NETLINK_NETWORK_CONFIGURE_PING_COUNT 18

#define PACKET_SIZE         4096
#define MAX_WAIT_TIME       1;//5
#define MAX_PACKETS_COUNT   4
#define MAX_PING_INTERVAL   300
#define PING_DEST_HOST1       "114.114.114.114"
#define PING_DEST_HOST2       "8.8.8.8"

class NetLinkWrapper {
public:
	static NetLinkWrapper *getInstance();

	void setCallback(INetLinkWrapperCallback *callback);

	void release();

	void notify_network_config_status(notify_network_status_type notify_type);

	void network_status_changed(InternetConnectivity current_status, bool wakeupTrigger);

	NetLinkNetworkStatus getNetworkStatus() const;

	void setNetworkStatus(NetLinkNetworkStatus networkStatus);

	void wakeupNetLinkNetworkStatus();

	void startNetworkRecovery();

	void stopNetworkRecovery();

	bool startNetworkConfig(int timeout);

	void stopNetworkConfig();

	bool isNetworkOnline() const;

	void setNetworkOnline(bool isNetworkOnline);

	void networkLinkOrRecoverySuccess();

	bool isFirstNetworkReady() const;

	void setFirstNetworkReady(bool isFirstNetworkReady);

	bool isFromConfigNetwork() const;

	void setFromConfigNetwork(bool isFromConfigNetwork);

	static void logFunction(const char* msg, ...);

	void OnNetworkReady();

	inline operation_type get_operation_type() {return m_operation_type;};
	inline void set_operation_type(operation_type type) {m_operation_type = type;};

	void initBTForHis();

private:
	NetLinkWrapper();

	virtual ~NetLinkWrapper();

	static void init();

	static void destroy();

	static void sigalrm_fn(int sig);

	void init_network_config_timeout_alarm();
	void start_network_config_timeout_alarm(int timeout);
	void stop_network_config_timeout_alarm();

	bool start_network_config();
	void stop_network_config();
	void start_network_monitor();
	static void *monitor_work_routine(void *arg);
	bool check_recovery_network_status();

	unsigned short getChksum(unsigned short *addr,int len);
	int packIcmp(int pack_no, struct icmp* icmp);
	bool unpackIcmp(char *buf,int len, struct IcmpEchoReply *icmpEchoReply);
	struct timeval timevalSub(struct timeval timeval1,struct timeval timval2);
	bool getsockaddr(const char * hostOrIp, sockaddr_in* sockaddr);
	bool sendPacket();
	bool recvPacket(PingResult &pingResult);
	bool ping(std::string host, int count, PingResult& pingResult);
	bool ping_network(bool wakeupTrigger);

	INetLinkWrapperCallback *m_callback;

	static NetLinkWrapper *s_netLinkWrapper;

	static pthread_once_t s_initOnce;

	static pthread_once_t s_destroyOnce;

	NetLinkNetworkStatus m_networkStatus;

	static void networkLinkSuccess();

	static void networkLinkFailed();

	/// Mutex used to serialize access to @_network_status.
	std::mutex m_mutex;

	/// is loop network config
	bool m_isLoopNetworkConfig;

	/// is network online
	bool m_isNetworkOnline;

	/// network is first ready
	bool m_isFirstNetworkReady;

	/// network is from config
	bool m_isFromConfigNetwork;

	operation_type m_operation_type;
	int m_stop_network_recovery;

	char m_sendpacket[PACKET_SIZE];
	char m_recvpacket[PACKET_SIZE];
	int m_maxPacketSize;
	int m_sockfd;
	int m_datalen;
	int m_nsend;
	int m_nreceived;
	int m_icmp_seq;
	struct sockaddr_in m_dest_addr;
	struct sockaddr_in m_from_addr;
	pid_t m_pid;
	pthread_mutex_t m_ping_lock;
	bool wifi_link_state;
	bool net_link_state;

};

}  // namespace DeviceIo

#endif  // NETLINKWRAPPER_H_

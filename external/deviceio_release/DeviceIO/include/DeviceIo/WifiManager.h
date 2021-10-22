#ifndef DEVICEIO_FRAMEWORK_WIFIMANAGER_H_
#define DEVICEIO_FRAMEWORK_WIFIMANAGER_H_

#include <list>
#include <string>

#include <string.h>

#include "Properties.h"
#include "ScanResult.h"
#include "WifiInfo.h"

namespace DeviceIOFramework {

class WifiManager {

public:
	enum Encryp {WPA, WEP, NONE};
	static WifiManager* getInstance();
	int init(Properties* properties);
	bool isWifiConnected();
	bool isWifiEnabled();
	int setWifiEnabled(const bool enable);
	int enableWifiAp(const std::string& ssid, const std::string& psk = "", const std::string& ip = "10.201.126.1");
	int disableWifiAp();
	int startScan();
	std::list<ScanResult*> getScanResults();
	int connect(const std::string& ssid, const std::string& psk = "", const Encryp encryp = WPA, const int hide = 0);
	int disconnect();
	WifiInfo* getConnectionInfo();
	virtual ~WifiManager(){};

private:
	WifiManager(){};
	WifiManager(const WifiManager&){};
	WifiManager& operator=(const WifiManager&){
		return *this;
	};

	int addNetwork();
	int setNetwork(const int id, const std::string& ssid, const std::string& psk = "", const Encryp encryp = WPA);
	int selectNetwork(const int id);
	int enableNetwork(const int id);
	int udhcpc();
	int saveConfiguration();
private:
	static WifiManager* m_instance;
	Properties* m_properties;
};

} // end of namespace DeviceIOFramework

#endif // end of DEVICEIO_FRAMEWORK_WIFIMANAGER_H_

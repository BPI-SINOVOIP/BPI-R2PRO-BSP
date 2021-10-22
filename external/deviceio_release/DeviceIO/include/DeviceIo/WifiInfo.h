#ifndef DEVICEIO_FRAMEWORK_WIFIINFO_H_
#define DEVICEIO_FRAMEWORK_WIFIINFO_H_

#include <string>

namespace DeviceIOFramework {

class WifiInfo {

public:
	WifiInfo();
	void setNetworkId(const int networkId);
	int getNetworkId();
	void setBssid(const std::string& bssid);
	std::string getBssid();
	void setSsid(const std::string& ssid);
	std::string getSsid();
	void setFrequency(const int frequency);
	int getFrequency();
	void setMode(const std::string& mode);
	std::string getMode();
	void setWpaState(const std::string& wpaState);
	std::string getWpaState();
	void setIpAddress(const std::string& ipAddress);
	std::string getIpAddress();
	void setMacAddress(const std::string& macAddress);
	std::string getMacAddress();
	std::string toString();
	~WifiInfo();

private:
	int networkId;
	std::string bssid;
	std::string ssid;
	int frequency;
	std::string mode;
	std::string wpaState;
	std::string ipAddress;
	std::string macAddress;
};

} // end of namespace DeviceIOFramework

#endif // end of DEVICEIO_FRAMEWORK_WIFIINFO_H_

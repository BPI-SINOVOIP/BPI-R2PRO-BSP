#ifndef DEVICEIO_FRAMEWORK_SCANRESULT_H_
#define DEVICEIO_FRAMEWORK_SCANRESULT_H_

#include <string>

namespace DeviceIOFramework {

class ScanResult {

public:
	ScanResult();
	ScanResult(const std::string& bssid, int frequency, int level, const std::string& flags, const std::string& ssid);
	void setBssid(const std::string& bssid);
	std::string getBssid();
	void setSsid(const std::string& ssid);
	std::string getSsid();
	void setFlags(const std::string& flags);
	std::string getFlags();
	void setLevel(const int level);
	int getLevel();
	void setFrequency(const int frequency);
	int getFrequency();
	std::string toString();
	~ScanResult();

private:
	std::string bssid;
	std::string ssid;
	std::string flags;
	int level;
	int frequency;
};

} // end of namespace DeviceIOFramework

#endif // end of DEVICEIO_FRAMEWORK_SCANRESULT_H_

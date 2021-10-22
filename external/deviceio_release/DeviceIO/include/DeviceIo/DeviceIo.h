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

#ifndef DEVICEIO_FRAMEWORK_DEVICEIO_H_
#define DEVICEIO_FRAMEWORK_DEVICEIO_H_

#include <pthread.h>
#include <stdint.h>
#include <string>

#include <DeviceIo/RkBtBase.h>

namespace DeviceIOFramework {

#define PLAYBACK_DEVICE_NUM 1

/* led control cmd */
enum class LedState {
    LED_NET_RECOVERY = 0,
    LED_NET_WAIT_CONNECT,
    LED_NET_DO_CONNECT,
    LED_NET_CONNECT_FAILED,
    LED_NET_CONNECT_SUCCESS,
    LED_NET_WAIT_LOGIN,
    LED_NET_DO_LOGIN,
    LED_NET_LOGIN_FAILED,
    LED_NET_LOGIN_SUCCESS,

    LED_WAKE_UP_DOA,    // param -180 ~ 180
    LED_WAKE_UP,        // no param
    LED_SPEECH_PARSE,
    LED_PLAY_TTS,
    LED_PLAY_RESOURCE,

    LED_BT_WAIT_PAIR,
    LED_BT_DO_PAIR,
    LED_BT_PAIR_FAILED,
    LED_BT_PAIR_SUCCESS,
    LED_BT_PLAY,
    LED_BT_CLOSE,

    LED_VOLUME,
    LED_MICMUTE,

    LED_DISABLE_MIC,

    LED_ALARM,

    LED_SLEEP_MODE,

    LED_OTA_DOING,
    LED_OTA_SUCCESS,

    LED_CLOSE_A_LAYER,
    LED_ALL_OFF,    // no param

    //special cmd, direct set led brightness
    LED_PWMR_SET,  //param 0 - 255
    LED_PWMG_SET,  //param 0 - 255
    LED_PWMB_SET   //param 0 - 255
};

/* bt control cmd */
enum class BtControl {
    BT_OPEN,
    BT_CLOSE,

    BT_SOURCE_OPEN,
    BT_SOURCE_SCAN,
    BT_SOURCE_CONNECT,
    BT_SOURCE_DISCONNECT, //5
    BT_SOURCE_STATUS,
    BT_SOURCE_REMOVE,
    BT_SOURCE_CLOSE,
    BT_SOURCE_IS_OPENED,

    BT_SINK_OPEN, //10
    BT_SINK_CLOSE,
    BT_SINK_RECONNECT,
    BT_SINK_IS_OPENED,

    BT_IS_CONNECTED,
    BT_UNPAIR,

    BT_PLAY,
    BT_PAUSE_PLAY,
    BT_RESUME_PLAY,
    BT_VOLUME_UP,
    BT_VOLUME_DOWN, //20
    BT_AVRCP_FWD,
    BT_AVRCP_BWD,
    BT_AVRCP_STOP,
    BT_HFP_RECORD,

    BT_BLE_OPEN, //25
    BT_BLE_COLSE,
    BT_BLE_IS_OPENED,
    BT_BLE_WRITE,
    BT_BLE_READ,
    BT_VISIBILITY,
    BT_BLE_DISCONNECT,

    BT_HFP_OPEN,
    BT_HFP_SINK_OPEN,

    GET_BT_MAC,
};

/* wifi control cmd */
enum class WifiControl {
    WIFI_OPEN,
    WIFI_CLOSE,
    WIFI_CONNECT,
    WIFI_DISCONNECT,
    WIFI_IS_OPENED,
    WIFI_IS_CONNECTED,
    WIFI_SCAN = 6,
    WIFI_IS_FIRST_CONFIG,
    WIFI_OPEN_AP_MODE,
    WIFI_CLOSE_AP_MODE,
    GET_WIFI_MAC,
    GET_WIFI_IP,
    GET_WIFI_SSID = 12,
    GET_WIFI_BSSID,
    GET_LOCAL_NAME,
    WIFI_RECOVERY,
	WIFI_GET_DEVICE_CONTEXT
};

enum NetLinkNetworkStatus {
    NETLINK_NETWORK_CONFIG_STARTED,
    NETLINK_NETWORK_CONFIGING,
    NETLINK_NETWORK_CONFIG_SUCCEEDED,
    NETLINK_NETWORK_CONFIG_FAILED,
    NETLINK_NETWORK_SUCCEEDED,
    NETLINK_NETWORK_FAILED,
    NETLINK_NETWORK_RECOVERY_START,
    NETLINK_NETWORK_RECOVERY_SUCCEEDED,
    NETLINK_NETWORK_RECOVERY_FAILED,
    NETLINK_WAIT_LOGIN,
    NETLINK_NETWORK_CONFIG_WRONG_KEY_FAILED,
};

/* input event */
enum class DeviceInput {
    KEY_ONE_SHORT,
    KEY_ONE_LONG,
    KEY_ONE_DOUBLE,
    KEY_ONE_LONG_10S,
    KEY_BLUETOOTH_SHORT,
    KEY_BLUETOOTH_LONG, //5
    KEY_MIC_MUTE,
    SLIDER_PRESSED,
    SLIDER_RELEASE,
    VOLUME_CHANGED,

    KEY_VOLUME_UP, //10
    KEY_VOLUME_DOWN,
    KEY_VOLUME_MUTE,
    KEY_WAKE_UP,
    KEY_ENTER_AP,
    KEY_EXIT_AP, //15

    //蓝牙等待配对
    BT_WAIT_PAIR,
    //蓝牙配对中
    BT_DO_PAIR,
    //蓝牙已被绑定,请解绑后再绑定其他设备
    BT_PAIR_FAILED_PAIRED,
    //配对失败,请重试
    BT_PAIR_FAILED_OTHER,
    //蓝牙配对成功
    BT_PAIR_SUCCESS, //20
    BT_CONNECT,
    //手机关闭蓝牙蓝牙断开
    BT_DISCONNECT,
    //蓝牙开始播放
    BT_START_PLAY,
    BT_PAUSE_PLAY,
    //蓝牙结束播放
    BT_STOP_PLAY, //25
    //ble
    BLE_CLIENT_CONNECT,
    BLE_CLIENT_DISCONNECT,
    BLE_SERVER_RECV,
    //BT CALL
    BT_HFP_AUDIO_CONNECT,
    BT_HFP_AUDIO_DISCONNECT, //30
    BT_POWER_OFF,

    DLNA_STOPPED,
    DLNA_PLAYING,
    DLNA_PAUSED,

    KEY_CLOSE_TIMER_ALARM, //35
    KEY_IS_SLEEP_STATE,
    KEY_PLAY_PAUSE,

    KEY_SHUT_DOWN,
    KEY_SLEEP_MODE,
    KEY_RK816_POWER,
    KEY_HEADPHONE_INSERT,
    KEY_FACTORY_RESET,

    KEY_RAW_INPUT_EVENT,

    GST_PLAYER_PLAYING, // The stream is in PLAYING state.
    GST_PLAYER_PAUSED, // The stream is in PAUSED state.
    GST_PLAYER_READY, // The stream is in READ state.
    GST_PLAYER_SEEKABLE, // The stream can perform a seek operation.
    GST_PLAYER_EOS, // Reached end of the stream.
    GST_PLAYER_ERROR, // Error occurs.
    GST_PLAYER_DURATION, //The stream duration changed.
	//BT EVENT
	BT_SINK_ENV_CONNECT,
	BT_SINK_ENV_DISCONNECT,
	BT_SINK_ENV_CONNECT_FAIL,
	BT_SRC_ENV_CONNECT,
	BT_SRC_ENV_DISCONNECT,
	BT_SRC_ENV_CONNECT_FAIL,
	BT_BLE_ENV_CONNECT,
	BT_BLE_ENV_DISCONNECT,
	BT_BLE_ENV_CONNECT_FAIL,

    //BT spp event
    SPP_CLIENT_CONNECT,
    SPP_CLIENT_DISCONNECT,

	//WIFI
	WIFI_ENV_CONNECT,
	WIFI_ENV_DISCONNECT,
	WIFI_ENV_CONNECT_FAIL,
};

enum class DevicePowerSupply {
	NULL_DEVICEPOWERSUPPLY = 0,

	USB_CURRENT_MAX = 1,
	USB_ONLINE,
	USB_VOLTAGE_MAX,
	USB_CHARGE_ENABLE_STATUS,
	USB_TYPE,

	AC_CURRENT_MAX = 50,
	AC_ONLINE,
	AC_VOLTAGE_MAX,
	AC_CHARGE_ENABLE_STATUS,
	AC_TYPE,

	BATTERY_CAPACITY = 100,
	BATTERY_CHARGE_COUNTER,
	BATTERY_CURRENT_NOW,
	BATTERY_VOLTAGE_NOW,
	BATTERY_PRESENT,
	BATTERY_TEMP,
	BATTERY_HEALTH,
	BATTERY_STATUS,
	BATTERY_TYPE,

	POWER_CFG_CAPACITY_DETECT_PERIOD = 200,
	POWER_CFG_LOW_POWER_THRESHOLD,
	POWER_CFG_BAT_TEMP_PERIOD_DETECT,
	POWER_CFG_BAT_TEMP_THRESHOLD_MIN,
	POWER_CFG_BAT_TEMP_THRESHOLD_MAX,
	POWER_CFG_BAT_CHARGE_ENABLE,
	POWER_CFG_BAT_CHARGE_DISABLE,
	POWER_CFG_GET_CHARGE_ENABLE_STATUS,
	POWER_CFG_NULL
};

enum class DeviceRTC {
	DEVICE_RTC_READ_TIME = 0,
	DEVICE_RTC_SET_TIME,
	DEVICE_RTC_READ_ALARM,
	DEVICE_RTC_ENABLE_ALARM_INTERRUPT,
	DEVICE_RTC_DISABLE_ALARM_INTERRUPT,
	DEVICE_RTC_READ_IRQ_RATE,
	DEVICE_RTC_SET_IRQ_PERIODIC,
	DEVICE_RTC_ENABLE_PERIODIC_INTERRUPT,
	DEVICE_RTC_DISABLE_PERIODIC_INTERRUPT,
	DEVICE_RTC_WAIT_ALARM_RING,
	DEVICE_RTC_NULL_COMMAND
};

enum class BT_Device_Class {
	BT_SINK_DEVICE,
	BT_SOURCE_DEVICE,
	BT_BLE_DEVICE,
	BT_IDLE,
};

#ifdef __cplusplus
extern "C" {
#endif
struct wifi_config {
	char ssid[512];
	int ssid_len;
	char psk[512];
	int psk_len;
	char key_mgmt[512];
	int key_len;
	bool hide;
	void (*wifi_status_callback)(int status, int reason);
};

#ifdef __cplusplus
}
#endif

//void bt_adv_set(RkBtContent *p_bt_content);

class INetLinkWrapperCallback {
public:
    virtual void networkReady() = 0;
    virtual void netlinkNetworkOnlineStatus(bool status) = 0;
    virtual void netlinkNetworkStatusChanged(NetLinkNetworkStatus networkStatus) = 0;
};

/* device input event notify */
class DeviceInNotify : public INetLinkWrapperCallback{
public:
    virtual void callback(DeviceInput event, void *data, int len) = 0;

    virtual ~DeviceInNotify() = default;
};

class DeviceIo {
public:
    /**
     * @brief get instance
     *
     * @return the instance pointer
     */
    static DeviceIo* getInstance();

    /**
     * @brief release instance
     *
     */
    void releaseInstance();

    /**
     * @brief set the notify
     *
     * @param notify called when some event happened
     *
     */
    void setNotify(DeviceInNotify* notify);

    /**
     * @brief set the notify
     *
     * @param notify called when some event happened
     *
     */
    DeviceInNotify* getNotify();

    /**
     * @brief control the led
     *
     * @param cmd see enum LedState
     * @param data data
     * @param len data length
     *
     * @return 0 on success
     */
    int controlLed(LedState cmd, void *data = NULL, int len = 0);

    /**
     * @brief control the rtc
     *
     * @param cmd  see enum DeviceRTC
     * @param data data
     * @param len  data length
     * @return 0 on success
     */
    int controlRtc(DeviceRTC cmd, void *data = NULL, int len = 0);

    /**
     * @brief control the power
     *
     * @param cmd  see enum DevicePowerSupply
     * @param data data
     * @param len  data length
     * @return 0 on success
     */
    int controlPower(DevicePowerSupply cmd, void *data = NULL, int len = 0);

    /**
     * @brief control the bt
     *
     * @param cmd  see enum BtControl
     * @param data data
     * @param len  data length
     * @return 0 on success
     */
    int controlBt(BtControl cmd, void *data = NULL, int len = 0);

    /**
     * @brief control the wifi
     *
     * @param cmd  see enum WifiControl
     * @param data data
     * @param len  data length
     * @return 0 on success
     */
    int controlWifi(WifiControl cmd, void *data = NULL, int len = 0);
    /**
    * @brief transimit infrared code
    *
    * @param infraredCode the infrared code
    * @return 0 on success
    */
    int transmitInfrared(std::string& infraredCode);

    /**
    * @brief open microphone
    *
    * @return 0 on success
    */
    int openMicrophone();

    /**
     * @brief close microphone
     *
     * @return 0 on success
     */
    int closeMicrophone();

    /**
     * @brief get microphone status
     *
     * @return true on opened
     */
    bool isMicrophoneOpened();

    /**
     * @brief get headphone status
     *
     * @return true on opened
     */
    bool isHeadPhoneInserted();

    /**
     * @brief get volume
     *
     * @return 0 ~ 100 volume
     */
    int getVolume(int trackId = 0);

    /**
     * @brief set volume
     *
     * @param vol volume 0 ~ 100
     */
    void setVolume(int vol, int trackId = -1);

    /**
     * @brief get/set sn
     *
     * @return void
     */

    bool getSn(char* sn);

    bool setSn(char* sn);
    bool getPCB(char* sn);
    bool setPCB(char *sn);

    /**
     * @brief get cpu id
     *
     * @return string
     */
    std::string getChipID();

    /**
     * @brief get version
     *
     * @return version string
     */
    std::string getVersion();

    /**
     * @brief setHostName for wifi and bt, must set hostname before wifi/bt start
     *
     * @return
     */
    int setHostName(const char *name, size_t len);
    /**
     * @brief getHostName
     *
     * @return
     */
    int getHostName(char *name, size_t len);

    /**
     * @brief set mute state
     *
     * @return 0 on success
     */
    int setMute(bool mute);

    /**
     * @brief get mute status
     *
     * @return true on mute
     */
    bool isMute();

    /**
     * @brief get mic array angle
     *
     * @return the angle
     */
    int getAngle();

    bool inOtaMode();

    void rmOtaFile();

    /**
     * @brief start network config
     * @timeout, networkconfig timeout in seconds
     *
     * @return true if started succeed
     */
    bool startNetworkConfig(int timeout_s);

    bool stopNetworkConfig();

    /**
     * @brief start network config
     *
     * @return true if started succeed
     */
    bool startNetworkRecovery();
    bool stopNetworkRecovery();
    void initBTForHis();

    NetLinkNetworkStatus getNetworkStatus() const;

    /**
     * @brief system ctl
     */
    void poweroff();
    /**
     * @brief reboot to recovery and reset userdata part
     */
    void factoryReset();
    /**
     * @brief reboot to recovery and start ota update
     *
     * @return true if started succeed
     */
    bool OTAUpdate(std::string path);
    /**
     * @brief suspend
     */
    void suspend();

    /**
     * @brief setEQParameter
     * @EQBinDir the directory of eq parameter files
     *
     * @return true if succeed
     */
    bool setEQParameter(std::string EQBinDir);

private:
    static DeviceIo* m_instance;
    static DeviceInNotify* m_notify;

    static pthread_once_t m_initOnce;
    static pthread_once_t m_destroyOnce;

    DeviceIo();
    ~DeviceIo();
    static void init();
    static void destroy();
};

} // namespace framework

#endif // DEVICEIO_FRAMEWORK_DEVICEIO_H_

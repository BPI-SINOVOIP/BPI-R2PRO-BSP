#ifndef __RK_SYSTEM_H__
#define __RK_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 *Version 1.3.6 Release 2020/12/02
  1.add rk_bt_get_dev_platform api
  2.bluez: don't judge COD when ble connect
  3.bsa: add pbap client api
  4.bsa: rk_ble_client_open add mtu_change parameter
  5.bsa: fix write failed when the chrc uuid only has WRITE_WITHOUT_RESP prop
  6.bsa: fix set device name failed
  7.bsa: hfp: call number report
  8.bsa: hfp: outgoing call event report
  9.bsa: hfp: outgoing call ring event report
  10.bsa: hfp: current active call list info report
  11.broadcom bsa: get ble device name from FullName/ShortName
  12.broadcom bsa: add PAN profile api
 *Version 1.3.5 Release 2020/07/02
  1.bluez: modify avrcp event report to use the standard bluetoothd process
  2.bluez: fix the class of device judgment
  3.bluez: scan off when source close
  4.bluez: fix source reconnect, only reconnect sink device
  5.bluez: fix ble continuous broadcast after rk_ble_stop
  6.bluez: fix phone->setting->connect ble server causes bt abnormal
  7.bluez: fix loop call rk_ble_disconnect coredump
  8.bluez: modify ble server/client connect and disconnect handle
  9.bluez: remove rk_ble_client_register_dev_found_callback
  10.bluez: ble client state callback add remote device address and name
  11.bluez: remove unpaired or unconnected historical scan device when scan on
  12.bluez: add obex state callback
  13.bluez: add rk_ble_set_address api
  14.bluez: ble chr uuid add write-without-response props
  15.bluez: fixed ble connection bug caused by two iPhones loop connections to ble
  16.bsa: add rk_bt_source_disconnect api
  17.bsa: support for setting up scan filtering(LE BR/EDR or both)
  18.bsa: source: add the first scan auto reconnect
  19.bsa: fix eq and source communication exception after source disconnect
  20.bsa: support ble client
  21.bsa: add rk_bt_read_remote_device_name api
  22.bsa: add rk_ble_register_request_data_callback api
  23.bsa: add rk_bt_get_scaned_devices and rk_bt_free_scaned_devices api
  24. add rk_ble_set_adv_interval api
  25. support ble MTU report
  26. hfp event callback add remote device address parameter
  27. add rk_bt_hfp_dial_number api
  28. rk_ble_client_write add data_len parameter
 *Version 1.3.4 Release 2020/03/17
  1.bsa: add rk_bt_set_bsa_server_path api
  2.bluez: add rk_bt_get_connected_properties api
  3.bluez: change bt source and and eq communication
  4.bluez: ble event callback takes the remotr device name and address
  5.bluez: fixed gatt init memory overbounds
  6.bluez: add remote device name change callback
  7.bluez: change ble connect event report
 *Version 1.3.3 Release 2020/02/27
  1.bluez: support for setting up scan filtering(LE BR/EDR or both)
  2.bluez: optimize the init/deinit execution time
  3.bluez: add rk_bt_get_scaned_devices/rk_bt_free_scaned_devices api
  4.bluez: source: add the first scan automatically reconnects
  5.bluez: source: fix connection failed during scan
  6.bluez: fix init crash in qt new thread(due to mainloop thread sync)
  7.rk_bt_source_connect rename rk_bt_source_connect_by_addr
  8.rk_bt_source_disconnect rename rk_bt_source_disconnect_by_addr
  9.bluez: change rk_bt_source_disconnect api to disconnect current connection
 *Version 1.3.2 Release 2019/12/12
  bluez: support ble client
 *Version 1.3.1 Release 2019/11/16
  bt source callback add device address and device name
 *Version 1.3.0 Release 2019/10/30
  1.bluez: add rk_bt_deinit api
  2.bluez: fix get lcoal device name and bt mac address api
  3.blueZ: support pbap profile
  4.bluez: support hfp 16K sample rate
  5.bluez: support sink underrun msg report
  6.bsa: support set bt mac address in rk_bt_init
  7.bsa: add rk_bt_sink_set_alsa_device api
  8.bsa: add rk_bt_ble_set_visibility, rk_ble_set_local_privacy api
  9.add bt state report
  10.add bt pair state report
  11.add rk_bt_start_discovery, rk_bt_cancel_discovery, rk_bt_is_discovering, rk_bt_display_devices api
  12.add rk_bt_pair_by_addr, rk_bt_unpair_by_addr api
  13.add rk_bt_get_paired_devices, rk_bt_free_paired_devices, rk_bt_display_paired_devices api
  14.add rk_bt_set_device_name api
  15. add song track info report
  16. add song playing progress change report
  17. add avdtp(a2dp sink) state report
  18. add rk_bt_sink_connect_by_addr, rk_bt_sink_disconnect_by_addr api
  19. add rk_bt_sink_get_play_status, rk_bt_sink_get_poschange api
  20. support log to syslog
 *Version 1.2.4 Release 2019/06/24
  1.add alsa control demo
  2.add rk_bt_hfp_disconnect api
  3.fix PICKUP and HANGUP events cannot be received when answering or rejecting calls on the phone
  4.bsa: add hfp enable cvsd(8k samplerate) api
  5.bsa: fix the cypress bsa pairing popup prompt
  6.bsa: update broadcom bsa version to rockchip_20190617
  7.bsa: fix can't identify individual COD while bluetooth scanning
  8.bsa: fix battery power report bug
 *Version 1.2.3 Release 2019/06/05
  1.bluez:add a2dp sink volume api
  2.bluez:fix rk_bt_sink_set_visibility api
  3.bluez:fix rk_bt_enable_reconnect api
  4.bluez:cancel spp's dependence on a2dp sink
  5.bluez:fix automatic reconnection coredump
  6.bsa: fix a2dp source can't auto-connect sink devices
  7.bsa：fix rk_bt_hfp_hangup api
 *Version 1.2.2 Release 2019/05/27
  1.add bt source avrcp function
  2.add hfp hf api
  3.add set bt device class api
  4.add auto reconnect for sink & hfp
  5.add softap network config
  6.add airkiss network config
 *Version 1.2.1 Release 2019/04/29
  1.deviceio_test blewifi: register ble data recv callback(BSA)
  2.fix bug with program stuck when starting bluetooth related service(BLUEZ)
  3.fixed a bug that failed to get a bluetooth device role(BLUEZ)
 *Version 1.2.0 Release 2019/04/15
  1. bluethood api: add support for bsa.
  2. add support for deviceio_test.
 *Version 1.1.0 Release 2019/03/27
  1.rewrite the bt api include ble/a2dp/spp
 *Version 1.0.4 Release 2019/01/29
  1.ble_wifi
    1. add config timeout to discon ble
    2. fix gatt_open flag
    3. add reconfigure when last config fail
 *Version 1.0.3 Release 2019/01/24
  1.ble_wifi
    1. fix netstatus print error when NETLINK_NETWORK_CONFIG_WRONG_KEY_FAILED
    2. notify NETLINK_NETWORK_SUCCEEDED when net recovery from config failed
 *Version 1.0.2 Release 2019/01/24
  1.ble_wifi
    1. support wep key
    2. optimize wpa_supplicant.conf
    3. fix many times reports sink connected
    4. add ble_config wifi timeout value
    5. fix chinese code for hisense
  2.Rk_key: add fix register long press callback with different keyevent

 *Version 1.0.1 Release 2019/01/07
  1.ble_wifi:	fix Chinese coding problem
		fix ble report event and add wifi priority
		add wrong key event callback
		add initBTForHis interface
  2.volume:	setVolume support zero
  3.propery:	implement RK_property
  4.player:	separate mediaplayer and deviceio
		add playlist function
 *Version 1.0.0 Release 2018/12/22
 */

#define DEVICEIO_VERSION "V1.3.6"

int RK_read_chip_id(char *buffer, const int size);
int RK_read_version(char *buffer, const int size);
int RK_system_factory_reset(const int reboot = 1);


#ifdef __cplusplus
}
#endif

#endif

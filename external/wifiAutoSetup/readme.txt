* About this demo
It demos how to start/stop Broadcom easy setup protocols, and 3rd-party ones such as airkiss.


* Usage on Linux
Usage: (type setup -h)
# setup -h
-h: show help message
-d: show debug message
-k <v>: set 16-char key for all protocols
-p <v>: bitmask of protocols to enable
  0x0001 - mcast
  0x0002 - neeze
  0x0004 - akiss
  0x0010 - changhong
  0x0020 - changhong
  0x0040 - jd JoyLink
  0x0080 - softap

To start mcast+neeze (also works for qqconnect):
# setup -p 3
or just:
# setup
(default protocol combination will be mcast+neeze)

A sample output:
# setup -p 1
state: 0 --> 1
state: 1 --> 2
state: 2 --> 3
state: 3 --> 5
ssid: TEST-AP
password: 12345678
sender ip: 192.168.43.149
sender port: 0
security: wpa2


* Usage on RTOS
Copy all files of jni/* to apps/snip/setup/, then you can build it with:
$ make snip.setup-xxxxx

When application is started, you will get following message on console after
successfully configured:

WWD SDIO interface initialised
WLAN MAC Address : 04:E6:76:61:95:F0
WLAN Firmware    : wl0: May 12 2015 09:26:33 version 5.90.195.r2 (TOB) FWID 01-5de4b5f9 es3.c2.n1.ch1
Enabled, waiting ...
state [1]
state [2]
state [2]
state [2]
state [2]
state [2]
state [2]
state [2]
state [3]
state [3]

Event Found => Success
SSID        : TEST-AP
PASSWORD    : password1234
BSSID       : b8:62:1f:7c:63:22
AP Channel  : 6
AP Security : WPA2-PSK AES
Storing received credentials in DCT
easy setup done.
Joining : TEST-AP
Successfully joined : TEST-AP
Obtaining IPv4 address via DHCP
IPv4 network ready IP: 10.148.8.106
Network up success


* API
You can enable various protocols with following APIs before calling easy_setup_start():
easy_setup_enable_mcast();
easy_setup_enable_neeze();
easy_setup_enable_akiss();
easy_setup_enable_changhong();

or just with one call:
easy_setup_enable_protocols(uint16 proto_mask);

where proto_mask is bitmask of various protocols.

For example, to start mcast+neeze+akiss:
easy_setup_enable_mcast(); /* also works for qqconnect */
easy_setup_enable_neeze(); /* included in new qqconnect */
easy_setup_enable_akiss();
easy_setup_start();
/* get ssid/password */
// easy_setup_get_xxx();
easy_setup_stop();

if needed, you can set decryption key for various protocols:
mcast_set_key("0123456789abcdef"); /* mcast decryption key */
akiss_set_key("fedcba9876543210"); /* set akiss decryption key */

With received ssid/password, it's time to feed it to wpa_supplicant:

#!/system/bin/sh

id=$(wpa_cli -i wlan0 add_network)
wpa_cli -i wlan0 set_network $id ssid \"TEST-AP\"
wpa_cli -i wlan0 set_network $id psk \"12345678\"
wpa_cli -i wlan0 set_network $id key_mgmt WPA-PSK
wpa_cli -i wlan0 set_network $id priority 0
wpa_cli -i wlan0 enable_network $id
wpa_cli -i wlan0 save_config

or you can write one entry to wpa_supplicant.conf and reload it:
network={
    ssid="TEST-AP"
    psk="12345678"
    key_mgmt=WPA-PSK
}


* About Broadcom ES (mcast+neeze) and QQ connect
Both are supported in chip, but with different keys:

(Broadcom demo)
  <-- (enc(mcast_key, mcast_payload+neeze_payload)) --

(Mobile QQ)
  <-- (enc(mcast_key_qqcon, mcast_payload+neeze_payload)) --

With following setting in device, it's ready to receive both:
easy_setup_enable_mcast();
easy_setup_enable_neeze();

(set the key if needed)
mcast_set_key(mcast_key); // if you have set mcast key in sender
neeze_set_key(mcast_key); // the same as mcast key, if you have set it

* Jingdong old and changhong
Old jingdong and changhong are just the same, except jingdong has added encryption.
For both, you can get jingdong sec_mode byte with following API:

int jingdong_get_sec_mode(&mode);
or:
int changhong_get_sec_mode(&mode);

* Jingdong JoyLink
In the end of 2015, Jingdong has introduced this new bcast+mcast protocol. To enable it:
    easy_setup_enable_jd();

You can pass a 16-byte key with jd_set_key(). If it's shorter than 16, following bytes are zeroed.

* ES+SOFTAP
To start ES+SOFTAP, enable 0x80 in protocol bitmask. For example, to start neeze+softap with setup utility:
  setup -p 0x83

  By default, softap is created with SSID "es". We can change it:
  setup -p 0x83 -n joyful

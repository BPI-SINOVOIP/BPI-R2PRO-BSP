include $(TOPDIR)/rules.mk

PKG_NAME:=rkwifibt
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

SOURCE_DIR:=./files/
# FIRMWARE_DIR=system
FIRMWARE_DIR=vendor

define Package/rkwifibt
	SECTION:=Firefly SDK
	CATEGORY:=Firefly SDK
	TITLE:=rkwifibt
endef

define Package/rkwifibt/config
	choice
		prompt "Select Wifi platfrom"
		default RKWIFIBT_AWCM256

		config RKWIFIBT_AP6181
   			bool "AP6181"
		config RKWIFIBT_AP6212A1
			bool "AP6212A1"
		config RKWIFIBT_AP6236
			bool "AP6236"
		config RKWIFIBT_AP6255
			bool "AP6255"
		config RKWIFIBT_AP6256
			bool "AP6256"
		config RKWIFIBT_AP6275S
			bool "AP6275S"
		config RKWIFIBT_AP6335
			bool "AP6335"
		config RKWIFIBT_AP6354
			bool "AP6354"
		config RKWIFIBT_AP6356
			bool "AP6356"
		config RKWIFIBT_AP6398S
			bool "AP6398S"
		config RKWIFIBT_AP6203BM
			bool "AP6203BM"
		config RKWIFIBT_AMPAKALL
			bool "AMPAKALL"
		config RKWIFIBT_AWCM256
			bool "AW-CM256"
		config RKWIFIBT_AWNB197
			bool "AW-NB197"
		config RKWIFIBT_RTL8188EU
			bool "RTL8188EU"
		config RKWIFIBT_RTL8188FU
			bool "RTL8188FU"
		config RKWIFIBT_RTL8189ES
			bool "RTL8189ES"
		config RKWIFIBT_RTL8189FS
			bool "RTL8189FS"
		config RKWIFIBT_RTL8723BS
			bool "RTL8723BS"
		config RKWIFIBT_RTL8723BU
			bool "RTL8723BU"
		config RKWIFIBT_RTL8723CS
			bool "RTL8723CS"
		config RKWIFIBT_RTL8723DS
			bool "RTL8723DS"
		config RKWIFIBT_RTL8812AU
			bool "RTL8812AU"
		config RKWIFIBT_RTL8821CS
			bool "RTL8821CS"
		config RKWIFIBT_RTL8822BE
			bool "RTL8822BE"
		config RKWIFIBT_RTL8822BS
			bool "RTL8822BS"
		config RKWIFIBT_RTL8822BU
			bool "RTL8822BU"
		config RKWIFIBT_RK912
			bool "RK912"
		config RKWIFIBT_COMPATIBLE
			bool "Rockchip Compatible with all supported wifi"
	endchoice

	config RKWIFIBT_CHIPNAME
		string
		default "AP6181" if RKWIFIBT_AP6181
		default "AP6212A1" if RKWIFIBT_AP6212A1
		default "AP6236" if RKWIFIBT_AP6236
		default "AP6255" if RKWIFIBT_AP6255
		default "AP6256" if RKWIFIBT_AP6256
		default "AP6275S" if RKWIFIBT_AP6275S
		default "AP6335" if RKWIFIBT_AP6335
		default "AP6354" if RKWIFIBT_AP6354
		default "AP6356" if RKWIFIBT_AP6356
		default "AP6398S" if RKWIFIBT_AP6398S
		default "AP6203BM" if RKWIFIBT_AP6203BM
		default "all" if RKWIFIBT_AMPAKALL
		default "AW-CM256" if RKWIFIBT_AWCM256
		default "AW-NB197" if RKWIFIBT_AWNB197
		default "RTL8188EU" if RKWIFIBT_RTL8188EU
		default "RTL8188FU" if RKWIFIBT_RTL8188FU
		default "RTL8189ES" if RKWIFIBT_RTL8189ES
		default "RTL8189FS" if RKWIFIBT_RTL8189FS
		default "RTL8723BS" if RKWIFIBT_RTL8723BS
		default "RTL8723BU" if RKWIFIBT_RTL8723BU
		default "RTL8723DS" if RKWIFIBT_RTL8723CS
		default "RTL8723DS" if RKWIFIBT_RTL8723DS
		default "RTL8812AU" if RKWIFIBT_RTL8812AU
		default "RTL8821CS" if RKWIFIBT_RTL8821CS
		default "RTL8822BE" if RKWIFIBT_RTL8822BE
		default "RTL8822BS" if RKWIFIBT_RTL8822BS
		default "RTL8822BU" if RKWIFIBT_RTL8822BU
		default "RK912" if RKWIFIBT_RK912


	config RKWIFIBT_VENDOR
		string
		default "BROADCOM" if RKWIFIBT_AP6181
		default "BROADCOM" if RKWIFIBT_AP6212A1
		default "BROADCOM" if RKWIFIBT_AP6236
		default "BROADCOM" if RKWIFIBT_AP6255
		default "BROADCOM" if RKWIFIBT_AP6256
		default "BROADCOM" if RKWIFIBT_AP6275S
		default "BROADCOM" if RKWIFIBT_AP6335
		default "BROADCOM" if RKWIFIBT_AP6354
		default "BROADCOM" if RKWIFIBT_AP6356
		default "BROADCOM" if RKWIFIBT_AP6398S
		default "BROADCOM" if RKWIFIBT_AP6203BM
		default "BROADCOM" if RKWIFIBT_AMPAKALL
		default "CYPRESS" if RKWIFIBT_AWCM256
		default "CYPRESS" if RKWIFIBT_AWNB197
		default "REALTEK" if RKWIFIBT_RTL8188EU
		default "REALTEK" if RKWIFIBT_RTL8188FU
		default "REALTEK" if RKWIFIBT_RTL8189ES
		default "REALTEK" if RKWIFIBT_RTL8189FS
		default "REALTEK" if RKWIFIBT_RTL8723BS
		default "REALTEK" if RKWIFIBT_RTL8723BU
		default "REALTEK" if RKWIFIBT_RTL8723CS
		default "REALTEK" if RKWIFIBT_RTL8723DS
		default "REALTEK" if RKWIFIBT_RTL8821CS
		default "REALTEK" if RKWIFIBT_RTL8812AU
		default "REALTEK" if RKWIFIBT_RTL8822BE
		default "REALTEK" if RKWIFIBT_RTL8822BS
		default "REALTEK" if RKWIFIBT_RTL8822BU
		default "ROCKCHIP" if RKWIFIBT_RK912


	config RKWIFIBT_WIFI_KO
		string
		default "bcmdhd.ko" if RKWIFIBT_AP6181
		default "bcmdhd.ko" if RKWIFIBT_AP6212A1
		default "bcmdhd.ko" if RKWIFIBT_AP6236
		default "bcmdhd.ko" if RKWIFIBT_AP6255
		default "bcmdhd.ko" if RKWIFIBT_AP6256
		default "bcmdhd.ko" if RKWIFIBT_AP6257S
		default "bcmdhd.ko" if RKWIFIBT_AP6335
		default "bcmdhd.ko" if RKWIFIBT_AP6354
		default "bcmdhd.ko" if RKWIFIBT_AP6356
		default "bcmdhd.ko" if RKWIFIBT_AP6398S
		default "bcmdhd_indep_power.ko" if RKWIFIBT_AP6203BM
		default "bcmdhd.ko" if RKWIFIBT_AMPAKALL
		default "cywdhd.ko" if RKWIFIBT_AWCM256
		default "cywdhd.ko" if RKWIFIBT_AWNB197
		default "8188eu.ko" if RKWIFIBT_RTL8188EU
		default "8188fu.ko" if RKWIFIBT_RTL8188FU
		default "8189es.ko" if RKWIFIBT_RTL8189ES
		default "8189fs.ko" if RKWIFIBT_RTL8189FS
		default "8723bs.ko" if RKWIFIBT_RTL8723BS
		default "8723bu.ko" if RKWIFIBT_RTL8723BU
		default "8723cs.ko" if RKWIFIBT_RTL8723CS
		default "8723ds.ko" if RKWIFIBT_RTL8723DS
		default "8812au.ko" if RKWIFIBT_RTL8812AU
		default "8822be.ko" if RKWIFIBT_RTL8822BE
		default "8822bs.ko" if RKWIFIBT_RTL8822BS
		default "8822bu.ko" if RKWIFIBT_RTL8822BU
		default "8821cs.ko" if RKWIFIBT_RTL8821CS

	config RKWIFIBT_BT_FW
		string
		default "BCM43430A1.hcd" if RKWIFIBT_AP6212A1
		default "BCM43430B0.hcd" if RKWIFIBT_AP6236
		default "BCM4345C0.hcd" if RKWIFIBT_AP6255
		default "BCM4345C5.hcd" if RKWIFIBT_AP6256
		default "BCM4362A2.hcd" if RKWIFIBT_AP6275S
		default "BCM4335C0.hcd" if RKWIFIBT_AP6335
		default "BCM4350C0.hcd" if RKWIFIBT_AP6354
		default "BCM4354A2.hcd" if RKWIFIBT_AP6356
		default "BCM4359C0.hcd" if RKWIFIBT_AP6398S
		default "*" if RKWIFIBT_AMPAK
		default "BCM4345C0.hcd" if RKWIFIBT_AWCM256
		default "BCM4343A1.hcd" if RKWIFIBT_AWNB197

	config RKWIFIBT_BT_EN
		string 
		default "DISABLE" if RKWIFIBT_AP6181
		default "ENABLE" if RKWIFIBT_AP6212A1
		default "ENABLE" if RKWIFIBT_AP6236
		default "ENABLE" if RKWIFIBT_AP6255
		default "ENABLE" if RKWIFIBT_AP6256
		default "ENABLE" if RKWIFIBT_AP6275S
		default "ENABLE" if RKWIFIBT_AP6335
		default "ENABLE" if RKWIFIBT_AP6354
		default "ENABLE" if RKWIFIBT_AP6356
		default "ENABLE" if RKWIFIBT_AP6398S
		default "ENABLE" if RKWIFIBT_AMPAKALL
		default "ENABLE" if RKWIFIBT_AWCM256
		default "ENABLE" if RKWIFIBT_AWNB197
		default "DISABLE" if RKWIFIBT_RTL8189FS
		default "DISABLE" if RKWIFIBT_RTL8188EU
		default "DISABLE" if RKWIFIBT_RTL8188FU
		default "ENABLE" if RKWIFIBT_RTL8723CS
		default "ENABLE" if RKWIFIBT_RTL8723DS
		default "ENABLE" if RKWIFIBT_RTL8821CS
		default "DISABLE" if RKWIFIBT_RK912
	
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -rf $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
endef

define Build/Configure
	true
endef

define Build/Compile
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/brcm_tools/brcm_patchram_plus1 $(PKG_BUILD_DIR)/brcm_tools/brcm_patchram_plus1.c
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/brcm_tools/dhd_priv $(PKG_BUILD_DIR)/brcm_tools/dhd_priv.c
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/src/rk_wifi_init $(PKG_BUILD_DIR)/src/rk_wifi_init.c 
	$(MAKE) -C $(PKG_BUILD_DIR)/realtek/rtk_hciattach CC="$(TARGET_CROSS)gcc"
	$(MAKE) -C $(PKG_BUILD_DIR)/src/CY_WL_API/ CC="$(TARGET_CROSS)gcc"
endef

define Package/rkwifibt/install
	if [ $(CONFIG_RKWIFIBT_VENDOR) == "BROADCOM" ] || [ $(CONFIG_RKWIFIBT_VENDOR) == "CATEGORY" ];then \
		$(INSTALL_DIR) $(1)/$(FIRMWARE_DIR)/etc/firmware; \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/firmware/broadcom/$(CONFIG_RKWIFIBT_CHIPNAME)/wifi/* $(1)/$(FIRMWARE_DIR)/etc/firmware; \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/firmware/broadcom/$(CONFIG_RKWIFIBT_CHIPNAME)/bt/* $(1)/$(FIRMWARE_DIR)/etc/firmware; \
		$(INSTALL_DIR) $(1)/usr/bin; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/brcm_tools/brcm_patchram_plus1 $(1)/usr/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/brcm_tools/dhd_priv $(1)/usr/bin/; \
	fi

	if [ $(CONFIG_RKWIFIBT_VENDOR) == "REALTEK" ];then \
		$(INSTALL_DIR) $(1)/lib/firmware; \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/realtek/$(CONFIG_RKWIFIBT_CHIPNAME)/mp_* $(1)/lib/firmware/; \
		$(INSTALL_DIR) $(1)/lib/firmware/rtlbt; \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/realtek/$(CONFIG_RKWIFIBT_CHIPNAME)/* $(1)/lib/firmware/rtlbt/; \
	fi

	if [ $(CONFIG_RKWIFIBT_VENDOR) == "ROCKCHIP" ];then \
		$(INSTALL_DIR) $(1)/lib/firmware; \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/firmware/rockchip/WIFI_FIRMWARE/rk912* $(1)/lib/firmware/; \
	fi

endef

$(eval $(call BuildPackage,rkwifibt))

#############################################################
#
# pcba test using adb
#
#############################################################

define PCBA_ADB_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Write_SN \
		$(TARGET_DIR)/usr/bin/Write_SN
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Read_SN \
		$(TARGET_DIR)/usr/bin/Read_SN
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/fa \
		$(TARGET_DIR)/usr/bin/fa
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Write_WiFimac \
		$(TARGET_DIR)/usr/bin/Write_WiFimac
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Read_WiFimac \
		$(TARGET_DIR)/usr/bin/Read_WiFimac
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Write_BTmac \
		$(TARGET_DIR)/usr/bin/Write_BTmac
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Read_BTmac \
		$(TARGET_DIR)/usr/bin/Read_BTmac
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Version \
		$(TARGET_DIR)/usr/bin/Version
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Ringmic_test \
		$(TARGET_DIR)/usr/bin/Ringmic_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/SIM_test \
		$(TARGET_DIR)/usr/bin/SIM_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Read_Size \
		$(TARGET_DIR)/usr/bin/Read_Size
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/led_show \
		$(TARGET_DIR)/usr/bin/led_show
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Battery_test \
		$(TARGET_DIR)/usr/bin/Battery_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Button_test \
		$(TARGET_DIR)/usr/bin/Button_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Aging_test \
		$(TARGET_DIR)/usr/bin/Aging_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Wlan_test \
		$(TARGET_DIR)/usr/bin/Wlan_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Tube_test \
		$(TARGET_DIR)/usr/bin/Tube_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/bin/Bt_test \
		$(TARGET_DIR)/usr/bin/Bt_test
	$(INSTALL) -m 0755 -D package/rockchip/pcba_adb/keytest.json \
		$(TARGET_DIR)/etc/keytest.json
endef
$(eval $(generic-package))

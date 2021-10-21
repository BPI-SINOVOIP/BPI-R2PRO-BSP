include $(TOPDIR)/rules.mk

PKG_NAME:=ff_wps_button
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/ff_wps_button/Default
  SECTION:=Firefly SDK
  CATEGORY:=Firefly SDK
  TITLE:=Firefly WPS Button Service
endef

define Package/ff_wps_button
$(call Package/ff_wps_button/Default,$(1))
endef

define Package/ff_wps_button/description
	A tool which listens recovery adc key pressed event
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
	true
endef

define Build/Compile
    $(TARGET_CC) -o $(PKG_BUILD_DIR)/ff_wps_button $(PKG_BUILD_DIR)/ff_wps_button.c
endef

define Package/ff_wps_button/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ff_wps_button $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ff_wps_button $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,ff_wps_button))

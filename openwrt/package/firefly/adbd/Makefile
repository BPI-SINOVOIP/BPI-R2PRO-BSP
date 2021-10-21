include $(TOPDIR)/rules.mk

PKG_NAME:=adbd
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/adbd
  SECTION:=Firefly SDK
  CATEGORY:=Firefly SDK
  TITLE:= Android tools adbd
  DEPENDS:=+libopenssl +zlib +libpthread
endef

define Package/adbd/description
	Android tools adbd
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/adbd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/adbd $(1)/usr/bin
	$(INSTALL_BIN) ./files/usbdevice $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/usbdevice_service $(1)/etc/init.d/usbdevice
	$(INSTALL_CONF) ./files/usb_config $(1)/etc/.usb_config

	$(INSTALL_DIR) $(1)/etc/hotplug.d/android_usb
	$(INSTALL_BIN) ./files/00-default $(1)/etc/hotplug.d/android_usb
endef

$(eval $(call BuildPackage,adbd))

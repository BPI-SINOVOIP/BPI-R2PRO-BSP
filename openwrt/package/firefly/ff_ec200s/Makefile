include $(TOPDIR)/rules.mk

PKG_NAME:=ff_ec200s
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/ff_ec200s/Default
  SECTION:=Firefly SDK
  CATEGORY:=Firefly SDK
  TITLE:= EC200S ECM Mode Config Tools
  DEPENDS:=+ff_net_forward
endef

define Package/ff_ec200s
$(call Package/ff_ec200s/Default,$(1))
endef

define Package/ff_ec200s/description
	EC200S ECM Mode Config Tools
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ff_ec200s/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/ec200s_init.sh $(1)/usr/bin/ec200s_init.sh
	$(INSTALL_BIN) ./files/ec200s_shutdown.sh $(1)/usr/bin/ec200s_shutdown.sh
	$(INSTALL_DIR) $(1)/etc/hotplug.d/usb
	$(INSTALL_BIN) ./files/20-ec200s $(1)/etc/hotplug.d/usb/20-ec200s
endef

$(eval $(call BuildPackage,ff_ec200s))

include $(TOPDIR)/rules.mk

PKG_NAME:=ff_net_forward
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/ff_net_forward/Default
  SECTION:=Firefly SDK
  CATEGORY:=Firefly SDK
  TITLE:= Firefly Net Forward Tools
endef

define Package/ff_net_forward
$(call Package/ff_net_forward/Default,$(1))
endef

define Package/ff_net_forward/description
	Firefly Net Forward Tools
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ff_net_forward/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/ff_net_forward $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/etc/hotplug.d/iface/99-ff_net_forward $(1)/etc/hotplug.d/iface
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/ff_net_forward $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,ff_net_forward))

include $(TOPDIR)/rules.mk

PKG_NAME:=ff_wifi
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/ff_wifi/Default
  SECTION:=Firefly SDK
  CATEGORY:=Firefly SDK
  TITLE:= Firefly Wifi Tools
endef

define Package/ff_wifi
$(call Package/ff_wifi/Default,$(1))
endef

define Package/ff_wifi/description
	Firefly Wifi Tools
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ff_wifi/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/ff_wifi $(1)/usr/bin/ff_wifi
endef

$(eval $(call BuildPackage,ff_wifi))

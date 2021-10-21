include $(TOPDIR)/rules.mk

PKG_NAME:=quectel-CM
PKG_VERSION:=1.0
PKG_RELEASE:=1

SOURCE_DIR:=./src/

include $(INCLUDE_DIR)/package.mk

define Package/quectel-CM
	SECTION:=Firefly SDK
	CATEGORY:=Firefly SDK
	TITLE:=quectel-CM
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		LD="$(TARGET_CROSS)ld"
endef

define Package/quectel-CM/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/hotplug.d/net
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/21-ec20 $(1)/etc/hotplug.d/net
endef

$(eval $(call BuildPackage,quectel-CM))

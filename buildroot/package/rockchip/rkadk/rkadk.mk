RKADK_SITE = $(TOPDIR)/../external/rkadk
RKADK_SITE_METHOD = local
RKADK_INSTALL_STAGING = YES

RKADK_DEPENDENCIES += rkmedia rockit iniparser

ifeq ($(BR2_PACKAGE_RKMEDIA_USE_AIQ), y)
RKADK_DEPENDENCIES += camera_engine_rkaiq
RKADK_CONF_OPTS += -DUSE_RKAIQ=ON
endif

RKADK_CONF_OPTS += -DRKMEDIA_HEADER_DIR=$(TOPDIR)/../external/rkmedia/include/rkmedia \
        -DROCKIT_HEADER_DIR=$(STAGING_DIR)/usr/include/rockit

RKADK_CONF_OPTS += -DCMAKE_INSTALL_STAGING=$(STAGING_DIR)

define RKADK_CP_DEF_SETTING_FILE
	cp -rfp $(@D)/rkadk_defsetting.ini $(TARGET_DIR)/etc/ | true
endef

RKADK_POST_INSTALL_TARGET_HOOKS += RKADK_CP_DEF_SETTING_FILE

$(eval $(cmake-package))

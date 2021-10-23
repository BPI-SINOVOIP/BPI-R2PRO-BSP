ifneq ($(filter rk312% rk322% rk3288 rk3328 rk3368 rk3399 rk3399pro px3 px5, $(TARGET_BOARD_PLATFORM)), )
$(info 'building rk_tee_user v1')
else
$(info 'building rk_tee_user v2')

LOCAL_PATH := $(call my-dir)
OPTEE_TEST_PATH := $(shell pwd)/$(LOCAL_PATH)

## include variants like TA_DEV_KIT_DIR
## and target of BUILD_OPTEE_OS
INCLUDE_FOR_BUILD_TA := false
include $(BUILD_OPTEE_MK)
INCLUDE_FOR_BUILD_TA :=

VERSION = $(shell git describe --always --dirty=-dev 2>/dev/null || echo Unknown)

# TA_DEV_KIT_DIR must be set to non-empty value to
# avoid the Android build scripts complaining about
# includes pointing outside the Android source tree.
# This var is expected to be set when OPTEE OS built.
# We set the default value to an invalid path.
TA_DEV_KIT_DIR ?= $(OPTEE_TEST_PATH)/export-ta_arm32

OPTEE_CLIENT_PATH ?= $(LOCAL_PATH)/client_export

include $(TA_DEV_KIT_DIR)/host_include/conf.mk

ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 11)))
ifeq ($(strip $(TARGET_ARCH)), arm64)
	CLIENT_LIB_PATH ?= $(shell pwd)/hardware/rockchip/optee/v2/arm64
else
	CLIENT_LIB_PATH ?= $(shell pwd)/hardware/rockchip/optee/v2/arm
endif
else
ifeq ($(strip $(TARGET_ARCH)), arm64)
	CLIENT_LIB_PATH ?= $(shell pwd)/vendor/rockchip/common/security/optee/v2/lib/arm64
else
	CLIENT_LIB_PATH ?= $(shell pwd)/vendor/rockchip/common/security/optee/v2/lib/arm
endif
endif

################################################################################
# Build testapp                                                                #
################################################################################
include $(CLEAR_VARS)
LOCAL_CFLAGS += -DANDROID_BUILD -DUSER_SPACE
LOCAL_CFLAGS += -Wall -Wno-error
LOCAL_LDFLAGS += $(CLIENT_LIB_PATH)/libteec.so
LOCAL_LDFLAGS += -llog

LOCAL_SRC_FILES += host/rkdemo/testapp_ca.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ta/testapp/include \
		$(OPTEE_CLIENT_PATH)/public

LOCAL_MODULE := testapp
LOCAL_MODULE_TAGS := optional
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8)))
	LOCAL_PROPRIETARY_MODULE := true
endif
include $(BUILD_EXECUTABLE)

################################################################################
# Build testapp_storage                                                        #
################################################################################
include $(CLEAR_VARS)
LOCAL_CFLAGS += -DANDROID_BUILD -DUSER_SPACE
LOCAL_CFLAGS += -Wall -Wno-error
LOCAL_LDFLAGS += $(CLIENT_LIB_PATH)/libteec.so
LOCAL_LDFLAGS += -llog

LOCAL_SRC_FILES += host/rkdemo/testapp_storage_ca.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ta/testapp_storage/include \
		$(OPTEE_CLIENT_PATH)/public

LOCAL_MODULE := testapp_storage
LOCAL_MODULE_TAGS := optional
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8)))
	LOCAL_PROPRIETARY_MODULE := true
endif
include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/ta/Android.mk
endif
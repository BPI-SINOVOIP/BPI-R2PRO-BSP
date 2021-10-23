LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := setup
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := main_linux.c easy_setup_linux.c easy_setup/easy_setup.c easy_setup/scan.c 
LOCAL_SRC_FILES += proto/mcast.c 
LOCAL_SRC_FILES += proto/neeze.c 
LOCAL_SRC_FILES += proto/akiss.c 
LOCAL_SRC_FILES += proto/changhong.c 
LOCAL_SRC_FILES += proto/jingdong.c 
LOCAL_SRC_FILES += proto/xiaoyi.c
LOCAL_SRC_FILES += proto/jd.c
LOCAL_SRC_FILES += proto/ap.c
LOCAL_CFLAGS := -I$(LOCAL_PATH)/proto -I$(LOCAL_PATH)/easy_setup
LOCAL_LDFLAGS := -static
#LOCAL_STATIC_LIBRARIES := libc
LOCAL_FORCE_STATIC_EXECUTABLE := true

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)

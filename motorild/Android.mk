LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := motorild.c
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin/
LOCAL_MODULE := motorild
LOCAL_MODULE_TAGS := optional
ifneq ($(wildcard external/libnl),)
LOCAL_C_INCLUDES += external/libnl/include
LOCAL_SHARED_LIBRARIES := libnl
else
LOCAL_C_INCLUDES += external/libnl-headers
LOCAL_STATIC_LIBRARIES := libnl_2
endif
LOCAL_SHARED_LIBRARIES += libcutils


include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := motorilc.c
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin/
LOCAL_MODULE := motorilc
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES += libcutils

include $(BUILD_EXECUTABLE)

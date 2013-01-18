LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := serialno_tracker.c
LOCAL_MODULE := serialno_tracker
LOCAL_SHARED_LIBRARIES := libcutils libc
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

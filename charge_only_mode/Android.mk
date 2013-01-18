# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	alarm.c \
	events.c \
	hardware.c \
	main.c

ifeq ($(TARGET_BOARD_PLATFORM), tegra)
   LOCAL_SRC_FILES+= draw_tegra.c \
                     screen_tegra.c
else
   LOCAL_SRC_FILES+= draw.c \
		     screen.c
endif

LOCAL_STATIC_LIBRARIES := libunz libcutils libc

LOCAL_C_INCLUDES := external/zlib

ifeq ($(TARGET_PRODUCT),cdma_venus2)
    LOCAL_CFLAGS := -I$(LOCAL_PATH)/venus2_assets
else
    ifeq ($(findstring solana,$(TARGET_PRODUCT)),solana)
        LOCAL_CFLAGS := -I$(LOCAL_PATH)/solana_assets
    else
        LOCAL_CFLAGS := -I$(LOCAL_PATH)/assets
    endif
endif

LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE:= charge_only_mode

include $(BUILD_EXECUTABLE)

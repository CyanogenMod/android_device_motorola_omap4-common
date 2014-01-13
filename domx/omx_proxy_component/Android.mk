LOCAL_PATH:= $(call my-dir)
COMMON_FOLDER := device/motorola/omap4-common

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 16 || echo 1),)
	FRAMEWORKS_MEDIA_BASE := $(TOP)/frameworks/native/include/media/hardware
else
ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 14 || echo 1),)
	FRAMEWORKS_MEDIA_BASE := $(TOP)/frameworks/base/include/media/stagefright
endif
endif

#
# libOMX.TI.DUCATI1.VIDEO.DECODER
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	hardware/libhardware/include \
	$(COMMON_FOLDER)/hwc/ \
	$(LOCAL_PATH)/../domx/plugins/inc/

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES -DUSE_ENHANCED_PORTRECONFIG
LOCAL_CFLAGS += -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION -DENABLE_GRALLOC_BUFFERS
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_dec/src/omx_proxy_videodec.c \
                  omx_video_dec/src/omx_proxy_videodec_utils.c

# Uncomment the below 2 lines to enable the run time
# dump of NV12 buffers from Decoder/Camera
# based on setprop control
#LOCAL_CFLAGS += -DENABLE_RAW_BUFFERS_DUMP_UTILITY
#LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.DECODER
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.MISC.SAMPLE
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	$(LOCAL_PATH)/../domx/plugins/inc/

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx

LOCAL_CFLAGS += -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES -DUSE_ENHANCED_PORTRECONFIG
LOCAL_CFLAGS += -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_sample/src/omx_proxy_sample.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.MISC.SAMPLE
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)


#
# libOMX.TI.DUCATI1.VIDEO.CAMERA
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(COMMON_FOLDER)/libion \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	$(LOCAL_PATH)/../domx/plugins/inc/ \
	$(LOCAL_PATH)/omx_camera/inc/

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libion_ti \
	libdomx

LOCAL_CFLAGS += -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES -DUSE_ENHANCED_PORTRECONFIG
LOCAL_CFLAGS += -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_camera/src/omx_proxy_camera.c \
                  omx_camera/src/proxy_camera_android_glue.c

LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.CAMERA
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.VIDEO.H264E
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	system/core/include/cutils \
	$(COMMON_FOLDER)/hwc \
	$(COMMON_FOLDER)/camera/inc \
	frameworks/base/include/media/stagefright \
	frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/../domx/plugins/inc/ \
        $(LOCAL_PATH)/omx_video_enc/inc

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware \
	libcutils

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES
LOCAL_CFLAGS += -DUSE_ENHANCED_PORTRECONFIG -DENABLE_GRALLOC_BUFFER -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_CFLAGS += -DANDROID_CUSTOM_OPAQUECOLORFORMAT
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_enc/src/omx_h264_enc/src/omx_proxy_h264enc.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264E
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.VIDEO.VC1E
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	system/core/include/cutils \
	$(COMMON_FOLDER)/hwc \
	$(COMMON_FOLDER)/camera/inc \
	frameworks/base/include/media/stagefright \
	frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/../domx/plugins/inc/ \
        $(LOCAL_PATH)/omx_video_enc/inc

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware \
	libcutils

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES
LOCAL_CFLAGS += -DUSE_ENHANCED_PORTRECONFIG -DENABLE_GRALLOC_BUFFER -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_CFLAGS += -DANDROID_CUSTOM_OPAQUECOLORFORMAT
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_enc/src/omx_vc1_enc/src/omx_proxy_vc1enc.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.VC1E
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.VIDEO.H264SVCE
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	system/core/include/cutils \
	$(COMMON_FOLDER)/hwc \
	$(COMMON_FOLDER)/camera/inc \
	frameworks/base/include/media/stagefright \
	frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/../domx/plugins/inc/ \
        $(LOCAL_PATH)/omx_video_enc/inc

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware \
	libcutils

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES
LOCAL_CFLAGS += -DUSE_ENHANCED_PORTRECONFIG -DENABLE_GRALLOC_BUFFER -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_CFLAGS += -DANDROID_CUSTOM_OPAQUECOLORFORMAT
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_enc/src/omx_h264svc_enc/src/omx_proxy_h264svcenc.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264SVCE
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.VIDEO.MPEG4E
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	system/core/include/cutils \
	$(COMMON_FOLDER)/hwc \
	$(COMMON_FOLDER)/camera/inc \
	frameworks/base/include/media/stagefright \
	frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/../domx/plugins/inc/ \
        $(LOCAL_PATH)/omx_video_enc/inc

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware \
	libcutils

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES
LOCAL_CFLAGS += -DUSE_ENHANCED_PORTRECONFIG -DENABLE_GRALLOC_BUFFER -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_CFLAGS += -DANDROID_CUSTOM_OPAQUECOLORFORMAT
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_enc/src/omx_mpeg4_enc/src/omx_proxy_mpeg4enc.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.MPEG4E
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.VIDEO.DECODER.secure
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	hardware/libhardware/include \
	$(COMMON_FOLDER)/hwc/ \
	$(LOCAL_PATH)/../domx/plugins/inc/

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware \
	libOMX.TI.DUCATI1.VIDEO.DECODER

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES -DUSE_ENHANCED_PORTRECONFIG
LOCAL_CFLAGS += -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION -DENABLE_GRALLOC_BUFFERS
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_dec/src/omx_proxy_videodec_secure.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.DECODER.secure
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

#
# libOMX.TI.DUCATI1.VIDEO.H264E.secure
#
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../omx_core/inc \
	$(LOCAL_PATH)/../mm_osal/inc \
	$(LOCAL_PATH)/../domx \
	$(LOCAL_PATH)/../domx/omx_rpc/inc \
	system/core/include/cutils \
	$(COMMON_FOLDER)/hwc \
	$(COMMON_FOLDER)/camera/inc \
	frameworks/base/include/media/stagefright \
	frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/../domx/plugins/inc/ \
        $(LOCAL_PATH)/omx_video_enc/inc

LOCAL_SHARED_LIBRARIES := \
	libmm_osal \
	libc \
	libOMX_Core \
	liblog \
	libdomx \
	libhardware \
	libcutils

LOCAL_CFLAGS += -DLINUX -DTMS32060 -D_DB_TIOMAP -DSYSLINK_USE_SYSMGR -DSYSLINK_USE_LOADER
LOCAL_CFLAGS += -D_Android -DSET_STRIDE_PADDING_FROM_PROXY -DANDROID_QUIRK_CHANGE_PORT_VALUES
LOCAL_CFLAGS += -DUSE_ENHANCED_PORTRECONFIG -DENABLE_GRALLOC_BUFFER -DANDROID_QUIRK_LOCK_BUFFER -DUSE_ION
LOCAL_CFLAGS += -DANDROID_CUSTOM_OPAQUECOLORFORMAT
LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_video_enc/src/omx_h264_enc/src/omx_proxy_h264enc_secure.c
LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264E.secure
include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

FRAMEWORKS_MEDIA_BASE :=

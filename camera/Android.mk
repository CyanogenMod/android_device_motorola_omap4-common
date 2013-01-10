LOCAL_PATH:= $(call my-dir)

OMAP4_CAMERA_HAL_SRC := \
	CameraHal_Module.cpp \
	CameraHal.cpp \
	CameraHalUtilClasses.cpp \
	AppCallbackNotifier.cpp \
	ANativeWindowDisplayAdapter.cpp \
	CameraProperties.cpp \
	MemoryManager.cpp \
	Encoder_libjpeg.cpp \
	SensorListener.cpp  \
	NV12_resize.c

OMAP4_CAMERA_COMMON_SRC:= \
	CameraParameters.cpp \
	TICameraParameters.cpp \
	CameraHalCommon.cpp

OMAP4_CAMERA_OMX_SRC:= \
	BaseCameraAdapter.cpp \
	OMXCameraAdapter/OMX3A.cpp \
	OMXCameraAdapter/OMXAlgo.cpp \
	OMXCameraAdapter/OMXCameraAdapter.cpp \
	OMXCameraAdapter/OMXCapabilities.cpp \
	OMXCameraAdapter/OMXCapture.cpp \
	OMXCameraAdapter/OMXDefaults.cpp \
	OMXCameraAdapter/OMXExif.cpp \
	OMXCameraAdapter/OMXFD.cpp \
	OMXCameraAdapter/OMXFocus.cpp \
	OMXCameraAdapter/OMXZoom.cpp \

OMAP4_CAMERA_USB_SRC:= \
	BaseCameraAdapter.cpp \
	V4LCameraAdapter/V4LCameraAdapter.cpp

#
# OMX Camera HAL 
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	$(OMAP4_CAMERA_HAL_SRC) \
	$(OMAP4_CAMERA_OMX_SRC) \
	$(OMAP4_CAMERA_COMMON_SRC)

ifeq ($(BOARD_USE_TI_ENHANCED_DOMX),true)
    CAMERAHAL_CFLAGS += -DENHANCED_DOMX
endif

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc/ \
    $(COMMON_FOLDER)/hwc \
    $(COMMON_FOLDER)/include \
    $(LOCAL_PATH)/inc/OMXCameraAdapter \
    hardware/ti/omap4xxx/libtiutils \
    $(COMMON_FOLDER)/libion \
    $(DOMX_PATH)/omx_core/inc \
    $(DOMX_PATH)/mm_osal/inc \
    frameworks/base/include/media/stagefright \
    frameworks/native/include/media/hardware \
    frameworks/native/include/media/openmax \
    external/jpeg \
    external/jhead

LOCAL_SHARED_LIBRARIES:= \
    libui \
    libbinder \
    libutils \
    libcutils \
    libtiutils \
    libmm_osal \
    libOMX_Core \
    libcamera_client \
    libgui \
    libdomx \
    libion_ti \
    libjpeg \
    libexif

LOCAL_CFLAGS := -fno-short-enums -DCOPY_IMAGE_BUFFER

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE:= camera.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)



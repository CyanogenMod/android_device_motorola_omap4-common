LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := etc/kexec/ramdisk.img
LOCAL_MODULE_TAGS := optional eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)

include $(BUILD_SYSTEM)/base_rules.mk

$(LOCAL_BUILT_MODULE): RDIN := $(PRODUCT_OUT)/ramdisk.img
$(LOCAL_BUILT_MODULE): RDOUT := $(TARGET_OUT)/$(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(PRODUCT_OUT)/ramdisk.img
	$(hide) echo "Safestrapped ramdisk: $(RDIN) -> $(RDOUT)"
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -rf $@
	cat device/motorola/omap4-common/kexec/safestrapped.cpio.gz $(RDIN) > $@

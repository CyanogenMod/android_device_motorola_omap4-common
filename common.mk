#
# This is the product configuration for a full common
#

COMMON_FOLDER := device/motorola/common

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_us_supl.mk)

## (3)  Finally, the least specific parts, i.e. the non-GSM-specific aspects

# Device overlay
DEVICE_PACKAGE_OVERLAYS += $(COMMON_FOLDER)/overlay

# high-density artwork where available
PRODUCT_AAPT_CONFIG := normal hdpi
PRODUCT_AAPT_PREF_CONFIG := hdpi

PRODUCT_PACKAGES += \
    charger \
    charger_res_images

# Hardware HALs
PRODUCT_PACKAGES += \
    libinvensense_mpl \
    audio.usb.default

PRODUCT_PACKAGES += \
    libaudioutils \
    libaudiohw_legacy

# BlueZ a2dp Audio HAL module
PRODUCT_PACKAGES += \
    audio.a2dp.default \

# BlueZ test tools
PRODUCT_PACKAGES += \
    hciconfig \
    hcitool

# Modem
PRODUCT_PACKAGES += \
    libaudiomodemgeneric \
    rild \
    radiooptions

# Wifi
PRODUCT_PACKAGES += \
    lib_driver_cmd_wl12xx \
    dhcpcd.conf \
    hostapd.conf \
    wifical.sh \
    wpa_supplicant.conf \
    TQS_D_1.7.ini \
    TQS_D_1.7_127x.ini \
    crda \
    regulatory.bin \
    calibrator \
    busybox

# Wifi Direct and WPAN
PRODUCT_PACKAGES += \
    ti_wfd_libs \
    ti-wpan-fw

# Bluetooth
PRODUCT_PACKAGES += \
    uim-sysfs \
    libbt-vendor

# OMAP4
PRODUCT_PACKAGES += \
    libdomx \
    libOMX_Core \
    libOMX.TI.DUCATI1.VIDEO.H264E \
    libOMX.TI.DUCATI1.VIDEO.MPEG4E \
    libOMX.TI.DUCATI1.VIDEO.DECODER \
    libOMX.TI.DUCATI1.VIDEO.DECODER.secure \
    libOMX.TI.DUCATI1.VIDEO.CAMERA \
    libOMX.TI.DUCATI1.MISC.SAMPLE \
    libstagefrighthw \
    libI420colorconvert \
    libtiutils \
    libion_ti \
    smc_pa_ctrl \
    tf_daemon \
    libtf_crypto_sst \
    libmm_osal

# Release utilities
PRODUCT_PACKAGES += \
    common_releaseutils-finalize_release \
    common_releaseutils-mke2fs \
    common_releaseutils-tune2fs

PRODUCT_PACKAGES += \
    evtest \
    DockAudio \

# Permissions files
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:system/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \

# WLAN firmware
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/firmware/ti-connectivity/wl128x-fw-4-mr.bin:system/etc/firmware/ti-connectivity/wl128x-fw-4-mr.bin \
    $(COMMON_FOLDER)/firmware/ti-connectivity/wl128x-fw-4-plt.bin:system/etc/firmware/ti-connectivity/wl128x-fw-4-plt.bin \
    $(COMMON_FOLDER)/firmware/ti-connectivity/wl128x-fw-4-sr.bin:system/etc/firmware/ti-connectivity/wl128x-fw-4-sr.bin \
    $(COMMON_FOLDER)/firmware/ti-connectivity/wl1271-nvs.bin:system/etc/firmware/ti-connectivity/wl1271-nvs.bin

# WPAN firmware
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/firmware/wpan/bluetooth/TIInit_10.6.15.bts:system/etc/firmware/TIInit_10.6.15.bts \
    $(COMMON_FOLDER)/firmware/wpan/bluetooth/TIInit_7.2.31.bts:system/etc/firmware/TIInit_7.2.31.bts \
    $(COMMON_FOLDER)/firmware/wpan/bluetooth/TIInit_7.6.15.bts:system/etc/firmware/TIInit_7.6.15.bts \
    $(COMMON_FOLDER)/firmware/wpan/bluetooth/TIInit_12.7.27.bts:system/etc/firmware/TIInit_12.7.27.bts \

# Kexec files
ifeq ($(BOARD_USES_KEXEC),true)
# Don't add these for solana -- they're in the solana device setup
ifneq ($(TARGET_DEVICE),solana)
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/kexec/arm_kexec.ko:system/etc/kexec/arm_kexec.ko \
    $(COMMON_FOLDER)/kexec/kexec.ko:system/etc/kexec/kexec.ko \
    $(COMMON_FOLDER)/kexec/uart.ko:system/etc/kexec/uart.ko
endif

# Common kexec files
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/kexec/atags:system/etc/kexec/atags \
    $(COMMON_FOLDER)/kexec/kexec:system/etc/kexec/kexec

# Kexec Boot support for Safestrap v3
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/prebuilt/bin/bbx:/root/sbin/bbx \
    $(COMMON_FOLDER)/prebuilt/bin/fixboot.sh:/root/sbin/fixboot.sh
else

# non-kexec setting
PRODUCT_PROPERTY_OVERRIDES += \
    persist.hwc.sw_vsync=1

endif

# Rootfs
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/root/init.rc:/root/init.rc

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# still need to set english for audio init
PRODUCT_LOCALES += en_US

# stuff specific to ti OMAP4 hardware
#$(call inherit-product, hardware/ti/omap4xxx/omap4.mk)
$(call inherit-product-if-exists, vendor/motorola/common/common-vendor.mk)


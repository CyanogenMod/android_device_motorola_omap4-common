# Copyright (C) 2013 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
 
#
# This is the product configuration for omap4-common
#

COMMON_FOLDER := device/motorola/omap4-common

# The gps config appropriate for this device
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/prebuilt/bin/pdsbackup.sh:system/bin/pdsbackup.sh \
    $(COMMON_FOLDER)/prebuilt/bin/wifical.sh:system/bin/wifical.sh \
    $(COMMON_FOLDER)/prebuilt/bin/wificalcheck.sh:system/bin/wificalcheck.sh \
    $(COMMON_FOLDER)/prebuilt/etc/gps.conf:system/etc/gps.conf \
    $(COMMON_FOLDER)/prebuilt/etc/wifi/wlan_fem.ini:system/etc/wifi/wlan_fem.ini

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
    audio.usb.default \
    audio.a2dp.default

PRODUCT_PACKAGES += \
    audio_policy.omap4 \
    libasound \
    libaudioutils \
    libaudiohw_legacy

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

# Misc
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory

# Motorola Binaries
PRODUCT_PACKAGES += \
    aplogd \
    charge_only_mode \
    modemlog \
    mot_boot_mode \
    motobox \
    usbd

#symlinks
PRODUCT_PACKAGES += \
    libion.so

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
    libtiutils_custom \
    libion_ti \
    smc_pa_ctrl \
    tf_daemon \
    libtf_crypto_sst \
    libmm_osal \
    gralloc.omap4.so

PRODUCT_PACKAGES += \
    evtest \
    DockAudio \
    libjni_filtershow_filters \
    libjni_mosaic \
    Torch

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
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml

# WLAN firmware
PRODUCT_PACKAGES += \
    wl1271-nvs_128x.bin \
    wl128x-fw-4-sr.bin \
    wl128x-fw-4-mr.bin \
    wl128x-fw-4-plt.bin

# system/etc Prebuilts
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/prebuilt/etc/media_codecs.xml:system/etc/media_codecs.xml \
    $(COMMON_FOLDER)/prebuilt/etc/audio_policy.conf:system/etc/audio_policy.conf

# Root files
PRODUCT_PACKAGES += \
    default.prop \
    init.mapphone_cdma.rc \
    init.mapphone_umts.rc \
    ueventd.mapphone_cdma.rc \
    ueventd.mapphone_umts.rc

PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/root/init.usb.rc:/root/init.usb.rc

PRODUCT_PACKAGES += fstab.mapphone_cdma

# Kexec files
ifndef TARGET_USES_CUSTOM_KEXECFILES
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/kexec/arm_kexec.ko:system/etc/kexec/arm_kexec.ko \
    $(COMMON_FOLDER)/kexec/kexec.ko:system/etc/kexec/kexec.ko \
    $(COMMON_FOLDER)/kexec/uart.ko:system/etc/kexec/uart.ko
endif
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/kexec/atags:system/etc/kexec/atags \
    $(COMMON_FOLDER)/kexec/kexec:system/etc/kexec/kexec

# Bin files for kexec load
PRODUCT_COPY_FILES += \
    $(COMMON_FOLDER)/prebuilt/bin/bbx:/root/sbin/bbx \
    $(COMMON_FOLDER)/prebuilt/bin/fixboot.sh:/root/sbin/fixboot.sh

# sw vsync setting
PRODUCT_PROPERTY_OVERRIDES += \
    persist.hwc.sw_vsync=1

# General
PRODUCT_PROPERTY_OVERRIDES += \
    ro.crypto.state=unencrypted \
    ro.hdcp.support=2 \
    ro.service.start.smc=1 \
    ro.sf.lcd_density=240 \
    windowsmgr.max_events_per_sec=90 \
    com.ti.omap_enhancement=true \
    ro.bq.gpu_to_cpu_unsupported=1 \
    hwui.render_dirty_regions=false \
    persist.sys.root_access=3 \
    ro.product.use_charge_counter=1 \
    persist.sys.usb.config=mtp,adb \
    ro.setupwizard.enable_bypass=1

# Media
PRODUCT_PROPERTY_OVERRIDES += \
    ro.media.camcorder.1080p=mp4,h264,30,15000000,aac,128000,44100,2 \
    ro.media.camcorder.720p=mp4,h264,30,10000000,aac,128000,44100,2 \
    ro.media.camcorder.d1NTSC=mp4,h264,30,6000000,aac,128000,44100,2 \
    ro.media.camcorder.vga=mp4,h264,30,4000000,aac,128000,44100,2 \
    ro.media.camcorder.cif=mp4,h264,30,1500000,aac,128000,44100,2 \
    ro.media.camcorder.qvga=mp4,h264,15,500000,aac,64000,44100,2 \
    ro.media.camcorder.mms=3gp,h264,15,128000,amrnb,12200,8000,1 \
    ro.media.camcorder.mmsres=qvga \
    ro.camcorder.zoom=true \
    ro.media.capture.maxres=5m \
    ro.media.capture.fast.fps=4 \
    ro.media.capture.slow.fps=120 \
    ro.media.capture.flash=led \
    ro.media.capture.flashMinV=3300000 \
    ro.media.capture.torchIntensity=28 \
    ro.media.capture.flashIntensity=100 \
    ro.media.capture.classification=classF \
    ro.media.panorama.defres=3264x1840 \
    ro.media.panorama.frameres=1280x720

# OpenglES
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072

# Radio and Telephony
PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim = true \
    persist.ril.mux.noofchannels=10 \
    ro.cdma.otaspnumschema=SELC,1,80,99 \
    ro.config.vc_call_vol_steps=7 \
    ro.kernel.android.ril=yes \
    ro.telephony.call_ring.multiple=false \
    persist.ril.mux.noofchannels=10 \
    persist.ril.mux.retries=500

# Wifi
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    softap.interface=wlan0 \
    wifi.ap.interface=wlan0 \
    wifi.supplicant_scan_interval=90 \
    persist.wlan.ti.calibrated=0

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# still need to set english for audio init
PRODUCT_LOCALES += en_US

# stuff specific to ti OMAP4 hardware
#$(call inherit-product, hardware/ti/omap4xxx/omap4.mk)
$(call inherit-product, hardware/ti/omap4xxx/security/Android.mk)

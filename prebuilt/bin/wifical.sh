#! /system/bin/sh

WIFION=`getprop init.svc.p2p_supplicant`
WL12xx_MODULE=/system/lib/modules/wl12xx_sdio.ko
PDS_NVS_FILE=/pds/wifi/nvs_map.bin
TARGET_FW_DIR=/system/etc/firmware/ti-connectivity
TARGET_NVS_FILE=$TARGET_FW_DIR/wl1271-nvs.bin

case "$WIFION" in
  "running") echo " ****************************************"
             echo " * Turning Wi-Fi OFF before calibration *"
             echo " ****************************************"
             svc wifi disable
             rmmod $WL12xx_MODULE;;
          *) echo " ******************************"
             echo " * Starting Wi-Fi calibration *"
             echo " ******************************";;
esac

if [ -e $WL12xx_MODULE ];
then
    echo ""
else
    echo "********************************************************"
    echo "* wl12xx_sdio module not found !!"
    echo "********************************************************"
    exit
fi

# Remount system partition as rw
mount -o remount rw /system

# Fresh install or update copy over the generic nvs
cp $TARGET_FW_DIR/wl1271-nvs_128x.bin $TARGET_NVS_FILE

# Set the MAC address if nvs_map exists in pds
if [ -e $PDS_NVS_FILE ];
then
    HW_MAC=`calibrator get nvs_mac $PDS_NVS_FILE | grep -o -E "([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}"`
    calibrator set nvs_mac $TARGET_NVS_FILE $HW_MAC
else
    echo "********************************************************"
    echo "* /pds/wifi/nvs_map.bin not found !! Not setting MAC   *"
    echo "********************************************************"
fi

echo " ******************************"
echo " * Finished Wi-Fi calibration *"
echo " ******************************"
case "$WIFION" in
  "running") echo " *************************"
             echo " * Turning Wi-Fi back on *"
             echo " *************************"
             svc wifi enable;;
esac

# Remount system partition as ro
mount -o remount ro /system

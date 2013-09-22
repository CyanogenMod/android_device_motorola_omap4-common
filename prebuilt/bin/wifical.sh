#! /system/bin/sh

WIFION=`getprop init.svc.p2p_supplicant`

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

PDS_NVS_FILE=/pds/wifi/nvs_map.bin
TARGET_FW_DIR=/system/etc/firmware/ti-connectivity
TARGET_NVS_FILE=$TARGET_FW_DIR/wl1271-nvs.bin
TARGET_INI_FILE=/system/etc/wifi/wlan_fem.ini
WL12xx_MODULE=/system/lib/modules/wl12xx_sdio.ko

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

# Remove old NVS file
if [ -e $PDS_NVS_FILE ];
then
    if [ -e $TARGET_NVS_FILE ];
    then
        rm $TARGET_NVS_FILE
    fi

    # Actual calibration...
    # calibrator plt autocalibrate <dev> <module path> <ini file1> <nvs file> <mac addr>
    # Leaving mac address field empty for random mac
    HW_MAC=`calibrator get nvs_mac /pds/wifi/nvs_map.bin | grep -o -E "([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}"`
    calibrator plt autocalibrate wlan0 $WL12xx_MODULE $TARGET_INI_FILE $TARGET_NVS_FILE $HW_MAC
else
    echo "********************************************************"
    echo "* /pds/wifi/nvs_map.bin not found !! Using generic NVS *"
    echo "********************************************************"
    cp $TARGET_FW_DIR/wl1271-nvs.bin_generic $TARGET_NVS_FILE
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

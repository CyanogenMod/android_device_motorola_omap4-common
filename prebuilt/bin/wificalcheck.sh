#! /system/bin/sh
# In some cases calibration is getting interupted before it finishes

if [ ! -f /system/etc/firmware/ti-connectivity/wl1271-nvs.bin -o ! -s /system/etc/firmware/ti-connectivity/wl1271-nvs.bin ];
    then
        rmmod wl12xx_sdio

        # Migrate config
        if [ -e /data/misc/wifi/p2p_supplicant.conf -a -e /data/misc/wifi/wpa_supplicant.conf ];
            then
                if ! grep -q '^network' /data/misc/wifi/wpa_supplicant.conf &&
                     grep -q '^manufacturer=TI' /data/misc/wifi/wpa_supplicant.conf &&
                     grep -q '^network' /data/misc/wifi/p2p_supplicant.conf &&
                   ! grep -q '^manufacturer=TI' /data/misc/wifi/p2p_supplicant.conf &&
                     grep -q '^p2p_disabled=1' /data/misc/wifi/p2p_supplicant.conf;
                    then
                        mv /data/misc/wifi/p2p_supplicant.conf /data/misc/wifi/wpa_supplicant.conf
                fi
        fi

        wifical.sh
fi

#!/system/bin/sh

route="`/system/bin/getprop ril.motoril.route`"
if [ "${route}" = "" ]; then
	echo "property empty"
	exit 1
fi

if=${route%:*}
gw=${route#*:}

echo /system/bin/ip route add "${gw}" dev "${if}" table local
/system/bin/ip route add "${gw}" dev "${if}" table local

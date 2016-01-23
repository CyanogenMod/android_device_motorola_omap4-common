#!/system/bin/sh

if [ "${0}" = "/system/bin/battd.sh" ]; then
	exec "/system/bin/battd" "${@}"
elif [ "${0}" = "/system/bin/msp430.sh" ]; then
	exec "/system/bin/msp430" "${@}"
elif [ "${0}" = "/system/bin/thermaldaemon.sh" ]; then
	exec "/system/bin/thermaldaemon" "${@}"
elif [ "${0}" = "/system/bin/whisperd.sh" ]; then
	exec "/system/bin/whisperd" "${@}"
elif [ "${0}" = "/system/bin/akmd8975.sh" ]; then
	exec "/system/bin/akmd8975" "${@}"
fi

exit 1

#!/bin/sh
#
# Sample script to start access point on settop box
#

me="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

ETH_IF=eth0                     # ethernet interface name
WL0_IF=eth1                     # first wifi interface name

WIFI_DRIVER_DIR=/lib/modules    # path to wifi driver module
WIFI_AP_GUI_DIR=/www            # path to AP GUI web pages
WIFI_AP_NVRAM_DIR=/etc          # path to AP NVRAM file

echo "$me: Starting access point."

cd ${WIFI_DRIVER_DIR}; insmod emf.ko; cd -
cd ${WIFI_DRIVER_DIR}; insmod igs.ko; cd -

# Load module for NIC
cd ${WIFI_DRIVER_DIR}; insmod wl.ko; cd -
sleep 2

killall udhcpc
ifconfig ${ETH_IF} 0.0.0.0

if [ -f ${WIFI_AP_NVRAM_DIR}/nvrams_ap_current.txt ]; then
        nvram_initfile="${WIFI_AP_NVRAM_DIR}/nvrams_ap_current.txt"
elif [ -f ${WIFI_AP_NVRAM_DIR}/nvrams_ap_backup.txt ]; then
        nvram_initfile="${WIFI_AP_NVRAM_DIR}/nvrams_ap_backup.txt"
else
        nvram_initfile="${WIFI_AP_NVRAM_DIR}/nvrams_ap_default.txt"
fi

#
# If restore_defaults is set, use default nvram.
#
if grep -q "^restore_defaults=1" $nvram_initfile; then
        if [ $nvram_initfile == "${WIFI_AP_NVRAM_DIR}/nvrams_ap_default.txt" ]; then
                echo "$me: Ignored restore_defaults because it is set in default nvram file"
        else
                echo "$me: restore_defaults is set in $nvram_initfile"
                rm -f ${WIFI_AP_NVRAM_DIR}/nvrams_ap_current.txt
                rm -f ${WIFI_AP_NVRAM_DIR}/nvrams_ap_backup.txt
                nvram_initfile="${WIFI_AP_NVRAM_DIR}/nvrams_ap_default.txt"
        fi
fi

nvramd -i $nvram_initfile -c ${WIFI_AP_NVRAM_DIR}/nvrams_ap_current.txt -b ${WIFI_AP_NVRAM_DIR}/nvrams_ap_backup.txt
sleep 2

if [ -e /sbin/hotplug ]; then
        :
else
        ln -s /sbin/rc /sbin/hotplug
        if [ $? -ne 0 ]; then
                # a workaround for filesystem that does not support symbolic link (e.g. FAT)
                echo "$me: cannot create symbolic link hotplug -> rc.  Making a copy instead."
                cp /sbin/rc /sbin/hotplug
        fi
fi

rc init
rc start

exit 0

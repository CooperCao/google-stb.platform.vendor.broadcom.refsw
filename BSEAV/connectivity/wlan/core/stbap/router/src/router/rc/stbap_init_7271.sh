#!/bin/sh
#
# Sample script to start access point on settop box
#

me="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

ETH_IF=eth0                     # ethernet interface name
WL0_IF=eth1                     # first wifi interface name
WL1_IF=eth2                     # second wifi interface name

WIFI_DRIVER_DIR=/lib/modules    # path to wifi driver module
WIFI_AP_GUI_DIR=/www            # path to AP GUI web pages
WIFI_AP_NVRAM_DIR=/etc          # path to AP NVRAM file
WIFI_WL_IDX=0
WIFI_WL_CHIP=()
WIFI_WL_FD[0]=n
WIFI_WL_FD[1]=n
WIFI_WL_PCINAME=()
WIFI_WL_CHIPTBL=(
#	device id	chip name	fd
	"43c3 (rev 03)"	"bcm4366b1"	1 \
	"43c3 (rev 04)" "bcm4366c0"	1 \
	"43c5 (rev 04)" "bcm4366c0"	1 \
	"4365 (rev 04)" "bcm4366c0"	1 \
	"43ba (rev 01)"	"bcm43602a1"	1 \
	"43ba (rev 03)"	"bcm43602a3"	1 \
	"43bb (rev 03)"	"bcm43602a3"	1 \
	"43bc (rev 01)" "bcm43602a1"	1 \
	"43bc (rev 03)" "bcm43602a3"	1 \
	"43a0"		"bcm4360b"	0 \
	"43a1"		"bcm4360b"	0 \
	"43a2"		"bcm4360b"	0 \
	"4360"		"bcm4360b"	0 \
	"7271 (rev 10)" "bcm7271b0"     0 \
)

echo "$me: Starting access point."

old_IFS=$IFS
IFS=$'\n'
PCIDEVS=($(lspci))
for PCIDEV in ${PCIDEVS[@]}; do
        PCINAME=$(echo "${PCIDEV%% *}" | sed 's/:/_/g;s/\./_/g')
        PCIDEVID="${PCIDEV##*Device }"
	for ((i = 0; i < ${#WIFI_WL_CHIPTBL[@]}; i += 3)); do
	        if [[ "$PCIDEVID" = *${WIFI_WL_CHIPTBL[i]}* ]]; then
                        WIFI_WL_CHIP[$WIFI_WL_IDX]=${WIFI_WL_CHIPTBL[i+1]}
                        WIFI_WL_FD[$WIFI_WL_IDX]=${WIFI_WL_CHIPTBL[i+2]}
                        WIFI_WL_PCINAME[$WIFI_WL_IDX]=$PCINAME
                        echo "WIFI_WL${WIFI_WL_IDX}_CHIP=${WIFI_WL_CHIP[$WIFI_WL_IDX]} WIFI_WL${WIFI_WL_IDX}_PCINAME=${WIFI_WL_PCINAME[$WIFI_WL_IDX]} WIFI_WL${WIFI_WL_IDX}_FD=${WIFI_WL_FD[$WIFI_WL_IDX]}"
                        WIFI_WL_IDX=`expr $WIFI_WL_IDX + 1`
                        break
	        fi
	done
done
IFS=${old_IFS}

echo "pci devices:"
lspci

# NVRAM for WIFI_WL_CHIP
if [[ "${WIFI_WL_FD[0]}" = "1" && "${WIFI_WL_FD[1]}" = "1" ]]; then
        if [[ ! -f /lib/firmware/brcm/${WIFI_WL_CHIP[0]}.nvm || \
                -z $(sed -n '/wlunit/p' /lib/firmware/brcm/${WIFI_WL_CHIP[0]}.nvm) ]]; then
                echo "wlunit=0" >> /lib/firmware/brcm/${WIFI_WL_CHIP[0]}.nvm
        fi
        if [[ ! -f /lib/firmware/brcm/${WIFI_WL_CHIP[1]}.nvm || \
                -z $(sed -n '/wlunit/p' /lib/firmware/brcm/${WIFI_WL_CHIP[1]}.nvm) ]]; then
                echo "wlunit=1" >> /lib/firmware/brcm/${WIFI_WL_CHIP[1]}.nvm
        fi
elif [[ "${WIFI_WL_FD[0]}" = "1" ]]; then
        if [[ ! -f /lib/firmware/brcm/${WIFI_WL_CHIP[0]}.nvm || \
                -z $(sed -n '/wlunit/p' /lib/firmware/brcm/${WIFI_WL_CHIP[0]}.nvm) ]]; then
                echo "wlunit=0" >> /lib/firmware/brcm/${WIFI_WL_CHIP[0]}.nvm
        fi
elif [[ "${WIFI_WL_FD[1]}" = "1" ]]; then
        if [[ ! -f /lib/firmware/brcm/${WIFI_WL_CHIP[1]}.nvm || \
                -z $(sed -n '/wlunit/p' /lib/firmware/brcm/${WIFI_WL_CHIP[1]}.nvm) ]]; then
                echo "wlunit=0" >> /lib/firmware/brcm/${WIFI_WL_CHIP[1]}.nvm
        fi
fi


cd ${WIFI_DRIVER_DIR}; insmod emf.ko; cd -
cd ${WIFI_DRIVER_DIR}; insmod igs.ko; cd -
cd ${WIFI_DRIVER_DIR}; insmod dpsta.ko; cd -

# Load module for Full Dongle
if [[ "${WIFI_WL_FD[0]}" = "1" || "${WIFI_WL_FD[1]}" = "1" ]]; then
        insmod ${WIFI_DRIVER_DIR}/dhd_secdma.ko secdma_addr=0x60000000 secdma_size=0x10b0000
        sleep 2

	if [ "${WIFI_WL_FD[0]}" = "1" ]; then
                wl -i ${WL0_IF} up
                sleep 1
                ifconfig ${WL0_IF} up
                sleep 1
        fi

        if [ "${WIFI_WL_FD[1]}" = "1" ]; then
                wl -i ${WL1_IF} up
                sleep 1
                ifconfig ${WL1_IF} up
                sleep 1
        fi
fi

# Load module for NIC
if [[ "${WIFI_WL_FD[0]}" = "0" || "${WIFI_WL_FD[1]}" = "0" ]]; then
        if [[ "${WIFI_WL_FD[0]}" = "1" || "${WIFI_WL_FD[1]}" = "1" ]]; then
                cd ${WIFI_DRIVER_DIR}; insmod wlplat.ko; cd -
                cd ${WIFI_DRIVER_DIR}; insmod wl.ko instance_base=1; cd -
        else
                cd ${WIFI_DRIVER_DIR}; insmod wlplat.ko; cd -
                cd ${WIFI_DRIVER_DIR}; insmod wl.ko; cd -
                sleep 1

		# Dual NIC driver for 4360, assume 4360 NIC driver name wlnic.ko
		if [[ "${WIFI_WL_FD[0]}" = "0" && "${WIFI_WL_FD[1]}" = "0" ]]; then
                        cd ${WIFI_DRIVER_DIR}; insmod wlnic.ko instance_base=1; cd -
	        fi
        fi
        sleep 2
fi

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

if [ ! -e /sbin/hotplug ]; then
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

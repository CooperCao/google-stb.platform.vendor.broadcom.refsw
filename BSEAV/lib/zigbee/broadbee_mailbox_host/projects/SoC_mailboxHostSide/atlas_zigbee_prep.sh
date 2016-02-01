#!/bin/sh
mknod /dev/zigbee c 105 0
insmod zigbee_drv.ko
SoC_mailboxHost_pid=`ps -eo pid,command | grep SoC_mailboxHost.elf | grep -v grep | awk '{print $1}'`
if [ "$SoC_mailboxHost_pid" == "" ]; then
    SoC_mailboxHost.elf broadbee_zrc11_target.bin &
else
    echo "SoC_mailboxHost.elf is running already!!!"
fi
mkdir /etc/zigbee

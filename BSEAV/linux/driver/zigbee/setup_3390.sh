#!/bin/bash
mknod /dev/zigbee c 105 0
insmod ../../../../obj.93390/BSEAV/linux/driver/zigbee/arm-linux/zigbee_drv.ko

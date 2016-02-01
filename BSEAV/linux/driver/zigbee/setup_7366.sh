#!/bin/bash
mknod /dev/zigbee c 105 0
insmod ../../../../obj.97366/BSEAV/linux/driver/zigbee/arm-linux/zigbee_drv.ko

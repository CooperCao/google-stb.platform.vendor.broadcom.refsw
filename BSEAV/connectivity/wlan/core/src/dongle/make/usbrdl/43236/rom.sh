#!/bin/sh
# based on usbrdl.exe, generate srec and quickturn codefile(chip specific rom size and bank size

arm-none-eabi-objcopy -O srec --srec-forceS3 usbrdl.exe H0E_L65RMHD16384X32R611VXVTSS.srec
srec2mem H0E_L65RMHD16384X32R611VXVTSS.srec 32
perl /projects/hnd_cores/gallery/bin/hnd_hex2bin.pl H0E_L65RMHD16384X32R611VXVTSS.mem -rom_size 262144 -rom_bank_size 65536 -convert_2_little

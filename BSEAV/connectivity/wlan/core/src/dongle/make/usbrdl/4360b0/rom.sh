#!/bin/sh
arm-none-eabi-objcopy -O srec --srec-forceS3  -R .reginfo -R .note -R .comment -R .mdebug -S usbrdl.exe H3E_L40RMHS4096X64R512COVTSSY.srec

/projects/hnd/tools/linux/bin/srec2mem_arm H3E_L40RMHS4096X64R512COVTSSY.srec -b 0xf0000 64

perl /projects/hnd_cores/gallery/bin/hnd_hex2bin.pl  H3E_L40RMHS4096X64R512COVTSSY.mem -rom_size 65536  -rom_bank_size 32768 -width 64 -fn_lead_zero 2 -convert_2_little
ls *.mem

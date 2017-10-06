# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

# Shared resources
source utfconf/stb01.tcl

set UTF::SummaryDir /projects/hnd_video7/USERS/utf/stb01a
set UTF::projswbuild /projects/hnd_video7/USERS/NIGHTLY

A configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    A attn default
}

package require UTF::STB
### REF
### [DK: check pwr port(x), usb console port??]
UTF::STB stbutf01tst1 -sta {4366 eth1} \
    -console stbutf01end1:40000 \
    -power {pwr 1} \
    -power_sta {pwr 1} -reloadoncrash 1 \
    -dhd_tag "EAGLE_BRANCH_10_10" \
    -dongleimage  "wl.ko" \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-armv7l" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl vht_features 7}

# UTF::Linux stbutf01tst1 -sta {4366 enp1s0} \
#    -console stbutf01end1:40000 \
#    -power {pwr 2} -reloadoncrash 1 \
#    -tag "EAGLE_BRANCH_10_10" \
#    -wlinitcmds {wl msglevel +assoc;wl dtim 3;wl down;wl bw_cap 2g -1;wl vht_features 7}
    
    
4366 configure -attngrp A -ipaddr 192.168.1.80 -lanpeer {4366 4366}

package require UTF::STB
### DUT
### [DK: check pwr port(X), usb console port??, attn ports, tftpserver, pad]
UTF::STB stbutf01tst2 -sta {7271 eth1} \
    -console stbutf01end1:40001 \
    -power {pwr 2} \
    -power_sta {pwr 2} -reloadoncrash 1 \
    -dongleimage  "wl.ko" \
    -tftpserver UTFTestO \
    -nvram components/nvram/bcm97271wlan.txt \
    -datarate {-i 0.5} \
    -yart {-attn5g 8-95 -attn2g 8-95 -pad 46} \
    -tcpwindow 4m -udp 1.7g -slowassoc 5 \
    -perfchans {36/80 3} \
    -channelsweep {-sort 1 -i 4}

#   -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 7}
# 7271 specifics: wl mpc 0;wl PM 0;wl band a;wl infra 1;wl vht_features 0x6;wl frameburst 1

7271 configure -ipaddr 192.168.1.99

7271 clone 15_10 -dhd_tag STB7271_BRANCH_15_10 -tag STB7271_BRANCH_15_10 -app_tag STB7271_BRANCH_15_10 \
    -brand linux-internal-stbsoc \
    -type "debug-apdef-stadef-extnvm-slvradar-stbsoc-armv8" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl assert_type 0;wl vht_features 7}

###

UTF::Q stb01a

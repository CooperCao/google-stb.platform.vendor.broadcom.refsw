# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

# Shared resources
source utfconf/stb01.tcl

set UTF::SummaryDir /projects/hnd_video7/USERS/utf/stb01c
set UTF::projswbuild /projects/hnd_video7/USERS/NIGHTLY

C configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    C attn default
}

package require UTF::STB
### REF
UTF::STB stbutf01tst5 -sta {7271ap eth1} \
    -console stbutf01end1:40004 \
    -power {pwr 5} \
    -power_sta {pwr 5} -reloadoncrash 1 \
    -dhd_tag STB7271_BRANCH_15_10 -tag STB7271_BRANCH_15_10 -app_tag STB7271_BRANCH_15_10 \
    -dongleimage  "wl.ko" \
    -brand linux-internal-stbsoc \
    -type "debug-apdef-stadef-extnvm-mfp-wet-pspretend-stbsoc-armv8" \
    -nvram components/nvram/bcm97271wlan.txt \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl vht_features 7}

### remove lanpeer hack (dual iperf)
7271ap configure -attngrp C -ipaddr 192.168.1.80

package require UTF::STB
### DUT
UTF::STB stbutf01tst6 -sta {7271 eth1} \
    -console stbutf01end1:40005 \
    -power {pwr 6} \
    -power_sta {pwr 6} -reloadoncrash 1 \
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

UTF::Q stb01c

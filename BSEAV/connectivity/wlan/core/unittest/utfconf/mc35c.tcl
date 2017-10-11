# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

# Shared resources
source utfconf/mc35.tcl

set UTF::SummaryDir /projects/hnd_video7/USERS/utf/mc35c
set UTF::projswbuild /projects/hnd_video7/USERS/NIGHTLY

C configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    C attn default
}

### REF

UTF::Linux mc16tst2 -sta {4366 enp1s0} \
    -console sr1end09:40005 \
    -power {mc16ips1 6} -reloadoncrash 1 \
    -tag "EAGLE_BRANCH_10_10" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl vht_features 7}

4366 configure -attngrp C -ipaddr 192.168.1.80 -lanpeer {4366 4366}

### DUT

package require UTF::STB
UTF::STB mc16tst1 -sta {7271 eth1} \
    -console sr1end09:40006 \
    -power {mc16ips1 5} \
    -power_sta {mc16ips1 2} -reloadoncrash 1 \
    -tftpserver UTFTestO -dongleimage  "wl.ko" \
    -nvram components/nvram/bcm97271wlan.txt \
    -datarate {-i 0.5} \
    -yart {-attn5g 8-95 -attn2g 8-95 -pad 46} \
    -tcpwindow 4m -udp 1.7g -slowassoc 5 \
    -perfchans {36/80 3} \
    -channelsweep {-sort 1 -i 4}

#   -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 7}
# 7271 specifics: wl mpc 0;wl PM 0;wl band a;wl infra 1;wl vht_features 0x6;wl frameburst 1

7271 configure -ipaddr 192.168.1.99

###  BCM7271_BRANCH_1_1 no band lock

7271 clone 15_10 -dhd_tag STB7271_BRANCH_15_10 -tag STB7271_BRANCH_15_10 -app_tag STB7271_BRANCH_15_10 \
    -brand linux-internal-stbsoc \
    -type "debug-apdef-stadef-extnvm-slvradar-stbsoc-armv7l" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl assert_type 0;wl vht_features 7}

###

UTF::Q mc35c

# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

source utfconf/mc35.tcl

set UTF::SummaryDir /projects/hnd_sig/utf/mc35b

B configure -default 25

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    B attn default
}

### REF

set UTF::PerfCacheMigrate {" 43602" " 4360" "\{43602" "\{4360"}

UTF::Linux mc35tst4 -sta {4360 enp1s0} \
    -console sr1end09:40004 \
    -power {mc16ips1 4} \
    -tag "BISON05T_BRANCH_7_35" \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3}

4360 configure -attngrp B -ipaddr 192.168.1.80 -lanpeer {4360 4360}

### DUT

package require UTF::STB
UTF::STB mc35tst3 -sta {7425/4360 eth2} \
    -console sr1end09:40003 \
    -power {mc16ips1 3} \
    -power_sta {mc16ips1 3} \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-stb-mips" \
    -tftpserver UTFTestO -dongleimage  "wl.ko" \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 22-95 -attn2g 22-95 -pad 46 -frameburst 1} \
    -tcpwindow 4m -udp 1.2g -slowassoc 5 -perfchans {36/80 3}

7425/4360 configure -ipaddr 192.168.1.99

7425/4360 clone 7425/4360b14 -dhd_tag BISON04T_BRANCH_7_14

###

UTF::Q mc35b

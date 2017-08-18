# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: d15bde92679d2b6d2f0edccb962655911435cb20 $
#

# Shared resources
source utfconf/mc35.tcl

set UTF::SummaryDir /projects/hnd_sig/utf/mc35a

A configure -default 25

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    A attn default
}

### REF

#    -lanpeer {43602 43602}

UTF::Linux mc35tst1 -sta {43602 enp1s0} \
    -console sr1end09:40000 \
    -power {mc16ips1 1} \
    -tag "BISON05T_BRANCH_7_35" -date 2015.1.20.1 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3}

43602 configure -attngrp A -ipaddr 192.168.1.80

### DUT

package require UTF::STB
UTF::STB mc35tst2 -sta {7445/43602 eth1} \
    -console sr1end09:40002 \
    -power {mc16ips1 2} \
    -power_sta {mc16ips1 2} -reloadoncrash 1 \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-stb-armv7l" \
    -tftpserver UTFTestO -dongleimage  "wl.ko" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 28-95 -attn2g 28-95 -pad 40 -frameburst 1} \
    -tcpwindow 4m -udp 1.2g -slowassoc 5 -iperfdaemon 0 \
    -perfchans {36/80 3}

7445/43602 configure -ipaddr 192.168.1.99 -lanpeer {7445/43602 7445/43602}

7445/43602 clone 7445/43602n -dhd_tag NANDP_BRANCH_1_2
7445/43602n configure -lanpeer {7445/43602 7445/43602}

7445/43602 clone 7445/43602f \
    -dongleimage 43602a1-ram/pcie-ag-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux/rtecdc.bin \
    -type dhd-msgbuf-pciefd-mfp-armv7l-debug \
    -noaes 1 -notkip 1

7445/43602f clone 7445/43602fx \
    -dongleimage 43602a1-ram/pcie-ag-err-splitrx/rtecdc.bin \
    -perfchans {36/80} -perfonly 1

7445/43602f clone 7445/43602fb35 \
    -tag BISON05T_{REL,BRANCH}_7_35{,_???} \
    -dhd_tag DHD_BRANCH_1_363 \
    -type dhd-msgbuf-pciefd-mfp-armv7l-debug \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-err-assert-dbgam.bin" \
    -dhd_brand linux-internal-media

7445/43602fb35 clone 7445/43602fb35x \
    -brand linux-external-media \
    -dhd_tag DHD_BRANCH_1_363 \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive.bin" \
    -perfchans {36/80} -perfonly 1

###

UTF::Q mc35a

# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for mc41c testbed
#

source utfconf/mc41shared.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc41c"

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    C attn default
}

##############

UTF::Linux mc41tst4 -sta {4360c enp1s0} \
    -console {mc41end1:40011} \
    -power {nb1 6} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
    -tcpwindow 4
4360c configure -ipaddr 192.168.1.92 -attngrp C -hasdhcpd 1 -ap 1

#######################################

# Note the board is actually a p159, but later nvrams are only in
# trunk

package require UTF::STB
UTF::STB mc41tst10 -sta {7448/43569 eth1} \
    -console mc41end1:40008 \
    -rteconsole mc41end1:40000 \
    -power {nb1 4} -power_sta {nb1 4} \
    -node 0xbd27 \
    -brand linux-internal-media \
    -dhd_brand linux-internal-media \
    -type "dhd-cdc-usb-comp-gpl-armv7l-debug" \
    -tftpserver UTFTestO \
    -slowassoc 5 \
    -dhd_tag DHD_BRANCH_1_363 \
    -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
    -dongleimage "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx" \
    -nvram bcm943569usbir_p156.txt \
    -yart {-attn5g 27-95 -attn2g 37-95 -pad 39} \
    -tcpwindow 640k -udp 320m \
    -wlinitcmds {wl down;wl bw_cap 2g -1;wl vht_features 3}

7448/43569 configure -ipaddr 192.168.1.93

7448/43569 clone 7448/43569x \
    -brand linux-external-media \
    -dongleimage "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \
    -perfonly 1 -perfchans {36/80}

7448/43569 clone 7448/43526 \
    -rteconsole mc41end1:40003 \
    -node 0xbd1d \
    -dhd_tag {} -tag trunk \
    -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-mfp-high-media-stb-armv7l" \
    -dongleimage "43526b-bmac/ag-assert-p2p-mchan-media-txbf.bin.trx" \
    -nvram bcm943526usb_p452.txt \
    -nobighammer 1 -modopts {assert_type=1}

# No performance advantage to testing external 43526
7448/43526 clone 7448/43526x \
    -brand linux-external-media \
    -dhd_brand linux-external-media \
    -type "nodebug-apdef-stadef-p2p-mchan-tdls-wowl-mfp-high-stb-armv7l" \
    -dongleimage "43526b-bmac/ag-p2p-mchan-media-txbf.bin.trx" \
    -perfonly 1 -perfchans {36/80}

7448/43526 clone 7448/43526b14 -tag BISON04T_BRANCH_7_14 \
    -dongleimage "43526b-bmac/ag-assert-p2p-mchan-media-txbf.bin.trx"

#####
UTF::Q mc41c

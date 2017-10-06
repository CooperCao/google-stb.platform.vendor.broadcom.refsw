# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for MC41testbed
#

source "utfconf/mc40shared.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc40b"

B configure -default 10

set UTF::SetupTestBed {
    B attn default
    foreach S {4360 7448/43570n} {
	catch {$S wl down}
	$S deinit
    }
    return
}

#####################

# SoftAP
UTF::Linux mc40tst4 -sta {4360 enp1s0} \
    -reloadoncrash 1 \
    -console mc40end1:40000 \
    -power {nb 4} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl vht_features 3}

4360 configure -attngrp B -ipaddr 192.168.1.80

#################################

UTF::Linux mc40tst2 -sta {43570n enp1s0} \
    -type debug-apdef-stadef-extnvm \
    -power {ws 1} \
    -console {mc40end1:40006} \
    -slowassoc 5 -reloadoncrash 1 \
    -nvram bcm943570pcieir_p150.txt \
    -nvram bcm943570pcie_p204.txt \
    -datarate {-i 0.5} -udp 800m \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl bw_cap 2g -1;wl vht_features 3} \
    -tcpwindow 3M -yart {-pad 25 -attn5g 25-95 -attn2g 38-95}

43570n configure -ipaddr 192.168.1.81

43570n clone 43570nb -tag BISON_BRANCH_7_10
43570n clone 43570nb35 -tag BISON05T_BRANCH_7_35
43570n clone 43570n143 -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
    -brand linux-internal-media \
    -yart {-pad 25 -attn5g 0-95 -attn2g 0-95}

###

UTF::DHD mc40tst2d -lan_ip mc40tst2 -sta {43570 eth0} \
    -hostconsole {mc40end1:40006} \
    -power {ws 1} \
    -tag BISON_BRANCH_7_10 \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -brand linux-external-media \
    -nvram bcm943570pcieir_p150.txt \
    -type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert/rtecdc.bin \
    -wlinitcmds {wl vht_features 3} \
    -slowassoc 5 -extsup 1 \
    -datarate {-i 0.5} \
    -tcpwindow 3m -udp 800m -nocal 1 \
    -docpu 1 \
    -yart {-pad 25 -attn5g 25-95 -attn2g 38-95} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43570 configure -ipaddr 192.168.1.81

43570 clone 43570x \
    -type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds/rtecdc.bin \
    -perfonly 1 -perfchans {36/80}

43570  clone 43570b35  -tag BISON05T_BRANCH_7_35 \
    -type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert/rtecdc.bin

43570x clone 43570b35x -tag BISON05T_BRANCH_7_35 \
    -type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds/rtecdc.bin

43570 clone 43570b143 -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
    -brand linux-internal-media \
    -type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert.bin
43570x clone 43570b143x -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
    -type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds.bin

####
# STB
###

package require UTF::STB
UTF::STB mc40end2 -sta {7448/43570n eth1} \
    -console mc40end1:40007 \
    -power {nb 5} -reloadoncrash 1 \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-extnvm-stb-armv7l" \
    -tftpserver UTFTestO -dongleimage  "wl.ko" \
    -datarate {-i 0.5} \
    -yart {-pad 25 -attn5g 25-95 -attn2g 38-95} \
    -tcpwindow 4m -udp 1.2g -slowassoc 5 \
    -perfchans {36/80 3} \
    -channelsweep {-no2g40} \
    -nvram bcm43570_bcm97448lcc_v10.txt \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl bw_cap 2g -1;wl vht_features 3}

7448/43570n configure -ipaddr 192.168.1.82

7448/43570n clone 7448/43570n35 -dhd_tag BISON05T_BRANCH_7_35
7448/43570n clone 7448/43570n143 -dhd_tag BISON05T_{TWIG,REL}_7_35_143{,_*}

7448/43570n clone 7448/43570f \
    -dongleimage 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert.bin \
    -type dhd-msgbuf-pciefd-mfp-armv7l-debug \
    -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
    -dhd_tag DHD_BRANCH_1_363 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 3}


7448/43570f clone 7448/43570fx \
    -brand linux-external-media \
    -dhd_brand linux-internal-media \
    -dongleimage 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds.bin \
    -perfchans {36/80} -perfonly 0

#####
UTF::Q mc40b

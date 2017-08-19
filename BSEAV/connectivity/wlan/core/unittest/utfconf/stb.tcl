# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: e72fd8e5a1e8f9b78836106e23b53b5b8a40ac7d $
#

######
#
# Local UTF configuration
#

# Shared resources
source utfconf/ramsey_shared.tcl

set ::UTF::SummaryDir "/projects/hnd_sig/utf/stb"

# Controller
UTF::Linux UTFTestD

UTF::Linux UTFTestO -sta {lan eth1 4360 eth2} \
    -tag AARDVARK_BRANCH_6_30 \
    -modopts assert_type=1 \
    -wlinitcmds {wl down; wl vht_features 1}
lan configure -ipaddr 192.168.1.50
4360 configure -ipaddr 192.168.2.50 -hasdhcpd 1 -ap 1


UTF::Linux dhcp7-sv1-165 -sta {4352 enp1s0} \
    -tag BISON05T_BRANCH_7_35 \
    -console "UTFTestC:40002" \
    -wlinitcmds {wl down;wl country US;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3}

4352 configure  -ipaddr 192.168.2.60 -hasdhcpd 1 -ap 1 -attngrp R7


# Local WebSwitch attached to AP, used to control power for the STBs
UTF::Power::WebSwitch stbpower -relay UTFTestO -lan_ip 192.168.1.60

set ::UTF::SetupTestBed {
    R7 attn default
    foreach S {7425/43526 43526} {
	catch {$S wl down}
	$S deinit
    }
}

package require UTF::STB
UTF::STB UTFTestE -sta {7425/43242 eth2} \
    -ssh ush \
    -brand linux-internal-media \
    -node 0xbd1f \
    -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-mfp-high-media-stb-mips" \
    -nvram "bcm943242usbref_p665.txt" \
    -dongleimage "43242a1-bmac/ag-assert-p2p-mchan-media-srvsdb-vusb.bin.trx" \
    -tftpserver UTFTestO \
    -console UTFTestO:40000 \
    -rteconsole UTFTestO:40008 \
    -power {stbpower 1} -power_sta {stbpower 1} \
    -slowassoc 5 -datarate {-i 0.5} -udp 300m \
    -yart {-attn5g 10-95 -attn2g 36-95 -frameburst 1 -pad 29}


7425/43242 clone 7425x/43242 \
    -brand linux-external-media \
    -dongleimage "43242a1-bmac/ag-p2p-mchan-media-srvsdb-vusb.bin.trx" \
    -type "nodebug-apdef-stadef-p2p-mchan-tdls-wowl-mfp-high-stb-mips" \
    -perfonly 1 -perfchans {36l}

# SoftAP
7425/43242 clone 7425/43242ap -apmode 1 \
    -noaes 1 -notkip 1 -nochannels 1 -nopm1 1 -nopm2 1
7425x/43242 clone 7425x/43242ap -apmode 1 \
    -noaes 1 -notkip 1 -nochannels 1 -nopm1 1 -nopm2 1

7425/43242 clone 7425/43242f \
    -dongleimage 43242a1-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-srvsdb-pclose-proptxstatus-vusb.bin.trx \
    -nvram "bcm943242usbref_p461.txt" \
    -type dhd-cdc-usb-comp-gpl-mips-debug \
    -dhd_tag DHD_BRANCH_1_363 \
    -tag PHOENIX2_REL_6_10_224_23 \
    -brand linux-external-dongle-usb \
    -app_brand linux-internal-media \
    -dhd_brand linux-internal-media

7425/43242f configure -wl wlmips26 -perfonly 1


# 4360 NIC mode
7425/43242 clone 7425/4360 \
    -type "debug-apdef-stadef-stb-mips" \
    -nvram "" -dongleimage "wl.ko" -rteconsole "" \
    -tcpwindow 4m -udp 1.2g -perfchans {36/80 3}
7425/4360 clone 7425/4360b14 -dhd_tag BISON04T_BRANCH_7_14




7425/43242 clone 7425/43526 \
    -rteconsole UTFTestO:40005 \
    -node 0xbd1d \
    -dongleimage "43526b-bmac/ag-assert-p2p-mchan-media-txbf.bin.trx" \
    -nvram bcm943526usb_p452.txt \
    -udp 320m -perfchans {36/80 3} -nobighammer 1 \
    -wlinitcmds {wl down; wl vht_features 3}

7425/43526    clone 7425/43526b14  -tag BISON04T_BRANCH_7_14 \
    -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-high-media-stb-mips" \
    -dongleimage "43526b-bmac/ag-assert-p2p-mchan-media-txbf.bin.trx"


#7425/43526b14   configure -tdlswith 43526b14

7425x/43242 clone 7425x/43526 \
    -dongleimage "43526b-bmac/ag-p2p-mchan-media.bin.trx" \
    -perfchans {36/80}

7425/43242f clone 7425/43569 \
    -rteconsole UTFTestO:40009 \
    -node 0xbd27 \
    -brand linux-internal-media \
    -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
    -dongleimage "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx" \
    -nvram bcm943569usbir_p156.txt \
    -tcpwindow 640k -udp 220m -perfchans {36/80 3}

7425/43569 clone 7425/43569x \
    -brand linux-external-media \
    -dongleimage "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \
    -perfonly 1 -perfchans {36/80}

########################

# Relay/Controllers
UTF::Linux UTFTestU

UTF::Power::WebRelay hub -lan_ip 192.168.1.12 -relay UTFTestU

UTF::DHD UTFTestH -sta {43526 eth0} \
    -console {UTFTestC:40006} \
    -hostconsole {UTFTestC:40007} \
    -power_sta {hub 1} \
    -brand linux-internal-media \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
    -type "43526b-bmac/ag-assert-p2p-mchan-media-txbf.bin.trx" \
    -nvram bcm943526usb_p452.txt \
    -nvram_add {macaddr=00:90:4c:0e:61:11} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -perfchans {36/80 3} -nobighammer 1 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 1} \
    -nocal 1 -yart {-attn5g 22-95 -attn2g 40-95 -frameburst 1 -pad 23} \
    -msgactions {
	{toss: uni-cast not to me.} WARN
    }

43526 configure -attngrp R7

43526 clone 43526b14    -tag BISON04T_BRANCH_7_14 \
    -type "43526b-bmac/ag-assert-p2p-mchan-media-txbf-err.bin.trx"

43526 clone 43526b14x   -tag BISON04T_BRANCH_7_14 \
    -brand linux-external-media \
    -driver "nodebug-apdef-stadef-high-p2p-mchan-tdls-wowl-media" \
    -type "43526b-bmac/ag-p2p-mchan-media.bin.trx"

43526b14 clone 43526b1489   -tag BISON04T_TWIG_7_14_89
43526b14x clone 43526b1489x -tag BISON04T_TWIG_7_14_89


43526 clone 43526b35 -tag BISON05T_BRANCH_7_35


43526b14 clone 43526b14m \
    -brand linux-mfgtest-media \
    -dhd_brand linux-internal-media \
    -driver "debug-apdef-stadef-high-p2p-mchan-tdls-media" \
    -type 43526b-bmac/ag-mfgtest.bin.trx

####
UTF::Q stb UTFTestD

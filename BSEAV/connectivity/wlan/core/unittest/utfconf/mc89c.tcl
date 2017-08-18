# -*-tcl-*-
#
# Testbed MC89 sub-rig configuration file
# Filename: mc89c.tcl
# Charles Chai 02/27/2013
# 
# Hardware
#   MC89end1  : FC11 controller
#   MC89tst9  : FC15 softAP
#   MC89tst10 : FC15 softAP
#   MC89tst6  : FC15 STA
#   MC89tst11 : FC19 STA
#   #AP1       : Netgear R6300/4706, 4360/4331 (Took out on 02/05/2014)
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc89.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc89c"

# Set default to use wl from trunk; Use -app_tag to modify.
set UTF::TrunkApps 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G3 attn 0

    # Make sure all systems are deinit and down 
    foreach S {4350ap X52cap 4339 43569} {
        UTF::Try "$S Down" {
            catch {$S wl down}
            catch {$S deinit}
        }
    }
    # unset S so it doesn't interfere
    unset S

    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}


###########################################
# MC89c 
# 4339wlipa_P203 A0 SDIO - 11ac 1x1 (2/14/2014)
###########################################
UTF::DHD mc89tst6 \
        -lan_ip mc89tst6 \
        -sta {4339 eth0} \
        -hostconsole "mc89end1:40009" \
        -power {npctst1 2} \
        -power_button "auto" \
        -dhd_tag trunk \
        -brand linux-external-dongle-sdio \
        -dhd_brand linux-internal-dongle \
        -nvram bcm94339wlipa.txt \
        -nocal 1 \
	-notkip 1 -noaes 1 -nobighammer 1 \
	-tcpwindow 1152k -udp 400m \
        -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
        -datarate {-b 400m -i 0.5 -frameburst 1} \
        -postinstall {dhd -i eth0 txglomsize 10} \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4339 clone 4339a -tag AARDVARK_BRANCH_6_30 \
	-type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-pktctx-dmatxrc-ltecx-idsup-idauth-err-assert.bin

4339a clone 4339ax -perfonly 1 -perfchans {36/80} \
	-type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-pktctx-dmatxrc-ltecx-idsup-idauth.bin

4339a clone 4339a-iv -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf_bfe_cap 0;}
4339a clone 4339a-ih -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl vhtmode 0;}
4339a clone 4339a-il -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl vhtmode 0; wl nmode 0;}

4339 clone 4339t   -tag trunk -type 4339a0-ram/sdio-ag-p2p-assert/rtecdc.bin

4339t clone 4339t-iv -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf_bfe_cap 0;}
4339t clone 4339t-ih -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl vhtmode 0;}
4339t clone 4339t-il -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl vhtmode 0; wl nmode 0;}

4339 clone 4339f -tag FOSSA_BRANCH_11_10 \
	-type 4339a0-ram/sdio-ag-p2p-mchan-aoe-keepalive-pktfilter-proptxstatus-dmatxrc-pktctx-assert/rtecdc.bin
4339 clone 4339f1 -tag FOSSA_BRANCH_11_10 \
	-type 4339a0-ram/sdio-ag-p2p-proptxstatus-dmatxrc-pktctx-assert/rtecdc.bin
4339 clone 4339f2 -tag FOSSA_BRANCH_11_10 \
	-type 4339a0-ram/sdio-ag-p2p-assert/rtecdc.bin


###########################################
# MC89C
# 43569usbir - 11ac 2x2
###########################################
UTF::DHD mc89tst11 \
        -lan_ip mc89tst11 \
        -sta {43569 eth0} \
        -console "mc89end1:40013" \
        -power {npcap1 1} \
        -power_button "auto" \
    	-dhd_brand linux-internal-media \
    	-brand linux-internal-media \
    	-driver dhd-cdc-usb-gpl \
    	-nvram "bcm943569usbir_p157.txt" \
	-nvram_add {boardrev=0x1159} \
    	-slowassoc 5 -nocal 1 -docpu 1 \
    	-datarate {-i 0.5 -frameburst 1} \
    	-tcpwindow 640k -udp 800m \
    	-wlinitcmds {wl down; wl vht_features 3}

43569 clone 43569b35 \
	-tag BISON05T_BRANCH_7_35 \
	-type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx"

43569 clone 43569b35x \
	-tag BISON05T_BRANCH_7_35 \
	-perfonly 1 -perfchans {36/80} \
	-brand linux-external-media \
	-type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx"

# dev test
43569b35 clone 43569b35x1 \
	-wlinitcmds {wl down; wl vht_features 3; wl rxchain 1; wl txchain 1;}

43569 clone 43569test \
	-tag BISON05T_REL_7_35_143_80 \
	-brand linux-external-media \
        -dhd_tag DHD_REL_1_201_88_19 \
        -dhd_brand linux-external-media \
        -app_tag DHD_REL_1_201_88_19 \
	-app_brand linux-external-media \
	-driver "dhd-cdc-usb-comp-gpl" \
	-type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr-assert-err.bin.trx"


# 4360_P198 - 11ac 3x3
#UTF::Linux mc89tst11 \
        -lan_ip mc89tst11 \
        -sta {4360 eth0} \
        -console "mc89end1:40013" \
        -power {npcap1 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 4m -udp 1.2g \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi} {%S wl dump phycal}} \
        -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0;}

#4360 clone 4360b -tag BISON_BRANCH_7_10
#4360b clone 4360x2x \
	-perfonly 1 -perfchans {36/80} \
	-wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}
#4360b clone 4360bx1 \
	-wlinitcmds {wl msglevel +assoc; wl rxchain 1; wl txchain 1;}


###########################################
# MC89c
# AP section
###########################################
# 4350fp P370 C1 11ac 2x2 - calibrated
###########################################
UTF::Linux mc89tst9 \
        -lan_ip mc89tst9 \
        -sta {4350ap eth0} \
        -console "mc89end1:40001" \
	-power {npcap1 2} \
	-power_button "auto" \
	-tcpwindow 2m -udp 1.2g \
	-slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4350ap clone 4350softap-bis -tag BISON_BRANCH_7_10
4350softap-bis configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1

4350ap clone 4350softap-tot -tag trunk
4350softap-tot configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1

4350ap clone 4350softap-bis-bf1 -tag BISON_BRANCH_7_10
4350softap-bis-bf1 configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1
4350ap clone 4350softap-bis-bf0 -tag BISON_BRANCH_7_10
4350softap-bis-bf0 configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl down; wl country US/0; wl txbf 0; wl txbf_imp 0;}

4350ap clone 4350softap-egl-bf1 -tag EAGLE_BRANCH_10_10
4350softap-egl-bf1 configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1
4350ap clone 4350softap-egl-bf0 -tag EAGLE_BRANCH_10_10
4350softap-egl-bf0 configure -ipaddr 192.168.1.145 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl down; wl country US/0; wl txbf 0; wl txbf_imp 0;}


###########################################
# softAP
# (uninstalled) 4354wlsagbi_P101 A0 SDIO (moved from mc86c on 4/7/14)
# (installed) X52c 
###########################################
UTF::Linux mc89tst10 \
        -lan_ip mc89tst10 \
        -sta {X52cap eth0} \
        -console "mc89end1:40003" \
        -power {npctst1 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag BISON_BRANCH_7_10 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 800m \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl country US/0; wl vht_features 3;}

X52cap clone X52csoftap-bis -tag BISON_BRANCH_7_10
X52csoftap-bis configure -ipaddr 192.168.1.146 -attngrp G3 -ap 1 -hasdhcpd 1

X52cap clone X52csoftap-tot -tag trunk
X52csoftap-tot configure -ipaddr 192.168.1.146 -attngrp G3 -ap 1 -hasdhcpd 1

X52cap clone X52csoftap-bis-bf1 -tag BISON_BRANCH_7_10
X52csoftap-bis-bf1 configure -ipaddr 192.168.1.146 -attngrp G3 -ap 1 -hasdhcpd 1
X52cap clone X52csoftap-bis-bf0 -tag BISON_BRANCH_7_10
X52csoftap-bis-bf0 configure -ipaddr 192.168.1.146 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl down; wl txbf 0; wl txbf_imp 0;}

X52cap clone X52csoftap-egl-bf1 -tag EAGLE_BRANCH_10_10
X52csoftap-egl-bf1 configure -ipaddr 192.168.1.146 -attngrp G3 -ap 1 -hasdhcpd 1
X52cap clone X52csoftap-egl-bf0 -tag EAGLE_BRANCH_10_10
X52csoftap-egl-bf0 configure -ipaddr 192.168.1.146 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl down; wl txbf 0; wl txbf_imp 0;}

###
UTF::Q mc89c



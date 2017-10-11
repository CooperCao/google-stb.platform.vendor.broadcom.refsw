# -*-tcl-*-
#
# Testbed MC86 sub-rig configuration file
# Filename: mc86c.tcl
# Charles Chai 05/29/2013
#
# Hardware
#   MC86END1  : FC11 controller
#   MC86TST9  : FC19 softap 
#   MC86TST10 : FC19 STA
#   MC86TST11 : FC19 STA 
#   MC86END2  : FC19 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc86.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc86c"

# Set default to use wl from trunk; Use -app_tag to modify.
set UTF::TrunkApps 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G3 attn 0

    # Make sure all systems are deinit and down
    foreach S {43602ap 43602 43438a1 43430a1} {
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
# MC86C
# 43430wlselgs_P101 A0 - SDIO 11n 1x1 (ultra low power 2G20-only)
# 08/19/2014: Dima wrote: A0 sold >80 millions and it's still selling)
# 43438wlpth_P101 A1 - SDIO 11n 1x1 (ultra low power 2G20-only, even lower cost than 43430, selling to Huawei)
# 10/23/2014: received 43438a1 from Hongbin Tong (replaced 43430a0)
# (use ant J4)
###########################################
UTF::DHD mc86tst11 \
        -lan_ip mc86tst11 \
        -sta {43438a1 eth0} \
        -hostconsole "mc86end1:40012" \
        -power {npctst11 2} \
        -power_button "auto" \
	-dhd_tag DHD_REL_1_201_59 \
	-brand linux-external-dongle-sdio \
	-nvram bcm943438wlpth_26MHz.txt \
	-datarate {-i 0.5 -frameburst 1} \
	-tcpwindow 512k -udp 75m \
	-modopts {sd_uhsimode=1} \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-wlinitcmds {wl msglevel +assoc;}

43438a1 clone 43438a1b45 \
	-tag BISON06T_BRANCH_7_45 \
	-perfchans {1 3} \
	-noaes 1 -notkip 1 -nocal 1 \
	-type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srfast-11nprop-err-assert.bin


###########################################
# MC86C
# 43430wlselgs_P202 A1 - SDIO 11ac 1x1 (ultra low power 2G20-only)
###########################################
UTF::DHD mc86end2 \
        -lan_ip mc86end2 \
        -sta {43430a1 eth0} \
        -hostconsole "mc86end1:40017" \
        -power {softap2 2} \
        -power_button "auto" \
        -dhd_tag DHD_REL_1_201_59 \
        -brand linux-external-dongle-sdio \
	-nvram bcm943430wlselgs.txt \
        -datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 512k -udp 75m \
        -modopts {sd_uhsimode=1} \
	-post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc;}
 
43430a1 clone 43430a1b45 \
        -tag BISON06T_BRANCH_7_45 \
	-perfchans {1 3} \
        -notkip 1 -noaes 1 -nocal 1 \
	-type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-srfast-11nprop-err-assert.bin


# For testing
43430a1b45 clone 43430a1b45test -perfonly 1 -perfchans {1}

###########################################
# MC86C
# removed: 43602X87_P201 - 11ac 3x3 - 1/21/2015 (previous one has dup mac issue)
# 43602mc_p103 (installed on 2/5/2015)
###########################################
UTF::Linux mc86tst10 \
        -lan_ip mc86tst10 \
        -sta {43602 enp1s0} \
        -console "mc86end1:40013" \
        -power {npctst11 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -tcpwindow 4m -udp 1.2g \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl dump rssi} {%S wl dump txbf}} \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

43602 clone 43602t -tag trunk -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl country XZ/9;}
43602 clone 43602b -tag BISON_BRANCH_7_10
43602 clone 43602e -tag EAGLE_BRANCH_10_10
43602e clone 43602x1 -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl rxchain 1; wl txchain 1;}
43602e clone 43602test -tag BISON_BRANCH_7_10 -wlinitcmds {wl msglevel +assoc; wl down; wl ratesel_nss 1; wl vht_features 3;}

43602 clone 43602b11n -tag BISON_BRANCH_7_10 -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0;}
43602 clone 43602b11nx1 -tag BISON_BRANCH_7_10 -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 0; wl vhtmode 0; wl rxchain 1; wl txchain 1;}

# softap configuration
#43602 clone 43602ap-bis -tag BISON05T_BRANCH_7_35
#43602ap-bis configure -ipaddr 192.168.1.130 -attngrp G3 -ap 1 -hasdhcpd 1

UTF::DHD mc86tst10d \
        -lan_ip mc86tst10 \
        -sta {43602fd eth0} \
        -hostconsole "mc86end1:40013" \
        -power {npctst11 1} \
        -power_button "auto" \
        -dhd_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
	-tag trunk \
        -tcpwindow 4m -udp 1.2g \
        -datarate {-i 0.5 -frameburst 1} \
        -slowassoc 5 -extsup 1 -nocal 1 -docpu 1 \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-clm_blob 43602_sig.clm_blob -clm_blob {} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3;}

43602fd clone 43602fdtx \
        -perfonly 1 -perfchans {36/80} \
	-type 43602a1-ram/pcie-ag-splitrx/rtecdc.bin

43602fd clone 43602fdt \
	-type 43602a1-ram/pcie-ag-err-assert-splitrx-txqmux/rtecdc.bin

# For testing
43602fd clone 43602fdtest \
	-perfchans {3} \
	-type 43602a1-ram/pcie-ag-err-assert-splitrx/rtecdc.bin


###########################################
# AP Section (MC86c)
###########################################
# softAP
# removed: 43602X87_P303, not calibrated (installed on 6/17/2014)
# 43602mc_p103 (installed on 2/5/2015)
###########################################
UTF::Linux mc86tst9 \
        -lan_ip mc86tst9 \
        -sta {43602ap enp1s0} \
        -console "mc86end1:40002" \
        -power {softap2 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -tcpwindow 4m \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3;}

43602ap clone 43602softap-bis -tag BISON_BRANCH_7_10
43602softap-bis configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1

43602ap clone 43602softap-tot -tag trunk
43602softap-bis configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1

43602ap clone 43602softap-egl -tag EAGLE_BRANCH_10_10 
43602softap-egl configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1

43602ap clone 43602softap-bis-bf1 -tag BISON_BRANCH_7_10
43602softap-bis-bf1 configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1
43602ap clone 43602softap-bis-bf0 -tag BISON_BRANCH_7_10
43602softap-bis-bf0 configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

43602ap clone 43602softap-egl-bf1 -tag EAGLE_BRANCH_10_10
43602softap-egl-bf1 configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1
43602ap clone 43602softap-egl-bf0 -tag EAGLE_BRANCH_10_10
43602softap-egl-bf0 configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

# For testing
43602ap clone 43602softap-tst-bf1 -tag EAGLE_BRANCH_10_10
43602softap-tst-bf1 configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl ratesel_nss 1; wl vht_features 3;}
43602ap clone 43602softap-tst-bf0 -tag EAGLE_BRANCH_10_10
43602softap-tst-bf0 configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl down; wl ratesel_nss 1; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

# for testing 11n explicit TxBF, 4/9/2014
43602ap clone 43602softap-bis-bf1n
43602softap-bis-bf1n configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl vhtmode 0;}

43602ap clone 43602softap-bis-bf0n
43602softap-bis-bf0n configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl vhtmode 0; wl txbf 0; wl txbf_imp 0;}

# For testing using 4360mc_p198
43602ap clone 4360softap-bis
4360softap-bis configure -ipaddr 192.168.1.129 -attngrp G3 -ap 1 -hasdhcpd 1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3;}
	

###
UTF::Q mc86c



# -*-tcl-*-
#
# Testbed MD16 sub-rig configuration file
# Filename: md16b.tcl
# Charles Chai 1/4/2016
#
# Hardware
#   MD16END1  : FC19 controller
#   MD16TST9  : FC19 softap
#   MD16TST10 : FC19 STA
#   MD16TST11 : FC19 STA
#   MD16TST12 : FC19 STA
#   MD16TST13 : FC19 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/md16.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/md16b"

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G2 attn 0

    # Make sure all systems are deinit and down
    foreach S {4366ap 4366 4360 X52c 4339} {
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
# AP Section
###########################################

###########################################
# MD16b - softap
# 4366mc_p143 C0 (1/4/2016)
# Attn Group: G3 (9 10 11 12)
###########################################
UTF::Linux md16tst9 \
        -lan_ip md16tst9 \
        -sta {4366ap enp1s0} \
        -console "md16end1:40009" \
        -power {npc913 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1}

4366ap clone 4366softap
4366softap configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1

4366ap clone 4366softap-txbf0
4366softap-txbf0 configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf 0; wl txbf_imp 0;}
4366ap clone 4366softap-txbf1
4366softap-txbf1 configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1

4366ap clone 4366softap-3x3-txbf0
4366softap-3x3-txbf0 configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-ap3coremode 1} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 7; wl txbf 0; wl txbf_imp 0;}
4366ap clone 4366softap-3x3-txbf1
4366softap-3x3-txbf1 configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-ap3coremode 1}

4366ap clone 4366softap-udp
4366softap-udp configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1

# for testing
4366ap clone 4366softap-t
4366softap-t configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1 \
	-rvrnightly {-apateachattn "wl 5g_rate -v 9x4"}


UTF::DHD md16tst9 \
        -lan_ip md16tst9 \
        -sta {4366apfd eth0} \
        -console "md16end1:40009" \
        -power {npc913 2} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -reloadoncrash 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl down; wl country US/0; wl bw_cap 2g -1; wl vht_features 7;} \
        -type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin

4366apfd clone 4366softap-fd
4366softap-fd configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1


###########################################
# STA Section
###########################################

###########################################
# MD16b
# 4366mc_p143 C0 (1/5/2016)
###########################################
UTF::Linux md16tst10 \
        -lan_ip md16tst10 \
        -sta {4366 enp1s0} \
        -console "md16end1:40010" \
        -power {npc910 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -tcpwindow 4m -udp 1.8g \
        -reloadoncrash 1 -slowassoc 5 \
	-nopm1 1 -nopm2 1 \
        -datarate {-i 0.5} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7;}

        #-perfonly 1 -perfchans {36/80}
	#-nocal 1 -noaes 1 -notkip 1 -nobighammer 1

# for testing dongle mode AP
4366 clone 4366x4 -nocal 1 -noaes 1 -notkip 1 -nobighammer 1

4366 clone 4366x1 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl rxchain 1; wl txchain 1;}
4366 clone 4366x2 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl rxchain 3; wl txchain 3;}
4366 clone 4366x3 \
	-post_perf_hook {{%S wl dump txbf}} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl rxchain 7; wl txchain 7;}

4366 clone 4366x1t \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl rxchain 1; wl txchain 1; wl PM 2;}

4366 clone 4366x2t \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl rxchain 5; wl txchain 5;}

4366 clone 4366x3t \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl rxchain 14; wl txchain 14;}

4366 clone 4366t \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2 -1; wl vht_features 7; wl PM 2;}

###########################################
# MD16b
# 4360_p198 (1/5/2016)
###########################################
UTF::Linux md16tst11 \
        -lan_ip md16tst11 \
        -sta {4360 enp1s0} \
        -console "md16end1:40011" \
        -power {npc910 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -tcpwindow 4m -udp 1.2g \
        -reloadoncrash 1 -slowassoc 5 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-post_perf_hook {{%S wl dump txbf}} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl bw_cap 2g -1; wl vht_features 3;}

4360 clone 4360b -tag BISON_BRANCH_7_10

###########################################
# MD16b
# X52c_A205 (1/5/2016)
###########################################
UTF::Linux md16tst12 \
        -lan_ip md16tst12 \
        -sta {X52c enp1s0} \
        -console "md16end1:40012" \
        -power {npc911 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -tcpwindow 2m -udp 800m \
        -reloadoncrash 1 -slowassoc 5 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl country '#a/0'; wl vht_features 3;}


##########################################
# MD16b
# 4339 (1/5/2016)
##########################################
UTF::DHD md16tst13 \
        -lan_ip md16tst13 \
        -sta {4339 eth0} \
        -hostconsole "md16end1:40013" \
        -power {npc911 2} \
        -power_button "auto" \
	-tag AARDVARK_BRANCH_6_30 \
        -dhd_tag trunk \
        -brand linux-external-dongle-sdio \
        -dhd_brand linux-internal-dongle \
        -nvram bcm94339wlbgaFEM_AM.txt \
        -nocal 1 \
        -notkip 1 -noaes 1 -nobighammer 1 \
        -tcpwindow 1152k -udp 400m \
        -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
        -datarate {-b 400m -i 0.5 -frameburst 1} \
        -postinstall {dhd -i eth0 txglomsize 10} \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;} \
	-date 2013.12.30.0 \
	-type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-mfp-okc-txbf-pktctx-dmatxrc-ltecx-idsup-idauth-err-assert.bin


### ----------------------------
UTF::Q md16b




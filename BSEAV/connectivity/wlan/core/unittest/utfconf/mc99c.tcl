# -*-tcl-*-
#
# Testbed MC99 sub-rig configuration file
# Filename: mc99c.tcl
# Charles Chai 9/29/2015
# 
# Hardware
#   MC99END1  : FC19 controller
#   MC99TST9  : FC19 softap
#   MC99TST1 :  FC19 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc99.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc99c"

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# To enable ChannelSweep performance test 
set UTF::ChannelPerf 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G3 attn 0

    # Make sure all systems are deinit and down
    foreach S {4366ap 4366} {
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
## MC99c
## 4366mc_p140 C0 (12/4/2015)
## Attn Group: G3 {9 10 11 12}
############################################
UTF::Linux mc99tst1 \
        -lan_ip mc99tst1 \
        -sta {4366 enp1s0} \
        -console "mc99end1:40001" \
        -power {npc95 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_TWIG_10_10_69 \
        -tcpwindow 4m -udp 1.8g \
        -slowassoc 5 -reloadoncrash 1 -nopm1 1 -nopm2 1 \
        -datarate {-i 0.5} \
        -post_perf_hook {{%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl country "US/0"; wl down; wl bw_cap 2 -1; wl vht_features 7;} \
        -modopts {ctdma=1}

4366 clone 4366egltob -tag EAGLE_BRANCH_10_10

UTF::DHD mc99tst1d \
        -lan_ip mc99tst1 \
        -sta {4366fd eth0} \
        -console "mc99end1:40001" \
        -power {npc95 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -nvram_add ctdma=1 \
        -tag EAGLE_TWIG_10_10_69 \
        -slowassoc 5 -nopm1 1 -nopm2 1 -nocal 1 -docpu 1 \
	-notkip 1 -noaes 1 -nobighammer 1 \
        -datarate {-i 0.5} \
        -tcpwindow 4m -udp 1.8g \
        -post_perf_hook {{%S wl dump rssi} {%S wl dump mutx} {%S wl dump vasip_counters}} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7;} \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump.bin

4366fd clone 4366fde -tag EAGLE_BRANCH_10_10 \
	-modopts {ctdma=0}


###########################################
# AP Section
###########################################

###########################################
# MC99c - softap
# 4366mc_p140 C0, 12/4/2015
# Attn Group: G3 (9 10 11 12)
###########################################
UTF::Linux mc99tst9 \
        -lan_ip mc99tst9 \
        -sta {4366ap enp1s0} \
        -console "mc99end1:40009" \
        -power {npc94 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -reloadoncrash 1 \
        -post_perf_hook {{%S wl dump rssi} {%S wl dump mutx} {%S wl dump vasip_counters}} \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country "US/0"; wl down; wl bw_cap 2 -1; wl vht_features 7;} \
        -modopts {assert_type=1 nompc=1 ctdma=1} \
        -msgactions {
            {ai_core_reset: Failed to take core} {
                $self worry $msg;
                $self power cycle;
                return 1
            }
        }

4366ap clone 4366softap-egl -tag EAGLE_TWIG_10_10_69
4366softap-egl configure -ipaddr 192.168.1.135 -attngrp G3 -ap 1 -hasdhcpd 1

4366ap clone 4366softap-egltob -tag EAGLE_BRANCH_10_10
4366softap-egltob configure -ipaddr 192.168.1.135 -attngrp G3 -ap 1 -hasdhcpd 1


UTF::DHD mc99tst9d \
        -lan_ip mc99tst9 \
        -sta {4366apfd eth0} \
        -hostconsole "mc99end1:40009" \
        -power {npc94 1} \
        -power_button "auto" \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -nvram_add ctdma=0 \
        -slowassoc 5 -nopm1 1 -nopm2 1 -nocal 1 -docpu 1 \
        -datarate {-i 0.5} \
        -tcpwindow 4m -udp 1.8g \
        -post_perf_hook {{%S wl dump rssi} {%S wl dump mutx} {%S wl dump vasip_counters}} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7;} \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump.bin

4366apfd clone 4366softap-fd -tag EAGLE_TWIG_10_10_69
4366softap-fd configure -ipaddr 192.168.1.135 -attngrp G3 -ap 1 -hasdhcpd 1
	
4366apfd clone 4366softap-fdt -tag EAGLE_BRANCH_10_10
4366softap-fdt configure -ipaddr 192.168.1.135 -attngrp G3 -ap 1 -hasdhcpd 1


###
UTF::Q mc99c



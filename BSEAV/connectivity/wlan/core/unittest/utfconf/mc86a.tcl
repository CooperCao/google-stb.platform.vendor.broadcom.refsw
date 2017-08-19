# -*-tcl-*-
#
# Testbed MC86 sub-rig configuration file
# Filename: mc86a.tcl
# Charles Chai 01/11/2013
#
# Hardware
#   MC86END1  : FC11 controller
#   MC86TST7  : FC15 softap
#   MC86TST1  : FC15 STA
#   MC86TST3  : FC15 STA
#   MC86TST6  : FC15 STA
#   MC86TST12 : FC15 STA
#   switch    : Cisco SG300-10
#   power     : netCommander Model NPC22(s)
#   attenuator: AeroFlex/Weinsche Model 8331 (0-95.5dB with 0.5 steps)
#
source "utfconf/mc86.tcl"

package require UTF::Linux
package require UTF::utils

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext14/$::env(LOGNAME)/mc86a"

# Set default to use wl from trunk; Use -app_tag to modify.
set UTF::TrunkApps 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

set UTF::IPerfBeta 1
set UTF::TcpReadStats 1

set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 0

    # Make sure all systems are deinit and down 
    foreach S {4360ap 4350c1 4331 43228 43224} {
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
# MC86a
# 4331mc_P150 - 11n 3x3 	
###########################################
UTF::Linux mc86tst1  \
        -lan_ip mc86tst1 \
        -sta {4331 eth0} \
        -console "mc86end1:40003" \
        -power {npcdut1a 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
	-tag BISON_BRANCH_7_10 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 800m \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0;}
	
4331 clone 4331a -tag AARDVARK_BRANCH_6_30
4331a clone 4331ax1 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

4331 clone 4331b -tag BISON_BRANCH_7_10
4331b clone 4331bx1 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

4331 clone 4331t -tag trunk
4331t clone 4331tx1 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

4331 clone 4331e -tag EAGLE_BRANCH_10_10
4331e clone 4331ex1 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}


###########################################
# MC86a
# 4350fp_P370 C1 11ac 2x2
# (Obtained from Jovica Jovanovski, Oct 2013) 
###########################################
UTF::Linux mc86tst3 \
        -lan_ip mc86tst3 \
        -sta {4350c1 eth0} \
        -console "mc86end1:40005" \
        -power {npcdut1a 2} \
        -power_button "auto" \
	-tag BISON_BRANCH_7_10 \
        -brand linux-internal-wl \
	-type obj-debug-p2p-mchan \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3m -udp 1.0g \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl assert_type 1; wl bw_cap 2g -1; wl vht_features 3;}

4350c1 clone 4350c1b -tag BISON_BRANCH_7_10
4350c1 clone 4350c1b35 -tag BISON05T_BRANCH_7_35
4350c1 clone 4350c1t -tag trunk
4350c1 clone 4350c1e -tag EAGLE_BRANCH_10_10


###########################################
# MC86a
# 43228_P201 - 11n 2x2
###########################################
UTF::Linux mc86tst6  \
        -lan_ip mc86tst6 \
        -sta {43228 eth0} \
        -console "mc86end1:40006" \
        -power {npcdut1b 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag AARDVARK_BRANCH_6_30 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 2m -udp 800m \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0;}

43228  clone 43228a -tag AARDVARK_BRANCH_6_30
43228a clone 43228x1a -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43228  clone 43228b -tag BISON_BRANCH_7_10
43228b clone 43228x1b -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43228  clone 43228j -tag AARDVARK01T_BRANCH_6_37
43228j clone 43228x1j -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43228  clone 43228t -tag trunk
43228t clone 43228x1t -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43228  clone 43228e -tag EAGLE_BRANCH_10_10
43228e clone 43228x1e -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

###########################################
# MC86a
# 43224_A204 11n 2x2
###########################################
UTF::Linux mc86tst12 \
        -lan_ip mc86tst12 \
        -sta {43224 eth0} \
        -console "mc86end1:40010" \
        -power {npcdut1b 2} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag AARDVARK_BRANCH_6_30 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 2m -udp 800m \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0;}

43224  clone 43224a
43224a clone 43224x1a -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43224  clone 43224b -tag BISON_BRANCH_7_10
43224b clone 43224x1a -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43224  clone 43224t -tag trunk
43224t clone 43224x1t -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

43224  clone 43224e -tag EAGLE_BRANCH_10_10
43224e clone 43224x1e -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 1; wl txchain 1;}

# For testing
43224e clone 43224et -perfchans {3}

###########################################
# AP Section
###########################################
# softAP
# 4360mc_P198 - 11ac 3x3 (calibrated for implicit txbf)
UTF::Linux mc86tst7 \
        -lan_ip mc86tst7 \
        -sta {4360ap eth0} \
        -console "mc86end1:40001" \
        -power {softap1 1} \
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag BISON_BRANCH_7_10 \
        -tcpwindow 4m \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

# -----------------------------------------
# For nightly tests. Modify with care!
# -----------------------------------------
#
4360ap clone 4360softap -tag BISON_BRANCH_7_10 
4360softap configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1

# txbf_imp is on by default since build 2013.5.9.1
# To turn on implicit txbf:  wl txbf_imp 2
# To turn off implicit txbf: wl txbf_imp 0
# wl ratesel_nss 1

# BISON
4360ap clone 4360softap-bis -tag BISON_BRANCH_7_10
4360softap-bis configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-bis-bf1 -tag BISON_BRANCH_7_10
4360softap-bis-bf1 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4360ap clone 4360softap-bis-bf0 -tag BISON_BRANCH_7_10
4360softap-bis-bf0 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

# TOT
4360ap clone 4360softap-tot -tag trunk 
4360softap-tot configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-tot-bf1 -tag trunk
4360softap-tot-bf1 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4360ap clone 4360softap-tot-bf0 -tag trunk
4360softap-tot-bf0 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

# EAGLE 
4360ap clone 4360softap-egl -tag EAGLE_BRANCH_10_10
4360softap-egl configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-egl-bf1 -tag EAGLE_BRANCH_10_10
4360softap-egl-bf1 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3;}

4360ap clone 4360softap-egl-bf0 -tag EAGLE_BRANCH_10_10
4360softap-egl-bf0 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}


# For 43224 only!
# Set stbc=0 because 43224 one-stream don't support it.
# Set country=ALL, otherwise mcs0-7 txbf2 target power is too low (1dBm) that txbf got turned off.
4360ap clone 4360softap-egl-bf1s -tag EAGLE_BRANCH_10_10
4360softap-egl-bf1s configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl ratesel_nss 1; wl country ALL; wl stbc_tx 0; wl stbc_rx 0;}

4360ap clone 4360softap-egl-bf0s -tag EAGLE_BRANCH_10_10
4360softap-egl-bf0s configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl ratesel_nss 1; wl country ALL; wl stbc_tx 0; wl stbc_rx 0; wl txbf 0; wl txbf_imp 0;}

4360ap clone 4360softap-tot-bf1s -tag trunk
4360softap-tot-bf1s configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl ratesel_nss 1; wl country ALL; wl stbc_tx 0; wl stbc_rx 0;}

4360ap clone 4360softap-tot-bf0s -tag trunk
4360softap-tot-bf0s configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl ratesel_nss 1; wl country ALL; wl stbc_tx 0; wl stbc_rx 0; wl txbf 0; wl txbf_imp 0;}

# -----------------------------------------
# For other tests
# -----------------------------------------
#
# AARDVARK01T_REL_6_37_14_105
4360ap clone 4360softap-01T-bf1 -tag AARDVARK01T_REL_6_37_14_105
4360softap-01T-bf1 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1

4360ap clone 4360softap-01T-bf0 -tag AARDVARK01T_REL_6_37_14_105
4360softap-01T-bf0 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 3; wl txbf 0; wl txbf_imp 0;}

# added shmem dump
4360ap clone 4360softap-shmem-impbf1 -tag AARDVARK_BRANCH_6_30
4360softap-shmem-impbf1 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl ratesel_nss 1;}

4360ap clone 4360softap-shmem-impbf0 -tag AARDVARK_BRANCH_6_30
4360softap-shmem-impbf0 configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl ratesel_nss 1; wl txbf_imp 0;}

4360ap clone 4360softap-stbc-on -tag AARDVARK01T_REL_6_37_6
4360softap-stbc-on configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl ratesel_nss 1;}

4360ap clone 4360softap-stbc-off -tag AARDVARK01T_REL_6_37_6
4360softap-stbc-off configure -ipaddr 192.168.1.127 -attngrp G1 -ap 1 -hasdhcpd 1 \
        -wlinitcmds {wl msglevel +assoc; wl vht_features 0; wl ratesel_nss 1; wl stbc_tx 0; wl stbc_rx 0;}


# Turn on post log processing ('parse_wl_logs' link on report html page)
#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl
#set ::aux_lsf_queue sj-hnd

###
UTF::Q mc86a


# -*-tcl-*-
#
# AWDL Testbed configuration file for BLR19B
# Edited by Rohit B  on 12 May 2016
# Last check-in on 12 May 2016


##########################################################
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr19end2" \
        -group {
		G1 {1 2 3 4}
                G2 {5 6 7 8}
		ALL {1 2 3 4 5 6 7 8}
               }

ALL configure -default 30

#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options

UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr19end2 -rev 1

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr19b/"

set UTF::SetupTestBed {
	foreach S {4360 4364b0-Slv 4364b0-Mst} {
	catch {$S wl down}
	$S deinit
	}
	ALL attn default
    return
}

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux blr19end2 \
	-lan_ip 10.132.116.130 \
	-sta {lan eth0} \
	 
UTF::Linux blr19softap2 \
    	-sta {4360 enp1s0} \
    	-lan_ip 10.132.116.132 \
    	-power "npc11 4"\
    	-power_button "auto" \
    	-slowassoc 5 -reloadoncrash 1 \
    	-tag BISON_BRANCH_7_10 \
    	-brand "linux-internal-wl" \
	-tcpwindow 4M \
	-udp 800m \
	-wlinitcmds {
                  wl down; wl country US/0; wl vht_features 3; wl bw_cap 2g -1; wl txbf 0
         }

4360 configure -ipaddr 192.168.1.30 -attngrp G2 -ap 1 \


UTF::DHD blr19tst3 \
     	-lan_ip 10.132.116.135 \
     	-sta {4364b0-Mst eth0 4364b0-AMst wl0.2} \
     	-power "npc11 5" \
     	-power_button "auto" \
     	-dhd_brand linux-internal-dongle-pcie \
     	-driver dhd-msgbuf-pciefd-debug \
     	-tag DIN2915T250RC1_BRANCH_9_30 \
     	-app_tag DHD_BRANCH_1_359 \
     	-dhd_tag DHD_BRANCH_1_359 \
     	-nocal 1 -slowassoc 5 \
     	-nvram "bcm94364fcpagb_2.txt" \
	-brand hndrte-dongle-wl \
	-type 4364b0-roml/config_pcie_debug/rtecdc.bin \
	-nvram_add {macaddr=66:55:44:33:22:12} \
	-clm_blob 4364b0.clm_blob  \
	-udp 800m \
	-tcpwindow 4m \
	-perfchans {36/80 36l 36 3} \
	-wlinitcmds {wl down; wl vht_features 3; wl country US} \
	-yart {-attn5g 20-95 -attn2g 20-95 -pad 30} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
	-post_perf_hook {{%S wl awdl_stats} {%S wl awdl_peer_op} {%S wl awdl_opmode}} \

# -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} 

4364b0-Mst clone 4364b0-Mst-MIMO \
	-wlinitcmds {wl down; wl vht_features 3; wl country US; wl rsdb_config -i 1 -n 1,1 -p 1,1} \
	 
4364b0-Mst configure -ipaddr 192.168.1.92 -ap 1 \

4364b0-AMst configure -ipaddr 192.168.5.92 \

4364b0-Mst clone 4364a0-Mst \
	-type 4364a0-roml/config_pcie_debug/rtecdc.bin \
	-nvram "bcm94364fcpagb.txt" \
	-clm_blob 4364a0.clm_blob  \
	
4364b0-Mst clone 4364b1-Mst \
	-type 4364b1-roml/config_pcie_release_sdb_udm/rtecdc.bin \

4364b0-Mst clone 4364b2-Mst \
	-type 4364b2-roml/config_pcie_perf_sdb_udm/rtecdc.bin \
	-nvram "src/shared/nvram/bcm94364fcpagb_2.txt" \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
	-clm_blob 4364b0.clm_blob \
    	
UTF::DHD blr19tst4 \
     	-lan_ip 10.132.116.136 \
     	-sta {4364b0-Slv eth0 4364b0-ASlv wl0.2} \
     	-power "npc11 6" \
     	-power_button "auto" \
     	-dhd_brand linux-internal-dongle-pcie \
     	-driver dhd-msgbuf-pciefd-debug \
     	-tag DIN2915T250RC1_BRANCH_9_30 \
     	-app_tag DHD_BRANCH_1_359 \
     	-dhd_tag DHD_BRANCH_1_359 \
     	-nocal 1 -slowassoc 5 \
     	-nvram "bcm94364fcpagb_2.txt" \
     	-brand hndrte-dongle-wl \
     	-type 4364b0-roml/config_pcie_debug/rtecdc.bin \
     	-clm_blob 4364b0.clm_blob  \
     	-nvram_add {macaddr=66:55:44:33:22:13} \
     	-udp 800m \
     	-tcpwindow 4m \
     	-wlinitcmds {wl down; wl vht_features 3; wl country US} \
     	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
     	-post_perf_hook {{%S wl awdl_stats} {%S wl awdl_peer_op} {%S wl awdl_opmode}} \
     	-channelsweep {-usecsa} \
     	-yart {-attn5g 16-95 -attn2g 20-95 -pad 30} \
     	-perfchans {36/80 36l 36 3} \
	 
#	 -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	 
4364b0-Slv clone 4364b0-Slv-MIMO \
	-wlinitcmds {wl down; wl vht_features 3; wl country US; wl rsdb_config -i 1 -n 1,1 -p 1,1} \
	
	 
4364b0-Slv configure -ipaddr 192.168.1.93 \

4364b0-ASlv configure -ipaddr 192.168.5.93 \

4364b0-Slv clone 4364b1-Slv \
	-type 4364b1-roml/config_pcie_release_sdb_udm/rtecdc.bin \

4364b0-Slv clone 4364b2-Slv \
	-type 4364b2-roml/config_pcie_perf_sdb_udm/rtecdc.bin \
	-nvram "src/shared/nvram/bcm94364fcpagb_2.txt" \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
	-clm_blob 4364b0.clm_blob \
	  
UTF::Q blr19b

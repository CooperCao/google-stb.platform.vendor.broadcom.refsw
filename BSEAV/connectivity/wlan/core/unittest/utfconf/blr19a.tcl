#Testbed configuration file for blr19end1
#Createded Anuj Gupta Date 25Sep2015
#Last checkin 03SEP2013 12AM
####### Controller section:
# blr19end1: FC15
#
#
####### SOFTAP section:

# AP1:4360c
#
####### STA section:
#
# blr19tst1: 4349 eth0 (10.132.30.32)
# blr19softap: 4360 (10.132.30.36)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr19end1 -rev 1

UTF::Aeroflex af -lan_ip 172.1.1.101 \
        -relay "blr19end1" \
        -group {
                G1 {1 2}
				G2 {3 4}
                ALL {1 2 3 4 }
                }

G1 configure -default 0
G2 configure -default 0
ALL configure -default 25

# Default TestBed Configuration Options



# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr19a/"


set UTF::SetupTestBed {

	foreach S {4360 4355b0-Slv 4355b0-Mst} {
	catch {$S wl down}
	$S deinit
    }
	ALL attn default
	
    return
}

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux blr19end1 \
     -lan_ip 10.132.116.129 \
     -sta {lan eth0} \

UTF::Linux blr19softap1 -sta {4360 enp1s0} \
    -lan_ip 10.132.116.131 \
	-power "npc11 1"\
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
	-tcpwindow 4M \
     -wlinitcmds {
        wl msglevel +assoc;wl down; wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360 configure -ipaddr 192.168.1.90 -attngrp G2 -ap 1 \


 
UTF::DHD blr19tst1 \
     -lan_ip 10.132.116.133 \
     -sta {4355b0-Mst eth0 4355b0-AMst wl0.2} \
	 -power "npc11 2"\
    -power_button "auto" \
     -dhd_brand linux-external-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO_BRANCH_9_10 \
     -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci-norsdb/rtecdc.bin \
     -nvram "bcm94355fcpagb.txt" \
     -nocal 1 -slowassoc 5 \
	-nvram_add {macaddr=66:55:44:33:22:12} \
     -udp 800m  \
	 -tcpwindow 4m \
	-wlinitcmds {wl msglevel 0x101; wl down ;wl msglevel +assoc ; wl vht_features 3 ;}  \
	-yart {-attn5g 20-95 -attn2g 20-95 -pad 30} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	 

4355b0-Mst configure -ipaddr 192.168.1.91 \

4355b0-AMst configure -ipaddr 192.168.5.91 \
	
4355b0-Mst clone 4355b2-Mst \
	-tag DINGO2_BRANCH_9_15 \
	-brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "bcm94355fcpagb.txt" \
	-clm_blob 4355_stabello.clm_blob \
	-dhd_tag trunk \
	-app_tag DHD_REL_1_359_70 \
	-type 4355b2-roml/config_pcie_debug_sdb/rtecdc.bin \
	-wlinitcmds {wl msglevel 0x101; wl down ;wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
    -perfchans {36/80 36l 36 3} \

4355b2-Mst clone 4355b2-GO-clm \
	-type 4355b2-roml/threadx-pcie-ag-splitbuf-pktctx-ptxs-nocis-kalive-idsup-sr-noclminc-logtrace-srscan-ahr-die0-clm_min-norsdb-bssinfo-pno-aoe-ndoe-pf2-mpf-awdl-consuartseci-swdiv-wl11u-wnm-txpwrcap-txcal-hchk-mfp-wapi-ltecx-cca-txbf-assert-err/rtecdc.bin \

4355b2-Mst clone 4355b3-Mst \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-brand linux-external-dongle-pcie \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "../../olympic/C-4355__s-B3/P-simbab_M-SBLO_V-m__m-6.5.txt" \
	-dhd_tag DHD_BRANCH_1_359 \
	-app_tag DHD_BRANCH_1_359 \
	-clm_blob "4355_simba_b.clm_blob" \
	-type 4355b3-roml/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu} \
	-perfchans {36/80 36l 36 3} \

4355b3-Mst clone 4355b3-Mst-RSDB \
	-type 4355b3-roml/config_pcie_debug_sdb/rtecdc.bin \
	-wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu; wl msglevel +assoc ; wl vht_features 3 ; wl rsdb_config -n 5 -i 5,5 -p 1,1}  \

4355b3-Mst-RSDB clone 4355b3-Mst-RSDB-disable \
	-wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu; wl msglevel +assoc ; wl vht_features 3 ; wl rsdb_config -n 1 -i 1,1 -p 1,1}  \

4355b3-Mst clone 4355b3-Mst41 \
		-tag DIN2915T165R6_BRANCH_9_41 \

4355b3-Mst clone 4355b3-Mst-trunk \
	-dhd_tag trunk \
	-app_tag trunk \
	
	
	
	
 ####4355b2 GC
 

UTF::DHD blr19tst2 \
     -lan_ip 10.132.116.134 \
     -sta {4355b0-Slv eth0 4355b0-ASlv wl0.2} \
	 -power "npc11 3"\
    -power_button "auto" \
     -dhd_brand linux-external-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO_BRANCH_9_10 \
	 -dhd_tag trunk \
     -type 4355b0-roml/pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci-norsdb/rtecdc.bin \
     -nvram "bcm94355fcpagb.txt" \
	 -nvram_add {macaddr=66:55:44:33:22:13} \
     -nocal 1 -slowassoc 5 \
     -udp 800m  \
	 -tcpwindow 4m \
	 -wlinitcmds {wl msglevel 0x101; wl down ;wl vht_features 3 ; wl mimo_bw_cap 1}  \
	-yart {-attn5g 16-95 -attn2g 20-95 -pad 30} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country} {%S wl rsdb_config} {%S wl rsdb_mode}}\
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	 

4355b0-Slv configure -ipaddr 192.168.1.95 \

4355b0-ASlv configure -ipaddr 192.168.5.95 \


4355b0-Slv clone 4355b2-Slv \
	-tag DINGO2_BRANCH_9_15 \
	-brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "bcm94355fcpagb.txt" \
	-clm_blob 4355_stabello.clm_blob \
	-dhd_tag DHD_BRANCH_1_359 \
	-app_tag DHD_BRANCH_1_359 \
	-type 4355b2-roml/config_pcie_debug_sdb/rtecdc.bin \
	-wlinitcmds {wl msglevel 0x101; wl down ;wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
    -perfchans {36/80 36l 36 3} \
	

4355b2-Slv clone 4355b3-Slv \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-brand linux-external-dongle-pcie \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "../../olympic/C-4355__s-B3/P-simbab_M-SBLO_V-m__m-6.5.txt" \
	-type 4355b3-roml/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu} \
	-clm_blob "4355_simba_b.clm_blob" \
	-perfchans {36/80 36l 36 3} \
   
4355b2-Slv clone 4355b2-GC-clm \
	-type 4355b2-roml/threadx-pcie-ag-splitbuf-pktctx-ptxs-nocis-kalive-idsup-sr-noclminc-logtrace-srscan-ahr-die0-clm_min-norsdb-bssinfo-pno-aoe-ndoe-pf2-mpf-awdl-consuartseci-swdiv-wl11u-wnm-txpwrcap-txcal-hchk-mfp-wapi-ltecx-cca-txbf-assert-err/rtecdc.bin \
   
4355b3-Slv clone 4355b3-Slv-RSDB \
	-type 4355b3-roml/config_pcie_debug_sdb/rtecdc.bin \
	-wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu; wl msglevel +assoc ; wl vht_features 3 ; wl rsdb_config -n 5 -i 5,5 -p 1,1}  \


4355b3-Slv clone 4355b3-Slv41 \
		-tag DIN2915T165R6_BRANCH_9_41 \

4355b3-Slv-RSDB clone 4355b3-Slv-RSDB-disable \
	-wlinitcmds {wl down;wl country US;wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu; wl msglevel +assoc ; wl vht_features 3 ; wl rsdb_config -n 1 -i 1,1 -p 1,1}  \


4355b3-Slv clone 4355b3-Slv-trunk \
	-dhd_tag trunk \
	-app_tag trunk \



UTF::Q blr19a

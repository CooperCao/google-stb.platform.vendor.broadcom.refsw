#Testbed configuration file for blr012end1 UTF StaNighly Teststation
#Created by Sumesh Nair on 02 APRIL 2015 10:00 PM  
#
####### Controller section:
# blr11end1: FC19
# IP ADDR 10.131.80.134
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.135 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.136
#
####### STA section:
#
# blr12tst1: FC 15       eth0 (10.131.80.137)
# blr12tst2: FC 15       eth0 (10.131.80.138)
# blr12tst3: FC 15       eth0 (10.131.80.139)
# blr12t4: FC 15 4355  eth0 (10.131.80.140)
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr11end1" \
        -group {
                G1 {1 2 3 4}
                G2 {5 6 7 8}
		ALL {1 2 3 4 5 6 7 8}
                }

G1 configure -default 20
ALL configure -default 20

#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
	foreach S {4360a 4359b1n } {
	catch {$S wl down}
	$S deinit
    }
 G1 attn default
    return
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr11"



#pointing Apps to trunk

set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \


####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr11end1 -rev 1

########################### Test Manager ################

UTF::Linux blr11end1 \
     -lan_ip 10.131.80.130 \
     -sta {lan eth0}

############################ blr12ref0 ##########################################
# blr12ref0      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power          - npc 31 port 1    172.1.1.31
################################################################################ 

UTF::Linux blr11softap -sta {4360a enp1s0} \
    -lan_ip 10.131.80.120 \
    -power "npc11 1" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON05T_BRANCH_7_35 \
    -brand "linux-internal-wl" \
    -perfchans {36/80} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -tcpwindow 4M \
    -udp 800m \
    -wlinitcmds {
                 wl msglevel 0x101;wl dtim 3; wl mimo_bw_cap 1; wl vht_features 3;
                }

4360a configure -ipaddr 192.168.1.100 -attngrp G1 -ap 1 \

4360a clone 4360ref 


4360ref configure -ipaddr 192.168.1.100  -attngrp G1 -ap 1 \

4360a clone 4360b \
-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl dump rssi} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
-post_perf_hook {{%S wl dump rssi}}



####################### blr12ref1 Acting as ACI#########################
# blr12ref1      - 4360mc_1(99)
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 2 - STE 4450
# Power          - npc41 port 1    172.1.1.41
################################################################################


#UTF::Linux blr12ref1 -sta {4360b enp1s0} \
    -lan_ip 10.131.80.136 \
    -power "npc21 1"\
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
    -wlinitcmds {
                # wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
           #      wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
              #  }

#4360b clone 4360aci

#4360aci configure -ipaddr 192.168.1.101 -ap 1  -attngrp G2 \


################################ blr12tst4 ######################################
# blr12tst4      - 4364
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
################################################################################


#UTF::Linux blr12tst4 \
        -lan_ip 10.131.80.140 \
        -sta {4359b1 eth0} \
        -power "npc41 1" \
        -power_button "auto" \
        -perfchans {36/80 36l 36 3} \
        -dhd_tag trunk \
        -app_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag DINGO_BRANCH_9_10 \
		-slowassoc 5  \
        -nvram "bcm943593fcpagbss_slna.txt"  \
        -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-slna-pktctx-sstput/rtecdc.bin \
        -tcpwindow 2m \
		-udp 800m \
        -extsup 1 -nocustom 1 \
        -wlinitcmds { wl msglevel 0x101; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} } \
        -yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
    
	 


#4359b1 clone 4359b1_10 \
	-dhd_tag DHD_TWIG_1_363_59 \
	-app_tag DHD_TWIG_1_363_59 \
    -channelsweep {-usecsa} \

#4359b1 configure -ipaddr 192.168.1.191 \


UTF::Linux blr11tst1gc2 \
    -lan_ip 10.131.80.125 \
    -sta {4359b1k enp1s0} \
	-type obj-debug-apdef-stadef-norsdb-extnvm \
    -nvram "bcm94355fcpagb.txt" \
    -tag DINGO_BRANCH_9_10 \
	-app_tag trunk \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5} \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
    -udp 800m -tcpwindow 3m \
    -yart {-attn5g {0-95 95-0} -attn2g {0-95 95-0} -pad 36} \
    -perfchans {36/80 36l 36 3l 3} -nopm1 1 -nopm2 1 \
	 
 4359b1k configure -ipaddr 192.168.1.195 \
 

 


UTF::DHD blr11tst1gc \
        -lan_ip 10.131.80.125 \
        -sta {4359b1n eth0} \
        -power "npc41 1" \
        -power_button "auto" \
        -perfchans {36/80 36l 36 3} \
        -dhd_tag trunk \
        -app_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -tag DINGO_BRANCH_9_10 \
		-slowassoc 5  \
		-brand hndrte-dongle-wl \
        -nvram "bcm943593fcpagbss_slna.txt"  \
        -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-slna-pktctx-sstput/rtecdc.bin \
        -tcpwindow 2m \
		-udp 800m \
        -nocustom 1 \
        -wlinitcmds { wl msglevel 0x101; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} } \
        -yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \

 
 4359b1n configure -ipaddr 192.168.1.191

4359b1n clone 4359j \
-wlinitcmds { wl txbf 1} 



4359b1n clone 4359v \
-tag DIN07T48RC50_BRANCH_9_75  \
-dhd_tag DHD_TWIG_1_363_59 \
-app_tag DHD_TWIG_1_363_59 \
-pre_perf_hook {{%S wl rssi} {%S wl nrate}} \
-post_perf_hook {{%S wl rssi} {%S wl nrate}} \

# 4359b1n clone 4359c 


 
UTF::DHD blr16tst2 \
         -lan_ip 10.131.80.195 \
         -sta {43430b0 eth0} \
         -tcpwindow 4M \
         -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -nobighammer 1 \
         -app_tag trunk \
         -nvram bcm943430fsdng_Bx.txt \
         -dhd_tag DHD_REL_1_363_42 \
         -dhd_brand linux-mfgtest-dongle-sdio\
         -brand hndrte-dongle-wl \
         -modopts {sd_divisor=1} \
         -type 43430b0-roml/sdio-g-mfgtest-seqcmds-11nprop-srfast-srmem/rtecdc.bin \
         -tag "DINGO_BRANCH_9_10" \
         -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
         -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \


43430b0 configure -ipaddr 192.168.1.95 \


UTF::Linux blr11tst2gc \
        -lan_ip 10.131.80.126 \
        -sta {43224 enp1s0} \
        -tcpwindow 4M \
         -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -nobighammer 1 \
         -brand linux-internal-wl \
         -tag "BISON05T_BRANCH_7_35" \
         -wlinitcmds {wl msglevel 0x101 ; wl chanspec 36} \
         -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
		 
43224 configure -ipaddr 192.168.1.234 \

43224 clone 43224t -tag trunk \
	 



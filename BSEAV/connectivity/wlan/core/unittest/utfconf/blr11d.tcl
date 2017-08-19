####### Controller section:
# blr11end1: FC15
# IP ADDR 10.131.80.87
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.88 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.89
#
####### STA section:
#
# blr11tst1: FC 15 43224 eth0 (10.131.80.90)
# blr11tst2: FC 15 
# blr11tst3: FC 15
# blr11tst4: FC 15
######################################################### #
# Load Packages
package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
		-relay "blr11end1" \
		-group {
			G1 {1 2 3}
			G2 {4 5 6}
			ALL {1 2 3 4 5 6}
				}

# G1 configure -default 20
# G2 configure -default 20
ALL configure -default 20

#pointing Apps to trunk

set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck


set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G2 attn 0;}
    catch {G1 attn 0;}
	foreach S {4360a 4361A0-GO 4361A0-GC} {
	catch {$S wl down}
	$S deinit
	ALL attn default
    }
    return
}

set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr11"

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.51 -relay blr11end1 -rev 1
UTF::Power::WebRelay  web43 -lan_ip 172.1.1.43 -invert 1
########################### Test Manager ################

UTF::Linux blr11end1 \
     -lan_ip 10.132.116.84 \
     -sta {lan em1}

###############blr11softap######################

UTF::Linux blr11softap -sta {4360a enp1s0} \
    -lan_ip 10.132.116.85 \
    -power "npc11 1" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10\
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -tcpwindow 4M \
    -udp 800m \
	-wlinitcmds {
                 wl msglevel 0x101;wl dtim 3; wl mimo_bw_cap 1; wl vht_features 3;
                }


4360a configure -ipaddr 192.168.1.100 -ap 1 -attngrp G1 \




#blr11tst2go

#UTF::DHD blr11tst2go \
#	-lan_ip 10.132.116.87 \
#	-sta {43012-GO eth0 43012-PGO wl0.1} \
#	-power "npc41 2" \
#	-power_button "auto" \
#	-nvram_add {macaddr=00:66:55:44:33:22} \
#	-perfchans {36 3} \
#	-dhd_tag DHD_BRANCH_1_579 \
#	-app_tag DHD_BRANCH_1_579 \
#	-dhd_brand linux-external-dongle-sdio \
#	-tag IGUANA_BRANCH_13_10 \
#	-brand hndrte-dongle-wl \
#	-nocal 1 \
#	-slowassoc 5 \
#	-nvram "bcm943012fcbga.txt" \
#	-type 43012a0-roml/config_sdio_p2p/rtecdc.bin \
#	-udp 800m -noaes 1 -notkip 1 \
#	-tcpwindow 2m \
#	-wlinitcmds {wl msglevel +assoc;} \
#	-pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
#	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \
#	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
#	-clm_blob 43012a0.clm_blob \
#
#43012-GO configure -ipaddr 192.168.1.125
#
#43012-PGO configure -ipaddr 192.168.5.125
#
#43012-GO clone 43012B0-GO \
#	 -type 43012b0-roml/config_sdio_p2p/rtecdc.bin \
#	 -clm_blob 43012b0.clm_blob \
#	 -nvram "src/shared/nvram/bcm943012wlref_04.txt" \
#
#43012B0-GO clone 43012B0-GO-SStwig \
#	-tag IGUANA_TWIG_13_10_69 \
#	-dhd_tag DHD_TWIG_1_579_91 \
#	-type 43012b0-roml/config_sdio_samsung/rtecdc.bin \
#
#UTF::DHD blr11tst2gc \
#	-lan_ip 10.132.116.91 \
#	-sta {43012-GC eth0 43012-PGC wl0.1} \
#	-power "npc21 2" \
#	-power_button "auto" \
#	-nvram_add {macaddr=00:66:55:44:33:11} \
#	-perfchans {36 3} \
#	-dhd_tag DHD_BRANCH_1_579 \
#	-app_tag DHD_BRANCH_1_579 \
#	-dhd_brand linux-external-dongle-sdio \
#	-tag IGUANA_BRANCH_13_10 \
#	-brand hndrte-dongle-wl \
#	-nocal 1 \
#	-slowassoc 5 \
#	-nvram "bcm943012fcbga.txt" \
#	-type 43012a0-roml/config_sdio_p2p/rtecdc.bin \
#	-udp 800m \
#	-tcpwindow 2m \
#	-wlinitcmds {wl msglevel +assoc;} \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
#	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
#	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
#	-clm_blob 43012a0.clm_blob \
#
#43012-GC configure -ipaddr 192.168.1.126
#
#43012-PGC configure -ipaddr 192.168.5.126
#
#43012-GC clone 43012B0-GC \
#	-type 43012b0-roml/config_sdio_p2p/rtecdc.bin \
#	-clm_blob 43012b0.clm_blob \
#	-nvram "src/shared/nvram/bcm943012wlref_04.txt" \
#
#43012B0-GC clone 43012B0-GC-SStwig \
#	-tag IGUANA_TWIG_13_10_69 \
#	-dhd_tag DHD_TWIG_1_579_91 \
#	-type 43012b0-roml/config_sdio_samsung/rtecdc.bin \

UTF::DHD blr11tst1gc \
	-lan_ip 10.132.116.90 \
	-sta {4355C0-GC eth0 4355C0-PGC wl0.2} \
	-power "npc21 2" \
	-power_button "auto" \
	-nvram_add {macaddr=66:55:44:33:22:11} \
	-dhd_brand linux-internal-dongle-pcie \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag DIN2930R18_BRANCH_9_44 \
	-dhd_tag DHD_BRANCH_1_359 \
	-app_tag DHD_BRANCH_1_359 \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
	-nvram "../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" \
	-nocal 1 -slowassoc 5 -reloadoncrash 1 \
	-clm_blob "4355_kristoff.clm_blob" \
	-perfchans {3} \
	-udp 800m \
	-tcpwindow 2m \
	-wlinitcmds {wl msglevel 0x101; wl down; wl country US;  wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1; }  \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl dump rssi} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl phy_rssi_ant}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump rssi} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \


4355C0-GC configure -ipaddr 192.168.1.128

4355C0-PGC configure -ipaddr 192.168.5.128



UTF::DHD blr11tst3go \
	-lan_ip 10.132.116.88 \
	-sta {4355C0-GO eth0 4355C0-PGO wl0.2} \
	-power "npc51 2" \
	-power_button "auto" \
	-nvram_add {macaddr=66:55:44:33:22:12} \
	-dhd_brand linux-internal-dongle-pcie \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag DIN2930R18_BRANCH_9_44 \
	-dhd_tag DHD_BRANCH_1_359 \
	-app_tag DHD_BRANCH_1_359 \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
	-nvram "../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" \
	-nocal 1 -slowassoc 5 -reloadoncrash 1 \
	-clm_blob "4355_kristoff.clm_blob" \
	-udp 800m \
	-tcpwindow 2m \
	-wlinitcmds {wl msglevel 0x101; wl down; wl country US; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1;}  \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl dump rssi} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl phy_rssi_ant}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump rssi} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \


4355C0-GO configure -ipaddr 192.168.1.127

4355C0-PGO configure -ipaddr 192.168.5.127



set UTF::StaNightlyCustom {
	  package require UTF::Test::RTT
	  RTT $Router $STA
}



UTF::DHD blr11tst2go \
	-lan_ip 10.132.116.87 \
	-sta {4361A0-GO eth0 4361A0-PGO wl0.2} \
	-power "npc41 2" \
	-power_button "auto" \
	-tag JAGUAR_BRANCH_14_10 \
	-type 4361a0-ram/config_pcie_utf/rtecdc.bin \
	-nvram "src/shared/nvram/bcm94361fcpagbss.txt" \
	-driver dhd-msgbuf-pciefd-debug \
	-brand hndrte-dongle-wl \
	-dhd_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-app_tag trunk \
	-udp 800m \
	-tcpwindow 2m \
	-nocal 1 \
	-slowassoc 5 \
	-wlinitcmds {wl msglevel +assoc;} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
	-clm_blob 4347a0.clm_blob \

	
4361A0-GO configure -ipaddr 192.168.1.125	

4361A0-PGO configure -ipaddr 192.168.5.125



UTF::DHD blr11tst2gc \
	-lan_ip 10.132.116.91 \
	-sta {4361A0-GC eth0 4361A0-PGC wl0.2} \
	-power "npc21 2" \
	-power_button "auto" \
	-tag JAGUAR_BRANCH_14_10 \
	-type 4361a0-ram/config_pcie_utf/rtecdc.bin \
	-nvram "src/shared/nvram/bcm94361fcpagbss.txt" \
	-driver dhd-msgbuf-pciefd-debug \
	-brand hndrte-dongle-wl \
	-dhd_tag DHD_BRANCH_1_579 \
	-dhd_brand linux-internal-dongle-pcie \
	-app_tag trunk \
	-udp 800m \
	-tcpwindow 2m \
	-nocal 1 \
	-slowassoc 5 \
	-wlinitcmds {wl msglevel +assoc;} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \
	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
	-clm_blob 4347a0.clm_blob \


4361A0-GC configure -ipaddr 192.168.1.126	

4361A0-PGC configure -ipaddr 192.168.5.126

UTF::Q blr11d

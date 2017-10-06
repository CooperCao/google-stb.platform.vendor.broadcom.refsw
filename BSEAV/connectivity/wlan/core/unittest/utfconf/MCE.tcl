# -*-tcl-*-

#
# MCE CIT testbed configuration for P2P
#
# VNC 10.19.61.72:5901
# Test/StaNightly.test -utfconf utfconf/MCE.tcl -title "MCE 4354e StaNightly" -sta "4354e-FC19-DUT" -ap "AP1-4331-4708 AP1-4360-4708"
# Test/P2PQoSNightly.test -utfconf utfconf/MCE.tcl -title "MCE BISON FC15 4354e Same Channel 36l P2P" -ap "AP1-4331-4708 AP1-4360-4708" -sta_go "4354e-FC19-GO" -sta_gc "4354e-FC19-DUT" -nom -nod -perftime 25 -ap_chan "36l" -p2p_chan "36l" -wlan_security aespsk2 -p2p_security -tests "[WLAN:RX][P2P:BI]" -ap_connect "GO" -noapload -norestore
# Direct P2P VSDB
# Test/P2PQoSNightly.test -utfconf MCE -title 'MCE 43596 PCIe TCP Direct Test' -ap '4360-SoftAP2' -apdate '2014.3.27.0' -sta_gc '43596-DUT' -sta_go '43596-GO' -nos -nom -rsdb -p2p_chan '149/80' -noapload -norestore
# RvR Direct P2P
# Test/P2PQoSNightlyRvR.test -utfconf MCE -title 'MCE 4356 PCIe 7_10_TOB FC15 RvR Direct P2P' -sta_gc '4355-DUTx' -sta_go '4355-GOx' -nos -nom -wlan_bandwidth_VI 30M -p2p_bandwidth_VI 30M -run_qos -qos_tests '[P2P:TCP:RX:0:5]' -ap_chan 36/80 -p2p_chan 36l -fb1 -attn_type 2 -ap_connect GC
#

# For using attenuator, need this package as UTF.tcl doesnt load it by default.
package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/$::env(LOGNAME)/2017/MCE"

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCEUTF -rev 1
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCEUTF -rev 1
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCEUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCEUTF -rev 1
UTF::Power::Synaccess npc70 -lan_ip 172.16.1.70 -relay MCEUTF -rev 1
UTF::Power::Synaccess npc80 -lan_ip 172.16.1.80 -relay MCCUTF -rev 1

# Attenuator - Aeroflex
# G1 - Channels 1 & 2 & 3 	- AP1 & AP2
# G2 - Channel  4 & 5 & 6	- AP3 & AP4
# G3 - Channel  7 & 8 & 9	- For P2P GC-GO RvR
# Going through attenuator for the P2P RvR adds about 15 dB to the GC Ramsey box
UTF::AeroflexDirect af -lan_ip 172.16.1.210 -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9}}
G1 configure -default 0
G2 configure -default 0
G3 configure -default 0
ALL configure -default 0

set ::UTF::SetupTestBedReboot {

	set rc 0
	set rc_msg ""
	
    foreach STA $::UTF::RebootList {
    
	    UTF::Try "STA $STA Check" {
	    
	    	set reboot_flag 0
	    	
			# ping test
			set lan_ip [$STA cget -lan_ip]
			set lan_ip [string trim $lan_ip]
			if {$lan_ip == ""} {
				set lan_ip [$STA cget -name]
				set lan_ip [string trim $lan_ip]
			}
			set lan_ip [string tolower $lan_ip]
			UTF::Message INFO "" "STA=$STA lan_ip=$lan_ip"

			UTF::Message INFO "" "*************** Ping Test ****************"
			
			if {[string match -nocase "*linux*" $::tcl_platform(os)]} {
				# Linux
				set ping_options "-c 2"
			} else {
				#Windows
				set ping_options "-n 2"
			}

			set catch_resp [catch {exec ping $lan_ip $ping_options} catch_msg]
			UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

			if {$catch_resp != 0} {
				UTF::Message INFO "" "Ping failed for $STA $lan_ip. Reboot it"
				set reboot_flag 1
			} else {			
				UTF::Message INFO "" "Ping passed for $STA $lan_ip"
				
				UTF::Message INFO "" "*************** SSH Test ****************"

				set user_name [$STA cget -user]
				UTF::Message INFO "" "ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip pwd"
				set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip rm -f mem_dump_*} catch_msg]
				set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip pwd} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

				if {$catch_resp != 0} {
					UTF::Message INFO "" "SSH failed for $STA $lan_ip. Reboot it"
					set reboot_flag 1
				}
			}
			
			if {$reboot_flag == 1} {
				UTF::Message INFO "" "*************** Reboot ****************"
				
				set catch_resp [catch {$STA power off} catch_msg]
				UTF::Sleep 15
				set catch_resp [catch {$STA power on} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"
				UTF::Sleep 120

				set catch_resp [catch {exec ping $lan_ip $ping_options} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

				if {$catch_resp != 0} {
					UTF::Message INFO "" "***Failed. Ping failed after reboot for $STA $lan_ip"
					set rc -1
					append rc_msg "$STA reboot failed "
					error "Reboot Failed"
				} else {
					UTF::Message INFO "" "Ping passed after reboot for $STA $lan_ip"

					set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 root@$lan_ip pwd} catch_msg]
					UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

					if {$catch_resp != 0} {
						UTF::Message INFO "" "***Failed. SSH failed after reboot for $STA $lan_ip"
						set rc -1
						append rc_msg "$STA reboot failed "
						error "Reboot Failed"
					} else {
						UTF::Message INFO "" "SSH passed after reboot for $STA $lan_ip"
						append rc_msg "$STA rebooted "
						return "Rebooted"
					}
				}
			}
		}
    }
    
    if {$rc == -1} {
    	error $rc_msg
    } else {
    	return $rc_msg
    }
}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default

	set ::UTF::APList "AP1-4360-4708 AP3-4360-4708"
	set ::UTF::STAList "snif 4355-DUT 4355-GO 43596-DUT 43596-GO"
	set ::UTF::RebootList "4360-SoftAP1 4360-SoftAP2 $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP1 4360-SoftAP2 $::UTF::APList $::UTF::STAList"

	UTF::Try "Reboot Testbed" {
		eval $::UTF::SetupTestBedReboot
	}
	UTF::Try "Sniffer Reload" {
		snif tcptune 2M
		snif reload
		#UTF::Test::rssinoise AP1-4360-4706 snif
	}

    # Make sure APs are on before testing
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    foreach AP "$::UTF::APList" {
	    UTF::Try "$AP Radio Down" {
			catch {$AP power on}
    		UTF::Sleep 5
			catch {$AP restart wl0_radio=0}
			catch {$AP restart wl1_radio=0}
			catch {$AP deinit}
			return
		}
    }
    # unset S so it doesn't interfere
    unset AP  

    # To prevent inteference.
    foreach S "$::UTF::DownList" {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
		    catch {$S deinit}
			return
	    }
    }
    # unset S so it doesn't interfere
    unset S  
    
    # delete myself
    unset ::UTF::SetupTestBed
    unset ::UTF::SetupTestBedReboot
    
    return
}

##########################################################################################
# UTF Endpoint - Traffic generators (no wireless cards)
##########################################################################################
# mcetst1
UTF::Linux MCEUTF \
	-lan_ip 10.19.61.65 \
	-iperf iperf208 \
    -sta "lan p16p1" \
    -tcpwindow 3M

lan configure -ipaddr 192.168.1.220

##########################################################################################
# Sniffer
##########################################################################################

UTF::Sniffer MCESNIF \
	-lan_ip 10.19.61.73 \
	-sta {snif enp1s0} \
    -power "npc80 1" \
    -power_button "auto" \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10

##########################################################################################
#						STA 4355b3 Chip - Brix
##########################################################################################
#    -post_assoc_hook {%S wl reassoc $BSSID} \

UTF::DHD MCE4355GO \
	-lan_ip 10.19.61.69 \
	-iperf iperf208 \
	-sta {4355-GO eth0} \
    -name "4355-GO" \
	-power "npc60 2" \
	-power_button "auto" \
    -hostconsole mcetst1:40062 \
	-perfchans {36/80 36l 36 3} \
    -slowassoc 5 \
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 4m -udp 800m \
    -yart {-attn5g 20-63 -attn2g 0-63 -pad 42 -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag DHD_BRANCH_1_359 \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-type 4355b3-roml/config_pcie_debug/rtecdc.bin \
	-clm_blob "4355_simba_b.clm_blob" \
	-nvram "../../olympic/C-4355__s-B3/P-simbab_M-SBLO_V-m__m-6.5.txt" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl country US; wl mpc 0; wl ampdu_mpdu 48; wl amsdu_aggsf; wl bw_cap 2g -1; wl vht_features 3; wl vht_features; wl PM; wl mpc; wl nmode; wl vhtmode; wl frameburst; wl ampdu_mpdu} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" WARN
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
    }

4355-GO configure -ipaddr 192.168.1.234

######## AWDL Master (GO) ########
4355-GO clone 4355-Master-WLAN \
	-name "4355-Master" \
	-sta {4355-Master-WLAN eth0 4355-Master-AWDL wl0.2}
4355-Master-WLAN configure -ipaddr 192.168.1.234    
4355-Master-AWDL configure -ipaddr 192.168.5.234
######################

#####################################

# 4355b3 PCIe Stella Cidre card with Murata connector
UTF::DHD MCE4355DUT \
	-lan_ip 10.19.61.68 \
	-iperf iperf208 \
	-sta {4355-DUT eth0} \
    -name "4355-DUT" \
	-power "npc50 2" \
	-power_button "auto" \
    -hostconsole mcetst1:40052 \
	-perfchans {36/80 36l 36 3} \
    -slowassoc 5 \
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 4m -udp 800m \
    -yart {-attn5g 20-63 -attn2g 0-63 -pad 42 -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-dhd_tag DHD_BRANCH_1_359 \
    -app_tag DHD_BRANCH_1_359 \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-type 4355b3-roml/config_pcie_debug/rtecdc.bin \
	-clm_blob "4355_simba_b.clm_blob" \
	-nvram "../../olympic/C-4355__s-B3/P-simbab_M-SBLO_V-m__m-6.5.txt" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl country US; wl mpc 0; wl ampdu_mpdu 48; wl amsdu_aggsf; wl bw_cap 2g -1; wl vht_features 3; wl vht_features; wl PM; wl mpc; wl nmode; wl vhtmode; wl frameburst; wl ampdu_mpdu} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" WARN
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
    }

4355-DUT configure -ipaddr 192.168.1.236    

######## AWDL Slave (GC) ########
4355-DUT clone 4355-Slave-WLAN \
	-name "4355-Slave" \
	-sta {4355-Slave-WLAN eth0 4355-Slave-AWDL wl0.2}
4355-Slave-WLAN configure -ipaddr 192.168.1.236    
4355-Slave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################

##########################################################################################
#						STA 43596 PCIe Chip 
##########################################################################################

UTF::DHD MCE43596GO \
	-lan_ip 10.19.61.71 \
	-iperf iperf208 \
	-sta {43596-GO eth0} \
    -name "43596-GO" \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole mcetst1:40061 \
	-perfchans {36/80 36l 36 11u 3} \
    -nocal 1 -slowassoc 10 \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 2m \
    -driver dhd-msgbuf-pciefd-debug \
    -dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-tag DIN07T48RC50_BRANCH_9_75 \
	-clm_blob "ss_mimo.clm_blob" \
	-type 43596a0-roml/config_pcie_utf/rtecdc.bin \
	-nvram "bcm943596fcpagbss.txt" \
	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl rsdb_mode; wl mpc; wl channel} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}

43596-GO configure -ipaddr 192.168.1.234

######## P2P ########
43596-GO clone 43596-GO-WLAN \
	-name "43596-GO" \
	-sta {43596-GO-WLAN eth0 43596-GO-P2P wl0.2}
43596-GO-WLAN configure -ipaddr 192.168.1.234    
43596-GO-P2P  configure -ipaddr 192.168.5.234
######################

######## P2P-RSDB ########
# For RSDB, "wl ocl_enable 0" is needed for STA
43596-GO clone 43596-GO-WLAN-RSDB \
	-name "43596-GO-RSDB" \
	-sta {43596-GO-WLAN-RSDB eth0 43596-GO-P2P-RSDB wl0.2} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl up; wl ocl_enable 0; wl rsdb_mode; wl mpc; wl channel}
43596-GO-WLAN-RSDB configure -ipaddr 192.168.1.234    
43596-GO-P2P-RSDB  configure -ipaddr 192.168.5.234
######################

43596-GO-WLAN clone 43596-GO-WLANx \
    -perfonly 1 -perfchans {36/80}

#####################################

UTF::DHD MCE43596DUT \
	-lan_ip 10.19.61.70 \
	-iperf iperf208 \
	-sta {43596-DUT eth0} \
    -name "43596-DUT" \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole mcetst1:40051 \
	-perfchans {36/80 36l 36 11u 3} \
    -nocal 1 -slowassoc 10 \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 2m \
    -driver dhd-msgbuf-pciefd-debug \
    -dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-tag DIN07T48RC50_BRANCH_9_75 \
	-type 43596a0-roml/config_pcie_utf/rtecdc.bin \
	-clm_blob "ss_mimo.clm_blob" \
	-nvram "bcm943596fcpagbss.txt" \
	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl rsdb_mode; wl mpc; wl channel} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}
	
43596-DUT configure -ipaddr 192.168.1.236

######## P2P ########
43596-DUT clone 43596-DUT-WLAN \
	-name "43596-GC" \
	-sta {43596-DUT-WLAN eth0 43596-DUT-P2P wl0.2}
43596-DUT-WLAN configure -ipaddr 192.168.1.236    
43596-DUT-P2P  configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## P2P-RSDB ########
# For RSDB, "wl ocl_enable 0" is needed for STA
43596-DUT clone 43596-DUT-WLAN-RSDB \
	-name "43596-GC-RSDB" \
	-sta {43596-DUT-WLAN-RSDB eth0 43596-DUT-P2P-RSDB wl0.2} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl up; wl ocl_enable 0; wl rsdb_mode; wl mpc; wl channel}
43596-DUT-WLAN-RSDB configure -ipaddr 192.168.1.236    
43596-DUT-P2P-RSDB  configure -ipaddr 192.168.5.236 -attngrp G3
######################

43596-DUT-WLAN clone 43596-DUT-WLANx \
    -perfonly 1 -perfchans {36/80}


#-----------------------------------------------------------------------------------------
#						                   APs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#										SoftAP1 4360
##########################################################################################
UTF::Linux MCESoftAP1 \
	-lan_ip 10.19.61.66 \
	-iperf iperf208 \
    -sta {4360-SoftAP1 eth0} \
    -console "mcetst1:40071" \
    -power "npc70 1" \
    -tcpwindow 3M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 3} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

4360-SoftAP1 configure -ipaddr 192.168.1.91 -attngrp G1 -hasdhcpd 1

##########################################################################################
#										SoftAP2 4360
##########################################################################################
UTF::Linux MCESoftAP2 \
	-lan_ip 10.19.61.67 \
	-iperf iperf208 \
    -sta {4360-SoftAP2 enp1s0} \
    -console "mcetst1:40072" \
    -power "npc70 2" \
    -tcpwindow 3M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 3} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

4360-SoftAP2 configure -ipaddr 192.168.1.97 -attngrp G2 -hasdhcpd 1

#####################
# For 43596 STA, "wl txcore -o 0x3 -k 0x3 -c 0x3 -s 1" is needed for SoftAP
4360-SoftAP2 clone 4360-SoftAP2-43596 \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 3; wl txcore -o 0x3 -k 0x3 -c 0x3 -s 1}
4360-SoftAP2-43596 configure -ipaddr 192.168.1.97 -attngrp G2 -hasdhcpd 1
#####################

##########################################################################################
#								AP 4331/4360 Netgrear N6300 (v2)
##########################################################################################
# UTF.tcl AP1-4708 findimages -all
#    -tag AARDVARK01T_REL_6_3\?_\?\?_\?\? \

UTF::Router AP1-4708 \
    -lan_ip 192.168.1.10 \
    -sta {AP1-4331-4708 eth1 AP1-4360-4708 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "mcetst1:40010" \
    -tag BISON_BRANCH_7_10 \
    -brand linux-2.6.36-arm-internal-router \
    -power "npc10 1" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-noradio_pwrsave 1 \
    -nvram {
        wl_msglevel=0x101
		macaddr=00:90:4C:0F:F2:66
		wl0_ssid=MCE2GAP1
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCE5GAP1
		wl1_chanspec=36
		wl1_radio=0
        wl1_bw_cap=-1
		wl1_obss_coex=0
        et_msglevel=0; # WAR for PR#107305
		#
		lan_ipaddr=192.168.1.10
		lan_gateway=192.168.1.10
		dhcp_start=192.168.1.110
		dhcp_end=192.168.1.119
		lan1_ipaddr=192.168.2.10
		lan1_gateway=192.168.2.10
		dhcp1_start=192.168.2.110
		dhcp1_end=192.168.2.119
		antswitch=0
    }

AP1-4360-4708 configure -attngrp G1
AP1-4331-4708 configure -attngrp G1

##########################################################################################
#								AP 4331/4360 Netgrear N6300
##########################################################################################
UTF::Router AP3-4706 \
    -lan_ip 192.168.1.30 \
    -sta {AP3-4331-4706 eth1 AP3-4360-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "mcetst1:40030" \
    -tag AARDVARK01T_TWIG_6_37_14 \
    -brand linux26-internal-router \
    -power "npc30 1" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-noradio_pwrsave 1 \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
		macaddr=00:90:4C:0D:B4:34
		wl0_ssid=MCE2GAP3
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCE5GAP3
		wl1_chanspec=36
		wl1_radio=0
        wl1_bw_cap=-1
		wl1_obss_coex=0
        et_msglevel=0; # WAR for PR#107305
		#
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.30
		dhcp_start=192.168.1.130
		dhcp_end=192.168.1.139
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.30
		dhcp1_start=192.168.2.130
		dhcp1_end=192.168.2.139
		antswitch=0
		1:boardflags2=0x4000000
    }

AP3-4360-4706 configure -attngrp G2
AP3-4331-4706 configure -attngrp G2

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat mcetst1
##########################################################################################
UTF::Q MCE

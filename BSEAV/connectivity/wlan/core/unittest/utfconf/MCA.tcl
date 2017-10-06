# -*-tcl-*-

#
# MCA testbed configuration for 3x3 4360X and 4360 Apple CCT
#
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA 4360X MBP WLAN QoS" -ap "AP1-4709-4360" -sta 'MacX-Gala' -wlan_security aespsk2 -ap_chan "36/80" -wlan_bandwidth_BE "5M" -wlan_bandwidth_BK "5M" -wlan_bandwidth_VI "5M" -wlan_bandwidth_VO "5M" -qos_ampdu_mode 1 -noapload -norestore -qos_tests "[WLAN:TCP:RX:0:20]" -date 2013.2.21.0
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA MBP WLAN Same Channel Current" -ap "AP1-4709-4360" -sta 'MacX-Gala' -perftime 25 -statag AARDVARK_BRANCH_6_30 -noapload -norestore -qos_tests "[WLAN:TCP:RX:0:20]" -measure_current_sta "4360X-WGC"
# WLAN only:
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA 4360X MBP WLAN QoS" -ap "AP1-4709-4360" -sta 'MacX-Gala' -wlan_security aespsk2 -ap_chan "36/80" -wlan_bandwidth_BE "5M" -wlan_bandwidth_BK "5M" -wlan_bandwidth_VI "5M" -wlan_bandwidth_VO "5M" -qos_ampdu_mode 1 -noapload -norestore -nobi -tests_tos VO"
# P2P specific test:
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA 4360X MBP QoS" -ap "AP1-4709-4360" -sta_go "4360-MBPX-GO" -sta_gc "4360-MBPX-DUT" -nom -nod -wlan_security aespsk2 -ap_chan "36l" -p2p_chan "36l" -wlan_bandwidth_BE "5M" -wlan_bandwidth_BK "5M" -wlan_bandwidth_VI "5M" -wlan_bandwidth_VO "5M" -qos_ampdu_mode 1 -noapload -norestore  -tests "[WLAN:TX][P2P:TX]"
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA 4360X MBP QoS" -ap "AP1-4709-4360" -sta_go "4360-MBPX-GO" -sta_gc "4360-MBPX-DUT" -nom -nod -wlan_security aespsk2 -ap_chan "36l" -p2p_chan "36l" -wlan_bandwidth_BE "5M" -wlan_bandwidth_BK "5M" -wlan_bandwidth_VI "5M" -wlan_bandwidth_VO "5M" -qos_ampdu_mode 1 -noapload -norestore  -tests "[WLAN:TX][P2P:TX]"
# CCT test:
# Test/CCTNightly.test -utfconf utfconf/MCA.tcl -title "MCA MBP 4360 X CCT" -sta 'MacX-Gala' -ap "AP1-4709-4360" -noapload -norestore
# SoftAP:
# Test/StaNightly.test -utfconf utfconf/MCA.tcl -title "MCA MBP 4360 X29 SoftAP StaNightly" -sta 'MacX-Gala' -ap "4360-SoftAP" -perfonly -rateonly -date 2013.11.9.0 -apdate 2013.11.9.0
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA MBP WLAN QoS" -ap "4360-SoftAP" -sta 'MacX-Gala' -ap_chan "36/80" -wlan_bandwidth_BE "5M" -wlan_bandwidth_BK "5M" -wlan_bandwidth_VI "5M" -wlan_bandwidth_VO "5M" -qos_tests "[WLAN:TCP:RX:0:20]" -date 2013.11.9.0 -apdate 2013.11.9.0 -pm_mode 0
# Test/P2PQoSNightly.test -utfconf utfconf/MCA.tcl -title "MCA MBP WLAN QoS" -ap "4360-SoftAP" -sta_go "4360-MBPX-GO" -sta_gc "4360-MBPX-DUT" -nom -nos -ap_chan "36/80" -wlan_bandwidth_BE "5M" -wlan_bandwidth_BK "5M" -wlan_bandwidth_VI "5M" -wlan_bandwidth_VO "5M" -qos_tests "[WLAN:TCP:RX:0:20]" -date 2013.11.9.0 -apdate 2013.11.9.0 -pm_mode 0
#

package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext19/$::env(LOGNAME)/2017/MCA"

# Define power controllers on cart.
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCAUTF -rev 1
UTF::Power::Synaccess npc20 -lan_ip 172.16.1.20 -relay MCAUTF
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCAUTF -rev 1
UTF::Power::Synaccess npc40 -lan_ip 172.16.1.40 -relay MCAUTF
UTF::Power::WebRelay WebRelay55 -lan_ip 172.16.1.55
UTF::Power::WebRelay WebRelay56 -lan_ip 172.16.1.56
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCAUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCAUTF -rev 1
UTF::Power::Agilent ag1 -lan_ip "172.16.1.90 5024" -model "N6700B" -voltage 3.3 -channel 1
UTF::Power::Agilent ag2 -lan_ip "172.16.1.90 5024" -model "N6700B" -voltage 3.3 -channel 2

# Attenuator - Aeroflex
# G1 - Channels 1 & 2 & 3 	- AP1 & AP2
# G2 - Channel  4 & 5 & 6	- AP3 & AP4
# G3 - Channel  7 & 8 & 9	- For P2P GC-GO RvR and beamforming
UTF::Aeroflex af -lan_ip 172.16.1.210 -relay MCAUTF -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9}}
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
				set catch_resp [catch {exec ssh -S none -o ConnectTimeout=20 $user_name\@$lan_ip rm -f debug_dump_*} catch_msg]
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

	set ::UTF::APList "AP1-4709-4360 AP3-4709-4360"
	#set ::UTF::STAList "snif MacX-Gala-GO MacX-Gala"
	set ::UTF::STAList "MacX-Gala-GO MacX-Gala"
	set ::UTF::RebootList "4360-SoftAP $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP $::UTF::APList $::UTF::STAList"

	UTF::Try "Reboot Testbed" {
		eval $::UTF::SetupTestBedReboot
	}
	#UTF::Try "Sniffer Reload" {
	#	snif tcptune 2M
	#	snif reload
		#UTF::Test::rssinoise 4360-SoftAP1 snif
	#}

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
UTF::Linux MCAUTF \
	-lan_ip 10.19.84.201 \
	-iperf iperf205 \
    -sta "lan p4p1" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

##########################################################################################
# Sniffer
##########################################################################################

UTF::Sniffer MCASNIF \
	-lan_ip 10.19.61.176 \
	-sta {snif enp1s0} \
    -power "npc60 1" \
    -power_button "auto" \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10

##########################################################################################
#						STA Macbook pro i7 43602x87 P315 Gala 10.11
##########################################################################################

UTF::Power::Laptop go_power -button {WebRelay56 1}
UTF::MacOS MacXGalaGO \
	-lan_ip 10.19.84.205 \
	-iperf iperf_awdl \
	-sta {MacX-Gala-GO en3} \
    -power {go_power} \
    -name "MacX-Gala-GO" \
	-console "/var/log/system.log" \
    -tcpwindow 3640K -udp 1.2g \
	-perfchans {149/80 149l 149 6} \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -yart {-attn5g 20-63 -attn2g 0-63 -pad 42 -frameburst 1} \
	-nativetools 1 \
	-nobighammer 1 -custom 1 \
    -brand "macos-internal-wl-gala" \
    -type Debug_10_11 \
    -wlinitcmds { Sleep 5; wl msglevel +assoc; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -tag "BIS715GALA_BRANCH_7_21"

MacX-Gala-GO configure -ipaddr 192.168.1.234    

######## AWDL Peer (GO) ########
MacX-Gala-GO clone MacX-Peer-WLAN \
		-name "MacX-Peer" \
		-sta {MacX-Peer-WLAN en3 MacX-Peer-AWDL awdl0}
MacX-Peer-WLAN configure -ipaddr 192.168.1.234    
MacX-Peer-AWDL configure -ipaddr 192.168.5.234
######################

########################

UTF::Power::Laptop dut_power -button {WebRelay55 1}
UTF::MacOS MacXGala \
	-lan_ip 10.19.85.93 \
	-iperf iperf_awdl \
	-sta {MacX-Gala en2} \
    -power {dut_power} \
	-power_sta "ag2 2" \
    -name "MacX-Gala" \
	-console "/var/log/system.log" \
    -tcpwindow 3640K -udp 1.2g \
	-perfchans {149/80 149l 149 6} \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -yart {-attn5g 20-63 -attn2g 0-63 -pad 42 -frameburst 1} \
	-nativetools 1 \
	-nobighammer 1 -custom 1 \
    -brand "macos-internal-wl-gala" \
    -type Debug_10_11 \
    -wlinitcmds { Sleep 5; wl msglevel +assoc; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -tag "BIS715GALA_BRANCH_7_21"

MacX-Gala configure -ipaddr 192.168.1.236    

######## AWDL DUT (GC) ########
MacX-Gala clone MacX-DUT-WLAN \
	-name "MacX-DUT" \
	-sta {MacX-DUT-WLAN en2 MacX-DUT-AWDL awdl0}
MacX-DUT-WLAN configure -ipaddr 192.168.1.236    
MacX-DUT-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################

##########################################################################################
#						STA Macbook Air 4360X Chip - NOT USED
##########################################################################################

# P2P GO - Macbook Air i5 4360
UTF::Power::Laptop 4360_go_power -button {WebRelay56 2}
UTF::MacOS MCAGOMBAX \
	-lan_ip 10.19.85.94 \
	-iperf iperf_awdl \
	-sta {4360-MBAX-GO en0 4360-MBAX-PGO p2p0} \
    -power {4360_go_power} \

# P2P DUT - Macbook Air i5 4360
UTF::Power::Laptop 4360_dut_power -button {WebRelay55 2}
UTF::MacOS MCADUTMBAX \
	-lan_ip 10.19.85.56 \
	-iperf iperf_awdl \
	-sta {4360-MBAX-DUT en0 4360-MBAX-PDUT p2p0} \
    -power {4360_dut_power} \
	-power_sta "ag1 1" \

##########################################################################################
#										SoftAP 4360
##########################################################################################
UTF::Linux MCASoftAP \
	-lan_ip 10.19.61.61 \
	-iperf iperf205 \
    -sta {4360-SoftAP eth0} \
    -console "mc01end1:40040" \
    -power "npc40 2" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 3} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

4360-SoftAP configure -ipaddr 192.168.1.95 -attngrp G2 -hasdhcpd 1

##########################################################################################
#								AP 4709C0/4360mc
##########################################################################################
UTF::Router AP1-4709 \
    -lan_ip 192.168.1.10 \
    -sta {AP1-4709-4360 eth1} \
    -power "npc10 1" \
    -relay lan \
    -lanpeer lan \
    -console "mc01end1:40010" \
    -tcpwindow 4M -udp 1.2G \
	-noradio_pwrsave 1 \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -brand linux-2.6.36-arm-internal-router  \
    -tag EAGLE_REL_10_10_57{,_*} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
		fw_disable=1
		macaddr=00:90:4C:0D:B0:D1
		wl0_ssid=MCAAP1
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		watchdog=6000; # PR#90439
		#
		lan_ipaddr=192.168.1.10
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.110
		dhcp_end=192.168.1.119
		lan1_ipaddr=192.168.2.10
		lan1_gateway=192.168.2.1
		dhcp1_start=192.168.2.110
		dhcp1_end=192.168.2.119
    }

AP1-4709-4360 configure -attngrp G1
    
##########################################################################################
#								AP 4709C0/4360mc
##########################################################################################
UTF::Router AP3-4709 \
    -lan_ip 192.168.1.30 \
    -sta {AP3-4709-4360 eth1} \
    -power "npc30 1" \
    -relay lan \
    -lanpeer lan \
    -console "mc01end1:40030" \
    -tcpwindow 4M -udp 1.2G \
	-noradio_pwrsave 1 \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -brand linux-2.6.36-arm-internal-router  \
    -tag EAGLE_REL_10_10_57{,_*} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
		fw_disable=1
		macaddr=00:90:4C:0D:B0:D3
		wl0_ssid=MCAAP3
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		watchdog=6000; # PR#90439
		#
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.130
		dhcp_end=192.168.1.139
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.1
		dhcp1_start=192.168.2.130
		dhcp1_end=192.168.2.139
    }

AP3-4709-4360 configure -attngrp G2

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat mc01end1
##########################################################################################
UTF::Q MCA

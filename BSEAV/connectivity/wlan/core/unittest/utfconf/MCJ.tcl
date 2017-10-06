# -*-tcl-*-

#
# MCJ CIT coex testbed configuration for P2P
# Test/p2p_coex.test -utfconf utfconf/MCJ.tcl -title MCJ -ap AP1-4322-4717 -stago 43228-P2PR -sta 43228-P2PD -nodopp -nosopp -nomopp -nodnoa -nosnoa -nomnoa -nom -nos
# Test/P2PQoSNightly.test -utfconf utfconf/MCJ.tcl -title "Direct P2P" -ap "AP1-4322-4717" -sta_go "43228-GO" -sta_gc "43228-DUT" -nom -nos -statag BISON_BRANCH_7_10 -perftime 25
#

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost_USB      		;# BlueTooth device under test
set ::bt_ref BTRef_2046USB      	;# BlueTooth reference board
set ::wlan_dut 4329-FC9-P2PDC0		;# HND WLAN device under test
set ::wlan_rtr AP1-4322-4717   		;# HND WLAN router
set ::wlan_tg lan              		;# HND WLAN traffic generator

# For using attenuator, need this package as UTF.tcl doesnt load it by default.
package require UTF::Aeroflex

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/$::env(LOGNAME)/2017/MCJ"

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess 172.16.1.10
UTF::Power::Synaccess 172.16.1.50
UTF::Power::Synaccess npc56 -lan_ip 172.16.1.56 -relay MCJUTF
UTF::Power::Synaccess npc57 -lan_ip 172.16.1.57 -relay MCJUTF -rev 1
UTF::Power::Synaccess npc70 -lan_ip 172.16.1.70 -relay MCJUTF -rev 1
UTF::Power::WebRelay 172.16.1.55

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.16.1.210 -group {G1 {1 2} G2 {3 4} G3 {5} ALL {1 2 3 4 5 6}}
ALL configure -default 103
G1 configure -default 0
G2 configure -default 0
G3 configure -default 103

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
				UTF::Message INFO "" "ssh -o ConnectTimeout=20 $user_name\@$lan_ip ls"
				set catch_resp [catch {exec ssh -o ConnectTimeout=20 $user_name\@$lan_ip ls} catch_msg]
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
				UTF::Sleep 60

				set catch_resp [catch {exec ping $lan_ip $ping_options} catch_msg]
				UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"

				if {$catch_resp != 0} {
					UTF::Message INFO "" "***Failed. Ping failed after reboot for $STA $lan_ip"
					set rc -1
					append rc_msg "$STA reboot failed "
					error "Reboot Failed"
				} else {
					UTF::Message INFO "" "Ping passed after reboot for $STA $lan_ip"

					set catch_resp [catch {exec ssh -o ConnectTimeout=20 root@$lan_ip ls} catch_msg]
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

    ALL attn default
    G1 attn default
    G2 attn default
    G3 attn default
	set ::UTF::APList "AP1-4322-4717 AP3-4322-4717"
	set ::UTF::STAList "43228-DUT 43228-GO"
	set ::UTF::RebootList "4360-SoftAP $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP $::UTF::APList $::UTF::STAList"

	UTF::Try "Reboot Testbed" {
		eval $::UTF::SetupTestBedReboot
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
		}
    }
    # unset S so it doesn't interfere
    unset AP  

    # To prevent inteference.
    foreach S "$::UTF::DownList" {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
		    catch {$S deinit}
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
#						STA 43228 Chip 
##########################################################################################

UTF::Linux MCJGO \
	-lan_ip 10.19.13.166 \
	-sta {43228-GO enp1s0 43228-PGO wl0.1} \
    -power "172.16.1.50 2"\
    -power_button "auto"\
	-tcpwindow 1152k \
	-perfchans {36l 36 11u 3} \
    -console lab23hnd-mirabadi:40051 \
	-type debug-apdef-stadef-p2p-mchan-tdls \
    -brand linux-internal-wl \
	-tag "EAGLE_BRANCH_10_10" \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}

43228-GO configure -ipaddr 192.168.1.234
43228-PGO configure -ipaddr 192.168.5.234

UTF::Linux MCJDUT \
	-lan_ip 10.19.13.167 \
	-sta {43228-DUT enp1s0 43228-PDUT wl0.1} \
    -power "172.16.1.50 1"\
    -power_button "auto"\
	-tcpwindow 1152k \
	-perfchans {36l 36 11u 3} \
    -console lab23hnd-mirabadi:40052 \
	-type debug-apdef-stadef-p2p-mchan-tdls \
    -brand linux-internal-wl \
	-tag "EAGLE_BRANCH_10_10" \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}

43228-DUT configure -ipaddr 192.168.1.236
43228-PDUT configure -ipaddr 192.168.5.236

##########################################################################################
# UTF Endpoint - Traffic generators (no wireless cards)
##########################################################################################
UTF::Linux MCJUTF \
	-lan_ip 10.19.13.206 \
    -sta "lan p8p1" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

##########################################################################################
#										SoftAP 4360
##########################################################################################
UTF::Linux MCJSoftAP \
	-lan_ip 10.19.13.152 \
    -sta {4360-SoftAP eth0} \
    -console "lab23hnd-mirabadi:40070" \
    -power "npc70 2" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl msglevel; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3; wl country US} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

4360-SoftAP configure -ipaddr 192.168.1.95 -attngrp G1 -hasdhcpd 1 -ap 1

##########################################################################################
#						Linksys 320N 4717/4322 wireless router
##########################################################################################
UTF::Router AP1 \
    -lan_ip 192.168.1.10 \
    -sta "AP1-4322-4717 eth1" \
    -relay "MCJUTF" \
    -power "172.16.1.10 1" \
    -lanpeer lan \
    -console "lab23hnd-mirabadi:40010" \
    -tag AKASHI_REL_5_110_65 \
	-brand linux-external-router \
    -date * \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-nvram {
       	fw_disable=1
       	wl_msglevel=0x101
		macaddr=00:90:4C:A6:00:10
        wl0_ssid=MCJ
		lan_ipaddr=192.168.1.10
		lan_gateway=192.168.1.10
		dhcp_start=192.168.1.110
		dhcp_end=192.168.1.119
		lan1_ipaddr=192.168.2.10
		lan1_gateway=192.168.2.10
		dhcp1_start=192.168.2.110
		dhcp1_end=192.168.2.119
		antswitch=0
		# 5 GHz band (2->2.4 GHz, 1->5 GHz) 
		wl0_nband=2
		# Channel
		wl0_channel=3
		# Bandwidth (0->20 GHz on both channels, 1->40 GHz on both channels, 2->20 GHz 0n 2.4 and 40 GHz on 5)
		wl0_nbw_cap=2
    }    

AP1-4322-4717 configure -attngrp G1

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#
UTF::Router AP3 \
    -lan_ip 192.168.1.30 \
    -sta "AP3-4322-4717 eth1" \
    -relay "MCJUTF" \
    -power "172.16.1.30 1" \
    -lanpeer lan \
    -console "lab23hnd-mirabadi:40030" \
    -tag AKASHI_REL_5_110_65 \
	-brand linux-external-router \
    -date * \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-nvram {
       	fw_disable=1
       	wl_msglevel=0x101
		macaddr=00:90:4C:A6:00:30
        wl0_ssid=MCJ
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.30
		dhcp_start=192.168.1.130
		dhcp_end=192.168.1.139
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.30
		dhcp1_start=192.168.2.130
		dhcp1_end=192.168.2.139
		antswitch=0
		# 5 GHz band (2->2.4 GHz, 1->5 GHz) 
		wl0_nband=2
		# Channel
		wl0_channel=3
		# Bandwidth (0->20 GHz on both channels, 1->40 GHz on both channels, 2->20 GHz 0n 2.4 and 40 GHz on 5)
		wl0_nbw_cap=2
    }    

AP3-4322-4717 configure -attngrp G2

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab23hnd-mirabadi
##########################################################################################
UTF::Q MCJ

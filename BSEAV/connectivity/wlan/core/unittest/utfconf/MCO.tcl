# -*-tcl-*-

#
# MCO testbed configuration for ACIM
#
# Test/ACIMNightly.test -utfconf utfconf/MCO.tcl -ap "APDUT-4331-4706 APDUT-4360-4706" -sta "4360-FC15-DUT-AVARK" -va af -title "ACIM 4360 Test" -nosniffer -int1ap "APAdj1-4322-4717" -int1sta "4322-FC15-Interfere1" -perftime 20 -perfsize 10 -interfere_mode 4 -interfere_ap_nrate 1 -interfere_sta_nrate 1 -attn_group G2 -attn_step_list "90 80 70 60 50 40 30 20"
# Test/ACIMNightly.test -utfconf utfconf/MCO.tcl -ap "APDUT-4331-4706 APDUT-4360-4706" -sta "43142-FC15-DUT" -va af -title "ACIM 43142 Test" -nosniffer -int1ap "APAdj1-4322-4717" -int1sta "4322-FC15-Interfere1" -perftime 20 -perfsize 10 -interfere_mode 4 -interfere_ap_nrate 1 -interfere_sta_nrate 1 -attn_group G2
# RvR:
#	Test/RvRNightly1.test -utfconf utfconf/MCO.tcl -ap "APDUT-4360-4706" -sta "4360-FC15-DUT" -va af -steplist "0 5 10 15 20 25 30 35 40" -nocontrvr -no5G40 -no5G20 -no2G40 -title "MCO RvR Test" -perftime 5 -upstreamonly
#	Test/RvRNightly1.test -utfconf utfconf/MCO.tcl -ap "APAdj1-4322-4717" -sta "4322-FC15-Interfere1" -va af -steplist "0 5 10 15 20 25 30 35 40" -nocontrvr -no5G40 -no5G20 -no2G40 -title "MCO RvR Test" -perftime 5 -upstreamonly
#
#   Test/StaNightly.test -utfconf utfconf/MCO.tcl -title "MCO 4360 StaNightly" -sta "4360-FC15-DUT-AVARK" -ap "APDUT-4331-4706 APDUT-4360-4706"
#   Test/StaNightly.test -utfconf utfconf/MCO.tcl -title "MCO 4331 StaNightly" -sta "4331-FC15-DUT-AVARK" -ap "APDUT-4331-4706 APDUT-4360-4706"
#   Test/StaNightly.test -utfconf utfconf/MCO.tcl -title "MCO 43142 StaNightly" -sta "43142-FC15-DUT" -ap "APDUT-4331-4706 APDUT-4360-4706" -rateonly
#   Test/StaNightly.test -utfconf utfconf/MCO.tcl -title "MCO 4322Int1 StaNightly" -sta "4322-FC15-Interfere1" -ap "APAdj1-4322-4717" -rateonly
#   Test/StaNightly.test -utfconf utfconf/MCO.tcl -title "MCO 4360Int2 StaNightly" -sta "4360-FC15-Interfere2" -ap "APAdj2-4360-4706" -rateonly
#
package require UTF::Sniffer
package require UTF::Linux
#package require UTF::Aeroflex
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/$::env(LOGNAME)/2017/MCO"

# Define power controllers on cart.
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCOUTF -rev 1
UTF::Power::Synaccess npc20 -lan_ip 172.16.1.20 -relay MCOUTF -rev 1
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCOUTF -rev 1
UTF::Power::WebRelay WebSwitch50 -lan_ip 172.16.1.50 -relay lan
UTF::Power::WebRelay WebSwitch51 -lan_ip 172.16.1.51 -relay lan

# Attenuator - Aeroflex
# G1 - Channels 1 & 2 & 3 	- APDUT-4360
# G2 - Channel  4 			- APAdj1-4705
# G3 - Channel  5 			- APAdj2-4704
UTF::AeroflexDirect af -lan_ip 172.16.1.210 -debug 1 -retries 0 -concurrent 0 -silent 0 -group {G1 {1 2 3} G2 {4} G3 {5} ALL {1 2 3 4 5}}
G1 configure -default 0
G2 configure -default 103
G3 configure -default 103
ALL configure -default 103

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

	set ::UTF::APList "APDUT-4360-4706 APAdj1-4322-4717 APAdj2-4360-4706"
	set ::UTF::STAList "43142-FC15-DUT 4360-FC15-DUT 4331-FC15-DUT 4322-FC15-Interfere1 4360-FC15-Interfere2"
	set ::UTF::RebootList "$::UTF::STAList"
	set ::UTF::DownList "$::UTF::APList $::UTF::STAList"

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

# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux MCOUTF \
	-lan_ip 10.19.85.48 \
    -sta "lan eth2" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

##################################################################################################################

# STA Laptop DUT Dell E6400
UTF::Linux MCODUT43142 \
	-lan_ip 10.19.85.53 \
	-sta {43142-FC15-DUT eth0} \
    -power "WebSwitch50 2"\
    -power_button "auto"\
	-tcpwindow 1152k \
    -perfchans {1 6 11} \
	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl rssi}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl nrate} {%S wl rate}} \
    -tag BISON_BRANCH_7_10

43142-FC15-DUT configure -ipaddr 192.168.1.204

# STA Laptop DUT Dell E6400
UTF::Linux MCODUT4331 \
	-lan_ip 10.19.85.55 \
	-sta {4331-FC15-DUT eth0} \
	-power "WebSwitch51 2"\
	-power_button {auto} \
	-tcpwindow 1152k \
	-tag BISON_BRANCH_7_10 \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl phy_cal_disable 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}}

4331-FC15-DUT configure -ipaddr 192.168.1.208

# STA Laptop DUT Dell E6400 FC15
UTF::Linux MCODUT4360  \
	-lan_ip 10.19.85.54 \
	-sta {4360-FC15-DUT eth0} \
	-power "WebSwitch50 1"\
	-power_button "auto"\
	-tcpwindow 4M \
	-perfchans {161/80 36l 3} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump}} \
	-nobighammer 0 \
	-brand linux-internal-wl \
	-tag AARDVARK_BRANCH_6_30

4360-FC15-DUT configure -ipaddr 192.168.1.207

##################################################################################################################

# STA Laptop DUT Dell E6400 FC15
UTF::Linux MCOInterfere1  \
	-lan_ip 10.19.85.50 \
	-sta {4322-FC15-Interfere1 wlan0} \
	-power "npc20 2"\
	-power_button "auto"\
	-tcpwindow 1152k \
    -perfchans {1 6} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump}} \
	-nobighammer 0 \
	-brand linux-internal-wl \
	-tag AARDVARK_BRANCH_6_30

4322-FC15-Interfere1 configure -ipaddr 192.168.1.205

# STA Laptop DUT Dell E6400 FC15
UTF::Linux MCOInterfere2  \
	-lan_ip 10.19.85.52 \
	-sta {4360-FC15-Interfere2 eth0} \
	-power "npc30 2"\
	-power_button "auto"\
	-tcpwindow 4M \
	-perfchans {36 40 52 64 100 108 120 140 144} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump}} \
	-nobighammer 0 \
	-brand linux-internal-wl \
	-tag AARDVARK_BRANCH_6_30

4360-FC15-Interfere2 configure -ipaddr 192.168.1.206

##################################################################################################################

#
# 4706/4360 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router APDUT \
    -lan_ip 192.168.1.10 \
    -sta {APDUT-4360-4706 eth1 APDUT-4331-4706 eth2 APDUT-4360-4706.%15 wl1.%} \
    -relay lan \
    -lanpeer lan \
    -console "lab35hnd-mirabadi:40010" \
	-tag AARDVARK_REL_6_30_317 \
    -brand linux26-internal-router \
    -power "npc10 1" \
    -txt_override {
        watchdog=6000
    } \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -nvram {
        watchdog=6000; # PR#90439
		boardtype=0x05b2; # 4706nr
        fw_disable=1
        wl_msglevel=0x101
        wl_stbc_rx=1
        wl_stbc_tx=1
		wl0_ssid=MCODUT5G
		wl0_chanspec=36
		wl0_radio=0
		wl1_ssid=MCODUT2G
		wl1_chanspec=1
		wl1_radio=0
		wl0_obss_coex=0
        wl_dmatxctl=0x24c0040
        wl_dmarxctl=0x24c0000
        wl_pcie_mrrs=128
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
    } \
    -perfchans {161/80 36l 36} -channelsweep {-band a} 

APDUT-4360-4706 configure -attngrp G1
APDUT-4331-4706 configure -attngrp G1

# Linksys 320N 4717/4322 wireless router.
# Adjecent 1 is used for 2.4G testing only
UTF::Router APAdj1 \
    -lan_ip 192.168.1.20 \
    -sta "APAdj1-4322-4717 eth1" \
    -relay "MCOUTF" \
    -power "npc20 1" \
    -lanpeer lan \
    -console "lab35hnd-mirabadi:40020" \
    -tag AKASHI_REL_5_110_65 \
	-brand linux-external-router \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
		macaddr=00:90:4C:A6:00:20
        wl0_ssid=MCOAdj1
		lan_ipaddr=192.168.1.20
		lan_gateway=192.168.1.20
		dhcp_start=192.168.1.120
		dhcp_end=192.168.1.129
		lan1_ipaddr=192.168.2.20
		lan1_gateway=192.168.2.20
		dhcp1_start=192.168.2.120
		dhcp1_end=192.168.2.129
		antswitch=0
		# 5 GHz band (2->2.4 GHz, 1->5 GHz) 
		wl0_nband=2
		# Channel
		wl0_channel=6
		# Bandwidth (0->20 GHz on both channels, 1->40 GHz on both channels, 2->20 GHz 0n 2.4 and 40 GHz on 5)
		wl0_nbw_cap=0
    }    

APAdj1-4322-4717 configure -attngrp G2

# Adjecent 2 is used for 5G testing therefore using only APAdj2-4360-4706
UTF::Router APAdj2 \
    -lan_ip 192.168.1.30 \
    -sta {APAdj2-4331-4706 eth1 APAdj2-4360-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab35hnd-mirabadi:40030" \
    -tag AARDVARK_REL_6_30_317 \
    -brand linux26-internal-router \
    -power "npc30 1" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-noradio_pwrsave 1 \
    -nvram {
        fw_disable=1
        wl_msglevel=0x101
		macaddr=00:90:4C:0D:C9:3F
		wl0_ssid=MCO2GAdj2
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCO5GAdj2
		wl1_chanspec=36
		wl1_radio=0
        wl1_bw_cap=-1
		wl1_obss_coex=0
        et_msglevel=0; # WAR for PR#107305
		#
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.30
		dhcp_start=192.168.1.310
		dhcp_end=192.168.1.319
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.30
		dhcp1_start=192.168.2.310
		dhcp1_end=192.168.2.319
		antswitch=0
		1:boardflags2=0x4000000
    }

APAdj2-4360-4706 configure -attngrp G3
APAdj2-4331-4706 configure -attngrp G3

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab35hnd-mirabadi
##########################################################################################
UTF::Q MCO

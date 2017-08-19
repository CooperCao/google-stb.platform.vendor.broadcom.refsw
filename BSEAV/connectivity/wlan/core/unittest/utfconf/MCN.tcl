# -*-tcl-*-

#
# MCN testbed configuration for 3x3 4331 Apple
#
#
# Test/StaNightly.test -utfconf utfconf/MCN.tcl -title "MCN StaNightly" -sta "4331-MacX19-DUT" -ap "AP1-4331-4706 AP1-4360-4706" -noscan -nochannels -nopm -nojoin -noshared -nobtests -nofrag -noaes -notkip -nowep -nobighammer -nocal
# Test/RoamingNightly.test -utfconf utfconf/MCN.tcl -title "MCN Roaming" -sta "4331-MacX19-DUT" -from_ap "AP1-4331" -to_ap "AP3-4331-4718" -no5G40 -no5G20 -no2G40 -va "af" -sniffer "snif" -novoiceroaming -noapload -norestore -nofailoverroaming
# Test/P2PQoSNightly.test -utfconf utfconf/MCN.tcl -title "4360 Same Channel P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-MBAX29-GO" -sta_gc "4360-MBAX29-DUT" -nom -nod -statag AARDVARK_BRANCH_6_30 -ap_connect "GC" -p2p_chan "36l 36 1" -ap_chan "157l 157 11" -perftime 25 -debug_flag -bin "/projects/hnd/software/work/ishen/test/branches/RUBY_BRANCH_6_20/src"
# 4360 using my build:
# Test/P2PQoSNightly.test -utfconf utfconf/MCN.tcl -title "MacOS Same Channel 4360 P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-MBAX29-GO" -sta_gc "4360-MBAX29-DUT" -nom -nod -perftime 25 -statag AARDVARK_BRANCH_6_30 -bin "/projects/hnd_sig_ext11/build/mac/r382933/AARDVARK_BRANCH_6_30/src"
# New 4706 router:
# Test/StaNightly.test -utfconf utfconf/MCN.tcl -title "MCN MacOS 4360 StaNightly" -sta "4360-MacX29c-DUT" -ap "AP1-4331-4706 AP1-4360-4706"
# Test/StaNightly.test -utfconf utfconf/MCN.tcl -title "MCN MacOS 4360 StaNightly" -sta "4360-MacX29c-DUT" -ap "AP1-4331-4706 AP1-4360-4706" -bin /projects/hnd_sig_ext11/build/prmcn/r352277/AARDVARK_BRANCH_6_30/src/wl/macos/build/Debug_10_7/AirPortBroadcom43XX.kext
# WLAN Beamforming:
# Test/RvRNightly1.test -utfconf utfconf/MCN.tcl -title "MCN ML 4360 RvR beam Test" -ap "AP1-4360-4706" -sta "4360-BFOFF-GO 4360-BFON-GO" -va af -cycle5G40AttnRange "10-65 65-10" -cycle5G20AttnRange "10-65 65-10" -cycle2G40AttnRange "10-70 70-10" -cycle2G20AttnRange "10-70 70-10" -nocontrvr -chan5G80 36/80 -no2G20 -no2G40 -no5G20 -no5G40 -perftime 5 -comparesta
# P2P Beamforming:
# Test/P2PQoSNightly.test -utfconf utfconf/MCN.tcl -title "MCN ML BF OFF Direct 4360 P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-BFOFF-GO" -sta_gc "4360-BFOFF-DUT" -nom -nos -perftime 25 -statag AARDVARK_BRANCH_6_30 -p2p_chan "36l" -ap_chan "36l"
# Test/P2PQoSNightly.test -utfconf utfconf/MCN.tcl -title "MCN ML BF ON Direct 4360 P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-BFON-GO" -sta_gc "4360-BFON-DUT" -nom -nos -perftime 25 -statag AARDVARK_BRANCH_6_30 -p2p_chan "36l" -ap_chan "36l"
# Test/P2PQoSNightlyRvR.test -utfconf utfconf/MCN.tcl -title "MCN ML BF ON RvR 4360 P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-BFON-GO" -sta_gc "4360-BFON-DUT" -nos -nod -perftime 5 -p2p_chan "36l" -ap_chan "149l" -ap_connect GC -attn_type 2 -statag AARDVARK_TWIG_6_30_223 -noapload -norestore
# Test/P2PQoSNightlyRvR.test -utfconf utfconf/MCN.tcl -title "MCN ML BF OFF RvR 4360 P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-BFOFF-GO" -sta_gc "4360-BFOFF-DUT" -nos -nod -perftime 5 -p2p_chan "36l" -ap_chan "149l" -ap_connect GC -attn_type 2 -statag AARDVARK_TWIG_6_30_223 -noapload -norestore
# Test/P2PQoSNightlyRvR.test -utfconf utfconf/MCN.tcl -title "MCN ML BF P2P RvR 4360 Same Chan" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-BFOFF-GO 4360-BFON-GO" -sta_gc "4360-BFOFF-GC 4360-BFON-GC" -nom -nod -perftime 5 -p2p_chan "36l" -ap_chan "36l" -ap_connect GC -attn_type 2 -nop2pbid -nop2pup -nowlanbid -nowlanup -noapload -norestore -composite_graphs
# Test/P2PQoSNightlyRvR.test -utfconf utfconf/MCN.tcl -title "MCN ML BF P2P RvR 4360 Multi-Chan" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "4360-BFOFF-GO 4360-BFON-GO" -sta_gc "4360-BFOFF-GC 4360-BFON-GC" -nos -nod -perftime 5 -p2p_chan "36l" -ap_chan "149l" -ap_connect GC -attn_type 2 -nop2pbid -nop2pup -nowlanbid -nowlanup -composite_graphs -bin /projects/hnd_sw_media/work/chandrr/6.30.223/AARDVARK_TWIG_6_30_223/src
#

package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/$::env(LOGNAME)/2017/MCN"

# Define power controllers on cart.
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCNUTF -rev 1
UTF::Power::Synaccess npc20 -lan_ip 172.16.1.20 -relay MCNUTF
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCNUTF
UTF::Power::WebRelay WebRelay51 -lan_ip 172.16.1.51
UTF::Power::WebRelay WebRelay52 -lan_ip 172.16.1.52
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCNUTF -rev 1

# sniffer
# uses wireless interface eth1
UTF::Sniffer MCNSNIF \
	-lan_ip 172.16.1.201 \
	-sta {snif eth1} \
    -power "npc50 1"\
    -power_button "auto"\
	-user root \
	-tag AARDVARK_REL_6_30_182 

# Attenuator - Aeroflex
# G1 - Channels 1 & 2 & 3 	- AP1 & AP2
# G2 - Channel  4 & 5 & 6	- AP3 & AP4
# G3 - Channel  7 & 8 & 9	- For P2P GC-GO RvR and beamforming
UTF::Aeroflex af -lan_ip 172.16.1.210 -relay MCNUTF -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9}}
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
				UTF::Message INFO "" "ssh $user_name\@$lan_ip ls"
				set catch_resp [catch {exec ssh $user_name\@$lan_ip ls} catch_msg]
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

					set catch_resp [catch {exec ssh root@$lan_ip ls} catch_msg]
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

	set ::UTF::APList "AP1-4360-4706 AP3-E4200"
	set ::UTF::STAList "4331-MBPX19-GO 4331-MBPX19-DUT 4360-MBAX29-GO 4360-MBAX29-DUT"
	set ::UTF::RebootList "snif $::UTF::STAList"
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

# Add for debugging P2P:
#-wlinitcmds {wl msglevel +p2p +mchan +assoc +apsta +ps +inform} \

##########################################################################################
#						STA 4331 Chip 
##########################################################################################

# P2P GO - Macbook pro i7 4331
UTF::Power::Laptop 4331_go_power -button {WebRelay52 1}
UTF::MacOS MCNGOMBP4331 \
	-lan_ip 10.19.13.228 \
	-sta {4331-MBPX19-GO en1 4331-MBPX19-PGO p2p0} \
    -power {4331_go_power} \
	-tcpwindow 1152k \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -coreserver AppleCore \
    -kextload true\
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-tag BISON_BRANCH_7_10

4331-MBPX19-GO configure -ipaddr 192.168.1.234
4331-MBPX19-PGO configure -ipaddr 192.168.5.234

# P2P DUT - Macbook pro i7 4331
UTF::Power::Laptop 4331_dut_power -button {WebRelay51 1}
UTF::MacOS MCNDUTMBP4331 \
	-lan_ip 10.19.13.249 \
	-sta {4331-MBPX19-DUT en1 4331-MBPX19-PDUT p2p0} \
    -power {4331_dut_power} \
	-tcpwindow 1152k \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1} \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -coreserver AppleCore \
    -kextload true\
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-tag BISON_BRANCH_7_10

4331-MBPX19-DUT configure -ipaddr 192.168.1.236
4331-MBPX19-PDUT configure -ipaddr 192.168.5.236

##########################################################################################
#						STA 4360 Chip 
##########################################################################################

# P2P GO - Macbook Air i5 4360
UTF::Power::Laptop 4360_go_power -button {WebRelay52 2}
UTF::MacOS MCNGOMBA4360 \
	-lan_ip 10.19.13.169 \
	-sta {4360-MBAX29-GO en0 4360-MBAX29-PGO p2p0} \
    -power {4360_go_power} \
	-tcpwindow 3640K \
	-wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl up; wl assert_type 1; wl dump txbf } \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl amsdu_clear_counters} {%S wl txbf} {%S wl txbf_bfe_cap} {%S wl txbf_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-perfchans {36/80 36l 3} \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-tag BISON_BRANCH_7_10

4360-MBAX29-GO clone MCN-BFOFF-GO
MCN-BFOFF-GO configure \
	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl assert_type 1; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl up }
4360-MBAX29-GO clone MCN-BFON-GO
MCN-BFON-GO configure \
	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl assert_type 1; wl txbf 1; wl txbf_bfe_cap 1; wl txbf_bfr_cap 1; wl up }

4360-MBAX29-GO configure -ipaddr 192.168.1.234
4360-MBAX29-PGO configure -ipaddr 192.168.5.234

# ; wl down; wl amsdu 0; wl up

# P2P DUT - Macbook Air i5 4360
UTF::Power::Laptop 4360_dut_power -button {WebRelay51 2}
UTF::MacOS MCNDUTMBA4360 \
	-lan_ip 10.19.13.168 \
	-sta {4360-MBAX29-DUT en0 4360-MBAX29-PDUT p2p0} \
    -power {4360_dut_power} \
	-tcpwindow 3640K \
	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl up; wl assert_type 1; wl dump txbf } \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl amsdu_clear_counters} {%S wl txbf} {%S wl txbf_bfe_cap} {%S wl txbf_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-perfchans {36/80 36l 3} \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-tag BISON_BRANCH_7_10

4360-MBAX29-DUT clone MCN-BFOFF-GC
MCN-BFOFF-GC configure \
	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl assert_type 1; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl txchain 1; wl rxchain 1; wl up }
4360-MBAX29-DUT clone MCN-BFON-GC
MCN-BFON-GC configure \
	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl assert_type 1; wl txbf 1; wl txbf_bfe_cap 1; wl txbf_bfr_cap 1; wl txchain 1; wl rxchain 1; wl up }

#	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl assert_type 1; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl txchain 1; wl rxchain 1; wl up }
#	-wlinitcmds { wl msglevel +assoc; wl btc_mode 0; wl down; wl bw_cap 2g -1; wl vht_features 3; wl assert_type 1; wl txbf 1; wl txbf_bfe_cap 1; wl txbf_bfr_cap 1; wl txchain 1; wl rxchain 1; wl up }

4360-MBAX29-DUT configure -ipaddr 192.168.1.236
4360-MBAX29-PDUT configure -ipaddr 192.168.5.236

##########################################################################################
# UTF Endpoint - Traffic generators (no wireless cards)
##########################################################################################
UTF::Linux MCNUTF \
	-lan_ip 10.19.13.193 \
    -sta "lan eth2" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

##################################################################################################################

##########################################################################################
#								AP 4331/4360 Netgrear N6300
##########################################################################################
UTF::Router AP1-4706 \
    -lan_ip 192.168.1.10 \
    -sta {AP1-4360-4706 eth1 AP1-4331-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab21hnd-mirabadi:40010" \
    -tag AARDVARK01T_TWIG_6_37_14 \
    -brand linux26-internal-router \
    -power "npc10 1" \
	-wlinitcmds { wl dump txbf } \
    -txt_override {
        watchdog=6000
    } \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -nvram {
        watchdog=6000; # PR#90439
		boardtype=0x05b2; # 4706nr
        fw_disable=1
        wl_msglevel=0x101
        et0macaddr=00:90:4C:0D:Bf:71
		macaddr=00:90:4C:02:11:00
        wl_stbc_rx=1
        wl_stbc_tx=1
		wl0_ssid=MCNDUT5G
		wl0_chanspec=36
		wl0_radio=0
		wl1_ssid=MCNDUT2G
		wl1_chanspec=1
		wl1_radio=0
		wl0_obss_coex=0
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

AP1-4360-4706 configure -attngrp G1
AP1-4331-4706 configure -attngrp G1
    
##########################################################################################
# Linksys E4200 3x3 4718/4331 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
##########################################################################################
UTF::Router AP3-E4200 \
    -lan_ip 192.168.1.30 \
    -sta "AP3-4718 eth1 AP3-4331 eth2" \
    -power "npc30 1" \
    -relay lan \
    -lanpeer lan \
    -console "lab21hnd-mirabadi:40030" \
    -tag "AKASHI_REL_5_110_65" \
	-brand linux-external-router \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
		boot_hw_model=E4200
		pci/1/1/macaddr=00:90:4c:02:31:00
		sb/1/macaddr=00:90:4c:02:30:00
       	et0macaddr=00:02:36:1F:02:59
		macaddr=00:90:4C:02:31:00
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.30
		dhcp_start=192.168.1.130
		dhcp_end=192.168.1.139
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.30
		dhcp1_start=192.168.2.130
		dhcp1_end=192.168.2.139
		antswitch=0
		#simultaneous dual-band router with 2 radios
		wl0_radio=0
        wl0_ssid=MCNDUT2GONLY
		wl1_radio=0
        wl1_ssid=MCNDUTCOMBO
		# 5 GHz band (2->2.4 GHz, 1->5 GHz) 
		wl1_nband=1
		# Channel
		wl1_channel=36
		# Bandwidth (0->20 GHz on both channels, 1->40 GHz on both channels, 2->20 GHz 0n 2.4 and 40 GHz on 5)
		wl1_nbw_cap=1
        wl1_obss_coex=0
        #wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
    }    

AP3-4718 configure -attngrp G2
AP3-4331 configure -attngrp G2

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab21hnd-mirabadi
##########################################################################################
UTF::Q MCN

# -*-tcl-*-

#
# MCG CIT testbed configuration for Linux P2P
#
#

# For using attenuator, need this package as UTF.tcl doesnt load it by default.
package require UTF::Sniffer
package require UTF::Linux
#package require UTF::Aeroflex
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/$::env(LOGNAME)/2017/MCG"

# Define power controllers on cart.
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCGUTF
UTF::Power::Synaccess npc20 -lan_ip 172.16.1.20 -relay MCGUTF
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCGUTF
UTF::Power::Synaccess npc40 -lan_ip 172.16.1.40 -relay MCGUTF
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCGUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCGUTF -rev 1

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

	set ::UTF::APList "AP1-4360-4706 AP3-4321-4705"
	set ::UTF::APList ""
	set ::UTF::STAList "43602-DUT 43602-GO 43602-DUTNP"
	set ::UTF::RebootList "4360-SoftAP $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP $::UTF::APList $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP $::UTF::STAList"

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
			return
		}
		
    	unset AP  
    }

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

UTF::Linux MCGUTF \
	-lan_ip 10.19.85.43 \
	-iperf iperf208 \
    -sta "lan p4p1" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

#-----------------------------------------------------------------------------------------
#						                   STAs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#						                43602 Chip 
##########################################################################################

UTF::DHD MCGGO43602 \
	-lan_ip 10.19.85.45 \
	-iperf iperf208 \
	-sta {43602-GO eth0 43602-PGO wl0.1} \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole lab30hnd-mirabadi:40061 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag trunk \
	-tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx-txqmux/rtecdc.bin \
    -clm_blob 43602_sig.clm_blob -clm_blob {} \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602-GO configure -ipaddr 192.168.1.234
43602-PGO configure -ipaddr 192.168.5.234

43602-GO clone 43602-Mstt \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux

UTF::DHD 43602-Msttawdl \
	-lan_ip 10.19.85.45 \
	-iperf iperf208 \
	-sta {43602-Mstt-awdl eth0 43602-PGO-awdl wl0.1} \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole lab30hnd-mirabadi:40061 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-tag trunk \
	-brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux \
	-driver dhd-msgbuf-pciefd-debug \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx-txqmux/rtecdc.bin \
    -clm_blob 43602_sig.clm_blob -clm_blob {} \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602-Mstt-awdl configure -ipaddr 192.168.1.234
43602-PGO-awdl configure -ipaddr 192.168.5.234

43602-GO clone 43602-Mstnt \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-nan-assert-redux.bin

#    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-logtrace-redux-proxd/rtecdc.bin

43602-GO clone 43602-Msttmc \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-splitrx-norsdb-proxd-assert-redux/rtecdc.bin

#	-name GOnic \

UTF::Linux MCGGO43602nic \
	-lan_ip 10.19.85.45 \
	-iperf iperf208 \
	-sta {43602nic-GO enp1s0 43602nic-PGO wl0.1} \
	-power "npc60 1" \
	-power_button "auto" \
    -console lab30hnd-mirabadi:40061 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-tag trunk \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc +regulatory; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602nic-GO configure -ipaddr 192.168.1.234
43602nic-PGO configure -ipaddr 192.168.5.234

UTF::Linux MCGGO43602nict \
	-lan_ip 10.19.85.45 \
	-iperf iperf208 \
	-sta {43602nict-GO enp1s0 43602nict-PGO wl0.1} \
	-power "npc60 1" \
	-power_button "auto" \
    -console lab30hnd-mirabadi:40061 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-tag trunk \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc +regulatory; wl vht_features 3; wl olpc 0; wl olpc; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602nict-GO configure -ipaddr 192.168.1.234
43602nict-PGO configure -ipaddr 192.168.5.234

#####################################

UTF::DHD MCGDUT43602 \
	-lan_ip 10.19.85.44 \
	-iperf iperf208 \
	-sta {43602-DUT eth0 43602-PDUT wl0.1} \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole lab30hnd-mirabadi:40051 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag trunk \
	-tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx-txqmux/rtecdc.bin \
    -clm_blob 43602_sig.clm_blob -clm_blob {} \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602-DUT configure -ipaddr 192.168.1.236
43602-PDUT configure -ipaddr 192.168.5.236 -attngrp G3

#        -perfonly 1 -perfchans {36/80} \

43602-DUT clone 43602-NonMstt \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux

UTF::DHD 43602-NonMsttawdl \
	-lan_ip 10.19.85.44 \
	-iperf iperf208 \
	-sta {43602-NonMstt-awdl eth0 43602-PDUT-awdl wl0.1} \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole lab30hnd-mirabadi:40051 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-tag trunk \
	-brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux \
	-driver dhd-msgbuf-pciefd-debug \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx-txqmux/rtecdc.bin \
    -clm_blob 43602_sig.clm_blob -clm_blob {} \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602-NonMstt-awdl configure -ipaddr 192.168.1.236
43602-PDUT-awdl configure -ipaddr 192.168.5.236 -attngrp G3

43602-DUT clone 43602-NonMstnt \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-nan-assert-redux.bin

43602-DUT clone 43602-NonMsttmc \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-splitrx-norsdb-proxd-assert-redux/rtecdc.bin

#	-name DUTnic \

UTF::Linux MCGDUT43602nic \
	-lan_ip 10.19.85.44 \
	-iperf iperf208 \
	-sta {43602nic-DUT enp1s0 43602nic-PDUT wl0.1} \
	-power "npc50 1" \
	-power_button "auto" \
    -console lab30hnd-mirabadi:40051 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-tag HORNET_BRANCH_12_10 \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc +regulatory; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602nic-DUT configure -ipaddr 192.168.1.236
43602nic-PDUT configure -ipaddr 192.168.5.236 -attngrp G3


UTF::Linux MCGDUT43602nict \
	-lan_ip 10.19.85.44 \
	-iperf iperf208 \
	-sta {43602nict-DUT enp1s0 43602nict-PDUT wl0.1} \
	-power "npc50 1" \
	-power_button "auto" \
    -console lab30hnd-mirabadi:40051 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-tag trunk \
    -type debug-apdef-stadef-p2p-mchan-tdls \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc +regulatory; wl vht_features 3; wl olpc 0; wl olpc; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602nict-DUT configure -ipaddr 192.168.1.236
43602nict-PDUT configure -ipaddr 192.168.5.236 -attngrp G3


#####################################

UTF::DHD MCGDUTNP43602 \
	-lan_ip 10.19.85.46 \
	-iperf iperf208 \
	-sta {43602-DUTNP eth0 43602-PDUTNP wl0.1} \
	-power "npc50 2" \
	-power_button "auto" \
    -hostconsole lab30hnd-mirabadi:40052 \
    -tcpwindow 4M -slowassoc 5 -udp 1.2g -extsup 0 -nocal 1 -docpu 1 -reloadoncrash 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag trunk \
	-tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-internal-dongle-pcie \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx-txqmux/rtecdc.bin \
    -clm_blob 43602_sig.clm_blob -clm_blob {} \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -msgactions {
	"Pktid pool depleted." WARN
    }

43602-DUTNP configure -ipaddr 192.168.1.238
43602-PDUTNP configure -ipaddr 192.168.5.238 -attngrp G3

43602-DUTNP clone 43602-NonMst2t \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux

43602-DUTNP clone 43602-NonMst2nt \
		-dhd_tag trunk \
		-tag trunk \
    	-brand linux-external-dongle-pcie \
		-dhd_brand linux-external-dongle-pcie \
    	-type 43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-nan-assert-redux.bin

#####################################

#-----------------------------------------------------------------------------------------
#						                   APs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#										SoftAP1 4360
##########################################################################################
UTF::Linux MCGSoftAP \
	-lan_ip 10.19.61.64 \
	-iperf iperf208 \
    -sta {4360-SoftAP enp1s0} \
    -console "lab30hnd-mirabadi:40042" \
    -power "npc40 2" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl msglevel; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3; wl country US} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

4360-SoftAP configure -ipaddr 192.168.1.91 -attngrp G2 -hasdhcpd 1 -ap 1

##########################################################################################
#								AP 4331/4360 Netgrear N6300
##########################################################################################
UTF::Router AP1-4706 \
    -lan_ip 192.168.1.10 \
    -sta {AP1-4331-4706 eth1 AP1-4360-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab30hnd-mirabadi:40010" \
    -tag AARDVARK01T_TWIG_6_37_14 \
    -brand linux26-internal-router \
    -power "npc10 1" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl rssi}} \
	-noradio_pwrsave 1 \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
		macaddr=00:90:4C:0E:50:d3
		wl0_ssid=MCG2GAP1
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCG5GAP1
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
		1:boardflags2=0x4000000
    }

AP1-4360-4706 configure -attngrp G1
AP1-4331-4706 configure -attngrp G1

##########################################################################################
#						               AP Linksys WRT600N 4705/4321 
##########################################################################################

UTF::Router AP3 \
	-lan_ip 192.168.1.30 \
	-sta "AP3-4321-4705 eth1" \
	-relay "MCGUTF" \
	-power "172.16.1.30 1" \
	-lanpeer lan \
	-console "lab30hnd-mirabadi:40030" \
    -tag "AKASHI_REL_5_110_65" \
	-brand linux-external-router \
	-date * \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
	-nvram {
	   	fw_disable=1
	   	wl_msglevel=0x101
		# Switch WRT600n LAN to vlan1, since vlan0 doesn't work
		"vlan1ports=1 2 3 4 8*"
		vlan1hwname=et0
		"landevs=vlan1 wl0 wl1"
		"lan_ifnames=vlan1 eth1 eth2"
		# Enable untagging on vlan2 else WAN doesn't work
		"vlan2ports=0 8u"
		macaddr=00:90:4C:D6:01:B5
		et0macaddr=00:90:4C:D6:01:B5
		antswitch=0
		#simultaneous dual-band router with 2 radios
		wl0_radio=0
		wl1_radio=0
        wl0_ssid=MCG-2G
        wl1_ssid=MCG-5G
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.30
		dhcp_start=192.168.1.130
		dhcp_end=192.168.1.139
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.30
		dhcp1_start=192.168.2.130
		dhcp1_end=192.168.2.139
		# 5 GHz band (2->2.4 GHz, 1->5 GHz) 
		wl0_nband=2
		# Channel
		wl0_channel=3
		# Bandwidth (0->20 GHz on both channels, 1->40 GHz on both channels, 2->20 GHz 0n 2.4 and 40 GHz on 5)
		wl0_nbw_cap=0
		wl0_obss_coex=0
		wl1_obss_coex=0
	}	

AP3-4321-4705 configure -attngrp G2

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab30hnd-mirabadi
# To check the params:
# 	ssh lab30hnd-mirabadi ps -wo pid,args -p 14666,8802,10227,11576,17628
##########################################################################################
UTF::Q MCG

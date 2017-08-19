# -*-tcl-*-

#
# MCI CIT testbed configuration for Linux P2P
#
#
# PC 10.19.61.178
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
set ::UTF::SummaryDir "/projects/hnd_sig_ext19/$::env(LOGNAME)/2017/MCI"

# Define power controllers on cart.
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCIUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCIUTF -rev 1
UTF::Power::Synaccess npc70 -lan_ip 172.16.1.70 -relay MCIUTF -rev 1
UTF::Power::Synaccess npc80 -lan_ip 172.16.1.80 -relay MCIUTF -rev 1

# Attenuator - Aeroflex
# G1 - Channels 1 & 2 & 3 & 4	- AP1 & AP2
# G2 - Channel  5 & 6 & 7 & 8	- AP3 & AP4
# G3 - Channel  9 & 10 & 11 & 12 - For P2P GC-GO RvR
# Going through attenuator for the P2P RvR adds about 15 dB to the GC Ramsey box
UTF::AeroflexDirect af -lan_ip 172.16.1.210 -group {G1 {1 2 3 4} G2 {5 6 7 8} G3 {9 10 11 12} ALL {1 2 3 4 5 6 7 8 9 10 11 12}}
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

	#set ::UTF::APList "AP1-4709-4366 AP3-4709-4366"
	set ::UTF::APList ""
	set ::UTF::STAList "4357 4357G"
	set ::UTF::RebootList "4366c0-SoftAP1 4366c0-SoftAP2 $::UTF::STAList"
	set ::UTF::DownList "4366c0-SoftAP1 4366c0-SoftAP2 $::UTF::APList $::UTF::STAList"

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
# mcptst11
UTF::Linux MCIUTF \
	-lan_ip 10.19.61.177 \
	-iperf iperf208 \
    -sta "lan p4p1" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

#-----------------------------------------------------------------------------------------
#						                   STAs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#						STA 4357 PCIe Chip - Brix
##########################################################################################

# wl and dhd from /projects/hnd/swbuild/build_linux/trunk/linux-combined-apps/2016.10.26.2/internal/bcm/x86_64/

UTF::DHD MCI4357GO \
	-lan_ip 10.19.61.182 \
	-iperf iperf208 \
	-sta {4357G eth0} \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole mcptst11:40066 \
    -nvram_add {macaddr=00:90:4C:12:D0:03} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
    -clm_blob 4357.clm_blob \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl band a;
	wl vht_features 7;
	wl country US/0;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
    -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" FAIL
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" WARN
            "MQ_ERROR" FAIL
            "segfault" FAIL
            "PSM microcode watchdog" FAIL
    }

4357G clone 4357G.2 -sta {_4357G eth0 4357G.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357G clone 4357Gx \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin
4357G.2 clone 4357Gx.2 \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin

4357G clone 4357Gt \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357G.2 clone 4357Gt.2 \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

#4357G configure -dualband {4366c0-SoftAP2 4357G.2 -c1 36/80 -c2 3l -b1 800m -b2 800m} -ipaddr 192.168.1.234
#4357Gx configure -dualband {4366c0-SoftAP2 4357Gx.2 -c1 36/80 -c2 3l -b1 800m -b2 800m} -ipaddr 192.168.1.234
#4357Gt configure -dualband {4366c0-SoftAP2 4357Gt.2 -c1 36/80 -c2 3l -b1 800m -b2 800m} -ipaddr 192.168.1.234

4357G configure -ipaddr 192.168.1.234

######## P2P GO ########
4357Gt clone 4357i-PGO-WLAN \
    -clm_blob 4357.clm_blob \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357i-PGO" \
	-sta {4357i-PGO-WLAN eth0 4357i-PGO-P2P wl0.1}
4357i-PGO-WLAN configure -ipaddr 192.168.1.234    
4357i-PGO-P2P  configure -ipaddr 192.168.5.234
######################

######## AWDL Master ########
4357G clone 4357-AMaster-WLAN \
	-postinstall {dhd -i eth0 dma_ring_indices 3; dhd -i eth0 h2d_phase 1; dhd -i eth0 force_trap_bad_h2d_phase 1} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
	wl bus:enable_ds_hc 0;
    } \
	-name "4357-AMaster" \
	-sta {4357-AMaster-WLAN eth0 4357-AMaster-AWDL wl0.2} \
	-dhd_tag DHD_REL_1_579_216 \
    -app_tag APPS_REL_1_46 \
    -app_brand linux-combined-apps \
    -clm_blob 4347a0.clm_blob \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin
4357-AMaster-WLAN configure -ipaddr 192.168.1.234    
4357-AMaster-AWDL configure -ipaddr 192.168.5.234
######################
4357-AMaster-WLAN clone 4357i-AMaster-WLAN \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
	-sta {4357i-AMaster-WLAN eth0 4357i-AMaster-AWDL wl0.2} \
    -clm_blob 4357.clm_blob \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
    -type 4357b1-roml/config_pcie_olympic/rtecdc.bin \
	-tag IGUANA_BRANCH_13_10
4357i-AMaster-WLAN configure -ipaddr 192.168.1.234    
4357i-AMaster-AWDL configure -ipaddr 192.168.5.234
######################

######## NAN Master ########
4357G clone 4357i-NanMaster \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357i-NanM" \
	-sta {4357i-NanMaster eth0 4357i-NanMaster-Aux wl0.2} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357_apolloe.clm_blob \
    -type 4357b1-roml/config_pcie_olympic_min_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -app_brand linux-combined-apps \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
	-tag IGUANA_BRANCH_13_10
4357i-NanMaster configure -ipaddr 192.168.1.234    
4357i-NanMaster-Aux configure -ipaddr 192.168.5.234
######################
4357i-NanMaster clone 4357t-NanMaster \
	-postcopy {
		$self touch .trunk
		if {![catch {$self rm .iguana}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
    } \
	-name "4357t-NanM" \
	-sta {4357t-NanMaster eth0 4357t-NanMaster-Aux wl0.2} \
    -brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-pciedw-debug \
    -clm_blob 4357{a0,}.clm_blob \
    -type 4357b1-ram/config_pcie_nan/rtecdc.bin \
    -nvram "bcm94357fcpagbe_p402.txt" \
	-tag trunk
4357t-NanMaster configure -ipaddr 192.168.1.234    
4357t-NanMaster-Aux configure -ipaddr 192.168.5.234
######################

#####################################

UTF::DHD MCI4357DUT \
	-lan_ip 10.19.61.181 \
	-iperf iperf208 \
	-sta {4357 eth0} \
    -name "4357-DUT" \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole mcptst11:40056 \
    -nvram_add {macaddr=00:90:4C:12:D0:02} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
    -clm_blob 4357.clm_blob \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl band a;
	wl vht_features 7;
	wl country US/0;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
    -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" FAIL
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" WARN
            "MQ_ERROR" FAIL
            "segfault" FAIL
            "PSM microcode watchdog" FAIL
    }

4357 clone 4357.2 -sta {_4357 eth0 4357.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357 clone 4357x \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin
4357.2 clone 4357x.2 \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin

4357 clone 4357t \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357.2 clone 4357t.2 \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

#4357 configure -dualband {4366c0-SoftAP2 4357.2 -c1 36/80 -c2 3l -b1 800m -b2 800m} -ipaddr 192.168.1.236
#4357x configure -dualband {4366c0-SoftAP2 4357x.2 -c1 36/80 -c2 3l -b1 800m -b2 800m} -ipaddr 192.168.1.236
#4357t configure -dualband {4366c0-SoftAP2 4357t.2 -c1 36/80 -c2 3l -b1 800m -b2 800m} -ipaddr 192.168.1.236

4357 configure -ipaddr 192.168.1.236

######## P2P GC ########
4357t clone 4357i-PGC-WLAN \
    -clm_blob 4357.clm_blob \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357i-PGC" \
	-sta {4357i-PGC-WLAN eth0 4357i-PGC-P2P wl0.1}
4357i-PGC-WLAN configure -ipaddr 192.168.1.236    
4357i-PGC-P2P  configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## AWDL Slave ########
4357 clone 4357-ASlave-WLAN \
	-postinstall {dhd -i eth0 dma_ring_indices 3; dhd -i eth0 h2d_phase 1; dhd -i eth0 force_trap_bad_h2d_phase 1} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
	wl bus:enable_ds_hc 0;
    } \
	-name "4357-ASlave" \
	-sta {4357-ASlave-WLAN eth0 4357-ASlave-AWDL wl0.2} \
	-dhd_tag DHD_REL_1_579_216 \
    -app_tag APPS_REL_1_46 \
    -app_brand linux-combined-apps \
    -clm_blob 4347a0.clm_blob \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin
4357-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357-ASlave-WLAN clone 4357i-ASlave-WLAN \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
	-sta {4357i-ASlave-WLAN eth0 4357i-ASlave-AWDL wl0.2} \
    -clm_blob 4357.clm_blob \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
    -type 4357b1-roml/config_pcie_olympic/rtecdc.bin \
	-tag IGUANA_BRANCH_13_10
4357i-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357i-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## NAN Slave ########
4357 clone 4357i-NanSlave \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357i-NanS" \
	-sta {4357i-NanSlave eth0 4357i-NanSlave-Aux wl0.2} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357_apolloe.clm_blob \
    -type 4357b1-roml/config_pcie_olympic_min_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -app_brand linux-combined-apps \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
	-tag IGUANA_BRANCH_13_10
4357i-NanSlave configure -ipaddr 192.168.1.236    
4357i-NanSlave-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357i-NanSlave clone 4357t-NanSlave \
	-postcopy {
		$self touch .trunk
		if {![catch {$self rm .iguana}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
    } \
	-name "4357t-NanS" \
	-sta {4357t-NanSlave eth0 4357t-NanSlave-Aux wl0.2} \
    -brand hndrte-dongle-wl \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357{a0,}.clm_blob \
    -type 4357b1-ram/config_pcie_nan/rtecdc.bin \
    -nvram "bcm94357fcpagbe_p402.txt" \
	-tag trunk
4357t-NanSlave configure -ipaddr 192.168.1.236    
4357t-NanSlave-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################

#####################################

UTF::DHD MCI4357DUT2 \
	-lan_ip 10.19.61.185 \
	-iperf iperf208 \
	-sta {4357N eth0} \
    -name "4357N-DUT" \
	-power "npc50 2" \
	-power_button "auto" \
    -hostconsole mcptst11:40052 \
    -nvram_add {macaddr=00:90:4C:12:D0:04} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
    -clm_blob 4357.clm_blob \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl band a;
	wl vht_features 7;
	wl country US/0;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
    -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" FAIL
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" WARN
            "MQ_ERROR" FAIL
            "segfault" FAIL
            "PSM microcode watchdog" FAIL
    }

4357N clone 4357N.2 -sta {_4357N eth0 4357N.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357N clone 4357Nx \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin
4357N.2 clone 4357Nx.2 \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin

4357N clone 4357Nt \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357N.2 clone 4357Nt.2 \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

4357N configure -ipaddr 192.168.1.235

######## P2P GC ########
4357Nt clone 4357Ni-PGC-WLAN \
    -clm_blob 4357.clm_blob \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357Ni-PGC" \
	-sta {4357Ni-PGC-WLAN eth0 4357Ni-PGC-P2P wl0.1}
4357Ni-PGC-WLAN configure -ipaddr 192.168.1.235    
4357Ni-PGC-P2P  configure -ipaddr 192.168.5.235 -attngrp G3
######################

######## AWDL Slave ########
4357N clone 4357N-ASlave-WLAN \
	-postinstall {dhd -i eth0 dma_ring_indices 3; dhd -i eth0 h2d_phase 1; dhd -i eth0 force_trap_bad_h2d_phase 1} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
	wl bus:enable_ds_hc 0;
    } \
	-name "4357N-ASlave" \
	-sta {4357N-ASlave-WLAN eth0 4357N-ASlave-AWDL wl0.2} \
	-dhd_tag DHD_REL_1_579_216 \
    -app_tag APPS_REL_1_46 \
    -app_brand linux-combined-apps \
    -clm_blob 4347a0.clm_blob \
    -type 4357b1-roml/config_pcie_release/rtecdc.bin
4357N-ASlave-WLAN configure -ipaddr 192.168.1.235    
4357N-ASlave-AWDL configure -ipaddr 192.168.5.235 -attngrp G3
######################
4357N-ASlave-WLAN clone 4357Ni-ASlave-WLAN \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
	-sta {4357Ni-ASlave-WLAN eth0 4357Ni-ASlave-AWDL wl0.2} \
    -clm_blob 4357.clm_blob \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
    -type 4357b1-roml/config_pcie_olympic/rtecdc.bin \
	-tag IGUANA_BRANCH_13_10
4357Ni-ASlave-WLAN configure -ipaddr 192.168.1.235    
4357Ni-ASlave-AWDL configure -ipaddr 192.168.5.235 -attngrp G3
######################

######## NAN Slave 2 ########
4357N clone 4357Ni-NanSlave \
	-postcopy {
		$self touch .iguana
		if {![catch {$self rm .trunk}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
	} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357Ni-NanS" \
	-sta {4357Ni-NanSlave eth0 4357Ni-NanSlave-Aux wl0.2} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357_apolloe.clm_blob \
    -type 4357b1-roml/config_pcie_olympic_min_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -app_brand linux-combined-apps \
    -nvram "src/shared/nvram/bcm94357fcpagbe_p404.txt" \
	-tag IGUANA_BRANCH_13_10
4357Ni-NanSlave configure -ipaddr 192.168.1.235    
4357Ni-NanSlave-Aux configure -ipaddr 192.168.5.235 -attngrp G3
######################
4357Ni-NanSlave clone 4357Nt-NanSlave \
	-postcopy {
		$self touch .trunk
		if {![catch {$self rm .iguana}]} {
			$self sync
			$self power cycle
			$self wait_for_boot
		}
    } \
	-name "4357Nt-NanS" \
	-sta {4357Nt-NanSlave eth0 4357Nt-NanSlave-Aux wl0.2} \
    -brand hndrte-dongle-wl \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357{a0,}.clm_blob \
    -type 4357b1-ram/config_pcie_nan/rtecdc.bin \
    -nvram "bcm94357fcpagbe_p402.txt" \
	-tag trunk
4357Nt-NanSlave configure -ipaddr 192.168.1.235    
4357Nt-NanSlave-Aux configure -ipaddr 192.168.5.235 -attngrp G3
######################

#-----------------------------------------------------------------------------------------
#						                   APs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#										SoftAP1 4366
##########################################################################################
UTF::Linux MCISoftAP1 \
	-lan_ip 10.19.61.184 \
	-iperf iperf208 \
    -sta {4366c0-SoftAP1 enp1s0} \
    -console "mcptst11:40026" \
    -power "npc70 2" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 -udp 1g \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 7; wl txchain 9; wl rxchain 9} \
    -brand "linux-internal-wl" \
    -tag EAGLE_REL_10_10_122 \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" FAIL
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" WARN
            "wlc_ampdu_dotxstatus_aqm_complete" WARN
            "MQ_ERROR" FAIL
            "segfault" FAIL
            "PSM microcode watchdog" FAIL
    }

4366c0-SoftAP1 configure -ipaddr 192.168.1.91 -attngrp G1 -hasdhcpd 1

##########################################################################################
#										SoftAP2 4366
##########################################################################################
UTF::Linux MCISoftAP2 \
	-lan_ip 10.19.61.183 \
	-iperf iperf208 \
    -sta {4366c0-SoftAP2 enp1s0} \
    -console "mcptst11:40046" \
    -power "npc80 2" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 -udp 1g \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 7; wl txchain 9; wl rxchain 9} \
    -brand "linux-internal-wl" \
    -tag EAGLE_REL_10_10_122 \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" FAIL
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" WARN
            "wlc_ampdu_dotxstatus_aqm_complete" WARN
            "MQ_ERROR" FAIL
            "segfault" FAIL
            "PSM microcode watchdog" FAIL
    }

4366c0-SoftAP2 configure -ipaddr 192.168.1.96 -attngrp G2 -hasdhcpd 1

##########################################################################################
#								Router (4709C0 board with 4366mc C0)
##########################################################################################
UTF::Router AP1-4709 \
    -lan_ip 192.168.1.10 \
    -sta {AP1-4709-4366 eth1 AP1-4709-4366.%15 wl0.%} \
    -relay lan \
    -lanpeer lan \
    -wanpeer wan \
    -console "mcptst11:40016" \
    -power "npc70 1" \
    -tag BISON04T_REL_7_14_131_44 \
    -brand linux-2.6.36-arm-internal-router-dhdap \
    -datarate {-i 0.5} -udp 1.8g \
    -noradio_pwrsave 1 \
    -perfchans {36/80} \
    -yart {-attn5g {15-90} -attn2g {45-90} -pad 29} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        watchdog=3000
        wl0_ssid=MCI5GAP1
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl1_ssid=MCI24GAP1
        wl1_chanspec=3
        wl1_radio=0
        wl1_atf=0
        wl1_vht_features=5
        wl1_mu_features="-1"
		#
		lan_ipaddr=192.168.1.10
		lan_gateway=192.168.1.10
		dhcp_start=192.168.1.110
		dhcp_end=192.168.1.119
		lan1_ipaddr=192.168.2.10
		lan1_gateway=192.168.2.10
		dhcp1_start=192.168.2.110
		dhcp1_end=192.168.2.119
    }

AP1-4709-4366 configure -attngrp G1

##########################################################################################
#								Router (4709C0 board with 4366mc C0)
##########################################################################################
UTF::Router AP3-4709 \
    -lan_ip 192.168.1.30 \
    -sta {AP3-4709-4366 eth1 AP3-4709-4366.%15 wl0.%} \
    -relay lan \
    -lanpeer lan \
    -wanpeer wan \
    -console "mcptst11:40036" \
    -power "npc80 1" \
    -tag BISON04T_REL_7_14_131_44 \
    -brand linux-2.6.36-arm-internal-router-dhdap \
    -datarate {-i 0.5} -udp 1.8g \
    -noradio_pwrsave 1 \
    -perfchans {36/80} \
    -yart {-attn5g {15-90} -attn2g {45-90} -pad 29} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        watchdog=3000
        wl0_ssid=MCI5GAP3
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
        wl1_ssid=MCI24GAP3
        wl1_chanspec=3
        wl1_radio=0
        wl1_atf=0
        wl1_vht_features=5
        wl1_mu_features="-1"
		#
		lan_ipaddr=192.168.1.30
		lan_gateway=192.168.1.30
		dhcp_start=192.168.1.130
		dhcp_end=192.168.1.139
		lan1_ipaddr=192.168.2.30
		lan1_gateway=192.168.2.30
		dhcp1_start=192.168.2.130
		dhcp1_end=192.168.2.139
    }

AP3-4709-4366 configure -attngrp G2

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab30hnd-mirabadi
# To check the params:
# 	ssh lab30hnd-mirabadi ps -wo pid,args -p 14666,8802,10227,11576,17628
##########################################################################################
UTF::Q MCI

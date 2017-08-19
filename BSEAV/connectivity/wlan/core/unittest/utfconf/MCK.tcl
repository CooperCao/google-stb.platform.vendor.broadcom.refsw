# -*-tcl-*-

#
# MCK CIT testbed configuration for Linux P2P
#
#
# Test/StaNightly.test -utfconf utfconf/MCK.tcl -title "MCK Linux 4331 StaNightly" -sta "4331-FC15-DUT" -ap "AP1-4331-4706 AP1-4360-4706" -rateonly -perfonly -nocache -branch TOT -statag RSDBDEV_REL_7_20_5
# Test/P2PQoSNightly.test -utfconf utfconf/MCK.tcl -title "Linux Same Channel 4360 P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "43602-GO" -sta_gc "43602-DUT" -nom -nod -perftime 25 -statag BISON_BRANCH_7_10 -wlan_security aespsk2 -p2p_security
# Private build
# Test/P2PQoSNightly.test -utfconf utfconf/MCK.tcl -title "MCK FC15 Same Band 4360 11ac P2P: APCh=36/80 P2PCh=36l" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "43602-GO" -sta_gc "43602-DUT" -nos -nod -perftime 25 -p2p_chan "36l" -ap_chan "36/80" -stabin /projects/hnd_sw_ndis/work/walker/a2/BISON_BRANCH_7_10/src/wl/linux/obj-debug-p2p-mchan-2.6.38.6-26.rc1.fc15.i686.PAE/wl.ko
#
# WLAN TCP test:
# Test/P2PQoSNightly.test -utfconf utfconf/MCK.tcl -title "MCK 4360 FC19 TCP WLAN Only" -ap "AP1-4331-4706 AP1-4360-4706" -sta 43602-DUT -perftime 25 -ap_chan 36/80 -fb1 -stadate 2013.12.1.0
# WLAN UDP test:
# Test/P2PQoSNightly.test -utfconf utfconf/MCK.tcl -title "MCK 4360 FC19 UDP WLAN Only" -ap "AP1-4331-4706 AP1-4360-4706" -sta 43602-DUT -perftime 25 -ap_chan 36/80 -fb1 -wlan_bandwidth_BE 1.2g -tests_tos BE
# or
# Test/P2PQoSNightly.test -utfconf utfconf/MCK.tcl -title "MCK 4360 FC19 UDP WLAN Only" -ap "AP1-4331-4706 AP1-4360-4706" -sta 43602-DUT -perftime 25 -ap_chan 36/80 -fb1 -wlan_bandwidth_BE 1.2g -run_qos -qos_tests "[WLAN:BE:RX:0:25]|[WLAN:BE:TX:0:25]|[WLAN:BE:BI:0:25]"
#
# Test/StaNightly.test -utfconf utfconf/MCK.tcl -title "MCK 4360 FC19" -ap "AP1-4331-4706 AP1-4360-4706" -sta 43602-DUT -perfonly
#
# RvR:
# Test/RvRNightly1.test -utfconf utfconf/MCK.tcl -title "MCK 4360 RvR" -ap "AP1-4331-4706 AP1-4360-4706" -apdate "2015.2.12.1" -sta "43602-DUT" -va af -cycle5G40AttnRange "10-70 70-10" -cycle5G20AttnRange "10-70 70-10" -cycle2G40AttnRange "10-70 70-10" -cycle2G20AttnRange "10-70 70-10" -nocontrvr -no5G20 -no2G40 -no2G20 -title "RvR Test" -perftime 5
# P2P RvR
# Test/P2PQoSNightlyRvR.test -utfconf MCK -title 'MCK 4360 RvR Direct P2P' -sta_gc '43602-DUT' -sta_go '43602-GO' -nos -nom -wlan_bandwidth_VI 30M -p2p_bandwidth_VI 30M -run_qos -qos_tests '[P2P:TCP:RX:0:5]' -ap_chan 36/80 -p2p_chan 36l -fb1 -attn_type 2 -ap_connect GC
# Test/P2PQoSNightlyRvR.test -utfconf MCK -title 'MCK 4360 RvR Same Channel P2P' -ap "AP1-4331-4706 AP1-4360-4706" -apdate "2015.2.12.1" -sta_gc '43602-DUT' -sta_go '43602-GO' -nom -nod -wlan_bandwidth_VI 30M -p2p_bandwidth_VI 30M -run_qos -qos_tests '[P2P:TCP:RX:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GC
# Test/P2PQoSNightlyRvR.test -utfconf MCK -title 'MCK 4360 RvR Same Channel Video P2P' -ap "AP1-4331-4706 AP1-4360-4706" -apdate "2015.2.12.1" -sta_gc '43602-DUT' -sta_go '43602-GO' -nom -nod -wlan_bandwidth_VI 30M -p2p_bandwidth_VI 30M -run_qos -qos_tests '[P2P:VI:RX:0:15]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GC
# Test/P2PQoSNightlyRvR.test -utfconf MCK -title 'MCK 4360 RvR Same Channel Video P2P' -ap "AP1-4331-4706 AP1-4360-4706" -apdate "2015.2.12.1" -sta_gc '43602-DUT' -sta_go '43602-GO' -nom -nod -wlan_bandwidth_VI 30M -p2p_bandwidth_VI 30M -run_qos -qos_tests '[WLAN:VI:RX:0:15][P2P:VI:RX:0:15]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GC
# Sniffer validation with StaNightly:
# Test/StaNightly.test -utfconf MCK -title 'MCK 4360 7_10_TOB FC15 snif StaNightly' -sta 'snif' -ap 'AP1-4331-4706 AP1-4360-4706' -apdate '2015.2.12.1' -rateonly -perfonly -nostaload
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
set ::UTF::SummaryDir "/projects/hnd_sig_ext19/$::env(LOGNAME)/2017/MCK"

# Define power controllers on cart.
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCKUTF -rev 1
UTF::Power::Synaccess npc20 -lan_ip 172.16.1.20 -relay MCKUTF
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCKUTF -rev 1
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCKUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCKUTF -rev 1
UTF::Power::Synaccess npc70 -lan_ip 172.16.1.70 -relay MCKUTF -rev 1
UTF::Power::Synaccess npc80 -lan_ip 172.16.1.80 -relay MCKUTF -rev 1
UTF::Power::WebRelay WebSwitch52 -lan_ip 172.16.1.52 -relay lan

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

	set ::UTF::APList "AP1-4360-4706 AP3-4360-4706"
	#set ::UTF::STAList "snif 4357 4357G 4357N 4359-DUT 4359-GO"
	set ::UTF::STAList "4357 4357G 4357N"
	set ::UTF::RebootList "4360-SoftAP $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP $::UTF::APList $::UTF::STAList"

	UTF::Try "Reboot Testbed" {
		eval $::UTF::SetupTestBedReboot
	}
	#UTF::Try "Sniffer Reload" {
	#	snif tcptune 2M
	#	snif reload
		#UTF::Test::rssinoise 4360-SoftAP snif
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

UTF::Linux MCKUTF \
	-lan_ip 10.19.13.160 \
	-iperf iperf208 \
    -sta "lan p4p1" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

##########################################################################################
# Sniffer
##########################################################################################
UTF::Sniffer MCKSNIF \
	-lan_ip 10.19.13.207 \
	-sta {snif enp1s0} \
    -power "WebSwitch52 1"\
    -power_button "auto"\
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10 \
    -date 2015.1.20.0

#-----------------------------------------------------------------------------------------
#						                   STAs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#						STA 4357 PCIe Chip - Frank's Station
##########################################################################################

UTF::DHD MCK4357GO \
	-lan_ip 10.19.13.226 \
	-iperf iperf208 \
	-sta {4357G eth0} \
    -name "4357-GO" \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole lab17hnd-mirabadi:40061 \
    -nvram_add {macaddr=00:90:4C:12:D0:03} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	#wl band a;
	wl vht_features 7;
	wl country US/0;
	#wl -w 1 interface_create sta;
	#sleep 1;
	#wl -i wl1.2 vht_features 3;
	#wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 0-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
    -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" WARN
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" FAIL
            "MQ_ERROR" FAIL
    }

4357G clone 4357G.2 -sta {_4357G eth0 4357G.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357G clone 4357Gx \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin
4357G.2 clone 4357Gx.2 \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin

4357G clone 4357Gt \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357G.2 clone 4357Gt.2 \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

#4357G configure -dualband {4360-SoftAP2 4357G.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Gx configure -dualband {4360-SoftAP2 4357Gx.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Gt configure -dualband {4360-SoftAP2 4357Gt.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}

4357G configure -ipaddr 192.168.1.234


# IGUANA

4357G clone 4357Gi \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357G.2 clone 4357Gi.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

4357Gx clone 4357Gix \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357Gx.2 clone 4357Gix.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

4357Gt clone 4357Git \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357Gt.2 clone 4357Git.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

#4357Gi configure -dualband {4360-SoftAP2 4357Gi.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Gix configure -dualband {4360-SoftAP2 4357Gix.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Git configure -dualband {4360-SoftAP2 4357Git.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}


4357Gi configure -ipaddr 192.168.1.234    

######## P2P GO ########
4357Git clone 4357i-PGO-WLAN \
    -clm_blob 4347a0.clm_blob \
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
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357-AMaster" \
	-sta {4357-AMaster-WLAN eth0 4357-AMaster-AWDL wl0.2} \
	-dhd_tag DHD_REL_1_579_53 \
    -app_tag DHD_REL_1_359_163 \
    -type 4357a0-ram/config_pcie_awdl/rtecdc.bin	
4357-AMaster-WLAN configure -ipaddr 192.168.1.234    
4357-AMaster-AWDL configure -ipaddr 192.168.5.234
######################
4357-AMaster-WLAN clone 4357i-AMaster-WLAN \
	-sta {4357i-AMaster-WLAN eth0 4357i-AMaster-AWDL wl0.2} \
	-type 4357a1-ram/config_pcie_awdl_rsdb/rtecdc.bin \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357i-AMaster-WLAN configure -ipaddr 192.168.1.234    
4357i-AMaster-AWDL configure -ipaddr 192.168.5.234
######################

######## NAN Master ########
4357-AMaster-WLAN clone 4357i-NanMaster \
	-name "4357i-NanM" \
	-sta {4357i-NanMaster eth0 4357i-NanMaster-Aux wl0.1} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -type 4357a0-ram/config_pcie_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357i-NanMaster configure -ipaddr 192.168.1.234    
4357i-NanMaster-Aux configure -ipaddr 192.168.5.234
######################
4357i-NanMaster clone 4357-NanMaster \
	-name "4357-NanM" \
	-sta {4357-NanMaster eth0 4357-NanMaster-Aux wl0.1} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -type 4357a0-ram/config_pcie_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
	-tag trunk
4357-NanMaster configure -ipaddr 192.168.1.234    
4357-NanMaster-Aux configure -ipaddr 192.168.5.234
######################

######## 4361 P2P GO ########
4357Gi clone 4361i-PGO-WLAN \
    -app_tag DHD_BRANCH_1_579 \
    -clm_blob ss_mimo.clm_blob \
    -type 4361a0-roml/config_pcie_utf/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features;
	wl vht_features 7;
	wl vht_features;
	wl country US/0;
    } \
	-name "4361i-PGO" \
	-sta {4361i-PGO-WLAN eth0 4361i-PGO-P2P wl0.2}
4361i-PGO-WLAN configure -ipaddr 192.168.1.234    
4361i-PGO-P2P  configure -ipaddr 192.168.5.234
######################

#####################################

UTF::DHD MCK4357DUT \
	-lan_ip 10.19.13.225 \
	-iperf iperf208 \
	-sta {4357 eth0} \
    -name "4357-DUT" \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole lab17hnd-mirabadi:40051 \
    -nvram_add {macaddr=00:90:4C:12:D0:02} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	#wl band a;
	wl vht_features 7;
	wl country US/0;
	#wl -w 1 interface_create sta;
	#sleep 1;
	#wl -i wl1.2 vht_features 3;
	#wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
   -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" WARN
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" FAIL
            "MQ_ERROR" FAIL
    }

4357 clone 4357.2 -sta {_4357 eth0 4357.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357 clone 4357x \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin
4357.2 clone 4357x.2 \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin

4357 clone 4357t \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357.2 clone 4357t.2 \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

#4357 configure -dualband {4360-SoftAP2 4357.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357x configure -dualband {4360-SoftAP2 4357x.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357t configure -dualband {4360-SoftAP2 4357t.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}

4357 configure -ipaddr 192.168.1.236


# IGUANA

4357 clone 4357i \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357.2 clone 4357i.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

4357x clone 4357ix \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357x.2 clone 4357ix.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

4357t clone 4357it \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357t.2 clone 4357it.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

#4357i configure -dualband {4360-SoftAP2 4357i.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357ix configure -dualband {4360-SoftAP2 4357ix.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357it configure -dualband {4360-SoftAP2 4357it.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}


4357i configure -ipaddr 192.168.1.236    

######## P2P GC ########
4357it clone 4357i-PGC-WLAN \
    -clm_blob 4347a0.clm_blob \
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
    } \
	-name "4357-ASlave" \
	-sta {4357-ASlave-WLAN eth0 4357-ASlave-AWDL wl0.2} \
	-dhd_tag DHD_REL_1_579_53 \
    -app_tag DHD_REL_1_359_163 \
    -type 4357a0-ram/config_pcie_awdl/rtecdc.bin	
4357-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357-ASlave-WLAN clone 4357i-ASlave-WLAN \
	-sta {4357i-ASlave-WLAN eth0 4357i-ASlave-AWDL wl0.2} \
	-type 4357a1-ram/config_pcie_awdl_rsdb/rtecdc.bin \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357i-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357i-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## NAN Slave ########
4357-ASlave-WLAN clone 4357i-NanSlave \
	-name "4357i-NanS" \
	-sta {4357i-NanSlave eth0 4357i-NanSlave-Aux wl0.1} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -type 4357a0-ram/config_pcie_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357i-NanSlave configure -ipaddr 192.168.1.236    
4357i-NanSlave-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357i-NanSlave clone 4357-NanSlave \
	-name "4357-NanS" \
	-sta {4357-NanSlave eth0 4357-NanSlave-Aux wl0.1} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -type 4357a0-ram/config_pcie_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
	-tag trunk
4357-NanSlave configure -ipaddr 192.168.1.236    
4357-NanSlave-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## 4361 P2P GC ########
4357i clone 4361i-PGC-WLAN \
    -app_tag DHD_BRANCH_1_579 \
    -clm_blob ss_mimo.clm_blob \
    -type 4361a0-roml/config_pcie_utf/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features;
	wl vht_features 7;
	wl vht_features;
	wl country US/0;
    } \
	-name "4361i-PGC" \
	-sta {4361i-PGC-WLAN eth0 4361i-PGC-P2P wl0.2}
4361i-PGC-WLAN configure -ipaddr 192.168.1.236    
4361i-PGC-P2P  configure -ipaddr 192.168.5.236 -attngrp G3
######################

#####################################
    
UTF::DHD MCK4357DUT2 \
	-lan_ip 10.19.13.249 \
	-iperf iperf208 \
	-sta {4357N eth0} \
    -name "4357-DUT2" \
	-power "npc50 2" \
	-power_button "auto" \
    -hostconsole lab17hnd-mirabadi:40052 \
    -nvram_add {macaddr=00:90:4C:12:D0:04} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	#wl band a;
	wl vht_features 7;
	wl country US/0;
	#wl -w 1 interface_create sta;
	#sleep 1;
	#wl -i wl1.2 vht_features 3;
	#wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}} \
    -perfchans {36/80} -channelsweep {-band a} \
	-msgactions {
            "SDIODEV: dma_rx: bad frame length" WARN
            "wlc_ampdu_watchdog: cleaning up ini tid 0 due to no progress" FAIL
            "wl0: _wlc_bss_update_beacon: out of mem" FAIL
            "wl0: wlc_bss_update_probe_resp: out of mem" FAIL
            "PHYTX error" FAIL
            "MQ_ERROR" FAIL
    }

4357N clone 4357N.2 -sta {_4357N eth0 4357N.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357N clone 4357Nx \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin
4357N.2 clone 4357Nx.2 \
    -type 4357a0-ram/config_pcie_release/rtecdc.bin

4357N clone 4357Nt \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357N.2 clone 4357Nt.2 \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

#4357N configure -dualband {4360-SoftAP2 4357N.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Nx configure -dualband {4360-SoftAP2 4357Nx.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Nt configure -dualband {4360-SoftAP2 4357Nt.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}


# IGUANA

4357N clone 4357Ni \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357N.2 clone 4357Ni.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

4357Nx clone 4357Nix \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357Nx.2 clone 4357Nix.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

4357Nt clone 4357Nit \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10
4357Nt.2 clone 4357Nit.2 \
    -nvram "src/shared/nvram/bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10

#4357Ni configure -dualband {4360-SoftAP2 4357Ni.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Nix configure -dualband {4360-SoftAP2 4357Nix.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
#4357Nit configure -dualband {4360-SoftAP2 4357Nit.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}

4357Ni configure -ipaddr 192.168.1.236

######## P2P GC ########
4357Nit clone 4357Ni-PGC-WLAN \
    -clm_blob 4347b0.clm_blob \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357Ni-PGC" \
	-sta {4357Ni-PGC-WLAN eth0 4357Ni-PGC-P2P wl0.1}
4357Ni-PGC-WLAN configure -ipaddr 192.168.1.236    
4357Ni-PGC-P2P  configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## AWDL Slave ########
4357N clone 4357N-ASlave-WLAN \
	-postinstall {dhd -i eth0 dma_ring_indices 3; dhd -i eth0 h2d_phase 1; dhd -i eth0 force_trap_bad_h2d_phase 1} \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl country US/0;
    } \
	-name "4357N-ASlave" \
	-sta {4357N-ASlave-WLAN eth0 4357N-ASlave-AWDL wl0.2} \
	-dhd_tag DHD_REL_1_579_142 \
    -app_tag APPS_REL_1_18 \
    -app_brand linux-combined-apps \
    -clm_blob 4347a0.clm_blob \
    -type 4357b0-roml/config_pcie_release/rtecdc.bin
4357N-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357N-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357N-ASlave-WLAN clone 4357Ni-ASlave-WLAN \
	-sta {4357Ni-ASlave-WLAN eth0 4357Ni-ASlave-AWDL wl0.2} \
    -clm_blob 4347b0.clm_blob \
    -nvram "src/shared/nvram/bcm94357GuinnessMurataMM.txt" \
    -type 4357b0-roml/config_pcie_olympic/rtecdc.bin \
	-tag IGUANA_BRANCH_13_10
4357Ni-ASlave-WLAN configure -ipaddr 192.168.1.236    
4357Ni-ASlave-AWDL configure -ipaddr 192.168.5.236 -attngrp G3
######################

######## NAN Slave ########
4357N-ASlave-WLAN clone 4357Ni-NanSlave \
	-name "4357Ni-NanS" \
	-sta {4357Ni-NanSlave eth0 4357Ni-NanSlave-Aux wl0.2} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357_apolloe.clm_blob \
    -type 4357b0-roml/config_pcie_olympic_min_nan/rtecdc.bin \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -app_brand linux-combined-apps \
    -nvram "src/shared/nvram/bcm94357GuinnessMurataMM.txt" \
	-tag IGUANA_BRANCH_13_10
4357Ni-NanSlave configure -ipaddr 192.168.1.236    
4357Ni-NanSlave-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################
4357Ni-NanSlave clone 4357N-NanSlave \
	-name "4357N-NanS" \
	-sta {4357N-NanSlave eth0 4357N-NanSlave-Aux wl0.2} \
    -brand linux-external-dongle-pcie \
	-dhd_brand linux-external-dongle-pcie \
    -clm_blob 4357_apolloe.clm_blob \
    -type 4357b0-roml/config_pcie_olympic_min_nan/rtecdc.bin \
	-tag trunk
4357N-NanSlave configure -ipaddr 192.168.1.236    
4357N-NanSlave-Aux configure -ipaddr 192.168.5.236 -attngrp G3
######################

##########################################################################################
#						STA 4359 PCIe Chip 
##########################################################################################
# 	-name GO \

UTF::DHD MCK4359GO \
	-lan_ip 10.19.13.251 \
	-iperf iperf208 \
	-sta {4359-GO eth0 4359-PGO wl0.2} \
	-power "npc60 2" \
	-power_button "auto" \
    -hostconsole lab17hnd-mirabadi:40062 \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag DHD_BRANCH_1_363 \
	-app_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DINGO_BRANCH_9_10 \
    -nocal 1 -slowassoc 5 \
    -nvram "bcm943593fcpagbss.txt" \
    -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-{,txbf-}die3-slna-pktctx-sstput/rtecdc.bin \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 3m \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl rsdb_mode; wl mpc; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -yart {-attn5g 16-95 -attn2g 48-95 -pad 26}

4359-GO configure -ipaddr 192.168.1.234
4359-PGO configure -ipaddr 192.168.5.234

4359-GO clone 4359-GOx \
    -type 4359b1-roml/pcie-wl11u-ccx-ve-mfp-okc-idsup-sr-ltecx-die3-tdls-pwrofs-gscan-wepso-sarctrl-linkstat-wnm-pfn-hs20sta-ulb-pwrstats-lpc-olpc-mobfd-txbf-mimopscan-rcc-noccxaka-btcdyn-sstput/rtecdc.bin \
    -perfonly 1 -perfchans {36/80}

4359-GOx clone 4359-GOd35x \
	-tag DINGO07T_BRANCH_9_35 \
	-dhd_tag DHD_BRANCH_1_363 \
    -type 4359b1-roml/pcie-wl11u-ve-mfp-okc-idsup-sr-ltecx-die3-tdls-pwrofs-gscan-wepso-sarctrl-linkstat-wnm-pfn-hs20sta-ulb-pwrstats-lpc-olpc-mobfd-txbf-mimopscan-apcs-pspretend-rcc-btcdyn/rtecdc.bin

4359-GO clone 4359-GOd35 \
	-tag DINGO07T_BRANCH_9_35 \
	-dhd_tag DHD_BRANCH_1_363 \
    -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin

#####################################
#	-dhd_image "/projects/hnd_sig_ext11/tmp/Anirban/dhdr549864.ko" \

# 	-name GC \

UTF::DHD MCK4359DUT \
	-lan_ip 10.19.13.250 \
	-iperf iperf208 \
	-sta {4359-DUT eth0 4359-PDUT wl0.2} \
	-power "npc80 1" \
	-power_button "auto" \
    -hostconsole lab17hnd-mirabadi:40081 \
	-perfchans {36/80 36l 36 11u 3} \
	-dhd_tag DHD_BRANCH_1_363 \
	-app_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DINGO_BRANCH_9_10 \
    -nocal 1 -slowassoc 5 \
    -nvram "bcm943593fcpagbss.txt" \
    -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-{,txbf-}die3-slna-pktctx-sstput/rtecdc.bin \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 3m \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl rsdb_mode; wl mpc; wl channel; wl ampdu_mpdu; wl pool; wl rsdb_mode; wl mpc} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -yart {-attn5g 16-95 -attn2g 48-95 -pad 26}
	
4359-DUT configure -ipaddr 192.168.1.236
4359-PDUT configure -ipaddr 192.168.5.236 -attngrp G3

4359-DUT clone 4359-DUTx \
    -type 4359b1-roml/pcie-wl11u-ccx-ve-mfp-okc-idsup-sr-ltecx-die3-tdls-pwrofs-gscan-wepso-sarctrl-linkstat-wnm-pfn-hs20sta-ulb-pwrstats-lpc-olpc-mobfd-txbf-mimopscan-rcc-noccxaka-btcdyn-sstput/rtecdc.bin \
    -perfonly 1 -perfchans {36/80}

4359-DUTx clone 4359-DUTd35x \
	-tag DINGO07T_BRANCH_9_35 \
	-dhd_tag DHD_BRANCH_1_363 \
    -type 4359b1-roml/pcie-wl11u-ve-mfp-okc-idsup-sr-ltecx-die3-tdls-pwrofs-gscan-wepso-sarctrl-linkstat-wnm-pfn-hs20sta-ulb-pwrstats-lpc-olpc-mobfd-txbf-mimopscan-apcs-pspretend-rcc-btcdyn/rtecdc.bin

4359-DUT clone 4359-DUTd35 \
	-tag DINGO07T_BRANCH_9_35 \
	-dhd_tag DHD_BRANCH_1_363 \
    -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin

#-----------------------------------------------------------------------------------------
#						                   APs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#										SoftAP1 4360
##########################################################################################
UTF::Linux MCKSoftAP \
 	-name SoftAP \
	-lan_ip 10.19.13.193 \
	-iperf iperf208 \
    -sta {4360-SoftAP enp1s0} \
    -console "lab17hnd-mirabadi:40071" \
    -power "npc70 1" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl down; wl country '#a/0'; wl dtim 3; wl vht_features 3} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

4360-SoftAP configure -ipaddr 192.168.1.91 -attngrp G2 -hasdhcpd 1

##########################################################################################
#								AP 4331/4360 Netgrear N6300
##########################################################################################
UTF::Router AP1-4706 \
    -lan_ip 192.168.1.10 \
    -sta {AP1-4360-4706 eth1 AP1-4331-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab17hnd-mirabadi:40010" \
    -tag AARDVARK01T_TWIG_6_37_14 \
    -brand linux26-internal-router \
    -power "npc10 1" \
    -txt_override {
        watchdog=6000
    } \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        watchdog=6000; # PR#90439
		boardtype=0x05b2; # 4706nr
        fw_disable=1
        wl_msglevel=0x101
		macaddr=00:90:4C:0D:B0:D3
        wl_stbc_rx=1
        wl_stbc_tx=1
		wl0_ssid=MCK5GAP1
		wl0_chanspec=36
		wl0_radio=0
		wl1_ssid=MCK2GAP1
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
    }

AP1-4360-4706 configure -attngrp G1
AP1-4331-4706 configure -attngrp G1

# did not work
#    -tag AARDVARK01T_TWIG_6_37_14 \
#    -tag BISON04T_REL_7_14_89_33 \
#    -date 2015.2.3.0 \

##########################################################################################
#								AP 4331/4360 Netgrear N6300
##########################################################################################
UTF::Router AP3-4706 \
    -lan_ip 192.168.1.30 \
    -sta {AP3-4331-4706 eth1 AP3-4360-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab17hnd-mirabadi:40030" \
    -tag AARDVARK01T_TWIG_6_37_14 \
    -brand linux26-internal-router \
    -power "npc30 1" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-noradio_pwrsave 1 \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
		macaddr=00:90:4C:0D:BF:72
		wl0_ssid=MCK2GAP3
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCK5GAP3
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

AP3-4331-4706 clone APx \
	-tag BISON04T_REL_7_14_89_33 

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab17hnd-mirabadi
##########################################################################################
UTF::Q MCK

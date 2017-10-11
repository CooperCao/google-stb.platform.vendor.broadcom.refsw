# -*-tcl-*-

#
# MCP CIT testbed configuration for P2P
#
# VNC 10.19.61.168:5901
# Test/StaNightly.test -utfconf MCP -title 'MCP 43569a2 USB 7_10_TOB StaNightly' -sta '43569-DUTx' -ap '4360-SoftAP1' -apdate '2014.3.27.0' -rateonly -perfonly
# 		or -nochannels -nobighammer -nofrag -nowep -noshared -notkip -nocal -norate -noaes -noscan -nojoin -nopm1


package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/$::env(LOGNAME)/2017/MCP"

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCPUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCPUTF -rev 1
UTF::Power::Synaccess npc70 -lan_ip 172.16.1.70 -relay MCPUTF -rev 1
UTF::Power::Synaccess npc80 -lan_ip 172.16.1.80 -relay MCPUTF -rev 1

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

	set ::UTF::APList ""
	set ::UTF::STAList "snif 43569-DUT2 43569-GO2 43569-DUT 43569-GO"
	set ::UTF::RebootList "4360-SoftAP1 4360-SoftAP2 $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP1 4360-SoftAP2 $::UTF::APList $::UTF::STAList"

	UTF::Try "Reboot Testbed" {
		eval $::UTF::SetupTestBedReboot
	}
	UTF::Try "Sniffer Reload" {
		snif tcptune 2M
		snif reload
		#UTF::Test::rssinoise 4360-SoftAP1 snif
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
# mcptst1
UTF::Linux MCPUTF \
	-lan_ip 10.19.61.167 \
	-iperf iperf208 \
    -sta "lan p4p1" \
    -tcpwindow 640k

lan configure -ipaddr 192.168.1.220

##########################################################################################
# Sniffer
##########################################################################################
UTF::Sniffer MCPSNIF \
	-lan_ip 10.19.61.175 \
	-sta {snif eth0} \
	-power "npc80 1" \
	-power_button "auto" \
    -tag AARDVARK_BRANCH_6_30 \
	-date 2013.11.11.0

##########################################################################################
#						STA 2 43569 USB Chip 
##########################################################################################

UTF::DHD MCP43569GO2 \
	-lan_ip 10.19.61.172 \
	-iperf iperf208 \
	-sta {43569-GO2 eth0 43569-PGO2 wl0.1} \
	-power "npc60 2" \
	-power_button "auto" \
    -hostconsole mcptst1:40062 \
    -console mcptst1:40067 \
    -nvram_add {macaddr=00:90:4c:12:d0:04} \
	-perfchans {36/80 36l 36 11u 3} \
    -dhd_brand linux-internal-media \
    -brand linux-internal-media \
    -tag BISON05T_TWIG_7_35_143 \
    -dhd_tag DHD_BRANCH_1_363 \
    -slowassoc 5 \
    -driver "dhd-cdc-usb-comp-gpl-nofb-debug" \
    -nvram "bcm943569usbir.txt" \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx" \
    -wlinitcmds {wl down; wl vht_features 3} \
    -tcpwindow 640k -udp 800m -nocal 1 -docpu 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 0-63 -attn2g 0-63 -pad 36 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl vht_features; wl PM ; wl scansuppress} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl scansuppress}} \
	-msgactions {
    	"BABBLE error" FAIL
    }

43569-GO2 configure -ipaddr 192.168.1.235
43569-PGO2 configure -ipaddr 192.168.5.235

43569-GO2 clone 43569-GO2x \
    -brand linux-external-media \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \
    -perfonly 1 -perfchans {36/80 3}

UTF::DHD MCP43569DUT2 \
	-lan_ip 10.19.61.171 \
	-iperf iperf208 \
	-sta {43569-DUT2 eth0 43569-PDUT2 wl0.1} \
	-power "npc50 2" \
	-power_button "auto" \
    -hostconsole mcptst1:40052 \
    -console mcptst1:40057 \
    -nvram_add {macaddr=00:90:4c:12:d0:03} \
	-perfchans {36/80 36l 36 11u 3} \
    -dhd_brand linux-internal-media \
    -brand linux-internal-media \
    -tag BISON05T_TWIG_7_35_143 \
    -dhd_tag DHD_BRANCH_1_363 \
    -slowassoc 5 \
    -driver "dhd-cdc-usb-comp-gpl-nofb-debug" \
    -nvram "bcm943569usbir.txt" \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx" \
    -wlinitcmds {wl down; wl vht_features 3} \
    -tcpwindow 640k -udp 800m -nocal 1 -docpu 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 0-63 -attn2g 0-63 -pad 36 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl vht_features; wl PM ; wl scansuppress} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl scansuppress}} \
	-msgactions {
    	"BABBLE error" FAIL
    }

43569-DUT2 configure -ipaddr 192.168.1.237
43569-PDUT2 configure -ipaddr 192.168.5.237 -attngrp G3

43569-DUT2 clone 43569-DUT2x \
    -brand linux-external-media \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \
    -perfonly 1 -perfchans {36/80 3}

##########################################################################################
#						STA 1 43569 USB Chip 
##########################################################################################

UTF::DHD MCP43569GO \
	-lan_ip 10.19.61.170 \
	-iperf iperf208 \
	-sta {43569-GO eth0 43569-PGO wl0.1} \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole mcptst1:40061 \
    -console mcptst1:40066 \
    -nvram_add {macaddr=00:90:4c:12:d0:02} \
	-perfchans {36/80 36l 36 11u 3} \
    -dhd_brand linux-internal-media \
    -brand linux-internal-media \
    -tag BISON05T_TWIG_7_35_143 \
    -dhd_tag DHD_BRANCH_1_363 \
    -slowassoc 5 \
    -driver "dhd-cdc-usb-comp-gpl-media-debug" \
    -nvram "bcm943569usbir.txt" \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx" \
    -wlinitcmds {wl down; wl vht_features 3} \
    -tcpwindow 640k -udp 800m -nocal 1 -docpu 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 0-63 -attn2g 0-63 -pad 36 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl vht_features; wl PM ; wl scansuppress} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl scansuppress}} \
	-msgactions {
    	"BABBLE error" FAIL
    }

43569-GO configure -ipaddr 192.168.1.234
43569-PGO configure -ipaddr 192.168.5.234

43569-GO clone 43569-GOx \
    -brand linux-external-media \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \
    -perfonly 1 -perfchans {36/80 3}

#    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \

UTF::DHD MCP43569DUT \
	-lan_ip 10.19.61.169 \
	-iperf iperf208 \
	-sta {43569-DUT eth0 43569-PDUT wl0.1} \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole mcptst1:40051 \
    -console mcptst1:40056 \
    -nvram_add {macaddr=00:90:4c:12:d0:01} \
	-perfchans {36/80 36l 36 11u 3} \
    -dhd_brand linux-internal-media \
    -brand linux-internal-media \
    -tag BISON05T_TWIG_7_35_143 \
    -dhd_tag DHD_BRANCH_1_363 \
    -slowassoc 5 \
    -driver "dhd-cdc-usb-comp-gpl-media-debug" \
    -nvram "bcm943569usbir.txt" \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-sr-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-wfds-vusb-err-assert.bin.trx" \
    -wlinitcmds {wl down; wl vht_features 3} \
    -tcpwindow 640k -udp 800m -nocal 1 -docpu 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 0-63 -attn2g 0-63 -pad 36 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3; wl vht_features; wl PM ; wl scansuppress} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl scansuppress}} \
	-msgactions {
    	"BABBLE error" FAIL
    }

43569-DUT configure -ipaddr 192.168.1.236
43569-PDUT configure -ipaddr 192.168.5.236 -attngrp G3

43569-DUT clone 43569-DUTx \
    -brand linux-external-media \
    -type "43569a2-roml/usb-ag-pool-pktctx-dmatxrc-idsup-idauth-keepalive-txbf-p2p-mchan-mfp-pktfilter-wowlpf-tdls-proptxstatus-vusb-wfds-sr.bin.trx" \
    -perfonly 1 -perfchans {36/80 3}

##########################################################################################
#										SoftAP1 4360
##########################################################################################
UTF::Linux MCPSoftAP1 \
	-lan_ip 10.19.61.173 \
	-iperf iperf208 \
    -sta {4360-SoftAP1 enp1s0} \
    -console "mcptst1:40071" \
    -power "npc70 1" \
    -tcpwindow 640k \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl msglevel; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3; wl country US} \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10

4360-SoftAP1 configure -ipaddr 192.168.1.91 -attngrp G1 -hasdhcpd 1 -ap 1

##########################################################################################
#										SoftAP2 4360
##########################################################################################
UTF::Linux MCPSoftAP2 \
	-lan_ip 10.19.61.174 \
	-iperf iperf208 \
    -sta {4360-SoftAP2 enp1s0} \
    -console "mcptst1:40072" \
    -power "npc70 2" \
    -tcpwindow 640k \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl msglevel; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3; wl country US} \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10

4360-SoftAP2 configure -ipaddr 192.168.1.97 -attngrp G2 -hasdhcpd 1 -ap 1

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat mcptst1
##########################################################################################
UTF::Q MCP

# -*-tcl-*-

#
# MCC CIT testbed configuration for P2P
#
# to run private bin and dhd with StaNightly:
# 	1) in STA object define the "dhd_image" paramter:
#	   '-dhd_image "/projects/hnd/swbuild/build_linux/DHD_REL_1_149/linux-external-dongle-pcie/2014.2.21.0/release/bcm/host/dhd-msgbuf-pciefd-debug-3.11.1-200.fc19.x86_64/dhd.ko"'
#	2) in StaNightly commandline, pass the "bin" paramter:
# 	   'Test/StaNightly.test -utfconf utfconf/MCC.tcl -title "MCC 4354e 7_10_TOB FC15 StaNightly" -sta "4354e-FC19-DUT" -ap "AP1-4331-4706 AP1-4360-4706" -apdate "2015.2.12.1" -rateonly -perfonly -bin "/projects/hnd_sig_ext11/tmp/4354-pcie-2/rtecdc.bin" 
#
# Test/StaNightly.test -utfconf utfconf/MCC.tcl -title "MCC 4354 StaNightly" -sta "4354-FC15-DUT" -ap "AP1-4331-4706 AP1-4360-4706"
# Test/ChannelSweep.test -web AP STA
# Test/P2PQoSNightly.test -utfconf utfconf/MCC.tcl -title "MCC BISON FC15 4354 Same Channel 36l P2P" -ap "AP1-4331-4706 AP1-4360-4706" -sta_go "BISON-4354-GO" -sta_gc "BISON-4354-DUT" -nom -nod -perftime 25 -ap_chan "36l" -p2p_chan "36l" -wlan_security aespsk2 -p2p_security -tests "[WLAN:RX][P2P:BI]" -ap_connect "GO" -noapload -norestore
#
#####################################
# -dhd_image "/projects/hnd/swbuild/build_linux/DHD_REL_1_149/linux-external-dongle-pcie/2014.2.21.0/release/bcm/host/dhd-msgbuf-pciefd-debug-3.11.1-200.fc19.x86_64/dhd.ko" \
# -dhd_image "/projects/hnd_sig_ext11/tmp/4354-pcie-2/dhd.ko" \
# -dhd_tag trunk \
# -type 4354a1-roml/pcie-ag-msgbuf-pktctx-splitrx-sr-txbf/rtecdc.bin \
# -dhd_type or -driver "dhd-cdc-usb-comp-gpl" \
# -image "/projects/hnd_sig_ext11/keep/fc19_sniffer/wl-r528371-amsdu-fix-ext-stadef.ko"
# -dhd_date
# -app_date
# -app_tag 
# -dhd_tag
# to enable console prints before starting the test - (dhd -i eth1 dconpoll 10)
#####################################
# DHD ucode debug:
#	in STA object set "-assertrecovery 0", so the DHD does not get reloaded after the crash
#	in STA "-wlinitcmds", set "wl assert_type 2"
#####################################
# stanightly flag:
# -chanonly
# -nochan -norate -norvr
# -noperf80 -noperf40
# -nopm -noudp -noaes -noframeburst
# Test/StaNightly.test -utfconf mc40c -title 'mc40c: 43602' -ap 43602tAP -sta '43602nfd' -nocache -kppsonly
#####################################
# resolv.conf should be:
# search sj.broadcom.com broadcom.com
# nameserver 10.17.21.20
# nameserver 10.17.18.20
# otherwise /projects will not work
#####################################


# For using attenuator, need this package as UTF.tcl doesnt load it by default.
package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext19/$::env(LOGNAME)/2017/MCC"

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc10 -lan_ip 172.16.1.10 -relay MCCUTF -rev 1
UTF::Power::Synaccess npc20 -lan_ip 172.16.1.20 -relay MCCUTF
UTF::Power::Synaccess npc30 -lan_ip 172.16.1.30 -relay MCCUTF -rev 1
UTF::Power::Synaccess npc40 -lan_ip 172.16.1.40 -relay MCCUTF
UTF::Power::Synaccess npc50 -lan_ip 172.16.1.50 -relay MCCUTF -rev 1
UTF::Power::Synaccess npc60 -lan_ip 172.16.1.60 -relay MCCUTF -rev 1
UTF::Power::Synaccess npc70 -lan_ip 172.16.1.70 -relay MCCUTF -rev 1
UTF::Power::Synaccess npc80 -lan_ip 172.16.1.80 -relay MCCUTF -rev 1
UTF::Power::WebRelay WebRelay55 -lan_ip 172.16.1.55

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

			#if {$STA == "Gala-DUT" || $STA == "Gala-AWDL"} {
			#	UTF::Try "STA $STA force reboot" {
			#		set catch_resp [catch {exec ssh -o ConnectTimeout=20 root@$lan_ip reboot} catch_msg]
			#		UTF::Message INFO "" "catch_resp=$catch_resp catch_msg=$catch_msg"
			#		UTF::Sleep 45
			#	}
			#}

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

	set ::UTF::APList "AP1-4360-4706 AP3-4360-4706"
	set ::UTF::STAList "snif 4355-DUT 4355-GO MacX-Gala MacX-Gala-GO"
	set ::UTF::RebootList "4360-SoftAP $::UTF::STAList"
	set ::UTF::DownList "4360-SoftAP $::UTF::APList $::UTF::STAList"

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
UTF::Linux MCCUTF \
	-lan_ip 10.19.13.252 \
	-iperf iperf205 \
	-sta "lan p4p1" \
    -tcpwindow 4M

lan configure -ipaddr 192.168.1.220

##########################################################################################
# Sniffer
##########################################################################################
UTF::Sniffer MCCSNIF \
	-lan_ip 10.19.13.233 \
	-sta {snif enp1s0} \
	-power "npc80 1" \
    -power_button "auto" \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10 \
    -date 2015.1.20.0

##########################################################################################
#						STA 4355 PCIe Chip 
##########################################################################################

# "-app_tag BIS120RC4_BRANCH_7_15" will cause the chip to become SISO and not MIMO. Use trunk instead only for stanightly
# "-app_tag trunk" causes AWDL failure with "wl -i eth0 awdl_if -C 2 00:ff:22:33:44:ff up"

# 4355c0 PCIe - Perennial ES5.7 MMK BRCM4355C0 UMC - Murata connector
UTF::DHD MCC4355GO \
	-lan_ip 10.19.13.231 \
	-iperf iperf208 \
	-sta {4355-GO eth0} \
    -name "4355-GO" \
	-power "npc60 1" \
	-power_button "auto" \
    -hostconsole lab3hnd-mirabadi:40061 \
    -nvram_add {macaddr=00:90:4C:12:D0:03} \
	-perfchans {36/80 36l 3} \
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
	-tag DIN2930R18_BRANCH_9_44 \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
	-clm_blob "4355_kristoff.clm_blob" \
	-nvram "../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" \
	-postinstall {dhd -i eth0 dma_ring_indices 3; dhd -i eth0 h2d_phase 1; dhd -i eth0 force_trap_bad_h2d_phase 1} \
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
# 4355c0 PCIe - Perennial ES5.7 MMK BRCM4355C0 UMC - Murata connector
UTF::DHD MCC4355DUT \
	-lan_ip 10.19.13.230 \
	-iperf iperf208 \
	-sta {4355-DUT eth0} \
    -name "4355-DUT" \
	-power "npc50 1" \
	-power_button "auto" \
    -hostconsole lab3hnd-mirabadi:40051 \
    -nvram_add {macaddr=00:90:4C:12:D0:02} \
	-perfchans {36/80 36l 3} \
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
	-tag DIN2930R18_BRANCH_9_44 \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \
	-clm_blob "4355_kristoff.clm_blob" \
	-nvram "../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" \
	-postinstall {dhd -i eth0 dma_ring_indices 3; dhd -i eth0 h2d_phase 1; dhd -i eth0 force_trap_bad_h2d_phase 1} \
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
#						STA 43xx Chip for Mac Gala 10.11
##########################################################################################

UTF::Power::Laptop go_power -button {WebRelay55 1}
UTF::MacOS MacXGalaGO \
	-lan_ip 10.19.13.232 \
	-iperf iperf_awdl \
	-sta {MacX-Gala-GO en2} \
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
		-sta {MacX-Peer-WLAN en2 MacX-Peer-AWDL awdl0}
MacX-Peer-WLAN configure -ipaddr 192.168.1.234    
MacX-Peer-AWDL configure -ipaddr 192.168.5.234
######################

########################

UTF::MacOS MacXGala \
	-lan_ip 10.19.13.178 \
	-iperf iperf_awdl \
	-sta {MacX-Gala en2} \
	-power "npc50 2" \
	-power_button "auto" \
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

# Mac BT playFileIniTunes command requires "user" account and root does not work
MacX-Gala clone MacX-DUT-User \
	-name "MacX-User" \
	-user user
	
#-----------------------------------------------------------------------------------------
#						                   APs 
#-----------------------------------------------------------------------------------------

##########################################################################################
#										SoftAP 4360
##########################################################################################
UTF::Linux MCCSoftAP \
	-lan_ip 10.19.13.129 \
	-iperf iperf205 \
    -sta {4360-SoftAP enp1s0} \
    -console "lab3hnd-mirabadi:40072" \
    -power "npc70 2" \
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
    -sta {AP1-4331-4706 eth1 AP1-4360-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab3hnd-mirabadi:40010" \
    -tag AARDVARK01T_TWIG_6_37_14 \
    -brand linux26-internal-router \
    -power "npc10 1" \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-noradio_pwrsave 1 \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl_msglevel=0x101
		macaddr=00:90:4C:0D:B0:D3
		wl0_ssid=MCC2GAP1
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCC5GAP1
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
#								AP 4331/4360 Netgrear N6300
##########################################################################################
UTF::Router AP3-4706 \
    -lan_ip 192.168.1.30 \
    -sta {AP3-4331-4706 eth1 AP3-4360-4706 eth2} \
    -relay lan \
    -lanpeer lan \
    -console "lab3hnd-mirabadi:40030" \
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
		wl0_ssid=MCC2GAP3
		wl0_chanspec=1
		wl0_radio=0
        wl0_bw_cap=-1
		wl0_obss_coex=0
		wl1_ssid=MCC5GAP3
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

#===========================
UTF::Linux MCCtemp \
	-lan_ip 10.19.13.233 \
	-sta {4360-temp enp1s0} \
	-power "npc80 1" \
    -power_button "auto" \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -wlinitcmds {wl msglevel; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3; wl country US} \
    -brand "linux-internal-wl" \
    -tag BISON05T_BRANCH_7_35

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat lab3hnd-mirabadi
##########################################################################################
UTF::Q MCC

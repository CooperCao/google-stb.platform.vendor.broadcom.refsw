# -*-tcl-*-
#
# P2P Testbed configuration file for BLR02
# Edited by Rohit B  on 21 March 2016
# Last check-in on 21 March 2016

####### Controller section:
# blr02end1: FC19 (10.132.116.25)
#
####### SoftAP section:
#
# AP1: 4360softap (10.132.116.26)
#      
####### STA section:
#
# blr02tst1: 43596A0 FC19 eth0 (10.132.116.27)
# blr02tst2: 43596A0 FC19 eth0 (10.132.116.28)


# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::doc



# Define power controllers on cart.
# package require UTF::Power
# UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr02end1 -rev 1
# UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr02end1 -rev 1
# UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr02end1 -rev 1


#Summary Directory
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr24"



UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr24end1.ban.broadcom.com" \
        -group {
                G1 {1 2}
                G2 {3 4}
		ALL {1 2 3 4}
				
               }
G1 configure -default 20
G2 configure -default 20
ALL configure -default 25

set UTF::ChannelPerf 1
set UTF::Use11 1

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

####################################

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

	ALL attn default
	set ::UTF::APList "4360softap"
	set ::UTF::STAList "4361B0-GO 4361B0-GC"
	set ::UTF::DownList "4360softap $::UTF::APList $::UTF::STAList"

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



UTF::Linux blr24end1.ban.broadcom.com \
	-lan_ip 10.132.116.198 \
	-iperf iperf208 \
	-sta {lan em1} \


UTF::Linux blr24softap \
	-lan_ip 10.132.116.199 \
	-iperf iperf208 \
	-power_button "auto" \
	-sta {4360softap enp1s0} \
	-tcpwindow 4M \
	-slowassoc 10 -reloadoncrash 1 \
	-nobighammer 1 \
	-preinstall_hook {{%S dmesg -n 7}} \
	-wlinitcmds { wl msglevel +assoc; wl down; wl country US/0; wl dtim 3;wl bw_cap 2g -1;wl vht_features 3;wl txcore -o 0x3  -k 0x3 -c 0x3 -s 1;} \
	-brand "linux-internal-wl" \
	-tag EAGLE_BRANCH_10_10 \
	-pre_perf_hook {{%S wl dump rssi}} \
	-post_perf_hook {{%S wl dump rssi}} \

4360softap configure -ipaddr 192.168.1.91 -ap 1 -attngrp G2 \


UTF::DHD blr24tst1 \
	-lan_ip 10.132.116.200 \
	-sta {4361B0-GO eth0 4361B0-PGO wl0.2} \
	-iperf iperf208 \
	-power_button "auto" \
	-perfchans {36/80 36l 3} \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag IGUANA08T_REL_13_35_22 \
	-type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-brand hndrte-dongle-wl \
	-nocal 1 -slowassoc 5 \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
	-clm_blob ss_mimo.clm_blob \
	-udp 800m -noaes 1 -notkip 1 \
	-tcpwindow 2m \
	-wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3;wl mimo_bw_cap 1; wl ampdu_tx_density 6; wl ampdu_rx_density 6;} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl phy_rssi_ant}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \

4361B0-GO configure -ipaddr 192.168.1.234 -attngrp G1
4361B0-PGO configure -ipaddr 192.168.5.234
	
UTF::DHD blr24tst2 \
	-lan_ip 10.132.116.17 \
	-iperf iperf208 \
	-sta {4361B0-GC eth0 4361B0-PGC wl0.2} \
	-power_button "auto" \
	-perfchans {36/80 36l 3} \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-tag IGUANA08T_REL_13_35_22 \
	-type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-brand hndrte-dongle-wl \
	-nocal 1 -slowassoc 5 \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
	-clm_blob ss_mimo.clm_blob \
	-udp 800m -noaes 1 -notkip 1 \
	-tcpwindow 2m \
	-wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3;wl mimo_bw_cap 1; wl ampdu_tx_density 6; wl ampdu_rx_density 6;} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl phy_rssi_ant}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \

4361B0-GC configure -ipaddr 192.168.1.236 -attngrp G1
4361B0-PGC configure -ipaddr 192.168.5.236

UTF::Q blr24

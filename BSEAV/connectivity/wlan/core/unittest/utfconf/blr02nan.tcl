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
package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr02end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr02end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr02end1 -rev 1


#Summary Directory
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr02"



UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr02end1" \
        -group {
                G1 {1 2}
                G2 {3 4}
		ALL {1 2 3 4}
				
               }
G1 configure -default 10
G2 configure -default 0
ALL configure -default 26

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
	set ::UTF::STAList "43596-GO 43596-GC"
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



UTF::Linux blr02end1 \
	-lan_ip 10.132.116.25 \
	-sta {lan em1} \


UTF::Linux blr02softap \
	-lan_ip 10.132.116.26 \
	-power "npc11 1" \
	-power_button "auto" \
	-sta {4360softap enp1s0} \
	-tcpwindow 4M \
	-slowassoc 5 -reloadoncrash 1 \
	-nobighammer 1 \
	-preinstall_hook {{%S dmesg -n 7}} \
	-wlinitcmds { wl msglevel +assoc; wl down; wl country US/0; wl dtim 3;wl bw_cap 2g -1;wl vht_features 3; wl txcore -o 0x3  -k 0x3 -c 0x3 -s 1 } \
	-brand "linux-internal-wl" \
	-tag EAGLE_BRANCH_10_10 \
	-pre_perf_hook {{%S wl dump rssi}} \
	-post_perf_hook {{%S wl dump rssi}} \

4360softap configure -ipaddr 192.168.1.91 -ap 1 -attngrp G2 \

#UTF::DHD blr02tst1 \
#       -lan_ip 10.132.116.27 \
#       -sta {43596-GO eth0 43596-PGO wl0.2 } \
#       -power "npc31 2" \
#       -power_button "auto" \
#       -perfchans {36/80 36l 3} \
#	-nvram_add {macaddr=66:55:44:33:22:13} \
#	-dhd_brand linux-internal-dongle-pcie \
#	-driver dhd-msgbuf-pciefd-debug \
#	-nocal 1 -slowassoc 5  \
#	-nvram "bcm943596fcpagbss.txt" \
#	-tag DIN975T155RC31_BRANCH_9_87 \
#	-type 43596a0-roml/config_pcie_utf/rtecdc.bin \
#	-dhd_tag DHD_REL_1_363_59_* \
#	-app_tag DHD_REL_1_363_59_* \
#	-postinstall {dhd -i eth0 dconpoll} \
#	-udp 800m -noaes 1 -notkip 1 \
#	-tcpwindow 2m \
#	-wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1;}  \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl dump rssi} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl phy_rssi_ant}} \
#	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump rssi} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
#	
#
#43596-GO configure -ipaddr 192.168.1.234 -attngrp G1 \
#
#43596-PGO configure -ipaddr 192.168.5.234 \
#
#43596-GO clone 43596-GO-RSDB \
#	 -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1; wl up; wl ocl_enable 0} \
#
#43596-GO clone 43596-GO-TXBF \
#   	-wlinitcmds {wl txbf 1;wl txbf_imp 1} \
#
#
#UTF::DHD blr02tst2 \
#       -lan_ip 10.132.116.28 \
#       -sta {43596-GC eth0 43596-PGC wl0.2} \
#       -power "npc21 2" \
#       -power_button "auto" \
#	-nvram_add {macaddr=66:55:44:33:22:12} \
#       -perfchans {36/80 36l 36 11u 3} \
#       -dhd_brand linux-internal-dongle-pcie \
#       -driver dhd-msgbuf-pciefd-debug \
#	-tag DIN975T155RC31_BRANCH_9_87 \
#	-type 43596a0-roml/config_pcie_utf/rtecdc.bin \
#	-dhd_tag DHD_REL_1_363_59_* \
#	-app_tag DHD_REL_1_363_59_* \
#       -nocal 1 -slowassoc 5 \
#       -nvram "bcm943596fcpagbss.txt" \
#	-postinstall {dhd -i eth0 dconpoll} \
#	-udp 800m -noaes 1 -notkip 1 \
#	-tcpwindow 2m \
#	-wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3;wl mimo_bw_cap 1;} \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl phy_rssi_ant}} \
#	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
#
#43596-GC configure -ipaddr 192.168.1.236 \
#
#43596-PGC configure -ipaddr 192.168.5.236 \
#
#	
#43596-GC clone 43596-GC-RSDB \
#	  -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1; wl up; wl ocl_enable 0} \
#
#43596-GC clone 43596-GC-TXBF \
#   -wlinitcmds {wl txbf 1; wl tfbf_imp 1} \



UTF::DHD blr23tst1 \
	-lan_ip 10.132.116.188 \
	-sta {4357-NanMaster eth0} \
	-power_button "auto" \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-driver dhd-msgbuf-pciefd-debug \
	-brand hndrte-dongle-wl \
	-nvram "bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10 \
	-clm_blob 4347a0.clm_blob \
	-type 4357a0-ram/config_pcie_nan/rtecdc.bin \
	-wlinitcmds {dhd -i eth0 msglevel -msgtrace; wl down; wl country US/0; wl vht_features 7;} \
	-nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
	-yart {-attn5g 30-95 -attn2g 30-95 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a} \

4357-NanMaster configure -ipaddr 192.168.1.234 -attngrp G1 \

4357-NanMaster clone 4357-NanMasterT \
	-tag trunk \

#####################################################################

UTF::DHD blr23tst2 \
	-lan_ip 10.132.116.189 \
	-sta {4357-NanSlave eth0} \
	-power_button "auto" \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-brand hndrte-dongle-wl \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10 \
	-clm_blob 4347a0.clm_blob \
	-type 4357a0-ram/config_pcie_nan/rtecdc.bin \
	-wlinitcmds {dhd -i eth0 msglevel -msgtrace; wl down; wl country US/0; wl vht_features 7;} \
	-nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
	-yart {-attn5g 30-95 -attn2g 30-95 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a} \

4357-NanSlave configure -ipaddr 192.168.1.236 \

4357-NanSlave clone 4357-NanSlaveT \
	-tag trunk \

4357-NanSlave clone 4357-debug \
	-type 4357a0-ram/config_pcie_debug/rtecdc.bin

####################################################################

UTF::DHD blr23tst3 \
	-lan_ip 10.132.116.190 \
	-sta {4357-NanSpare eth0} \
	-power_button "auto" \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
	-dhd_brand linux-internal-dongle-pcie \
	-dhd_tag DHD_BRANCH_1_579 \
	-brand hndrte-dongle-wl \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram "bcm94357fcpagbe.txt" \
	-tag IGUANA_BRANCH_13_10 \
	-clm_blob 4347a0.clm_blob \
	-type 4357a0-ram/config_pcie_nan/rtecdc.bin \
	-wlinitcmds {dhd -i eth0 msglevel -msgtrace; wl down; wl country US/0; wl vht_features 7;} \
	-nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
	-yart {-attn5g 30-95 -attn2g 30-95 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a} \

4357-NanSpare configure -ipaddr 192.168.1.235 \

4357-NanSpare clone 4357-NanSpareT \
	-tag trunk \


UTF::Q blr02

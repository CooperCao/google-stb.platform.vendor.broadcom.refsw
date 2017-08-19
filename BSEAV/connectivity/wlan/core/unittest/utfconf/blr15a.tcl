# -*-tcl-*-
#
#4359 A2 P2P Testbed configuration file for blr15end1
#Edited Anuj Gupta   29 Nov 2014
#Last checkin 26 feb 15, Edit Location BLR 
####### Controller section:
# blr15end1: FC19 (10.131.80.148)
#
####### Router AP section:
#
# AP1: 4360softap 
#      
####### STA section:
#
# blr15tst1: 4359b0FC19 eth0 (10.131.80.60)
# blr15tst2: 4359b0FC19 eth0 (10.131.80.61)
#
######################################################## #



# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::doc

set UTF::ChannelPerf 1
set UTF::Use11 1

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr15end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr15end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr15end1 -rev 1

#UTF::Power::Agilent ag1 -lan_ip "172.1.1.31 5024" -model "N6700B" -voltage 3.6 -channel 1

#Summary Directory
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr15"


# Define Attenuator
# G1  Channel 1 & 2 Attenuation between SoftAP and blr15ts1
# G2  Channel 3 & 4 Attenuation between blor02tst1 and blr15tst2   G2 not present in the path between softap and blr15tst1

# Define Attenuator
# G1  Channel 1 & 2 Attenuation between SoftAP and blr15ts1
# G2  Channel 3 & 4 Attenuation between blor02tst1 and blr15tst2   G2 not present in the path between softap and blr15tst1

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr15end1" \
        -group {
                G1 {1 2 }
                G2 {3 4 }
				ALL {1 2 3 4}
				
               }
G1 configure -default 0
G2 configure -default 0
ALL configure -default 22 

set UTF::ChannelPerf 1
set UTF::Use11 1

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
    #
    # Make Sure Attenuators are set to 0 value
    #
   ALL attn default

      set ::UTF::APList "4360softap"
       set ::UTF::STAList "4359b0-FC19-DUT 4359b0-FC19-GO"
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



##########################################################################################
# UTF Endpoint - Traffic generators (no wireless cards)
##########################################################################################
UTF::Linux blr15end1 \
     -lan_ip 10.131.80.148 \
     -sta {lan em2} \
	 -power "npc21 2" \
     -power_button "auto" \



##########################################################################################
#                                  STA 4359b0 Chip 
##########################################################################################

UTF::DHD blr15tst1 \
        -lan_ip 10.131.80.150 \
        -sta {4359b0-FC19-GO eth0 4359b0-FC19-PGO wl0.2 } \
        -power "npc31 2" \
        -power_button "auto" \
        -perfchans {36/80 36l 36 3} \
        -dhd_tag trunk \
        -app_tag trunk \
		-nvram_add {macaddr=66:55:44:33:22:13} \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DINGO_BRANCH_9_10 \
    -nocal 1 -slowassoc 5  \
    -nvram "bcm94359fcpagbss.txt" \
    -type 4359b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-sr-idsup-idauth-proptxstatus-ampduhostreorder-sstput-die5-rsdbsw/rtecdc.bin \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 2m \
    -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
	-yart {-attn5g 16-95 -attn2g 48-95 -pad 30} \
    -pre_perf_hook {{%S wl dump rssi} {%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	

4359b0-FC19-GO configure -ipaddr 192.168.1.234 -attngrp G2 \

4359b0-FC19-PGO configure -ipaddr 192.168.5.234 \

4359b0-FC19-GO clone  4359b0-FC19-GOdbg \
	 -type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die5-rsdbsw/rtecdc.bin \

4359b0-FC19-GO clone 4359d3-FC19-GOdbg \
	-tag DINGO_BRANCH_9_10 \
	-type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die3-rsdbsw/rtecdc.bin \
	  -nocustom 1

4359d3-FC19-GOdbg clone 4359b1-FC19-GOdbg \
   -type 4359b1-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die3-rsdbsw/rtecdc.bin \
   -nvram "bcm943593fcpagbss.txt" \

4359b1-FC19-GOdbg clone 4359b1-FC19-GOsr \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-pktctx-sstput/rtecdc.bin \

4359b1-FC19-GOdbg clone 4359b1-FC19-GOsr363 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-pktctx-sstput/rtecdc.bin \
	-dhd_tag DHD_TWIG_1_363_59 \
	-app_tag DHD_TWIG_1_363_59 \

4359b1-FC19-GOsr363 clone 4359b1-FC19-GO10 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-slna-pktctx-sstput/rtecdc.bin \
	-nvram "bcm943593fcpagbss_slna.txt" \

4359b1-FC19-GOsr clone 4359b1-FC19-GO35 \
	-tag DINGO07T_BRANCH_9_35 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \

#4359b0-FC19-GO clone  4359b0-FC19-GO \
#	-type 4359a2-roml/threadx-pcie-ag-p2p-pno-pktfilter-keepalive-mchan-wl11u-pktctx-ccx-ve-mfp-okc-splitrx-idsup-idauth-sr-ltecx-proptxstatus-ampduhostreorder-aoe-ndoe-btcdyn-die2-norsdb-sstput/rtecdc.bin \

4359b1-FC19-GO35 clone 4359b1-FC19-GOslna \
	-dhd_tag DHD_TWIG_1_363_59 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx-slna/rtecdc.bin \
	-nvram "bcm943593fcpagbss_slna.txt" \

4359b1-FC19-GOslna  clone 4359b1-FC19-GO75 \
	-tag DIN07T48RC50_BRANCH_9_75 \
	-dhd_tag DHD_TWIG_1_363_59 \
	
4359b1-FC19-GOslna clone 4359b1-FC19-GOslnat \
	-dhd_tag trunk \

4359b1-GOslna clone 43596a0-GO105 \
	-dhd_tag DHD_REL_1_363_59_105 \
	-app_tag DHD_REL_1_363_59_105 \

43596a0-GO105 clone 43596a0-GO105-RSDB \
	-wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3;wl mimo_bw_cap 1; wl up; wl ocl_enable 0} \

	
UTF::DHD blr15tst2 \
        -lan_ip 10.131.80.151 \
        -sta {4359b0-FC19-DUT eth0 4359b0-FC19-PDUT wl0.2} \
        -power "npc21 1" \
        -power_button "auto" \
		-power_sta "ag1 1" \
	 -nvram_add {macaddr=66:55:44:33:22:12} \
        -perfchans {36/80 36l 36 11u 3} \
    -dhd_tag trunk \
        -app_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DINGO_BRANCH_9_10 \
    -nocal 1 -slowassoc 5 \
    -nvram "bcm94359fcpagbss.txt" \
    -type 4359b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-sr-idsup-idauth-proptxstatus-ampduhostreorder-sstput-die5-rsdbsw/rtecdc.bin \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 2m \
    -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3;wl mimo_bw_cap 1} \
    -pre_perf_hook {{%S wl dump rssi} {%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump rssi} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -yart {-attn5g 16-95 -attn2g 48-95 -pad 26}

4359b0-FC19-DUT configure -ipaddr 192.168.1.236 \

4359b0-FC19-PDUT configure -ipaddr 192.168.5.236 \

4359b0-FC19-DUT clone 4359b0-FC19-DUTdbg \
	 -type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die5-rsdbsw/rtecdc.bin \

4359b0-FC19-DUT clone 4359d3-FC19-DUTdbg \
	-tag DINGO_BRANCH_9_10 \
	-type 4359b0-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die3-rsdbsw/rtecdc.bin \
	  -nocustom 1

4359d3-FC19-DUTdbg clone 4359b1-FC19-DUTdbg \
   -type 4359b1-roml/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-sstput-proptxstatus-idsup-assert-err-die3-rsdbsw/rtecdc.bin \
   -nvram "bcm943593fcpagbss.txt" \

4359b1-FC19-DUTdbg clone 4359b1-FC19-DUTsr \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-pktctx-sstput/rtecdc.bin \

4359b1-FC19-DUTdbg clone 4359b1-FC19-DUTsr363 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-pktctx-sstput/rtecdc.bin \
	-dhd_tag DHD_TWIG_1_363_59 \
	-app_tag DHD_TWIG_1_363_59 \

4359b1-FC19-DUTsr363 clone 4359b1-FC19-DUT10 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-slna-pktctx-sstput/rtecdc.bin \
	-nvram "bcm943593fcpagbss_slna.txt" \
	

4359b1-FC19-DUTsr clone 4359b1-FC19-DUT35 \
	-tag DINGO07T_BRANCH_9_35 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx/rtecdc.bin \

4359b1-FC19-DUT35 clone 4359b1-FC19-DUTslna \
	-dhd_tag DHD_TWIG_1_363_59 \
	-type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx-slna/rtecdc.bin \
	-nvram "bcm943593fcpagbss_slna.txt" \
	
4359b1-FC19-DUTslna clone 4359b1-FC19-DUTslnat \
	-dhd_tag trunk \
	
4359b1-FC19-DUTslna  clone 4359b1-FC19-DUT75 \
	-tag DIN07T48RC50_BRANCH_9_75 \
	-dhd_tag DHD_TWIG_1_363_59 \

4359b1-FC19-DUTslna clone 43596a0-GO105 \
	-dhd_tag DHD_REL_1_363_59_105 \
	-app_tag DHD_REL_1_363_59_105 \

43596a0-GO105 clone 43596a0-GO105-RSDB \
	-wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3;wl mimo_bw_cap 1; wl up; wl ocl_enable 0} \

#4359b0-FC19-GO clone  4359b0-FC19-GO \
#	-type 4359a2-roml/threadx-pcie-ag-p2p-pno-pktfilter-keepalive-mchan-wl11u-pktctx-nan-ccx-ve-mfp-okc-splitrx-idsup-idauth-sr-ltecx-proptxstatus-ampduhostreorder-aoe-ndoe-btcdyn-txbf-die2-norsdb-tdls-sstput/rtecdc.bin \

##########################################################################################
#                                 SoftAP1 4360
##########################################################################################
UTF::Linux blr15softap \
        -lan_ip 10.131.80.149 \
		-power "npc31 1" \
		-power_button "auto" \
    -sta {4360softap enp1s0} \
    -tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -preinstall_hook {{%S dmesg -n 7}} \
    -wlinitcmds { wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3;} \
    -brand "linux-internal-wl" \
    -tag BISON_BRANCH_7_10 \
	-pre_perf_hook {{%S wl dump rssi}} \
	-post_perf_hook {{%S wl dump rssi}} \
	

4360softap configure -ipaddr 192.168.1.91 -ap 1 -attngrp G1 \


##########################################################################################
# Cron job test locking queue
##########################################################################################
UTF::Q blr15


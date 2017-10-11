#Testbed configuration file for blr11end1 UTF P2P Teststation
#Created by Anuj Gupta on 23Nov2014 18:00  
#Last checkin 30June2014 
####### Controller section:
# blr11end1: FC15
# IP ADDR 10.131.80.87
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.88 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.89
#
####### STA section:
#
# blr11tst1: FC 15 43224 eth0 (10.131.80.90)
# blr11tst2: FC 15 
# blr11tst3: FC 15
# blr11tst4: FC 15
######################################################### #
# Load Packages
package require UTF::Sniffer
package require UTF::Linux
package require UTF::AeroflexDirect
package require UTF::Airport
package require UTF::MacOS
package require UTF::Power

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
		-relay "blr11end1" \
		-group {
			G1 {1 2 3}
			G2 {4 5 6}
			ALL {1 2 3 4 5 6}
				}

G1 configure -default 20
G2 configure -default 20
ALL configure -default 20

#G1 configure default 0
#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G2 attn 0;}
    catch {G1 attn 0;}
	foreach S {4360softap 4359b1n} {
	catch {$S wl down}
	$S deinit
    }
    return
}

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



# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr11"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

set UTF::ChannelPerf 1


# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""


####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr11end1 -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.51 -relay blr11end1 -rev 1
UTF::Power::WebRelay  web43 -lan_ip 172.1.1.43 -invert 1
########################### Test Manager ################

UTF::Linux blr11end1 \
     -lan_ip 10.131.80.130 \
     -sta {lan em1}

############################ blr11ref0 ##########################################
# blr07ref0      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power	  - npc 31 port 1    172.1.1.31
################################################################################
 
UTF::Linux blr11softap -sta {4360 enp1s0} \
    -lan_ip 10.131.80.120 \
    -power "npc11 1" \
    -power_button "auto" \
	-tcpwindow 4M \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl vht_features 3;} \
	-pre_perf_hook {{%S wl roam_trigger}} \

4360 configure -ipaddr 192.168.1.100 -ap 1  -attngrp G1 \

4360 clone 4360b35 \
	-tag BISON05T_BRANCH_7_35 \
	
4360 clone 4360e \
	-tag EAGLE_BRANCH_10_10 \

4360b35 clone 4360softap \

4360b35 clone 4360-bf1 -tag BISON05T_BRANCH_7_35
4360-bf1 configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1
4360b35 clone 4360-bf0 -tag BISON05T_BRANCH_7_35
4360-bf0 configure -ipaddr 192.168.1.125 -attngrp G2 -ap 1 -hasdhcpd 1 \
		-wlinitcmds {wl txbf 0; wl txbf_imp 0; wl txbf 0; wl txbf_bfe_cap 0; wl txbf_bfr_cap 0; wl band a}


###########################################
# blr11tst1go
# 4358-GO
# 
#  
###########################################
UTF::Sniffer blr11tst1go \
    -lan_ip 10.131.80.121 \
    -sta {4358-GO eth0} \
    -power {npc41 1} \
    -power_button "auto" \

	# -nvram_add {macaddr=00:90:4C:18:60:58} \
	# -brand linux-external-dongle-pcie \
    # -nvram bcm94358wlpciebt.txt\
	# -slowassoc 5 \
	# -extsup 1 \
    # -datarate {-i 0.5 -frameburst 1} \
    # -tcpwindow 2m -udp 1.2g \
	# -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3}

# 4358-GO configure -ipaddr 192.168.1.104 \

# 4358-PGO configure -ipaddr 192.168.5.104 \

# 4358-GO clone 4358-GO-b112x \
	# -tag  BISNC105RC66_BRANCH_7_112\
	# -type 4358a3-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-xorcsum-proxd.bin \

# 4358-GO-b112x clone 4358-GO-b112 \
	# -type 4358a3-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-idsup-idauth-proxd-assert.bin 
	
# 4358-GO clone 4358-GOb35 \
	# -tag BISON05T_BRANCH_7_35 \
	# -brand hndrte-dongle-wl \
	# -notkip 1 -noaes 1 -nobighammer 1 \
	# -type 4358a3-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-proxd-nan-hs20sta-assert/rtecdc.bin

# 4358-GOb35 clone 4358-GOb35x \
	# -perfonly 1 -perfchans {36/80} \
	# -type 4358a3-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds/rtecdc.bin

# PCIe NIC mode
UTF::Linux blr11tst1gol \
    -lan_ip 10.131.80.121\
    -sta {4358l-GO eth0} \
	-power {npc41 1} \
    -power_button "auto" \
    -tag BISON_BRANCH_7_10 \
	-nvram_add {macaddr=00:90:4C:18:60:58} \
    -type debug-apdef-stadef-extnvm \
	-nvram bcm94358wlpciebt.txt\
	-tcpwindow 3m -udp 800m \
	-docpu 1 -reloadoncrash 1 \
	-slowassoc 5 -datarate {-i 0.5 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl vht_features 3;}

4358l-GO configure -ipaddr 192.168.1.111 \

4358l-GO clone 4358l-GO-b35 \
	-tag BISON05T_BRANCH_7_35 \

###########################################
# blr11tst1gc
# 4358-GO
# 
#  
###########################################
UTF::DHD blr11tst1gcfe \
    -lan_ip 10.131.80.125 \
    -sta {4358-GC eth0 4358-PGC wl0.1} \
    -power {npc21 1} \
    -power_button "auto" \
	-nvram_add {macaddr=00:90:4C:18:60:68} \
	-dhd_tag DHDNC39RC65_BRANCH_1_47 \
	-dhd_brand linux-internal-dongle-pcie \
	-brand linux-external-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-nvram bcm94358wlpciebt.txt\
	-slowassoc 5 \
	-extsup 1 \
	-datarate {-i 0.5 -frameburst 1} \
	-tcpwindow 2m -udp 1.2g \
	-wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3}

4358-GC configure -ipaddr 192.168.1.106 \

4358-PGC configure -ipaddr 192.168.5.106 \

4358-GC clone 4358-GC-b112x \
    -tag  BISNC105RC66_BRANCH_7_112\
	-type 4358a3-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-xorcsum-proxd.bin \

4358-GC-b112x clone 4358-GC-b112 \
	-type 4358a3-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-idsup-idauth-proxd-assert.bin 

4358-GC clone 4358-GCb35 \
	-tag BISON05T_BRANCH_7_35 \
	-brand hndrte-dongle-wl \
	-notkip 1 -noaes 1 -nobighammer 1 \
	-type 4358-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-proxd-nan-hs20sta-assert/rtecdc.bin

4358-GCb35 clone 4358-GCb35x \
	-perfonly 1 -perfchans {36/80} \
	-type 4358-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds/rtecdc.bin

# PCIe NIC mode
UTF::Linux blr11tst1gcl \
    -lan_ip 10.131.80.125\
    -sta {4358l-GC eth0 4358l-PGC wl0.1} \
    -power {npc21 1} \
    -power_button "auto" \
	-nvram_add {macaddr=00:90:4C:18:60:68} \
	-tag BISON_BRANCH_7_10 \
	-type debug-apdef-stadef-extnvm \
	-nvram bcm94358wlpciebt.txt\
	-tcpwindow 3m -udp 800m \
	-docpu 1 -reloadoncrash 1 \
	-slowassoc 5 -datarate {-i 0.5 -frameburst 1} \
	-wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl vht_features 3}

4358l-GC configure -ipaddr 192.168.1.105 \

4358l-PGC configure -ipaddr 192.168.5.105 \

4358l-GO clone 4358l-GO-b35 \
	-tag BISON05T_BRANCH_7_35 \

####################################################### card 4355-GO  ############################################################


#PCIe Full Dongle mode
UTF::DHD blr11tst4gowe \
    -lan_ip 10.131.80.124 \
    -sta {43570 eth0} \
    -power {npc31 1} \
    -power_button "auto" \
	-dhd_tag trunk \
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug \
	-brand linux-external-media \
	-nvram bcm943570pcieir_p150.txt \
	-slowassoc 5 -extsup 1 -docpu 1 \
	-datarate {-i 0.5 -frameburst 1} \
	-tcpwindow 3m -udp 800m -nocal 1 \
	-wlinitcmds {sysctl -w net.ipv4.tcp_limit_output_bytes=4193104; wl msglevel +assoc; wl vht_features 3; wl sgi_tx -1; wl sgi_rx 3} \
	-msgactions {
            "Pktid pool depleted." WARN
    	}

43570 configure -ipaddr 192.168.1.102 \
		

43570 clone 43570b \
	-tag BISON_BRANCH_7_10 \
	-brand hndrte-dongle-wl \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert/rtecdc.bin

43570b clone 43570bx \
	-perfonly 1 -perfchans {36/80} \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds/rtecdc.bin \
	
	#-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive/rtecdc.bin

43570 clone 43570b35 \
	-tag BISON05T_BRANCH_7_35 \
	-brand hndrte-dongle-wl \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert/rtecdc.bin

43570b35 clone 43570b35x \
	-perfonly 1 -perfchans {36/80} \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds/rtecdc.bin

43570 clone 43570b143 -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
	-brand linux-internal-media \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds-err-assert.bin

43570 clone 43570b143x -tag BISON05T_{TWIG,REL}_7_35_143{,_*} \
	-brand linux-external-media \
	-type 43570a2-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-wfds.bin
	


#########################  Full dongle mode ###################

UTF::DHD blr11tst2go \
        -lan_ip 10.131.80.122 \
        -sta {4355-GO eth0 4355-PGO wl0.1 } \
        -power "npc41 2" \
        -power_button "auto" \
	-nvram_add {macaddr=00:90:4C:18:60:58} \
        -perfchans {36/80 36l 36 3} \
        -dhd_tag trunk \
        -app_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DINGO_BRANCH_9_10 \
    -nocal 1 -slowassoc 5  \
    -nvram "bcm94355fcpagb.txt" \
    -type 4355b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-ampduhostreorder-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-die0-norsdb/rtecdc.bin \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 2m \
    -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} } \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -yart {-attn5g 16-95 -attn2g 48-95 -pad 30}

4355-GO configure -ipaddr 192.168.1.233 \

4355-PGO configure -ipaddr 192.168.5.233 \

4355-GO clone 4355b2-GO \
	-tag DINGO2_BRANCH_9_15 \
	-nvram "bcm94355fcpagb.txt" \
	-type 4355b2-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-txpwrcap-txcal-err-pktctx-die0/rtecdc.bin \

######################## NIC mode ##########################

# UTF::Linux blr11tst2gol \
	# -lan_ip 10.131.80.122 \
	# -sta {4355l-GO eth0 4355l-PGO wl0.1} \
	# -power "npc21 2"\
	# -power_button "auto" \
	# -tag BISON05T_BRANCH_7_35 \
	# -brand "linux-internal-wl" \
	# -nvram bcm94355fcpagb.txt \
	# -tcpwindow 4M \
	# -slowassoc 5 -reloadoncrash 1 \
    # -nobighammer 1 \
	# -brand linux-internal-wl \
	# -type debug-apdef-stadef-p2p-mchan-tdls \
	# -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	# -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

# 4355l-GO configure -ipaddr 192.168.1.91 \

# 4355l-PGO con -ipaddr 192.168.5.91 \

# 4355l-GO clone 4355l-GO-b35 \
	# -tag BISON05T_BRANCH_7_35 \


################################################################# 4355 GC #####################################################


####################### NIC MODE ######################

UTF::Linux blr11tst2gcl \
	-lan_ip 10.131.80.126 \
	-sta {4355l-GC eth0 4355l-PGC wl0.1} \
	-power "npc21 2"\
	-power_button "auto" \
	-tag BISON05T_BRANCH_7_35 \
	-brand "linux-internal-wl" \
	-nvram bcm94355fcpagb.txt \
	-tcpwindow 4M \
	-slowassoc 5 -reloadoncrash 1 \
	-nobighammer 1 \
	-brand linux-internal-wl \
	-type debug-apdef-stadef-p2p-mchan-tdls \
	-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

4355l-GC configure -ipaddr 192.168.1.92 \

4355l-PGC configure -ipaddr 192.168.5.92 \

4355l-GC clone 4355l-GC-b35 \
	-tag BISON05T_BRANCH_7_35 \

###################### Dongle Mode #####################

UTF::DHD blr11tst2gc \
        -lan_ip 10.131.80.126 \
        -sta {4355-GC eth0 4355-PGC wl0.1 } \
        -power "npc21 2" \
        -power_button "auto" \
	-nvram_add {macaddr=00:90:4C:18:60:68} \
        -perfchans {36/80 36l 36 3} \
        -dhd_tag trunk \
        -app_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -tag DINGO_BRANCH_9_10 \
    -nocal 1 -slowassoc 5  \
    -nvram "bcm94355fcpagb.txt" \
    -type 4355b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-ampduhostreorder-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-die0-norsdb/rtecdc.bin \
    -udp 800m -noaes 1 -notkip 1 \
    -tcpwindow 2m \
    -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} } \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -yart {-attn5g 16-95 -attn2g 48-95 -pad 30}

4355-GC configure -ipaddr 192.168.1.234 \

4355-PGC configure -ipaddr 192.168.5.234 \

4355-GC clone 4355b2-GC \
	-tag DINGO2_BRANCH_9_15 \
	-nvram "bcm94355fcpagb.txt" \
	-type 4355b2-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-txpwrcap-txcal-err-pktctx-die0/rtecdc.bin \

############################# blr05tst6 ######################################
# blr05tst4      - 43909
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 4 - Ramsey STE 5000 
# Power          - npc61  port 1    172.1.1.61
################################################################################

UTF::DHD blr11tst1gc \
        -lan_ip 10.131.80.125 \
        -sta {43909-GC eth0 43909-PGC wl0.1} \
        -power "npc21 1"\
        -power_button "auto" \
        -app_tag DHD_REL_1_201_12_3 \
        -dhd_tag DHD_REL_1_201_12_3  \
		-dhd_brand linux-external-dongle-sdio \
        -brand linux-external-dongle-sdio \
        -nvram bcm943909wcd1_p202.txt \
		-nvram_add {macaddr=00:90:4C:18:60:68} \
        -slowassoc 5 \
		-datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 2m  \
		-udp 800m \
        -nocal 1 \
		-postinstall {dhd -i eth0 sd_divisor 4;} \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl chanspec 36/80} \
        -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \

43909-GC configure -ipaddr 192.168.1.96  \

43909-PGC configure -ipaddr 192.168.5.96  \


43909-GC clone 43909-GC-b120 -tag BIS120RC4_TWIG_7_15_168 \
                   -type 43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin  \
	
43909-GC clone 43909-GC-b120_10 -tag BIS120RC4_REL_7_15_168_10 \
                   -type  43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
				   
		
		
43909-GC clone 43909bx -tag BIS120RC4_REL_7_15_168_9 \
				   -type 43909a0-roml/sdio-ag-mfgtest-seqcmds.bin \


################################ blr11tst1 ######################################
# blr11tst1      - 43570
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter	- Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power	 - npc41 port 2     172.1.1.41
################################################################################

UTF::DHD blr11tst3gcdf \
	-lan_ip 10.131.80.127 \
	-sta {43438-GC eth0 43438-PGC wl0.1} \
	-power {npc51 2} \
	-power_button auto \
	-dhd_tag DHD_REL_1_201_59 \
	-dhd_brand linux-external-dongle-sdio \
	-app_tag trunk \
	-app_brand linux-external-dongle-sdio \
	-nvram_add {macaddr=00:90:4C:18:60:58} \
	-modopts {sd_uhsimode=1 } \
	-nvram bcm943438wlpth_26MHz.txt \
	-brand hndrte-dongle-wl \
	-tag DINGO_REL_9_10_126 \
	-type 43430a1-ram/sdio-g-mfgtest \
	-datarate {-i 0.5 -frameburst 1} \
	-perfchans { 3 } \
	-nocal 1 \
	-tcpwindow 2m -udp 800m \
	-pre_perf_hook {{%S wl ampdu_clear_dump}} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate}} \
	-wlinitcmds {wl msglevel +assoc;} \
	
#-dhd_brand linux-internal-dongle
# -brand linux-mfgtest-dongle-sdio
# -postinstall {dhd -i eth0 sd_blocksize 2 256; dhd -i eth0 txglomsize 10} \

43438-GC configure -ipaddr 192.168.1.123 \

43438-PGC configure -ipaddr 192.168.5.123 \

43438-GC clone 43438BIS -tag BISON04T_BRANCH_7_14 \

43438-GC clone 43438bt -tag BISON_TWIG_7_10_323 \
        -brand linux-external-dongle-sdio \
        -nvram bcm943438wlpth_26MHz.txt \
        -type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-tdls-hs20sta-err-assert-biglog.bin \


43438-GC clone 43438t -tag trunk \

43438-GC clone 43438b -tag BISON_BRANCH_7_10 \
        -brand linux-external-dongle-sdio \
        -nvram bcm943438wlpth_26MHz.txt \
        -type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-err-assert-biglog.bin \

43438-GC clone 43438rel710 -tag BISON_REL_7_10_323 \
        -brand linux-external-dongle-sdio \
        -nvram bcm943438wlpth_26MHz.txt \
        -type 43430a0-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-idsup-idauth.bin.gz \
		
43438-GC clone 43438-GC-b45x -tag BISON06T_BRANCH_7_45 \
		-type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-11nprop-tdls-hs20sta-mchandump.bin \
		
		
43438-GC clone 43438-GC-b45 -tag BISON06T_BRANCH_7_45 \
        -brand linux-external-dongle-sdio \
        -type 43430a1-roml/sdio-g-pool-p2p-pno-pktfilter-keepalive-aoe-mchan-proptxstatus-ampduhostreorder-lpc-wl11u-rcc-fmc-wepso-ccx-okc-anqpo-ltecx-sr-err-assert.bin  \

UTF::DHD blr11tst3go \
        -lan_ip 10.131.80.123 \
        -sta {43909-GO eth0 43909-PGO wl0.1} \
        -power "npc31 1"\
        -power_button "auto" \
		-tag BIS120RC4_TWIG_7_15_168 \
        -app_tag DHD_REL_1_201_12_3 \
        -dhd_tag DHD_REL_1_201_12_3  \
		-dhd_brand linux-external-dongle-sdio \
        -brand linux-external-dongle-sdio \
		-nvram_add {macaddr=00:90:4C:18:60:58} \
        -nvram bcm943909wcd1_p202.txt \
		-type 43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth/rtecdc.bin \
        -slowassoc 5 \
		-datarate {-i 0.5 -frameburst 1} \
        -tcpwindow 2m  \
		-udp 800m \
        -nocal 1 \
        -wlinitcmds {wl vht_features 3; wl chanspec 36/80} \
        -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \

43909-GO configure -ipaddr 192.168.1.97  \

43909-PGO configure -ipaddr 192.168.5.97  \

43909-GO clone 43909-GO-b120 -brand linux-external-dongle-sdio \
	-tag BIS120RC4_TWIG_7_15_168 \
    -type 43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
	
43909-GO clone 43909-GO-b120_10 -tag BIS120RC4_REL_7_15_168_10 \
                   -type  43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
				   
		
		
43909-GO clone 43909bx -tag BIS120RC4_REL_7_15_168_9 \
				   -type 43909a0-roml/sdio-ag-mfgtest-seqcmds.bin \
				   


UTF::DHD blr11tst1nic \
    -lan_ip 10.131.80.125 \
    -sta {4359b1n enp1s0} \
	-power "npc21 1" \
    -power_button "auto" \
	-type obj-debug-apdef-stadef-norsdb-extnvm \
    -nvram "bcm943593fcpagbss.txt" \
    -tag DINGO_BRANCH_9_10 \
	-app_tag trunk \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5} \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3;} \
    -udp 800m -tcpwindow 3m \
    -yart {-attn5g {20-95 95-20} -attn2g {20-95 95-20} -pad 36} \
    -perfchans {36/80 36l 36 3l 3} -nopm1 1 -nopm2 1 \
	-pre_perf_hook {{%S wl roam_trigger}} \
	 
4359b1n configure -ipaddr 192.168.1.234 \
	 
4359b1n clone 4359nAP \
    -apmode 1 -nochannels 1 -datarate 0 -yart 0 -nopm1 0 -nopm2 0

	
UTF::DHD blr11tst1gc_55b3 \
     -lan_ip 10.131.80.125 \
     -sta {4355b0 eth0} \
     -dhd_brand linux-external-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO_BRANCH_9_10 \
     -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci-norsdb/rtecdc.bin \
     -nvram "bcm94355fcpagb.txt" \
     -nocal 1 -slowassoc 5 \
     -udp 800m  \
	 -tcpwindow 8m \
	 -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
     -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \
     -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl rsdb_mode 0;wl ampdu_mpdu} \
     -yart {-attn5g 05-95 -attn2g 30-95} \
	
4355b0 clone 4355b3 \
	-tag DINGO2_BRANCH_9_15 \
	-brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "bcm94355fcpagb.txt" \
	-dhd_tag trunk \
	-app_tag trunk \
	-type 4355b3-roml/threadx-pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-clm_min-consuartseci-norsdb/rtecdc.bin \
    -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl rsdb_mode 0;wl ampdu_mpdu} \
    -perfchans {36/80 36l 36 3} \

UTF::Q blr11


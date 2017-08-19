#Testbed configuration file for blr07end1 UTF ACT Teststation
#Created by Sumesh Nair on 23SEPT2014 10:00 PM  
#Last checkin 30June2014 
####### Controller section:
# blr07end1: FC15
# IP ADDR 10.131.80.11
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.12 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.13
#
####### STA section:
#
# blr05tst1: FC 15 43224 eth0 (10.131.80.14)
# blr05tst2: FC 15 43228 eth0 (10.131.80.15)
# blr05tst3: FC 15 43217 eth0 (10.131.80.17)
# blr05tst4: FC 15 4331  eth0 (10.131.80.18)
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::Heatmap

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr07end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
		ALL {1 2 3}
                }
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
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr07"

set UTF::SetupTestBed {
    G1 attn default
	G2 attn default
    foreach S {4360ref 4360aci 43602l 43602mc5 43909b0} {
	catch {$S wl down}
	$S deinit
    }
    return
}


#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \


# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""
set UTF::Use11h 1
set UTF::ChannelPerf 1
####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr07end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr07end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr07end1 -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr07end1 -rev 1

########################### Test Manager ################

UTF::Linux blr07end1 \
     -lan_ip 10.131.80.11 \
     -sta {lan eth0}

############################ blr07ref0 ##########################################
# blr07ref0      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power          - npc 31 port 1    172.1.1.31
################################################################################ 
UTF::Linux blr07ref0 -sta {4360ref eth0} \
    -lan_ip 10.131.80.12 \
    -power "npc21 1" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} } \
    -tcpwindow 4M \
    -wlinitcmds {
                 wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
                 wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                }


4360ref configure -ipaddr 192.168.1.100  -ap 1 -attngrp G1 \



#4360ref clone 4360a 
#tag AARDVARK01T_REL_6_37_14_105 \


####################### blr07ref1 Acting as Int source #########################
# blr07ref1      - 4360mc_1(99)
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 2 - STE 4450
# Power          - npc41 port 1    172.1.1.41
################################################################################


UTF::Linux blr07ref1 -sta {4360aci eth0} \
    -lan_ip 10.131.80.13 \
    -power "npc31 1"\
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -tcpwindow 4M \
    -wlinitcmds {
                 wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
                 wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                }
4360aci configure -ipaddr 192.168.1.101 -ap 1  -attngrp G2 \


################################ blr07tst1 ######################################
# blr07tst1      - 43602MC2_1
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc41 port 2     172.1.1.11
################################################################################

######################NIC Mode##############################################

UTF::Linux blr07tst1 -lan_ip 10.131.80.14 -sta {43602l eth0} \
    -power "npc41 2"\
    -wlinitcmds {wl msglevel +assoc;wl down;wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g -docpu 1 \
    -yart {-pad 23 -attn2g 40-95 -frameburst 1} \
	 -channelsweep {-usecsa} \
	 -brand "linux-internal-wl" \
	 -perfchans {36/80 36l 36 3l 3} \
	 -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 3} \
     -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
     -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \
	 -iperf iperf20x 

43602l configure -ipaddr 192.168.1.91 -ap 1 \

43602l clone 43602lt -tag trunk \
	

43602l clone 43602lb -tag BISON05T_BRANCH_7_35
					
43602l clone 43602le -tag ERRORFLOW_BRANCH_10_20
43602l clone 43602Lbis -tag BISON_BRANCH_7_14

43602l clone 43602l.11n \
    -wlinitcmds {wl msglevel +assoc;wl down;wl vhtmode 0} -nocustom 1
43602l.11n clone 43602lb.11n -tag BISON_BRANCH_7_10



UTF::DHD blr07tst6d -lan_ip 10.131.80.64 -sta {43602d eth0} \
    -power "npc61 2"\
    -tag BISON_BRANCH_7_10 \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -brand linux-internal-dongle-pcie \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-srom12-err-assert-noclminc-clm_min/rtecdc.bin \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-err-assert/rtecdc.bin \
    -wlinitcmds {wl down;wl vht_features 3} \
    -slowassoc 5 -extsup 1 \
    -channelsweep {-skip {1l 2l 3l 4l 5l 5u 6l 6u 7l 7u 8u 9u 10u 11u}} \
    -datarate {-i 0.5 -frameburst 1} \
    -perfchans {36/80 3} \
    -tcpwindow 4m -udp 1.2g -nocal 1 -docpu 1 \
    -yart {-pad 23 -attn5g 25-95 -attn2g 40-95 -frameburst 1} \
    -clm_blob 43602_sig.clm_blob -clm_blob {} \
    -iperf iperf20x

43602d configure -ipaddr 192.168.1.81

43602d clone 43602dx \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-srom12-noclminc-clm_min/rtecdc.bin \
    -type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus/rtecdc.bin \
    -perfonly 1 -perfchans {36/80}

43602d clone 43602dt \
    -tag trunk \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx-clm_min/rtecdc.bin \
    -type 43602a1-ram/pcie-ag-err-assert-splitrx/rtecdc.bin \
    -perfchans {36/80}

43602dt clone 43602dtx \
    -type 43602a1-ram/pcie-ag-err-splitrx-clm_min/rtecdc.bin \
    -type 43602a1-ram/pcie-ag-err-splitrx/rtecdc.bin \
    -perfchans {36/80} -perfonly 1

# Dingo
43602dt clone 43602dtd -tag DINGO_BRANCH_9_10
43602dtx clone 43602dtdx -tag DINGO_BRANCH_9_10

  



############################### blr05tst2 ####################################
# blr07tst2      - 43602MC5_1
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc12 port 2    172.1.1.12
################################################################################


UTF::Linux blr07tst2 \
        -lan_ip 10.131.80.15 \
        -sta {43602mc5 eth0} \
        -power "npc42 1"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON_BRANCH_7_10" \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

43602mc5 configure -ipaddr 192.168.1.91 -ap 1 \

43602mc5 clone 43602mc5t -tag trunk \

43602mc5 clone 43602mc5b -tag BISON_BRANCH_7_10 \

43602mc5 clone 43602mc5BIS -tag  BISON04T_BRANCH_7_14 \


43602mc5 clone 43602mc5b735 -tag BISON05T_BRANCH_7_35 \

43602mc5 clone 43602mc5a108  -tag AARDVARK01T_REL_6_37_14_108 \

43602mc5 clone 43602mc5a87   -tag AARDVARK01T_REL_6_37_14_87 \

#***************************************
UTF::DHD blr07tst2d \
        -lan_ip 10.131.80.15 \
        -sta {43602mc5fd eth0} \
        -power {npc42 1} \
        -power_button "auto" \
        -dhd_tag trunk \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tcpwindow 4m -udp 1.2g \
        -datarate {-i 0.5 -frameburst 1} \
        -slowassoc 5 -extsup 1 -nocal 1 -docpu 1 \
        -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi} {%S wl down} {%S wl up}} \
        -clm_blob 43602_sig.clm_blob \
        -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3;}

43602mc5fd clone 43602mc5fdb -tag trunk \
	-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-srom12-err-assert-noclminc-clm_min/rtecdc.bin
	# assert: ASSERT(result == CLM_RESULT_OK) in wlc_channel_init_ccode (wlc_channel.c:1055)
        #-type 43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-assert-err-dbgam/rtecdc.bin
	# Tim target
	#-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-srom12-err-assert-noclminc-clm_min/rtecdc.bin

43602mc5fdb clone 43602mc5fdbx \
        -perfonly 1 -perfchans {36/80} \
	-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-srom12-noclminc-clm_min/rtecdc.bin
        #-type 43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive/rtecdc.bin
	# Tim target
	#-type 43602a1-roml/pcie-ag-splitrx-fdap-mbss-mfp-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-srom12-noclminc-clm_min/rtecdc.bin

43602mc5fd clone 43602mc5fdt -tag trunk \
	-type 43602a1-ram/pcie-ag-err-assert-splitrx-clm_min/rtecdc.bin
43602mc5fdt clone 43602mc5fdtx \
        -perfonly 1 -perfchans {36/80} \
	-type 43602a1-ram/pcie-ag-err-splitrx-clm_min/rtecdc.bin

	43602mc5fd configure -ipaddr 192.168.1.91 -ap 1 \
	
UTF::DHD blr07tst2d64 \
     -lan_ip 10.131.80.15 \
     -sta {4364 eth0 } \
	 -power "npc41 2"\
    -power_button "auto" \
     -dhd_brand linux-internal-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO2_BRANCH_9_15 \
     -nocal 1 -slowassoc 5 \
     -nvram "bcm94364fcpagb.txt"\
	 -brand hndrte-dongle-wl \
	 -dhd_tag trunk \
	 -app_tag trunk \
     -type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-assert/rtecdc.bin \
     -udp 1600m  \
     -tcpwindow 8m \
     -wlinitcmds {wl down; wl vht_features 3;} \
	 -pre_perf_hook {{%S wl nrate} {%S wl dump rsdb}} \
	 -post_perf_hook {{%S wl nrate } {%S wl dump rsdb}} \
	 -channelsweep {-usecsa} \
	 -perfchans {36/80 36l 36 3} \


4364 clone 4364a0 \
	-type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-rxoverthrust-assert/rtecdc.bin \
	


4364a0 configure -ipaddr 192.168.1.210  \

 
4364 clone 4364a0_wl \
	-type 4364a0-roml/threadx-pcie-ag-msgbuf-mfgtest-seqcmds-splitrx-p2p/rtecdc.bin \



    

############################## blr05tst3 #####################################
# blr07tst3      - x87
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc11 port 1     172.1.1.11
################################################################################

UTF::DHD blr07tst3 \
        -lan_ip 10.131.80.17 \
		-sta {43909b0 eth0} \
        -power "npc11 1"\
        -power_button "auto" \
        -app_tag DHD_REL_1_201_12_5 \
        -dhd_tag DHD_REL_1_201_12_5  \
		-dhd_brand linux-external-dongle-sdio \
		-tag BIS120RC4_TWIG_7_15_168 \
        -brand linux-external-dongle-sdio \
        -nvram bcm943909wcd1_p305.txt \
		-type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
		-slowassoc 5 \
		-datarate {-i 0.5 -frameburst 1} \
		-perfchans {36/80 36l 36 3l 3} \
        -tcpwindow 2m  \
		-udp 800m \
	    -yart {-attn5g 05-95 -attn2g 30-95} \
        -nocal 1 \
        -wlinitcmds {sd_uhsimode=2;dhd -i eth0 sd_divisor 2;wl msglevel +assoc; wl down; wl vht_features 3; wl chanspec 36;dmesg} \
        -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \

43909b0 configure -ipaddr 192.168.1.96  \


43909b0 clone 43909b -tag BIS120RC4_TWIG_7_15_168 \
                   -type 43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin  \
	
43909b0 clone 43909bb -tag BIS120RC4_REL_7_15_168_10 \
                   -type  43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \


43909b0 clone 43909bb0 -tag BIS120RC4_REL_7_15_168_10 \
                   -type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
				   -perfchans {36/80,1}

43909b0 clone 43909b0_mfgc -tag BIS120RC4_REL_7_15_168_44 \
					 -brand linux-mfgtest-dongle-sdio \
	                 -type 43909b0-roml/sdio-ag-mfgtest-seqcmds-sr-dbgsr.bin \

       



############################# blr07tst4 ######################################
# blr07tst4      - 43909ap
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc61 port 1      172.1.1.12
################################################################################
#UTF::DHD blr07tst4 \
#        -lan_ip 10.131.80.18 \
#       -sta {4355b0 eth0} \
#       -dhd_brand linux-external-dongle-pcie \
#	   -app_tag trunk  \
#       -driver dhd-msgbuf-pciefd-debug \
#       -tag DINGO_BRANCH_9_10 \
#       -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci-norsdb/rtecdc.bin \
#       -nvram "bcm94355fcpagb.txt" \
#       -nocal 1 -slowassoc 5 \
#       -udp 800m  \
#       -tcpwindow 2m \
#       -wlinitcmds {wl vht_features 3} \
#       -yart {-attn5g 05-95 -attn2g 30-95}

#4355b0 configure -ipaddr 192.168.1.91 \

#4355b0 clone 4355b0t \

#4355b0 clone 4355b0.1 -sta {4355b0.1 eth0 _4355b0.2 wl1.2} \
#    -tag DINGO_BRANCH_9_10 \
#    -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci/rtecdc.bin \
#    -wlinitcmds {wl mpc 0;wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta;wl -i wl1.2 chanspec 1} \
#    -perfchans {36/80 36l} -nocustom 1 \

#_4355b0.2 clone 4355b0.2 -sta {_4355b0.1 eth0 4355b0.2 wl1.2} \
    -perfonly 1 -nocustom 1 \

#4355b0 clone 4355b0op \
#     -type 4355b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-clm_min-txpwrcap-swdiv-die0-norsdb/rtecdc.bin \
#     -wlinitcmds { wl vht_features 3} \
	 
#4355b0 clone 4355b2 \
#	-tag DINGO2_BRANCH_9_15 \
#	-nvram "bcm94349fcpagb.txt" \
#	-type 4355b2-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-txpwrcap-dfsradar-txcal-err-pktctx-die0/rtecdc.bin \



				 
set UTF::StaNightlyCustom {
    if {[regexp {43602mc5} $STA]} {
	if {[catch {
	    package require UTF::Test::YART
	    $STA wl down
	    $STA wl vhtmode 0
	    YART $Router $STA -msg "$STA: 11n" -key 11n -chanspec 36l \
		-loop $(perfloop) -history $(history) -symmetric $(symmetric)
	    $STA wl down
	    $STA wl vhtmode 1
	} ret]} {
	    UTF::Message WARN $STA $ret
	}
    } else {
	UTF::Message WARN $STA "skip nmode rvr"
    }
}


UTF::Q blr07


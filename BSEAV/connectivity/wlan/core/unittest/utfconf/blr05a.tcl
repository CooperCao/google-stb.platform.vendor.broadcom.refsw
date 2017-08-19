#Testbed configuration file for blr05end1 UTF ACT Teststation
#Created by Sumesh Nair on 23JUNE2014 10:00 PM  
#Last checkin 30June2014 
####### Controller section:
# blr05end1: FC15
# IP ADDR 10.131.80.22
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.23 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.24
#
####### STA section:
#
# blr05tst1: FC 15 43224 eth0 (10.131.80.25)
# blr05tst2: FC 15 43228 eth0 (10.131.80.26)
# blr05tst3: FC 15 43217 eth0 (10.131.80.27)
# blr05tst4: FC 15 4331  eth0 (10.131.80.28)
# blr05tst5: FC 15 43909  eth0 (10.131.80.63) 
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
        -relay "blr05end1" \
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
	catch {ALL attn 0;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr05"



#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \


# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""


####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc12 -lan_ip 172.1.1.12 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc61 -lan_ip 172.1.1.61 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc62 -lan_ip 172.1.1.62 -relay blr05end1 -rev 1

########################### Test Manager ################

UTF::Linux blr05end1 \
     -lan_ip 10.131.80.22 \
     -sta {lan eth0}

############################ blr05ref0 ##########################################
# blr05ref0      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power          - npc 31 port 1    172.1.1.31
################################################################################ 
UTF::Linux blr05ref0 -sta {4360ref eth0} \
    -lan_ip 10.131.80.23 \
    -power "npc31 1" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
	-perfchans {36/80} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S w/l rate} {%80S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} } \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -tcpwindow 4M \
	-udp 800m \
    -wlinitcmds {
                 wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
                 wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                }
4360ref configure -ipaddr 192.168.1.100  -attngrp G1 -ap 1 \



####################### blr05ref1 Acting as Int source #########################
# blr05ref1      - 4360mc_1(99)
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 2 - STE 4450
# Power          - npc41 port 1    172.1.1.41
################################################################################


UTF::Linux blr05ref1 -sta {4360aci eth0} \
    -lan_ip 10.131.80.24 \
   -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON05T_BRANCH_7_35 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
    -wlinitcmds {
                 wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
                 wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                }
4360aci configure -ipaddr 192.168.1.101 -ap 1  -attngrp G2 \


################################ blr05tst1 ######################################
# blr05tst1      - 43224
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
################################################################################



UTF::Linux blr05tst1 \
        -lan_ip 10.131.80.25 \
        -sta {43224 eth0} \
        -power "npc11 2"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON_BRANCH_7_10" \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \


43224 configure -ipaddr 192.168.1.90 -ap 0 \

43224 clone 43224t -tag trunk \

43224 clone 43224m0 -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {wl interference_override 0}} \

43224 clone 43224b -tag BISON05T_BRANCH_7_35 \

43224 clone 43224BIS -tag BISON04T_BRANCH_7_14 \
	-brand linux-mfgtest-wl \
   
43224 clone 43224a  -tag AARDVARK_BRANCH_6_30 \


############################### blr05tst2 ####################################
# blr05tst2      - 43217
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc12 port 2    172.1.1.12
################################################################################


UTF::Linux blr05tst2 \
        -lan_ip 10.131.80.26 \
        -sta {43217 eth0} \
        -power "npc12 2"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON_BRANCH_7_10" \
		-wlinitcmds {wl phymsglevel 0x800} \
        -pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

43217 configure -ipaddr 192.168.1.91 \

43217 clone 43217t -tag trunk \

43217 clone 43217b -tag BISON05T_BRANCH_7_35 \

43217 clone 43217B -brand linux-MFGC-wl \


43217 clone 43217a  -tag BISON04T_TWIG_7_14_89 \

43217 clone 43217a87   -tag AARDVARK01T_REL_6_37_14_87 \




############################## blr05tst3 #####################################
# blr05tst3      - 43228
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc11 port 1     172.1.1.11
################################################################################



UTF::Linux blr05tst3 \
        -lan_ip 10.131.80.27 \
        -sta {43228 eth0} \
        -power "npc11 1"\
        -power_button "auto" \
        -tcpwindow 4M \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON_BRANCH_7_10" \
		-wlinitcmds {wl phymsglevel +0x800} \
		-pre_perf_hook  {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl counters}} \

       


43228 configure -ipaddr 192.168.1.92 -ap 1 \

43228 clone 43228BIS -tag BISON04T_BRANCH_7_14 \
					
43228 clone 43228B35 -tag BISON05T_BRANCH_7_35 \

43228 clone 43228t -tag trunk \

43228 clone 43228b -tag BISON_BRANCH_7_10 \
        -brand linux-internal-wl \

43228 clone 43228a108  -tag AARDVARK01T_REL_6_37_14_108 \

43228 clone 43228a87   -tag AARDVARK01T_REL_6_37_14_87 \


############################# blr05tst4 ######################################
# blr05tst4      - 4331
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc12 port 1      172.1.1.12
################################################################################

UTF::Linux blr05tst4 \
        -lan_ip 10.131.80.28 \
        -sta {4331 eth0} \
        -tcpwindow 4M \
        -power "npc12 1"\
        -power_button "auto" \
        -slowassoc 5 -reloadoncrash 1 \
        -nobighammer 1 \
        -brand linux-internal-wl \
        -tag "BISON_BRANCH_7_10" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi}} \

4331 configure -ipaddr 192.168.1.94 \

4331 clone 4331int0 -tag BISON05T_BRANCH_7_35 \
    -wlinitcmds {
                 wl interference_override 0;wl interference 0; \
                }

4331 clone 4331t  -tag trunk \

4331 clone 4331b -tag BISON05T_BRANCH_7_35 \

4331 clone 4331BIS -tag BISON04T_BRANCH_7_14 \
	-brand linux-mfgtest-wl \


############################# blr05tst6 ######################################
# blr05tst4      - 43909
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 4 - Ramsey STE 5000 
# Power          - npc61  port 1    172.1.1.61
################################################################################

# UTF::DHD blr05tst5 \
#       -lan_ip 10.131.80.64 \
#      -sta {43909 eth0} \
#     -power "npc61 1"\
#        -power_button "auto" \
#       -app_tag DHD_REL_1_201_12_5 \
#        -dhd_tag DHD_REL_1_201_12_5  \
#		-dhd_brand linux-external-dongle-sdio \
#		-tag BIS120RC4_TWIG_7_15_168 \
#       -brand linux-external-dongle-sdio \
#        -nvram bcm943909wcd1_p302.txt \
#		-type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
#		-slowassoc 5 \
#		-datarate {-i 0.5 -frameburst 1} \
#       -tcpwindow 2m  \
#		-udp 800m \
#        -nocal 1 \
#        -wlinitcmds {sd_uhsimode=2;dhd -i eth0 sd_divisor 4;wl msglevel +assoc; wl down; wl vht_features 3; wl chanspec 36;dmesg} \
#        -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi}} \
#        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} } \
#
#43909 configure -ipaddr 192.168.1.96  \


#43909 clone 43909b -tag BIS120RC4_TWIG_7_15_168 \
                   -type 43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin  \
	
#43909 clone 43909bb -tag BIS120RC4_REL_7_15_168_10 \
                   -type  43909a0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \


#43909 clone 43909b0 -tag BIS120RC4_REL_7_15_168_10 \
                   -type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
				   -perfchans {36/80,1}

		
#43909 clone 43909bx -tag BIS120RC4_REL_7_15_168_9 \
				   -type 43909a0-roml/sdio-ag-mfgtest-seqcmds.bin \
		
				        
# UTF::DHD blr05tst58 \
        # -lan_ip 10.131.80.63 \
        # -sta {4358 eth0} \
        # -power {npc41 1} \
        # -power_button "auto" \
	# -dhd_tag DHDNC39RC65_BRANCH_1_47 \
	 # -dhd_brand linux-internal-dongle-pcie \
	# -brand linux-external-dongle-pcie \
	# -driver dhd-msgbuf-pciefd-debug \
        # -nvram bcm94358wlpciebt.txt\
	# -slowassoc 5 \
	# -extsup 1 \
	# -perfchans {36/80 36l 36 1l 3} \
        # -datarate {-i 0.5 -frameburst 1} \
        # -tcpwindow 2m -udp 1.2g \
        # -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3}

# 4358 configure -ipaddr 192.168.1.104 \
		
		
# 4358 clone 4358b112x \
	# -tag  BISNC105RC66_BRANCH_7_112\
	# -type 4358a3-roml/pcie-ag-p2p-pktctx-aibss-relmcast-proptxstatus-ampduhostreorder-sr-txbf-aoe-pktfilter-keepalive-clm_ss_mimo-xorcsum-proxd.bin \

# 4358b112x clone 4358b112 \
	# -type 4358a3-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-wnm-tdls-amsdutx5g-ltecx-okc-ccx-ve-clm_ss_mimo-txpwr-fmc-btcdyn-wepso-rcc-noccxaka-sarctrl-xorcsum-idsup-idauth-proxd-assert-err.bin \

# 4358 clone 4358b35 \
	# -tag BISON05T_BRANCH_7_35 \
	# -brand hndrte-dongle-wl \
	# -type 4358a2-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-proxd-nan-hs20sta-assert/rtecdc.bin

# 4358b35 clone 4358b35x \
	# -type 4358a2-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds/rtecdc.bin

				   
				   
#########################blr05tst5###########################################

################ 4359b0 ###################

#UTF::DHD blr05tst6 \
#        -lan_ip 10.131.80.64 \
#        -sta {4359b0x eth0} \
#        -power "npc41 1" \
#        -power_button "auto" \
#        -perfchans {36/80 36l 36 3} \
#        -dhd_tag trunk \
#        -app_tag trunk \
#        -dhd_brand linux-internal-dongle-pcie \
#        -driver dhd-msgbuf-pciefd-debug \
#        -tag DINGO_BRANCH_9_10 \
#		-slowassoc 5  \
#        -nvram "bcm943593fcpagbss_slna.txt"  \
#        -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-slna-pktctx-sstput/rtecdc.bin \
#        -tcpwindow 2m \
#		-udp 800m \
#        -extsup 1 -nocustom 1 \
#        -wlinitcmds { wl msglevel 0x101; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1}  \
#        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} } \
#        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} } \
#        -yart {-attn5g 16-95 -attn2g 48-95 -pad 30}

#4359b0x clone 4359b1_35 \
#   -tag DINGO07T_BRANCH_9_35 \
#   -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-die3-pktctx-slna/rtecdc.bin \
#   -nvram "bcm943593fcpagbss_slna.txt" \
#   -wlinitcmds {wl rsdb_mode 0; wl down; wl vht_features 3} \
#   -channelsweep {-usecsa} \

#4359b0x clone 4359b1 \
#   -type 4359b1-roml/pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-idsup-assert-err-txbf-die3-pktctx-sstput/rtecdc.bin \
#   -nvram "bcm943593fcpagbss.txt" \
   
#4359b1 configure -ipaddr 192.168.1.234 -ap 0 \


#UTF::DHD blr12tst4 \
#     -lan_ip 10.131.80.64 \
#    -sta {4364a0 eth0} \
#	 -power "npc41 2"\
#    -power_button "auto" \
#     -dhd_brand linux-internal-dongle-pcie \
#     -driver dhd-msgbuf-pciefd-debug \
#     -tag DINGO2_BRANCH_9_15 \
#	 -app_tag trunk \
#     -nocal 1 -slowassoc 5 \
#     -nvram "bcm94364fcpagb.txt"\
#	 -brand hndrte-dongle-wl \
#     -type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-assert/rtecdc.bin \
#     -udp 1600m  \
#     -tcpwindow 8m \
#     -wlinitcmds {wl vht_features 3} \
#     -yart {-attn5g {20-83 83-20} -attn2g {20-83 83-20} } \
#	 -channelsweep {-usecsa} \
#	 -perfchans {36/80 36l 36 3l 3} \
	 
	 
#4364a0 configure -ipaddr 192.168.1.92  \
   
#   ##########################################################################################   
#   UTF::Sniffer sniffer \
#         -lan_ip 10.131.80.57 \
#         -sta {snif eth1} \
#         -power_button "auto" \
#		 -tag "BISON_BRANCH_7_10" \
#		 -brand "linux-internal-wl" \
		 
		 
		 
		 
		 
#UTF::Q blr05


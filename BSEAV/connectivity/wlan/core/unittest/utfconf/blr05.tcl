#Testbed configuration file for blr05end1 UTF ACI Teststation

#Created by Sumesh Nair on 23JUNE2014 10:00 PM  
#Last checkin 30June2014 
####### Controller section:
# blr05end1: FC15
# IP ADDR 10.132.116.43
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
UTF::Aeroflex af -lan_ip 172.1.1.32 \
        -relay "blr05end1" \
        -group {
                G1 {1 2 3 4}
                G2 {5 6 7 8}
		ALL {1 2 3 4 5 6 7 8}
                }
# G1 configure -default 0
# G2 configure -default 0
ALL configure -default 0


# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G2 attn 0;}
    catch {G1 attn 0;}
	foreach S {4366} {
	catch {$S wl down}
	$S deinit
	ALL attn default
    }
    return
}


UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

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
UTF::Power::Synaccess npc12 -lan_ip 172.1.1.21 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc61 -lan_ip 172.1.1.61 -relay blr05end1 -rev 1
UTF::Power::Synaccess npc62 -lan_ip 172.1.1.62 -relay blr05end1 -rev 1

########################### Test Manager ################

UTF::Linux blr05end1 \
     -lan_ip 10.132.116.43 \
     -sta {lan eth0}

############################ blr05ref0 ##########################################
# blr05ref0      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power          - npc 31 port 1    172.1.1.31
################################################################################ 
UTF::Linux blr05ref0 -sta {4360 eth0} \
    -lan_ip 10.132.116.44 \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
	-perfchans {36/80} \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%80S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} } \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1}} \
    -tcpwindow 4M \
	-udp 800m \
    -wlinitcmds {
                 wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
                 wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;:
                }
4360 configure -ipaddr 192.168.1.102 -attngrp G1 -ap 1 \



####################### blr05ref1 Acting as Int source #########################
# blr05ref1      - 4360mc_1(99)
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 2 - STE 4450
# Power          - npc41 port 1    172.1.1.41
################################################################################



UTF::Linux blr05ref1 \
     -lan_ip 10.132.116.45 \
	 -sta {4366 eth1} \
	 -power_button "auto" \
	 -tcpwindow 4M \
	 -udp 1.8g \
     -datarate {-frameburst 1} \
	 -tag EAGLE_BRANCH_10_10 \
     -wlinitcmds {wl down; wl country US; wl obss_coex 0; wl vht_features 7} \
     -slowassoc 5 -reloadoncrash 1 \
     -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl vht_features}} \
     -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl vht_features}} \
	 -lanpeer {4366 4366} \
         

4366 configure -ipaddr 192.168.1.103 -attngrp G1 -ap 1 \
				



package require UTF::STB


UTF::STB 7252a -sta {4366a-GC eth1 4366a-PGC wl0.1} \
	-lan_ip 10.132.117.81 \
    -console "10.132.116.43:40000" \
    -power_button "auto" \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -dhd_tag "EAGLE_BRANCH_10_10" \
    -type "debug-apdef-stadef-armv7l" \
	-dongleimage "wl.ko" \
    -datarate {-i 0.5} \
	-yart {-attn5g 8-95 -attn2g 8-95 -pad 46} \
    -tcpwindow 4m -udp 1.8g -slowassoc 5 \
    -perfchans {36/80 3} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1; wl vht_features 7} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \

4366a-GC configure -ipaddr 192.168.1.104
4366a-PGC configure -ipaddr 192.168.2.204 \

4366a-GC clone 4366GC \
    -tag "EAGLE_BRANCH_10_10" -dhd_tag "DHD_TWIG_1_363_110" \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \
    -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l-debug \
	-dhd_brand linux-internal-media \
	
4366a-GC clone 4366_7603GC \
     -dhd_tag "DHD_REL_1_363_110_2001" \
     -tag "EAGLE_REL_10_10_69_7603" \
	 -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-tempsense.bin \
	 -type dhd-msgbuf-pciefd-media-mfp-secdma-armv7l \
	 -brand linux-external-media \
     -dhd_brand linux-external-media \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x10b0000}
	 
4366a-GC clone 4366_7603GCnsec \
     -dhd_tag "DHD_REL_1_363_110_2001" \
     -tag "EAGLE_REL_10_10_69_7603" \
	 -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-tempsense.bin \
	 -brand linux-external-media \
     -dhd_brand linux-external-media \
	 -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l
	 
4366a-GC clone 4366t-GC \
    -dhd_tag "trunk" \
	-type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l" \
	-modopts {secdma_addr=0x80000000 secdma_size=0x10b0000}

4366t-GC clone 4366_15_10gc \
     -dhd_tag STB7271_BRANCH_15_10 \
     -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l" \

4366GC clone 4366_122gc \
    -tag "EAGLE_TWIG_10_10_122" \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \
	

##RELEASE##
4366GC clone 4366_REL_GC \
     -tag EAGLE_REL_10_10_128 -date 2016.11.16.0 \
	 -brand linux-external-media \
	 -dongleimage "4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-wl11k-slvradar-wnm-stb.bin" \
	 -dhd_tag DHD_REL_1_363_110_46 \
	 -dhd_brand linux-external-media \
	 -type dhd-msgbuf-pciefd-media-mfp-secdma-wet-armv7l \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x10b0000} \

4366_15_10gc clone 4366_7271gc_REL \
     -dhd_tag STB7271_REL_15_10_36 -date 2016.12.14.0 \
	 -brand linux-external-media  \
	 -type "nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l" \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
       

##	
	
	
UTF::STB 7252b -sta {4366b-GO eth1 4366b-PGO wl0.2} \
	-lan_ip 10.132.117.110 \
    -console "10.132.116.43:40001" \
    -power_button "auto" \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -dhd_tag "EAGLE_BRANCH_10_10" \
    -type "debug-apdef-stadef-armv7l" \
	-dongleimage "wl.ko" \
    -datarate {-i 0.5} \
	-yart {-attn5g 8-95 -attn2g 8-95 -pad 46} \
    -tcpwindow 4m -udp 1.8g -slowassoc 5 \
    -perfchans {36/80 3} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1; wl vht_features 7} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \
	


4366b-GO configure -ipaddr 192.168.1.105
4366b-PGO configure -ipaddr 192.168.2.105 \

4366b-GO clone 4366GO \
    -tag "EAGLE_BRANCH_10_10" -dhd_tag "DHD_TWIG_1_363_110" \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \
    -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l-debug \
	-dhd_brand linux-internal-media \
	
4366b-GO clone 4366_7603GO \
     -dhd_tag "DHD_REL_1_363_110_2001" \
     -tag "EAGLE_REL_10_10_69_7603" \
	 -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-tempsense.bin \
	 -type dhd-msgbuf-pciefd-media-mfp-secdma-armv7l \
	 -brand linux-external-media \
     -dhd_brand linux-external-media \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x10b0000}
	 
	 
4366b-GO clone 4366_7603GOnsec \
     -dhd_tag "DHD_REL_1_363_110_2001" \
     -tag "EAGLE_REL_10_10_69_7603" \
	 -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-tempsense.bin \
	 -brand linux-external-media \
     -dhd_brand linux-external-media \
	 -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l
	 
4366b-GO clone 4366t-GO \
    -dhd_tag "trunk" \
	-type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l" \
	-modopts {secdma_addr=0x80000000 secdma_size=0x10b0000}

4366t-GO clone 4366_15_10go \
     -dhd_tag STB7271_BRANCH_15_10 \
     -type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l" \

4366GO clone 4366_122go \
    -tag "EAGLE_TWIG_10_10_122" \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \

4366GO clone 4366_REL_GO \
     -tag EAGLE_REL_10_10_128 -date 2016.11.16.0 \
	 -brand linux-external-media \
	 -dongleimage "4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-wl11k-slvradar-wnm-stb.bin" \
	 -dhd_tag DHD_REL_1_363_110_46 \
	 -dhd_brand linux-external-media \
	 -type dhd-msgbuf-pciefd-media-mfp-secdma-wet-armv7l \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x10b0000} \
	 
4366_15_10go clone 4366_7271go_REL \
     -dhd_tag STB7271_REL_15_10_36 -date 2016.12.14.0 \
	 -brand linux-external-media \
	 -type "nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l" \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \

	 

UTF::STB 7252c -sta {43602a-GC eth1 43602a-PGC wl0.1} \
	-lan_ip 10.132.117.213 \
    -console "10.132.116.43:40002" \
    -power_button "auto" \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-stb-armv7l" \
	-dongleimage "wl.ko" \
    -datarate {-i 0.5 -frameburst 1} \
	-yart {-attn5g 28-95 -attn2g 28-95 -pad 40 -frameburst 1} \
    -tcpwindow 4m -udp 1.2g -slowassoc 5 -iperfdaemon 0 \
    -perfchans {36/80 3} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 2} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \

43602a-GC configure -ipaddr 192.168.1.106
43602a-PGC configure -ipaddr 192.168.2.206 \

43602a-GC clone 43602GC \
    -tag BISON05T_TWIG_7_35_177 \
    -dhd_tag DHD_TWIG_1_363_110 \
    -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l-debug \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-err-assert-dbgam.bin" \
    -dhd_brand linux-internal-media

	
43602a-GC clone 43602t-GC \
    -dhd_tag "trunk" \
	-type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l" \
	-modopts {secdma_addr=0x80000000 secdma_size=0x880000} \



##

	
UTF::STB 7252d -sta {43602b-GO eth1 43602b-PGO wl0.2} \
	-lan_ip 10.132.117.182 \
    -console "10.132.116.43:40003" \
    -power_button "auto" \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-stb-armv7l" \
	-dongleimage "wl.ko" \
    -datarate {-i 0.5 -frameburst 1} \
	-yart {-attn5g 28-95 -attn2g 28-95 -pad 40 -frameburst 1} \
    -tcpwindow 4m -udp 1.2g -slowassoc 5 -iperfdaemon 0 \
    -perfchans {36/80 3} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 2} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \

    
43602b-GO configure -ipaddr 192.168.1.107
43602b-PGO configure -ipaddr 192.168.2.107 \

43602b-GO clone 43602GO \
    -tag BISON05T_TWIG_7_35_177 \
    -dhd_tag DHD_TWIG_1_363_110 \
    -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l-debug \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-mfp-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-err-assert-dbgam.bin" \
    -dhd_brand linux-internal-media
	
43602b-GO clone 43602t-GO \
    -dhd_tag "trunk" \
	-type "debug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-armv7l" \
	-modopts {secdma_addr=0x80000000 secdma_size=0x880000} \

	

# set UTF::StaNightlyCustom {
    # if {$(ap2) ne ""} {
	# if {[$STA hostis Linux DHD] && ![$STA cget -extsup]} {
	    # Can't do MultiSTA with wpa_supplicant yet
	    # package require UTF::Test::MultiSTANightly
	    # MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
		# -nosetuptestbed -nostaload -nostareload -nosetup \
		# -noapload -norestore -nounload
	    # package require UTF::Test::APSTA
	    # APSTA $Router $(ap2) $STA
	# }

	# Move ap2 to the same subnet and turn off dhcp
	# set ip [$(ap2) cget -ipaddr]
	# $(ap2) configure -ipaddr 192.168.1.80 -hasdhcpd 0
	# try {
	    # TDLS $Router $STA $(ap2) -chanspec 36 -symmetric $(symmetric)
	# } finally {
	    # $(ap2) configure -ipaddr $ip -hasdhcpd 1
	# }


    # }
# }	
	
	
UTF::Q blr05

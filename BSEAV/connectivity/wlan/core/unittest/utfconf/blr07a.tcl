#Testbed configuration file for blr07end1 UTF ACT Teststation
#Created by Sumesh Nair on 23SEPT2014 10:00 PM  
#Last checkin 30June2014 
####### Controller section:
# blr07end1: FC15
# IP ADDR 10.132.116.58
# NETMASK 255.255.254.0 
# GATEWAY 10.132.116.1
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
                G1 {1 2 3 4}
				G2 {5 6 7 8}
		ALL {1 2 3 4 5 6 7 8}
                }
G1 configure -default 0
G2 configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    foreach S {7252} {
	catch {$S wl down}
	$S deinit
    }
    G1 attn default
	G2 attn default
    return
}



UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck



# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr07"



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
     -lan_ip 10.132.116.58 \
     -sta {lan p16p2}


package require UTF::STB

UTF::STB 7252 -sta {7252/4366 eth1} \
	-lan_ip 10.132.117.104 \
    -console "blr07end1.ban.broadcom.com:40001" \
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
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 7;wl ver} \
	-post_perf_hook {{%S wl channel} {%S wl chanspec}} \
    -pre_perf_hook {{%S wl channel} {%S wl chanspec}} \


7252/4366 configure -ipaddr 192.168.1.103


7252/4366 clone 7252/4366ap \
	-ap 1 \
	-sta {7252/4366ap eth1} \


7252/4366 clone 7252/4366_15_10 \
   -dhd_tag STB7271_BRANCH_15_10 \
   -type debug-apdef-stadef-stb-armv7l \

7252/4366_15_10 clone 7252/4366_15_10ap \
        -ap 1 \
        -sta {7252/4366_15_10ap eth1} \

7252/4366 clone 7252/4366t \
    -dhd_tag "trunk" \
	-type "debug-apdef-stadef-stb-armv7l" \

7252/4366t clone 7252/4366t-ap \
	-ap 1 \
	-sta {7252/4366t-ap eth1} \

7252/4366 clone 7252/4366f \
    -tag "EAGLE_BRANCH_10_10" -dhd_tag "DHD_TWIG_1_363_110" \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \
    -type dhd-msgbuf-pciefd-media-mfp-wet-armv7l-debug \
	-dhd_brand linux-internal-media \
	

7252/4366f clone 7252/4366f_ap \
	-ap 1 \
	-sta {7252/4366f_ap eth1} \
	
7252/4366f clone 7252/4366_122 \
    -tag "EAGLE_TWIG_10_10_122" \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-assert-dbgam-err-wl11k-slvradar.bin \


7252/4366_122 clone 7252/4366_122ap \
	-ap 1 \
	-sta {7252/4366_122ap eth1} \

7252/4366f clone 7252/4366fx \
    -brand linux-external-media \
    -dhd_brand linux-internal-media \
    -dongleimage 4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-wl11k-slvradar.bin \
    -type dhd-msgbuf-pciefd-media-mfp-secdma-armv7l-debug \
	-modopts {secdma_addr=0x80000000 secdma_size=0x10b0000} \
	-perfonly 1 -perfchans {36/80 3} \
	
7252/4366 clone 7252/4366_122x \
     -tag "EAGLE_TWIG_10_10_122" \
	 -dhd_tag "DHD_TWIG_1_363_110" \
	 -brand linux-external-media \
	 -dhd_brand linux-internal-media \
     -dongleimage "4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16.bin" \
     -perfonly 1 -perfchans {36/80 3} \
	 -type dhd-msgbuf-pciefd-media-mfp-secdma-armv7l-debug \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x10b0000} \
	 
7252/4366f clone 7252/4366_REL \
     -tag EAGLE_REL_10_10_128 -date 2016.11.16.0 \
	 -brand linux-external-media \
	 -dongleimage "4366c0-roml/pcie-ag-pktctx-splitrx-amsdutx-idauth-idsup-tdls-txbf-p2p-mchan-mfp-sr-proptxstatus-wowlpf-pktfilter-keepalive-xorcsum-ringer-dmaindex16-wl11k-slvradar-wnm-stb.bin" \
	 
7252/4366_15_10 clone 7252/4366_7271_REL \
     -dhd_tag STB7271_REL_15_10_36 -date 2016.12.14.0 \
	 -brand linux-external-media  \
	 -type "nodebug-apdef-stadef-p2p-mchan-tdls-wowl-secdma-mfp-stb-wet-slvradar-armv7l" \
	 -modopts {secdma_addr=0x80000000 secdma_size=0x880000} \
	 	
7252/4366_7271_REL clone 7252/4366_7271_RELap \
        -ap 1 \
        -sta {7252/4366_7271_RELap eth1} \
	
##	 

UTF::STB 7445a -sta {7445/43602 eth1} \
    -lan_ip 10.132.116.62 \
    -console blr07end1.ban.broadcom.com:40000 \
    -power {npc41 2} \
    -reloadoncrash 1 \
    -brand linux-internal-media \
    -type "debug-apdef-stadef-stb-armv7l" \
    -dongleimage  "wl.ko" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl vht_features 3} \
    -datarate {-i 0.5 -frameburst 1} \
    -yart {-attn5g 28-95 -attn2g 28-95 -pad 40 -frameburst 1} \
    -tcpwindow 4m -udp 1.2g -slowassoc 5 -iperfdaemon 0 \
    -perfchans {36/80 3} \


7445/43602 configure -ipaddr 192.168.1.105 -attngrp G2 -lanpeer {7445/43602 7445/43602} \

7445/43602 clone 7445/43602t \
    -dhd_tag "trunk" \
	-type "debug-apdef-stadef-stb-armv7l" \
	-lanpeer {7445/43602t 7445/43602t}
	

7445/43602t clone 7445/43602t-ap \
	-ap 1 \
	-sta {7445/43602t-ap eth1} \

7445/43602 clone 7445/43602fb177 \
    -tag BISON05T_TWIG_7_35_177 \
	-dhd_tag DHD_TWIG_1_363_110 \
	-type dhd-msgbuf-pciefd-media-mfp-wet-armv7l-debug \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-idauth-idsup-mfp-proptxstatus-pktfilter-wowlpf-keepalive-err-assert-slvradar.bin" \
	-dhd_brand linux-internal-media \

7445/43602fb177 clone 7445/43602ap \
	-ap 1 \
	-sta {7445/43602ap eth1} \

7445/43602fb177 clone 7445/43602fb177x \
	-brand linux-external-media \
    -dongleimage "43602a1-roml/pcie-ag-pktctx-splitrx-amsdutx-txbf-p2p-mchan-idauth-idsup-tdls-mfp-sr-proptxstatus-pktfilter-wowlpf-ampduhostreorder-keepalive-slvradar.bin" \
	-dhd_brand linux-internal-media \
	-type dhd-msgbuf-pciefd-media-mfp-secdma-armv7l-debug \
	-modopts {secdma_addr=0x40000000 secdma_size=0x10b0000} \
	
	
################################ blr07tst1 ######################################
# blr07tst1      - 43602MC2_1
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000s
# Power          - npc41 port 2     172.1.1.11
################################################################################



UTF::Linux blr07ref0 \
    -lan_ip 10.132.116.59 \
	-sta {4366a enp1s0} \
    -power "npc41 1"\
	-tag EAGLE_BRANCH_10_10 \
	-datarate {-i 0.5 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl obss_coex 0; wl down; wl country US; wl vht_features 7} \
    -slowassoc 5 -reloadoncrash 1 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl counters} {%S wl vht_features}} \
    -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl vht_features}} \
	-perfchans {36/80 3} \
	-lanpeer {4366a 4366a} \
	-tcpwindow 4m \
	-udp 1.7g \
	 


# -preinstall_hook {{%S dmesg -n 7}} \	  
	 
4366a configure -ipaddr 192.168.1.102 -attngrp G1 \

4366a clone 4366sta \
	-ap 0 \
	-yart {-attn5g 30-95 -attn2g 30-95} \
	
4366a clone 4366s \
    -lanpeer {4366a} \




############################## blr05tst3 #####################################
# blr07tst3      - x87
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half  Miniflex
# RF Enclosure 3 - Ramsey STE 5000
# Power          - npc11 port 1     172.1.1.11
################################################################################


UTF::Linux blr07ref1 \
         -lan_ip 10.132.116.60 \
         -sta {4360 enp1s0} \
		 -power {npc11 1} \
		 -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -tag "BISON05T_BRANCH_7_35" -date 2015.1.20.1 \
		 -brand "linux-internal-wl" \
         -preinstall_hook {{%S dmesg -n 7}} \
		 -datarate {-b 1.2g -i 0.5 -frameburst 1} \
         -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl counters}} \
         -pre_perf_hook {{%S wl rate} {%S wl nrate} {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -tcpwindow 4M \
         -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3} \
		 -perfchans {36/80 3} \

		 
		 
	
4360 configure -ipaddr 192.168.1.101 -attngrp G2 \

4360 clone 4360sta \
	-ap 0 \
	-wlinitcmds {wl msglevel +assoc;} \
	-yart {-attn5g 30-95 -attn2g 30-95} \
	-udp 1.8g


		
		
UTF::Q blr07


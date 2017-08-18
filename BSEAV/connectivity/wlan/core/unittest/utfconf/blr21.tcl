#Test bed configuration file for blr21
#Edited Rohit B Date 15 July 2015
#Last check-in 15 July 2015

####### 
# blr21end1 : FC19 : 10.131.80.188 
####### 

#######
# blr21ref : 4360 : FC19 : 10.131.80.189
####### 

#######
# blr21dut0 : 43909 : FC19 : 10.132.80.190
#######


#########################################################
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr21end1" \
        -group {
                G1 {1}
				G2 {2}
				G3 {3}
				G4 {1 2}
				}
#G1 configure default 0
#G2 configure default 80

set ::cycle5G80AttnRange "0-95 95-0"
set ::cycle5G40AttnRange "0-95 95-0"
set ::cycle5G20AttnRange "0-95 95-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"

set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 95
	G2 attn 95
    catch {G1 attn 95;}
    catch {G2 attn 95;}
	catch {G3 attn 10;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr21"

UTF::Linux blr21end1 \
     -lan_ip 10.131.80.188 \
     -sta {lan eth0} \

UTF::Linux blr21ref \
	-sta {4360 enp1s0} \
    -lan_ip 10.131.80.189 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag "EAGLE_BRANCH_10_10" \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}  {%S wl scansuppress 1}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} } \
    -tcpwindow 212992 \
    -wlinitcmds {ifdown eth0; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; wl rxchain 1; wl txchain 1;} \

4360 configure -ipaddr 192.168.1.91 -ap 1 -attngrp G1 \


UTF::DHD blr21dut0 \
         -lan_ip 10.131.80.190 \
         -sta {43909 eth0} \
         -tcpwindow 212992 \
         -power_button "auto" \
         -slowassoc 5 -reloadoncrash 1 \
         -nobighammer 1 \
         -app_tag DHD_REL_1_201_12_5 \
         -dhd_tag DHD_REL_1_201_12_5 \
		 -tag BIS120RC4_TWIG_7_15_168 \
         -brand linux-external-dongle-sdio \
		 -dhd_brand linux-external-dongle-sdio \
         -nvram bcm943909wcd1_p302.txt \
		 -type 43909b0-roml/sdio-ag-p2p-pno-aoe-pktfilter-keepalive-mchan-proptxstatus-lpc-wl11u-txbf-pktctx-dmatxrc-idsup-idauth.bin \
		 -slowassoc 5 \
		 -datarate {-i 0.5 -frameburst 1} \
		 -udp 800m \
         -nocal 1 \
         -wlinitcmds {sd_uhsimode=2; wl msglevel +assoc; wl down; wl vht_features 3; dmesg; } \
         -pre_perf_hook {{%S wl rssi}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
         -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl phy_rssi_ant} } \

43909 configure -ipaddr 192.168.1.92 \

43909 clone 43909a0 \
		-wlinitcmds {wl antdiv 0; wl txant 0; wl swdiv_tx_policy 0x00; wl swdiv_rx_policy 0x00;} \
		
43909 clone 43909a1 \
		-wlinitcmds {wl antdiv 1; wl txant 1; wl swdiv_tx_policy 0x11; wl swdiv_rx_policy 0x11;} \
		
43909 clone 43909a3 \
		-wlinitcmds {wl antdiv 3; wl txant 3; wl swdiv_tx_policy 0x34; wl swdiv_rx_policy 0x34;} \

# UTF::DHD blr21dut0 \
        # -lan_ip 10.131.80.190 \
        # -sta {43012 eth0} \
        # -power_button auto \
		# -dhd_tag DHD_BRANCH_1_363\
		# -dhd_brand linux-external-dongle-sdio \
        # -nvram bcm943012fcbga.txt \
		# -brand hndrte-dongle-wl \
		# -tag FOSSA_BRANCH_11_10 \
		# -app_tag DHD_BRANCH_1_363 \
		# -type 43012a0-roml/threadx-sdio-ag-p2p-pool-pno-aoe-pktfilter-keepalive-proptxstatus-idsup-idauth-ulp-wowl-romuco1-assert/rtecdc.bin \
        # -tcpwindow 2m -udp 800m \
		# -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        # -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate}} \
		# -wlinitcmds {wl msglevel +assoc; wl PM 0;} \
		# -perfchans {3 36} \
		# -yart {-attn5g 30-95 -attn2g 30-95} \

# 43012 configure -ipaddr 192.168.1.122 \


UTF::Q blr21

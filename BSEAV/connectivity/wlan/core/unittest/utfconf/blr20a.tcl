#Testbed configuration file for blr04end1
#Edited Battu kaushik Date 25April2014
#Last checkin 03SEP2013 12AM
####### Controller section:
# blr20end1: FC15
#
#
####### SOFTAP section:

# AP1:4360c
#
####### STA section:
#
# blr20tst1: 4349 eth0 (10.132.30.32)
# blr20softap: 4360 (10.131.80.185)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr20"

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr20end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
		ALL {1 2 3}
                }
G1 configure -default 20
G2 configure -default 20
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
	#
	foreach S {4360a 4360b 4355b3} {
	catch {$S wl down}
	$S deinit
    }
	G1 attn default
	G2 attn default
    return
	
}

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux blr20end1 \
     -lan_ip 10.132.116.137 \
     -sta {lan eth0} \

UTF::Linux blr20softap -sta {4360a enp1s0} \
    -lan_ip 10.132.116.138 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360a configure -ipaddr 192.168.1.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

	
UTF::Linux blr20ref1 -sta {4360b enp1s0} \
    -lan_ip 10.132.116.139 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360b configure -ipaddr 192.168.2.90 -attngrp G2 -ap 1 -hasdhcpd 1 \

 
 
UTF::DHD blr20tst1 \
     -lan_ip 10.132.116.140 \
     -sta {4355b3 eth0} \
     -dhd_brand linux-internal-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO2_BRANCH_9_15 \
     -type 4355b3-roml/pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-clm_min-consuartseci-lpas-swdiv-pmbcnrx-norsdb-assert/rtecdc.bin \
     -nvram "bcm94355fcpagb.txt" \
     -nocal 1 -slowassoc 5 -reloadoncrash 1 \
     -udp 800m  \
	 -tcpwindow 8m \
	 -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi} {%S wl nrate} {%S wl chanspec}} \
     -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl chanspec}} \
     -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu} \
     -yart {-attn5g 05-95 -attn2g 30-95} \
	 -dhd_tag trunk \
     -app_tag trunk \
	 -clm_blob "4355_stabello.clm_blob" \
     -perfchans {36/80 36l 36 3} \
	 -yart {-attn5g 30-95 -attn2g 30-95} \


4355b3 clone 4355b3n \
	 -type 4355b3-roml/config_pcie_debug/rtecdc.bin \
	 
4355b3n clone 4355b3_9_41 \
	 -tag DIN2915T165R6_BRANCH_9_41 \
	 
#UTF::Q blr20a
	 
4355b3 configure -ipaddr 192.168.2.90 \


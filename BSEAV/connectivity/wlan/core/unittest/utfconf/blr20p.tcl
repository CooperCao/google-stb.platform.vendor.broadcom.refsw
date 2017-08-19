#Testbed configuration file for blr20end1
#Edited Fidha Rahman
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
# blr20softap: 4360 (10.132.30.36)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
#############Power################################
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr20end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr20end1 -rev 1



UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr20end1" \
        -group {
                G1 {1 2 3}
                ALL {1 2 3}
                }
G1 configure -default 20
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options



# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr20/"


set UTF::SetupTestBed {

	foreach S {4360a 4355b0 4360b} {
	catch {$S wl down}
	$S deinit
    }
	G1 attn default
    return
}

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux blr20end1 \
     -lan_ip 10.131.80.184 \
     -sta {lan eth0} \

UTF::Linux blr20softap -sta {4360a enp1s0} \
    -lan_ip 10.131.80.185 \
	-power "npc21 1" \
	-power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360a configure -ipaddr 192.168.1.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

	
UTF::Linux blr20ref1 -sta {4360b enp1s0} \
    -lan_ip 10.131.80.186 \
	-power "npc21 2" \
	-power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360b configure -ipaddr 192.168.2.90 -attngrp G1 -ap 1 -hasdhcpd 1 \
 
UTF::DHD blr20tst1 \
     -lan_ip 10.131.80.187 \
     -sta {4355b0 eth0} \
	 -power "npc11 1" \
	 -power_button "auto" \
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
	 

#4355b0 configure -ipaddr 192.168.1.91 \

4355b0 clone 4355b0t \

4355b0 clone 4355b0.1 -sta {4355b0.1 eth0 _4355b0.2 wl1.2} \
    -tag DINGO_BRANCH_9_10 \
    -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci/rtecdc.bin \
    -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf 2;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta;wl -i wl1.2 chanspec 1} \
    -perfchans {36/80 36l} -nocustom 1 \

_4355b0.2 clone 4355b0.2 -sta {_4355b0.1 eth0 4355b0.2 wl1.2} \
    -perfonly 1 -nocustom 1 \

4355b0 clone 4355b0op \
     -type 4355b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-clm_min-txpwrcap-swdiv-die0-norsdb/rtecdc.bin \
     -wlinitcmds { wl vht_features 3} \
	 
4355b0 clone 4355b2 \
	-tag DINGO2_BRANCH_9_15 \
	-brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "bcm94355fcpagb.txt" \
	-dhd_tag trunk \
	-app_tag trunk \
	-type 4355b2-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-txpwrcap-dfsradar-txcal-err-pktctx-die0/rtecdc.bin \

4355b0 clone 4355b2x \
	-tag DINGO2_BRANCH_9_15 \
	-brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "bcm94355fcpagb.txt" \
	-dhd_tag trunk \
	-app_tag trunk \
	-type 4355b2-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-logtrace-srscan-txpwrcap-txcal-err-pktctx-die0/rtecdc.bin \
    -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl rsdb_mode 0;wl ampdu_mpdu} \
    -perfchans {36/80 36l 36 3} -nocustom 1
	
	

   
4355b2x clone 4355b2.1 -sta {4355b2.1 eth0 _4355b2.2 wl1.2} \
    -wlinitcmds {wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta } \
	

_4355b2.2 clone 4355b2.2 -sta {_4355b2.1 eth0 4355b2.2 wl1.2} \
        -perfchans 3l -perfonly 1 -nocustom 1 \



4355b2.2 configure -dualband {4360b 4355b2.1 -c1 36/80 -c2 3l -b1 800m -b2 800m } \
   
   
# MIMO
4355b0 clone 4355b3 \
	-tag DINGO2_BRANCH_9_15 \
	-brand hndrte-dongle-wl \
	-dhd_brand linux-internal-dongle-pcie \
	-nvram "bcm94355fcpagb.txt" \
	-dhd_tag trunk \
	-app_tag trunk \
	-type 4355b3-roml/pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-clm_min-consuartseci-lpas-swdiv-pmbcnrx-norsdb-assert/rtecdc.bin \
    -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl amsdu_aggsf ;wl nmode;wl vhtmode;wl frameburst;wl vht_features 3;wl ampdu_mpdu} \
	-clm_blob "4355_stabello.clm_blob" \
	-perfchans {36/80 36l 36 3} \
	
	
	#-channelsweep {-usecsa -band b}
	#4355b3-roml/pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-clm_min-consuartseci-lpas-swdiv-norsdb-assert
	#4355b3-roml/threadx-pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-clm_min-consuartseci-norsdb-assert

# RSDB
4355b3 clone 4355b3.1 \
    -sta {4355b3.1 eth0 _4355b3.2 wl0.2} \
    -type 4355b3-roml/pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-consuartseci-lpas-swdiv-pmbcnrx-assert/rtecdc.bin \
	-wlinitcmds {wl vht_features 3;wl bss -C 2 sta} \
	-perfchans {36/80 36l 36 3} \
	
	#4355b3-roml/pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-consuartseci-lpas-swdiv-assert

_4355b3.2 clone 4355b3.2 \
    -sta {_4355b3.1 eth0 4355b3.2 wl0.2} \
	-perfchans {36/80 36l 36 3} -perfonly 1 -nocustom 1 \


4355b3.2 configure -dualband {4360b 4355b3.1 -c1 36/80 -c2 3 -b1 800m -b2 800m} \
	
	#4355b3-roml/pcie-ag-splitbuf-pktctx-ptxs-pno-nocis-kalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrst-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-logtrace_pcie-srscan-txpwrcap-txcal-ahr-ecntr-hchk-bssinfo-txbf-err-die0-consuartseci-lpas-swdiv-assert
	
4355b3 clone 4355b3mfg \
    -type threadx-pcie-ag-msgbuf-mcnx-mfgtest-seqcmds-sr-wlota-txcal-txpwrcap-die0-consuartseci-pwrstats-swdiv-ltecx-nvramadj-phydump/rtecdc.bin \

########   4364 #######################
 
UTF::DHD blr20tst1_64 \
     -lan_ip 10.131.80.187 \
     -sta {4364 eth0 } \
	 -power "npc11 1"\
    -power_button "auto" \
     -dhd_brand linux-internal-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO2_BRANCH_9_15 \
     -nocal 1 -slowassoc 5 \
     -nvram "bcm94364fcpagb.txt"\
	 -brand hndrte-dongle-wl \
     -type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-assert/rtecdc.bin \
     -udp 1600m  \
     -tcpwindow 8m \
     -wlinitcmds {wl down; wl vht_features 3;} \
	 -pre_perf_hook {{%S wl nrate} {%S wl dump rsdb}} \
	 -post_perf_hook {{%S wl nrate } {%S wl dump rsdb}} \
	 -channelsweep {-usecsa} \
	 -perfchans {36/80 } \


4364 clone 4364a0 \
	-type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-rxoverthrust-assert/rtecdc.bin \

		
4364a0 clone 4364a0.1 -sta {4364a0.1 eth0 _4364a0.2 wl1.2} \
    -wlinitcmds {wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta} \
    -perfchans {36/80 36l 36 3}  -nocustom 1 \

_4364a0.2 clone 4364a0.2 -sta {_4364a0.1 eth0 4364a0.2 wl1.2} \
    -perfonly 1 -nocustom 1 \
	-perfchans {36/80 36l 36 3} \
	
4364a0.2 configure -dualband {4360b 4364a0.1 -c1 36/80 -c2 3 -b1 800m -b2 800m } \

4364a0.1 configure -dualband {4360b 4364a0.2 -c1 36/80 -c2 3 -b1 800m -b2 800m }

set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	if {[regexp {(4355b2x.)$} $STA - r]} {
	    UTF::Try "+RSDB Mode Switch" {
		package require UTF::Test::RSDBModeSwitch
		RSDBModeSwitch $Router $(ap2) $STA ${r}.2
	    }
	}
}
	package require UTF::Test::MultiSTANightly
	MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
	    -nosetuptestbed -nostaload -nostareload -nosetup \
	    -noapload -norestore -nounload

}

UTF::Q blr20

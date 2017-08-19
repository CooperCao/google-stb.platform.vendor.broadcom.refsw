####### Controller section:
# blr19end1: FC15
#
#
####### SOFTAP section:

# AP1:4360c
#
####### STA section:
#
# blr19tst1: 4349 eth0 (10.132.30.32)
# blr19softap: 4360 (10.132.30.36)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix



UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr19end2" \
        -group {
                G1 {1 2}
                ALL {1 2 3 4}
                }
ALL configure -default 40
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options



# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr19/"


set UTF::SetupTestBed {

	foreach S {4360a 4364-Mst 4364-Slv} {
	catch {$S wl down}
	$S deinit
    }
	ALL attn default
    return
}

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux blr19end1 \
     -lan_ip 10.131.80.176 \
     -sta {lan eth0} \

UTF::Linux blr19end2 \
     -lan_ip 10.131.80.180 \
     -sta {lan eth0} \
	 
UTF::Linux blr19softap2 \
    -sta {4360a enp1s0} \
    -lan_ip 10.131.80.181 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
	-tcpwindow 4M \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360a configure -ipaddr 192.168.1.105 -attngrp G1 -ap 1 \

UTF::DHD blr19tst3 \
     -lan_ip 10.131.80.182 \
     -sta {4364-Mst eth0 4364-AMst w10.2} \
     -dhd_brand linux-internal-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO2_BRANCH_9_15 \
     -type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-rxoverthrust-assert/rtecdc.bin \
     -nvram "bcm94364fcpagb.txt" \
     -nocal 1 -slowassoc 5 \
	 -brand hndrte-dongle-wl \
	 -nvram_add {macaddr=0E:90:78:C6:89:48} \
     -udp 800m \
	 -tcpwindow 4m \
     -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1} \
	 -yart {-attn5g 20-95 -attn2g 20-95 -pad 30} \
     -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
     -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	 
4364-Mst configure -ipaddr 192.168.1.107 
4364-AMst configure -ipaddr 192.168.5.107 
	 
UTF::DHD blr19tst4 \
     -lan_ip 10.131.80.183 \
     -sta {4364-Slv eth0 4364-ASlv w10.2} \
     -dhd_brand linux-internal-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO2_BRANCH_9_15 \
     -type 4364a0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-rxoverthrust-assert/rtecdc.bin \
     -nvram "bcm94364fcpagb.txt" \
     -nocal 1 -slowassoc 5 \
	 -nvram_add {macaddr=90:87:4C:F6:13:50} \
     -udp 800m \
	 -tcpwindow 4m \
     -wlinitcmds {wl msglevel 0x101; wl down ; wl msglevel +assoc ; wl vht_features 3 ; wl mimo_bw_cap 1} \
	 -yart {-attn5g 20-95 -attn2g 20-95 -pad 30} \
     -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
     -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	 
4364-Slv configure -ipaddr 192.168.1.110 
4364-ASlv configure -ipaddr 192.168.5.110 

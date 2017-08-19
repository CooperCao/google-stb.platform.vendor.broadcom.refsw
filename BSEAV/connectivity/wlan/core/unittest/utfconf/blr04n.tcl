#Testbed configuration file for blr04end1
#Edited Battu kaushik Date 25April2014
#Last checkin 03SEP2013 12AM
####### Controller section:
# blr04end1: FC15
#
#
####### SOFTAP section:

# AP1:4360c
#
####### STA section:
#
# blr04tst1: 4349 eth0 (10.132.30.32)
# blr04softap: 4360 (10.132.30.36)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix



UTF::Aeroflex af -lan_ip 172.1.1.22 \
        -relay "blr04end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
                G3 {7 8 9}
                ALL {1 2 3 4 5 6 7 8 9}
                }
#G1 configure default 0
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options


set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G3 attn 0;}
    catch {G2 attn 0;}
    catch {G1 attn 0;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr04/"


set UTF::SetupTestBed {

    G1 attn default
    foreach S {4360a 4360b} {
	catch {$S wl down}
	$S deinit
    }
    return
}

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

UTF::Linux blr04end1 \
     -lan_ip 10.131.80.41 \
     -sta {lan eth0} \

UTF::Linux blr04softap -sta {4360a eth1} \
    -lan_ip 10.131.80.46 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360a configure -ipaddr 192.168.1.90 -attngrp G1 -ap 1   -hasdhcpd 1  \
	
UTF::Linux blr04ref1 -sta {4360b eth0} \
    -lan_ip 10.131.80.97 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }
4360b configure -attngrp G1 -ap 1 -ipaddr 192.168.1.92  \
 
 
 
UTF::DHD blr04tst1 \
     -lan_ip 10.131.80.42 \
     -sta {4355b0 eth0} \
     -dhd_brand linux-external-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO_BRANCH_9_10 \
     -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci-norsdb/rtecdc.bin \
     -nvram "bcm94355fcpagb.txt" \
     -nocal 1 -slowassoc 5 \
     -udp 800m  \
     -tcpwindow 2m \
     -wlinitcmds {wl vht_features 3} \
     -yart {-attn5g 05-95 -attn2g 30-95}

4355b0 configure -ipaddr 192.168.1.91 \

4355b0 clone 4355b0t \

4355b0 clone 4355b0.1 -sta {4355b0.1 eth0 _4355b0.2 wl1.2} \
    -tag DINGO_BRANCH_9_10 \
    -type 4355b0-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-splitrx-proptxstatus-ampduhostreorder-idsup-die0-hightput-sr-consuartseci/rtecdc.bin \
    -wlinitcmds {wl mpc 0;wl vht_features 3;wl rsdb_mode 1;wl -w 1 bss -C 2 sta;wl -i wl1.2 chanspec 1} \
    -perfchans {36/80 36l} -nocustom 1 \

_4355b0.2 clone 4355b0.2 -sta {_4355b0.1 eth0 4355b0.2 wl1.2} \
    -perfonly 1 -nocustom 1 \

4355b0 clone 4355b0op \
     -type 4355b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-ve-awdl-ndoe-pf2-cca-pwrstats-wnm-wl11u-anqpo-noclminc-mpf-mfp-ltecx-txbf-logtrace-srscan-clm_min-txpwrcap-swdiv-die0-norsdb/rtecdc.bin \
     -wlinitcmds { wl vht_features 3} \


UTF::Linux blr04tst4 \
    -lan_ip 10.131.80.45 \
    -sta {4359B1NIC eth0} \
	-type debug-apdef-stadef-norsdb-extnvm \
    -nvram "bcm943593fcpagbss.txt" \
    -tag DINGO_BRANCH_9_10 \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5} \
    -modopts {assert_type=1} \
    -wlinitcmds {wl msglevel +assoc;wl bw_cap 2g -1;wl vht_features 3} \
    -udp 800m -tcpwindow 3m \
    -yart {-attn5g 0-95 -attn2g 0-95 -pad 36} \
    -perfchans {36/80 3} -nopm1 1 -nopm2 1
	 
4359B1NIC configure -ipaddr 192.168.1.91 \

4359B1NIC clone 4359nAP \
    -apmode 1 -nochannels 1 -datarate 0 -yart 0 -nopm1 0 -nopm2 0

UTF::Q blr04


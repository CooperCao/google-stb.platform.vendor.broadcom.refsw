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
# blr04softap: 4366 (10.132.30.36)

######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::Android

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr04end1" \
        -group {
                G1 {1 2 3}
                ALL {1 2 3}
                }
G1 configure -default 20
#G2 configure default 0
#G3 configure default 0
# Default TestBed Configuration Options



# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr04a"


set UTF::SetupTestBed {

	foreach S {4366a 4361a 4360b} {
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

UTF::Linux blr04end1 \
     -lan_ip 10.132.116.36 \
     -sta {lan eth0} \

UTF::Linux blr04softap -sta {4366a eth0} \
    -lan_ip 10.132.116.37 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl vht_features 7
    }

4366a configure -ipaddr 192.168.1.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

	
UTF::Linux blr04ref1 -sta {4360b eth0} \
    -lan_ip 10.132.116.38 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3
    }

4360b configure -ipaddr 192.168.2.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

	
UTF::Android blr04tst1 \
	-relay blr04end1 \
	-adbdevice 10.132.116.39 \
	-lan_ip "-s 10.132.116.39:5555 shell" \
    -sta {4361a wlan0} \
	-power {npc11 2} \
	-extsup 1 \
	-supp_brand android-brix-kk-wpa-supp \
	-tag IGUANA_BRANCH_13_10 \
	-brand hndrte-dongle-wl \
    -dhd_brand android-external-dongle \
	-driver dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug-3.10.20-vendorext-gcc264a6-androidx86 \
    -dhd_tag DHD_BRANCH_1_579 \
	-clm_blob ss_mimo.clm_blob \
    -nvram src/shared/nvram/bcm94357fcpagbe.txt \
    -type 4361a0-roml/config_pcie_release/rtecdc.bin \
    -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down ;
	wl vht_features 7 ;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 
    } \
	-nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
	-perfchans {36/80 3} -channelsweep {-band a } \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu} {%S wl dump rsdb}}\
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}}  \
	
set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA  -chan1 36/80 -chan2 6
	APSTA $Router $(ap2) $STA  -chan1 6 -chan2 11 -nosta

}
}

UTF::Q blr04

#Testbed configuration file for blr04end1
#Edited Battu kaushik Date 11April2016

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

##########################################################
# Load Packages
package require UTF::Android 
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr04end1" \
        -group {
                G1 {1 2 3}
				G2 {4 5 6} 
                ALL {1 2 3}
                }
G1 configure -default 0
G2 configure -default 0
#G3 configure default 0
# Default TestBed Configuration Options



# SummaryDir sets the location for test results
if {$::tcl_platform(user) eq "tima"} {
    set ::UTF::SummaryDir "/projects/hnd_sig/utf/blr04"
} else {
    set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr04"
}

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.41 -relay blr04end1 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.20 -relay blr04end1 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.1.1.81 -relay blr04end1 -rev 1

set UTF::SetupTestBed {

	foreach S {4366a  4360b 4361a} {
	catch {$S wl down}
	$S deinit
    }
	G1 attn default
    return
}

#pointing Apps to trunk
#set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

set ::cycle5G80AttnRange "0-95 95-0"
set ::cycle5G40AttnRange "0-95 95-0"
set ::cycle5G20AttnRange "0-95 95-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"



UTF::Linux blr04end1 \
     -lan_ip 10.132.116.36 \
     -sta {lan eth0} \

UTF::Linux blr04softap -sta {4366a eth0} \
    -lan_ip 10.132.116.37 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl down;wl vht_features 7;wl dtim 3; 
    }

4366a configure -ipaddr 192.168.1.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

	
UTF::Linux blr04ref1 -sta {4360b eth0} \
    -lan_ip 10.132.116.38 \
    -slowassoc 10 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
     -wlinitcmds {
        wl down;wl vht_features 3;wl dtim 3;
    }

4360b configure -ipaddr 192.168.2.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

UTF::Android blr04tst1 \
	-relay blr04end1 \
	-adbdevice 10.132.116.39 \
	-lan_ip "-s 10.132.116.39:5555 shell" \
    -sta {4361a wlan0} \
	-power {npc11 2} \
	-extsup 1 \
	-tag IGUANA_BRANCH_13_10 \
	-brand hndrte-dongle-wl \
    -dhd_brand android-external-dongle \
	-driver dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug-3.10.20-vendorext-gcc264a6-androidx86 \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag DHD_BRANCH_1_579 \
	-clm_blob ss_mimo.clm_blob \
    -nvram src/shared/nvram/bcm94357fcpagbe.txt \
    -type 4361a0-roml/config_pcie_utf1/rtecdc.bin \
    -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
    } \
	-nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
    -yart {-attn5g 25-103+3 -attn2g 3-103+3 } \
	-perfchans {36/80 3} -channelsweep {-band a } \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}} \

UTF::Android blr04tst1a \
	-relay blr04end1 \
	-adbdevice 10.132.116.39 \
	-lan_ip "-s 10.132.116.39:5555 shell" \
    -sta {4361proxd wlan0} \
	-power {npc11 2} \
	-extsup 1 \
	-tag IGUANA_BRANCH_13_10 \
	-brand hndrte-dongle-wl \
    -dhd_brand android-external-dongle \
	-driver dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug-3.10.20-vendorext-gcc264a6-androidx86 \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk\
	-clm_blob ss_mimo.clm_blob \
    -nvram src/shared/nvram/bcm94357fcpagbe.txt \
    -type 4361a0-roml/config_pcie_proxd/rtecdc.bin \
    -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
    } \
	-nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
	-perfchans {36/80 3} -channelsweep {-band a } \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}} \

 
UTF::DHD blr04tst2 \
     -lan_ip 10.132.116.40 \
     -sta {4357 eth0} \
	 -power {npc11 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
	-app_tag trunk \
    -driver dhd-msgbuf-pciefd-debug \
	-brand hndrte-dongle-wl \
    -nvram "src/shared/nvram/bcm94361fcpagbss.txt" \
    -clm_blob 4347a0.clm_blob \
    -type 4357a0-roml/config_pcie_utf1/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl band a;
	wl vht_features 7;
	wl interface_create sta -c 1;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 up;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
    -yart {-attn5g 25-103+1 -attn2g 3-103+1 -pad 35 } \
	-perfchans {36/80 36l 3} -channelsweep {-usecsa} \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}}\
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}}  \

4357 clone 4357.2 -sta {_4357 eth0 4357.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

4357 clone 4357m \
    -type 4357a0-ram/config_pcie_mfgtest/rtecdc.bin \
    -nocal 1 -noaes 1 -notkip 1 -clm_blob {}
4357.2 clone 4357m.2 \
    -type 4357a0-ram/config_pcie_mfgtest/rtecdc.bin \
    -nocal 1 -noaes 1 -notkip 1 -clm_blob {}

4357 clone 4357t \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0  -noaes 1 -notkip 1 -clm_blob {}
4357.2 clone 4357t.2 \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

4357 configure -dualband {4360b 4357.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357m configure -dualband {4360b 4357m.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357t configure -dualband {4360b 4357t.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}


# IGUANA

4357 clone 4357i -tag IGUANA_BRANCH_13_10
4357.2 clone 4357i.2 -tag IGUANA_BRANCH_13_10

4357m clone 4357im -tag IGUANA_BRANCH_13_10
4357m.2 clone 4357im.2 -tag IGUANA_BRANCH_13_10

4357t clone 4357it -tag IGUANA_BRANCH_13_10 

4357i clone 4357_mu \
    -type 4357a0-ram/config_pcie_tput_mu/rtecdc.bin \
    -noaes 1 -notkip 1 -clm_blob {} -perfchans {36/80}
	
4357t.2 clone 4357it.2 -tag IGUANA_BRANCH_13_10

4357 clone 4361t  -tag IGUANA_BRANCH_13_10 \
	-type 4361a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -slowassoc 5 \
	-wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl interface_create sta -c 1;
	sleep 1;
	wl -i wl1.2 vht_features 3 ;
	wl -i wl1.2 up;
    } \
	-perfchans {36/80}
	

4357it.2 clone 4361t.2 -tag IGUANA_BRANCH_13_10 \
	-type 4361a0-roml/config_pcie_release/rtecdc.bin \

4361t clone 4361d \
	-type 4361a0-roml/config_pcie_utf/rtecdc.bin \
	-wlinitcmds {
	wl down;
	wl vht_features 7;
	} \
	-clm_blob ss_mimo.clm_blob \
	-perfchans {36/80 36l 3}

4361d clone 4361d.2 -sta {_4361d eth0 4361d.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1 \
	-clm_blob ss_mimo.clm_blob \

4361d clone 4361B0 \
	-nvram "src/shared/nvram/bcm94361fcpagbi_B0_p301.txt" \
	-type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-extsup 1 \

4361B0 clone 4361B0.2 -sta {_4361B0 eth0 4361B0.2 wl1.2} \
	-perfchans {3} -channelsweep {-band b} -nocustom 1 \
	-wlinitcmds {wl interface_create sta -c 1;} \
	-clm_blob ss_mimo.clm_blob \

4361d clone 4361T \
	-type 4361a0-roml/config_pcie_tput/rtecdc.bin \
	-wlinitcmds {
	wl down;
	wl vht_features 7;
	wl interface_create sta -c 1;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 up;
	} \
	-clm_blob ss_mimo.clm_blob \
	-notkip 1 -nocustom 1 \
	-perfchans {36/80 3} \

4361T clone 4361T.2 -sta {_4361T eth0 4361T.2 wl1.2} \
    -perfchans {36/80 3} -channelsweep {-band b} -nocustom 1 \
	-clm_blob ss_mimo.clm_blob \

4361T clone 4361Tput \

4361T clone 4361-trunk \
	-type  4361a0-ram/config_pcie_release/rtecdc.bin \
	-nvram "src/shared/nvram/bcm94361fcpagbss.txt" \
	-tag trunk \
	-clm_blob 4347a0.clm_blob \

4361-trunk clone 4361-Jaguar \
	-tag JAGUAR_BRANCH_14_10 \
	-type 4361a0-ram/config_pcie_utf/rtecdc.bin \



4361-trunk clone 4357-trunk \
	-type  4357a0-ram/config_pcie_release/rtecdc.bin \
	-tag trunk \
	-clm_blob 4347a0.clm_blob \
	-extsup 1 \

4361-trunk clone 4361-trunk.2 -sta {_4361-trunk eth0 4361trunk.2 wl1.2} \
    -perfchans {36/80 3} -channelsweep {-band b} -nocustom 1 \

4361T clone 4361256QAM \
	-type 4361a0-roml/config_pcie_tput/rtecdc.bin \
	-wlinitcmds {
	wl down;
	wl vht_features 3;
	} \
	-clm_blob ss_mimo.clm_blob \
	-noaes 1 -notkip 1 \
	-perfchans {36/80 3} \
	

4361t.2 clone 4361T.2 \
	-wlinitcmds {
	wl down;
	wl vht_features 7;
	wl interface_create sta -c 1;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 up;
	} \
	-type 4361a0-roml/config_pcie_utf/rtecdc.bin \
	-clm_blob ss_mimo.clm_blob \
	
4361t clone 4361R \
	-type 4361a0-roml/config_pcie_release/rtecdc.bin \
	-wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	wl interface_create sta -c 1;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 up;
	} \
	-perfchans {36/80 3} \
	-noaes 1 -notkip 1 -nocustom 1 \
	-clm_blob ss_mimo.clm_blob \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}}\
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}}  \
	
4361R configure -ipaddr 192.168.1.91 \
	
4361t clone 4361rtt \
	-type 4361a0-roml/config_pcie_proxd/rtecdc.bin \
	-wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
	} \
	-clm_blob ss_mimo.clm_blob \
	
4361t clone 4361tdls \
	-type 4361a0-roml/config_pcie_utf1/rtecdc.bin \
	-wlinitcmds {
	wl down;
	wl vht_features 7;
	} \
	-clm_blob ss_mimo.clm_blob \
	

4357i configure -dualband  {4360b 4357i.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357im configure -dualband {4360b 4357im.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357it configure -dualband {4360b 4357it.2 -c1 36/80 -c2 3l -b1 1.5G -b2 800m -rate1 11x2s -rate2 9x2s}
4361t configure -dualband {4360b 4361t.2 -c1 36/80 -c2 3l -b1 1G -b2 200m -rate1 11x2s -rate2 9x2s}
4361T configure -dualband {4360b 4361T.2 -c1 36/80 -c2 3l -b1 1G -b2 200m -rate1 11x2s -rate2 9x2s}
4361d configure -dualband {4360b 4361d.2 -c1 36/80 -c2 3l -b1 1.5G -b2 800m -rate1 11x2s -rate2 9x2s}
4361R configure -dualband {4360b 4361R.2 -c1 36/80 -c2 3l -b1 1.5G -b2 800m -rate1 11x2s -rate2 9x2s}

# HORNET

4357 clone 4357h -tag HORNET_BRANCH_12_10 \
    -type 4357a0-roml/config_pcie_debug/rtecdc.bin
4357.2 clone 4357h.2 -tag HORNET_BRANCH_12_10 \
    -type 4357a0-roml/config_pcie_debug/rtecdc.bin

4357h clone 4357hm \
    -type 4357a0-roml/config_pcie_mfgtest/rtecdc.bin \
    -nocal 0 -noaes 1 -notkip 1 -clm_blob {}
4357h.2 clone 4357hm.2 \
    -type 4357a0-roml/config_pcie_mfgtest/rtecdc.bin \
    -nocal 0 -noaes 1 -notkip 1 -clm_blob {}

4357h clone 4357ht \
    -type 4357a0-roml/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}
4357h.2 clone 4357ht.2 \
    -type 4357a0-roml/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}



4357 configure -dualband {4360b 4357.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357m configure -dualband {4360b 4357m.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357h configure -dualband {4360b 4357h.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357ht configure -dualband {4360b 4357ht.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357hm configure -dualband {4360b 4357hm.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}
4357im configure -dualband {4360b 4357im.2 -c1 36/80 -c2 3l -b1 800m -b2 800m}



set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	package require UTF::Test::MultiSTANightly
	MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
	    -nosetuptestbed -nostaload -nostareload -nosetup \
	    -noapload -norestore -nounload \
    
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA  -chan1 36 -chan2 36  -core2 1
	APSTA $Router $(ap2) $STA  -chan1 6 -chan2 36 -nosta -core2 1
	APSTA $Router $(ap2) $STA  -chan1 6 -chan2 6 -nosta -core2 1
	
	package require UTF::Test::RTT
	set sta1 4361rtt
	set ap2 4361proxd
	$sta1 load 
	$ap2 load
	RTT $Router $ap2 $sta1  -chan2 36/80 -macaddr 00:12:34:45:56:67
	RTT $Router $ap2 $sta1  -chan2 6 -macaddr 00:12:34:45:56:67
	unset sta1 
	unset ap2
	
	# Move ap2 to the same subnet and turn off dhcp
	set ap2 4361a
	set sta 4361tdls
	$ap2 load 
	$sta load
 	set ip [$ap2 cget -ipaddr] 
	$ap2 configure -ipaddr 192.168.1.80 -hasdhcpd 0
	try {
	    TDLS $Router $sta $ap2 -chanspec 36
	} finally {
	    $ap2 configure -ipaddr $ip -hasdhcpd 1
	}
	$ap2 unload
	unset ap2
	unset sta
	    
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA -chan2 {161/80 11} 
	    
}
}

UTF::Q blr04

# UTF Linux Embedded test rig in ramsey2

# WAR for AP failing to disable on an RSDB device when switching from
# AP mode to STA mode.
set ::UTF::RSDBAPSTAWAR 1

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig/utf/ramsey2"

package require UTF::Aeroflex
UTF::Aeroflex Af -lan_ip 192.168.21.60 -relay UTFTestD -group {G {1 2} G1 {1} G2 {2}}

G configure -default 18

UTF::Power::Synaccess UTFPower8
UTF::Power::Synaccess npc2 -relay lan -lan_ip 192.168.21.10

# Available for reuse...
UTF::Linux UTFTestI

# LAN Endpoint and Relay
UTF::Linux UTFTestD -sta {lan eth1}
lan configure -ipaddr 192.168.21.5


set UTF::SetupTestBed {
    G attn default
}


########################

UTF::Linux UTFTestK -sta {4366a enp1s0} \
    -console {UTFTestD:40001} -tag EAGLE_BRANCH_10_10 \
    -power {UTFPower8 1} -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl down;wl country '#a/0';wl dtim 3;wl vht_features 7} \
    -tcpwindow 4m

4366a configure -ipaddr 192.168.1.80 -hasdhcpd 1 -attngrp G

#UTF::Linux UTFTestL -sta {4366b enp1s0} \
#    -console {UTFTestD:40004} -tag EAGLE_BRANCH_10_10 \
#    -power {npc2 1} -slowassoc 5 -reloadoncrash 1 \
#    -wlinitcmds {wl down;wl country '#a/0';wl dtim 3;wl vht_features 7;wl txchain 9; wl rxchain 9} \
#    -tcpwindow 3m -udp 1g
#
#4366b configure -ipaddr 192.168.2.81 -hasdhcpd 1 -attngrp G

UTF::Linux UTFTestJn -lan_ip UTFTestJ -sta {4357n enp2s0} \
    -console UTFTestD:40002 \
    -power {UTFPower8 2} -slowassoc 5 -reloadoncrash 1 \
    -type debug-apdef-stadef-extnvm \
    -nvram "bcm94357fcpagbe_p402.txt" \
    -wlinitcmds {wl down;wl country '#a/0';wl vht_features 7} \
    -tcpwindow 3m -datarate {-i 0.5 -auto} -udp 1g \
    -yart {-attn5g 18-103+3 -attn2g 27-103 -pad 23} \
    -perfchans {36/80 3}



set wlinitcmds {
    dhd -i eth1 msglevel -msgtrace;
    wl down;
    wl band a;
    wl vht_features 7;
    wl interface_create sta -c 1;
    sleep 3;
    wl -i wl1.2 vht_features 3;
    wl -i wl1.2 band b;
}

UTF::DHD UTFTestJ -sta {4357b1 eth1 4357b1.%3 wl0.%} \
    -power {UTFPower8 2} -hostconsole UTFTestD:40002 \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-pciedw-debug \
    -nvram "bcm94357fcpagbe_p402.txt" \
    -clm_blob 4357a0.clm_blob \
    -type 4357b1-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds $wlinitcmds \
    -uploadcmds {
	dhd -i eth1 sbreg 0x2000;
	dhd -i eth1 sbreg 0x2008;
	dhd -i eth1 sbreg 0x2010;
	dhd -i eth1 sbreg 0x2014;
	dhd -i eth1 sbreg 0x2018;
	dhd -i eth1 sbreg 0x2020;
	dhd -i eth1 sbreg 0x2024;
	/home/lut/bin/dump-arm-trap-pcie;:
    } \
    -preinstall_hook {
 	{$self rexec ./skylake_aspm.sh enable}
    } \
    -postcopy {
	if {![catch {$self rm .iguana}]} {
	    $self sync
	    $self power cycle
	    $self wait_for_boot
	}
    } \
    -nocal 1 -datarate {-i 0.5 -auto} -udp 1g -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 18-103 -attn2g 21-103 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a} -escan 1 \
    -post_assoc_hook {%S rte mu} \
    -msgactions {
	{txerr valid } FAIL
	{PCIe Bus Error} WARN
    }

4357b1 clone 4357b1.2 -sta {_4357b1 eth1 4357b1.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

set xconfig {
    -nopm1 1 -nopm2 1 -nobighammer 1 -nowep 1 -notkip 1
    -nofragmentation 1 -perfonly 1
}

4357b1 clone 4357b1t \
    -type 4357b1-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 {*}$xconfig
4357b1.2 clone 4357b1t.2 \
    -type 4357b1-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 {*}$xconfig

4357b1 configure -dualband {ref2 4357b1.2 -c1 36/80 -c2 3 -b1 800m -b2 800m}
4357b1t configure -dualband {ref2 4357b1t.2 -c1 36/80 -c2 3 -b1 800m -b2 800m}

set mconf {-noaes 1 -notkip 1 -clm_blob {} -nobighammer 1}
lappend mconf -wlinitcmds "$wlinitcmds
wl mpc 0
wl -i wl1.2 mpc 0
"



4357b1 clone 4357b1m {*}$mconf \
    -type 4357b1-ram/config_pcie_mfgtest/rtecdc.bin
4357b1.2 clone 4357b1m.2 {*}$mconf \
    -type 4357b1-ram/config_pcie_mfgtest/rtecdc.bin

# IGUANA

package require UTF::Test::APConfigureSecurity
package require UTF::Test::APChanspec
package require UTF::AWDL

set opti {
    -tag IGUANA_BRANCH_13_10
    -postcopy {
	$self touch .iguana
    }
    -clm_blob 4357.clm_blob
    -nvram "bcm94357fcpagbe_p404.txt"
    -nvram_add {
	fastlpo_dis=1
    }
    -usecsa 0 -nomimo_bw_cap 1
    -postinstall {dhd -i eth1 h2d_phase 1;dhd -i eth1 force_trap_bad_h2d_phase 1}
    -initscript {
	%S dhd -i eth1 flow_prio_map 1
	%S wl apsta 1
#	%S wl PM 2
	%S wl pm2_sleep_ret 60
	%S wl pm2_bcn_sleep_ret 120
	%S wl pm2_radio_shutoff_dly 10
	%S wl pm2_refresh_badiv 1
	set softap [%S wl_interface_create ap -f 2]
	$softap wl bss_rateset 1
	UTF::Test::APConfigureSecurity $softap -security open
	UTF::Test::APChanspec $softap 36
	$softap wl bss down

	set bssid "c0:ff:ee:c0:ff:ee"
	set awdl [UTF::AWDL::create %S $bssid]
	UTF::AWDL::idle $awdl $bssid -A 149/80

	# WAR for 1024QAM issue
	%S wl awdl 1
	%S wl awdl 0

#	%S wl scan_ps 1
	%S wl ocl_enable 1
	%S wl mimo_ps_cfg 1 3 0 1
	%S wl mimo_ps_cfg 3 3 0 1
	%S wl mrc_rssi_threshold -75
	%S wl ocl_rssi_threshold -75
    }
}

set db {
    -wlinitcmds {
	dhd -i eth1 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
    }
    -perfchans {36/80 3}
    -channelsweep {-no2g40}
}

4357b1 clone 4357b1i {*}$opti {*}$db \
    -type 4357b1-roml/config_pcie_debug/rtecdc.bin

4357b1t clone 4357b1ix {*}$opti {*}$db \
    -type 4357b1-roml/config_pcie_olympic_perf/rtecdc.bin \
    -perfchans {36/80}

4357b1m clone 4357b1im {*}$mconf {*}$opti -initscript {} \
    -type 4357b1-roml/config_pcie_mfgtest/rtecdc.bin
4357b1m.2 clone 4357b1im.2 {*}$mconf {*}$opti -initscript {} \
    -type 4357b1-roml/config_pcie_mfgtest/rtecdc.bin

4357b1ix configure -dualband {ref2 4357b1ix -c1 36/80 -c2 3 -b1 800m -b2 800m \
				  -icargs {-c 1}}

###

set UTF::PerfCacheMigrate {
    4357b1i#1 4357b1i.3
    4357b1ix#1 4357b1ix.3
}


4357b1ix clone ref2 -sta {ref2 eth0} \
    -hostconsole {UTFTestD:40004} -lan_ip UTFTestL -name UTFTestL \
    -power {npc2 1} \
    -preinstall_hook {} \
    -wlinitcmds {
	wl bus:enable_ds_hc 0;
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl vht_features 7;
    } \
    -nvram_add {
	macaddr=00:90:4C:12:D0:02
    } \
    -postcopy {} \
    -postinstall {dhd -i eth0 h2d_phase 1;dhd -i eth0 force_trap_bad_h2d_phase 1} \
    -initscript {}

ref2 configure -ipaddr 192.168.2.81 -hasdhcpd 1 -attngrp G
#ref2 configure -ipaddr 192.168.1.81 -hasdhcpd 0

set UTF::StaNightlyCustom {
    #package require UTF::Test::MiniP2P
    package require UTF::Test::APSTA
    #package require UTF::Test::TDLS
    package require UTF::Test::MiniMu
    package require UTF::Test::MiniAWDL
    package require UTF::Test::BeaconRatio2

    if {$(ap2) ne ""} {
	if {[$STA cget -tag] eq "trunk"} {
	    catch {$STA wl -i wl1.2 interface_remove}
	    #BeaconRatio2 $Router $(ap2) $STA -core2 1
	    APSTA $Router $(ap2) $STA -chan2 11 -core2 1
	    $STA reload
	} else {
	    MiniAWDL $STA $(ap2) -ap $Router -sync 36/80 -P1I ${STA}.3
	    BeaconRatio2 $Router $(ap2) $STA -core2 1
	    APSTA $Router $(ap2) $STA -chan2 11 -core2 1
	}

	if {1} {
	    # Move ap2 to the same subnet and turn off dhcp
	    set ip [$(ap2) cget -ipaddr]
	    $(ap2) configure -ipaddr 192.168.1.81 -hasdhcpd 0
	    try {
		#TDLS $Router $STA $(ap2) -chanspec 36/80 -security open
		UTF::Test::MiniMu $Router [list $STA $(ap2)] -PM 0
	    } finally {
		$(ap2) configure -ipaddr $ip -hasdhcpd 1
	    }
	}
    }
}


##

UTF::Q ramsey2

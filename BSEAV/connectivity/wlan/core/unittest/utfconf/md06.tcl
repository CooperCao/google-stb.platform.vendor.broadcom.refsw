#
#  $Copyright Broadcom Corporation$
#  $Id$ 2013/4/18 16:22:00 sotmishi Exp $
#
#  Testbed configuration file for MD06 Testbed
#

# Main NS 172.16.1.10
#

# WAR for AP failing to disable on an RSDB device when switching from
# AP mode to STA mode.
set ::UTF::RSDBAPSTAWAR 1

set ::UTF::SummaryDir "/projects/hnd_sig/utf/md06"

package require UTF::Aeroflex
UTF::Aeroflex Af -lan_ip  172.16.1.144:20000/udp -relay md06end1 \
    -group {G {1 2 3 4}}

G configure -default 10

UTF::Power::Synaccess NP25 -lan_ip 172.16.1.25 -relay md06end1 -rev 1
UTF::Power::Synaccess NP35 -lan_ip 172.16.1.35 -relay md06end1 -rev 1

UTF::Linux md06end1 -sta {lan p10p1}
lan configure -ipaddr 192.168.1.50

UTF::Linux md06end2 -sta {lan2 p16p1}
lan2 configure -ipaddr 192.168.1.60


set UTF::SetupTestBed {
    G attn default
}


########################

UTF::Linux md06tst3 -sta {4366a enp1s0} \
    -console {md06end1:40001} -tag EAGLE_BRANCH_10_10 \
    -power {NP35 2} -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl down;wl country '#a/0';wl dtim 3;wl vht_features 7} \
    -tcpwindow 4m

4366a configure -ipaddr 192.168.1.80 -hasdhcpd 1 -attngrp G

#UTF::Linux md06tst2 -sta {4366b enp1s0} \
#    -console {md06end1:40012} -tag EAGLE_BRANCH_10_10 \
#    -power {NP25 2} -slowassoc 5 -reloadoncrash 1 \
#    -wlinitcmds {wl down;wl country '#a/0';wl dtim 3;wl vht_features 7;wl txchain 9;wl rxchain 9} \
#    -tcpwindow 3m -udp 1g
#
#4366b configure -ipaddr 192.168.2.81 -hasdhcpd 1 -attngrp G

UTF::Linux md06tst1n -lan_ip md06tst1 -sta {4357b0n enp4s0} \
    -console {md06end1:40011} \
    -power {NP25 1} -slowassoc 5 -reloadoncrash 1 \
    -type debug-apdef-stadef-extnvm \
    -nvram "bcm94357fcpagbe_p402.txt" \
    -wlinitcmds {wl down;wl country '#a/0';wl vht_features 7} \
    -tcpwindow 3m -datarate {-i 0.5 -auto} -udp 1g \
    -yart {-attn5g 30-99 -attn2g 30-99 -pad 23} \
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

UTF::DHD md06tst1 -sta {4357b0 eth1} \
    -power {NP25 1} -hostconsole md06end1:40011 \
    -brand hndrte-dongle-wl \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-pciedw-debug \
    -nvram "bcm94357GuinnessUsiKK.txt" \
    -clm_blob 4357a0.clm_blob \
    -type 4357b0-ram/config_pcie_debug/rtecdc.bin \
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
    -yart {-attn5g 30-99 -attn2g 0-99 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a} -usecsa 0 -escan 1 \
    -post_assoc_hook {%S rte mu} \
    -msgactions {
	{txerr valid } FAIL
	{PCIe Bus Error} WARN
    }

4357b0 clone 4357b0.2 -sta {_4357b0 eth1 4357b0.2 wl1.2} \
    -perfchans {3} -channelsweep {-band b} -nocustom 1

set xconfig {
    -nopm1 1 -nopm2 1 -nobighammer 1 -nowep 1 -notkip 1
    -nofragmentation 1 -perfonly 1
}

4357b0 clone 4357b0t \
    -type 4357b0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 {*}$xconfig
4357b0.2 clone 4357b0t.2 \
    -type 4357b0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 {*}$xconfig

4357b0 configure -dualband {ref2 4357b0.2 -c1 36/80 -c2 3 -b1 800m -b2 800m}
4357b0t configure -dualband {ref2 4357b0t.2 -c1 36/80 -c2 3 -b1 800m -b2 800m}

set mconf {-noaes 1 -notkip 1 -clm_blob {} -nobighammer 1}
lappend mconf -wlinitcmds "$wlinitcmds
wl mpc 0
wl -i wl1.2 mpc 0
"



4357b0 clone 4357b0m {*}$mconf \
    -type 4357b0-ram/config_pcie_mfgtest/rtecdc.bin
4357b0.2 clone 4357b0m.2 {*}$mconf \
    -type 4357b0-ram/config_pcie_mfgtest/rtecdc.bin

# IGUANA

package require UTF::Test::APConfigureSecurity
package require UTF::Test::APChanspec
package require UTF::AWDL

set opti {
    -tag IGUANA_BRANCH_13_10
    -postcopy {
	$self touch .iguana
    }
    -clm_blob 4357_apolloe.clm_blob
    -clm_blob 4357.clm_blob
    -nvram_add {
	ccode=US

	pa5ga0=-25,7573,-1076,-38,7389,-1061,-46,7049,-1025,-55,6887,-1003
	pa5ga1=-32,7542,-1058,-32,7458,-1049,-39,7243,-1032,-5,7395,-1042

    }
    -usecsa 0 -nomimo_bw_cap 1
    -postinstall {dhd -i eth1 h2d_phase 1;dhd -i eth1 force_trap_bad_h2d_phase 1}
    -initscript {
	%S dhd -i eth1 flow_prio_map 1
	%S wl apsta 1
	# Disable PM2 as it makes ChannelSweep unstable.
	#%S wl PM 2
	%S wl pm2_sleep_ret 60
	%S wl pm2_bcn_sleep_ret 120
	%S wl pm2_radio_shutoff_dly 10
	%S wl pm2_refresh_badiv 1

	if {![regexp {x} %S]} {
	    # SoftAP disabled on release since we then don't have
	    # enough memory for AWDL.
	    set softap [%S wl_interface_create ap -f 2]
	    $softap wl bss_rateset 1
	    UTF::Test::APConfigureSecurity $softap -security open; #aespsk2
	    $softap wl bss down
	}

	# AWDL disabled until we can rewrite the MiniAWDL tests to use it
#	set bssid "c0:ff:ee:c0:ff:ee"
#	set awdl [UTF::AWDL::create %S $bssid]
#	UTF::AWDL::idle $awdl $bssid -A 149/80

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

4357b0 clone 4357b0i {*}$opti {*}$db \
    -type 4357b0-roml/config_pcie_debug/rtecdc.bin

4357b0t clone 4357b0ix {*}$opti {*}$db \
    -type 4357b0-roml/config_pcie_olympic_perf/rtecdc.bin \
    -perfchans {36/80}

4357b0 clone 4357b0im {*}$mconf {*}$opti -initscript {} \
    -type 4357b0-roml/config_pcie_mfgtest/rtecdc.bin
4357b0.2 clone 4357b0im.2 {*}$mconf {*}$opti -initscript {} \
    -type 4357b0-roml/config_pcie_mfgtest/rtecdc.bin

4357b0ix configure -dualband {ref2 4357b0ix -c1 36/80 -c2 3 -b1 800m -b2 800m \
				  -icargs {-c 1}}

###

#set UTF::PerfCacheMigrate {
#    4366b ref2
#}


4357b0ix clone ref2 \
    -hostconsole {md06end1:40012} -lan_ip md06tst2 -name md06tst2 \
    -power {NP25 2} \
    -nvram "bcm94357fcpagbe_p402.txt" \
    -nvram_add {
	macaddr=00:90:4C:12:D0:02
    } \
    -postcopy {} \
    -initscript {%S wl bus:enable_ds_hc 0;%S wl country US}

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
	    MiniAWDL $STA $(ap2) -ap $Router -sync 36/80
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
		#UTF::Test::MiniMu $Router [list $STA $(ap2)] -PM 2 -msg "MuMimo PM2"
	    } finally {
		$(ap2) configure -ipaddr $ip -hasdhcpd 1
	    }
	}
    }
}


##

UTF::Q md06

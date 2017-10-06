# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for MC41testbed
#

source utfconf/mc41shared.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc41"

# Define power controllers on cart
UTF::Power::Synaccess npc22 -lan_ip 192.168.1.10 -relay lan
UTF::Power::WebRelay hub -lan_ip 192.168.1.12 -relay lan

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {

    G attn default
    foreach S {sta1 sta2} {
	catch {$S wl down}
	$S deinit
    }
    foreach S {4708/4331 4708/4360} {
	catch {$S apshell ifconfig [$S cget -device] down}
    }
}

UTF::Linux SR1End08 -sta {lan1 eth1} -onall false
lan1 configure -ipaddr 192.168.1.45

UTF::Linux SR1End10 -sta {lan2 eth1} -onall false
lan2 configure -ipaddr 192.168.1.52

UTF::Linux SR1End06 -sta {wan eth1}

# UTF::Linux SR1End05  - eth1 only runs at 100mbps

set stacommon {
    -date 2017.1.4.0
    -type debug-apdef-stadef-p2p-mchan-tdls
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 3;
    }
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1
}

UTF::Linux mc41tst1 -sta {sta1 enp1s0} \
    -power {webswitch1 1} \
    -console {mc41end1:40001} \
    {*}$stacommon

sta1 configure -attngrp G

UTF::Linux mc41tst2 -sta {sta2 enp1s0} \
    -power {nb1 1} \
    -console {mc41end1:40002} \
    {*}$stacommon

sta2 configure -attngrp G

#############################

UTF::Router 4708t -sta {
    4708t/4360 eth2 4708t/4360.%15 wl1.%
    _4708t/4331 eth1 _4708t/4331.%15 wl0.%
} \
    -relay lan1 \
    -power {nb1 3} \
    -lanpeer {lan1 lan2} \
    -wanpeer wan \
    -console "mc41end1:40006" \
    -brand "linux-2.6.36-arm-internal-router" \
    -nvram {
	watchdog=2000
	wl0_ssid=4708/4331
	wl0_chanspec=3
	wl0_radio=0
	wl1_ssid=4708/4360
	wl1_chanspec=36
	wl1_radio=0
	wl1_vht_features=2
	# samba_mode=2
	wl0_country_code=#a
	wl0_country_rev=0
	wl1_country_code=#a
	wl1_country_rev=0
    } \
    -datarate {-i 0.5 -auto} -udp 1.2g \
    -noradio_pwrsave 1 -perfchans {36/80} -nosamba 1 \
    -yart {-attn5g 21-95 -attn2g 45-95 -pad 26}

# WAR for SWWLAN-31390
4708t/4360 configure -perfchans {36/80 36l 52}

# Clone for 4331 on 2G
_4708t/4331 clone 4708t/4331 \
    -perfchans {3} -channelsweep {-band b} \
    -datarate {-i 0.5 -frameburst 1} -udp 400m \
    -noradio_pwrsave 0

# Clones for external
4708t/4360 clone 4708tx/4360 -sta {4708tx/4360 eth2} \
    -brand linux-2.6.36-arm-external-vista-router-full-src \
    -perfonly 1 -perfchans {36/80} -noaes 1 -docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -nocustom 1

4708tx/4360 clone 4708tx/4331 -sta {4708tx/4331 eth1} \
    -perfchans {3l 3} -channelsweep {-band b} \
    -udp 400m

4708t/4360 configure -dualband {4708t/4331 -c1 36/80}

# Branches

4708t/4360 clone 4708b14/4360 -tag BISON04T_BRANCH_7_14
4708t/4331 clone 4708b14/4331 -tag BISON04T_BRANCH_7_14
4708tx/4360 clone 4708b14x/4360 -tag BISON04T_BRANCH_7_14
4708tx/4331 clone 4708b14x/4331 -tag BISON04T_BRANCH_7_14
4708b14x/4360 configure -dualband {4708b14x/4331 -c1 36/80}

4708t/4360 clone 4708e/4360 -tag EAGLE_BRANCH_10_10
4708t/4331 clone 4708e/4331 -tag EAGLE_BRANCH_10_10
4708tx/4360 clone 4708ex/4360 -tag EAGLE_BRANCH_10_10
4708tx/4331 clone 4708ex/4331 -tag EAGLE_BRANCH_10_10
4708ex/4360 configure -dualband {4708ex/4331 -c1 36/80}

# Clone for cxo
4708t/4360 clone 4708t14c/4360 \
    -tag AARDVARK01T_{REL,TWIG}_6_37_14{,_*} -trx linux-cxo
4708t/4331 clone 4708t14c/4331 \
    -tag AARDVARK01T_{REL,TWIG}_6_37_14{,_*} -trx linux-cxo

4708t14c/4360 configure -dualband {4708t14c/4331 -c1 36/80}

4708tx/4360 clone 4708t14cx/4360 \
    -tag AARDVARK01T_{REL,TWIG}_6_37_14{,_*} -trx linux-cxo
4708tx/4331 clone 4708t14cx/4331 \
    -tag AARDVARK01T_{REL,TWIG}_6_37_14{,_*} -trx linux-cxo

4708t14cx/4360 configure -dualband {4708t14cx/4331 -c1 36/80}

# WAR for SWWLAN-143231, SWWLAN-143520
4708t/4360 configure -noaes 1 -notkip 1 -nopm1 1

set UTF::RouterNightlyCustom {
    UTF::Try "$Router: Vendor IE" {
	package require UTF::Test::vndr_ie
	UTF::Test::vndr_ie $Router $STA1
    }
    catch {
	package require UTF::Test::MiniUplinks
	UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 \
	    -pstareboot -dodwdsr 1 -dowetr 1
    }
}

#####
UTF::Q mc41

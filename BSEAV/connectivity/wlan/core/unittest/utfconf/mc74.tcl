# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: fb2bd54ecd109ec69c2a1095446eff82e52753e2 $
#

# UTF Bringup test rig
#

set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc74"

package require UTF::Aeroflex

UTF::Power::Synaccess nb -lan_ip 192.168.254.10 -relay wan -rev 1
UTF::Aeroflex 192.168.254.15 -group {G1 {1 2 3} G2 {4 5 6}} -relay wan
G1 configure -default 17
G2 configure -default 14

set ::UTF::SetupTestBed {
    # reset attenuators
    G1 attn default
    G2 attn default
    foreach S {sta1 sta2 sta3} {
	catch {$S wl down}
	$S deinit
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {atlas/0 atlas/1 atlas/2} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    return
}


UTF::Linux mc74end1 -sta {lan eth1}
lan configure -ipaddr 192.168.1.50

UTF::Linux mc74end2 -sta {lan2 eth1}
lan2 configure -ipaddr 192.168.1.30

UTF::Linux mc74end3 -sta {lan3 p1p1}
lan3 configure -ipaddr 192.168.1.40

UTF::Linux mc74end4 -sta {wan p1p1}

UTF::Linux mc74tst3 -sta {sta2 enp1s0} \
    -tag "EAGLE_BRANCH_10_10" \
    -console mc74end1:40002 -power {nb 2} \
    -tcpwindow 4m -reloadoncrash 1 -slowassoc 5 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 3}

sta2 configure -attngrp G2

UTF::Linux mc74tst2 -sta {sta3 enp1s0} \
    -tag "EAGLE_BRANCH_10_10" \
    -console mc74end1:40001 -power {nb 3} \
    -tcpwindow 4m -reloadoncrash 1 -slowassoc 5 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 3}

sta3 configure -attngrp G2

UTF::Linux mc74tst1 -sta {sta1 enp1s0} \
    -tag "EAGLE_BRANCH_10_10" \
    -power {nb 1} \
    -console mc74end1:40008 \
    -tcpwindow 4m -reloadoncrash 1 -slowassoc 5 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl bw_cap 2g -1;wl vht_features 3}

sta1 configure -attngrp G1


#############################
#
# Atlas
#
#############################

# No advantage to using 2 lan endpoints - reduces rx by 100mbps

UTF::Router atlas -sta {dummy dummy} \
    -relay lan2 \
    -lanpeer lan2 \
    -wanpeer wan \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -console {mc74end1:40009} \
    -power {nb 8} \
    -nvram {
	watchdog=2000
	lan_stp=0
	lan1_stp=0
	wl0_ssid=atlas/0
	wl0_chanspec=36
	wl0_radio=0
	wl0_vht_features=2
	wl1_ssid=atlas/1
	wl1_chanspec=11
	wl1_radio=0
	wl1_vht_features=1
	wl2_ssid=atlas/2
	wl2_chanspec=161
	wl2_radio=0
	wl2_vht_features=2
	samba_mode=2
    } \
    -datarate {-i 0.5 -frameburst 1} -udp 1.5g \
    -yart {-frameburst 1 -attn5g 30-93 -attn2g 45-93 -pad 21} \
    -noradio_pwrsave 1 -perfchans {36/80} -nosamba 1

dummy clone atlas/0 -sta {atlas/0 eth1 atlas/0.%15 wl0.%} \
    -perfchans {36/80} -channelsweep {-max 64}
dummy clone atlas/1 -sta {atlas/1 eth2 atlas/1.%15 wl1.%} -perfchans {3}
dummy clone atlas/2 -sta {atlas/2 eth3 atlas/2.%15 wl2.%} \
    -perfchans {161/80} -channelsweep {-min 124}

dummy destroy

set xargs {
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
    -perfonly 1 -datarate 0 -docpu 1 -nosamba 0
}


######

atlas/0 clone atlasb14/0 -tag BISON04T_BRANCH_7_14
atlas/1 clone atlasb14/1 -tag BISON04T_BRANCH_7_14
atlas/2 clone atlasb14/2 -tag BISON04T_BRANCH_7_14

atlasb14/0 clone atlasb14x/0 {*}$xargs
atlasb14/1 clone atlasb14x/1 {*}$xargs
atlasb14/2 clone atlasb14x/2 {*}$xargs

atlasb14x/0 configure \
    -dualband {atlasb14x/2 -c1 36/80 -c2 161/80 -lan1 lan -lan2 lan3}

#####

atlas/0 clone atlasb43/0 -tag BISON04T_{TWIG,REL}_7_14_43{,_*}
atlas/1 clone atlasb43/1 -tag BISON04T_{TWIG,REL}_7_14_43{,_*}
atlas/2 clone atlasb43/2 -tag BISON04T_{TWIG,REL}_7_14_43{,_*}

atlasb43/0 clone atlasb43x/0 {*}$xargs
atlasb43/1 clone atlasb43x/1 {*}$xargs
atlasb43/2 clone atlasb43x/2 {*}$xargs

atlasb43x/0 configure \
    -dualband {atlasb43x/2 -c1 36/80 -c2 161/80 -lan1 lan -lan2 lan3}

#####

atlas/0 clone atlasb89/0 -tag BISON04T_{TWIG,REL}_7_14_89{,_*}
atlas/1 clone atlasb89/1 -tag BISON04T_{TWIG,REL}_7_14_89{,_*}
atlas/2 clone atlasb89/2 -tag BISON04T_{TWIG,REL}_7_14_89{,_*}

atlasb89/0 clone atlasb89x/0 {*}$xargs
atlasb89/1 clone atlasb89x/1 {*}$xargs
atlasb89/2 clone atlasb89x/2 {*}$xargs

atlasb89x/0 configure \
    -dualband {atlasb89x/2 -c1 36/80 -c2 161/80 -lan1 lan -lan2 lan3}

#####

atlas/0 clone atlasb124/0 -tag BISON04T_TWIG_7_14_124
atlas/1 clone atlasb124/1 -tag BISON04T_TWIG_7_14_124
atlas/2 clone atlasb124/2 -tag BISON04T_TWIG_7_14_124

atlasb124/0 clone atlasb124x/0 {*}$xargs
atlasb124/1 clone atlasb124x/1 {*}$xargs
atlasb124/2 clone atlasb124x/2 {*}$xargs

atlasb124x/0 configure \
    -dualband {atlasb124x/2 -c1 36/80 -c2 161/80 -lan1 lan -lan2 lan3}


#####

atlas/0 clone atlasb131/0 -tag BISON04T_TWIG_7_14_131
atlas/1 clone atlasb131/1 -tag BISON04T_TWIG_7_14_131
atlas/2 clone atlasb131/2 -tag BISON04T_TWIG_7_14_131

atlasb131/0 clone atlasb131x/0 {*}$xargs
atlasb131/1 clone atlasb131x/1 {*}$xargs
atlasb131/2 clone atlasb131x/2 {*}$xargs

atlasb131x/0 configure \
    -dualband {atlasb131x/2 -c1 36/80 -c2 161/80 -lan1 lan -lan2 lan3}

#####

atlas/0 clone atlasb164/0 -tag BISON04T_TWIG_7_14_164
atlas/1 clone atlasb164/1 -tag BISON04T_TWIG_7_14_164
atlas/2 clone atlasb164/2 -tag BISON04T_TWIG_7_14_164

atlasb164/0 clone atlasb164x/0 {*}$xargs
atlasb164/1 clone atlasb164x/1 {*}$xargs
atlasb164/2 clone atlasb164x/2 {*}$xargs

atlasb164x/0 configure \
    -dualband {atlasb164x/2 -c1 36/80 -c2 161/80 -lan1 lan2 -lan2 lan3}

#set UTF::PerfCacheMigrate {
#    atlasb14 atlasb164
#}



set UTF::RouterNightlyCustom {

    if {[regexp {(.*x)/0} $Router - base]} {
	# external
	if {$STA3 ne ""} {

	    package require UTF::Test::TripleBand

	    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
		-c1 44/80 -c2 3l -c3 157/80 -lan1 lan -lan2 lan2 -lan3 lan3

	}
    } else {
	# Internal
	UTF::Try "$Router: Vendor IE" {
	    package require UTF::Test::vndr_ie
	    UTF::Test::vndr_ie $Router $STA1
	}
	catch {
	    package require UTF::Test::MiniUplinks
	    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 \
		-otherlans {lan lan3}
	}
    }

}

UTF::Q mc74

# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 3a6e93e5b0d62a84fe0969310bee525e8759af07 $
#

######
#
# Second UTF Vertical MIMO Coffin in 2nd floor UTF Lab
#

source utfconf/mc18.tcl

set ::UTF::SummaryDir "/projects/hnd_software/utf/mc18b"

set ::UTF::SetupTestBed {
    B attn default
    return
}

B configure -default 10

set stacmn {
    -tag EAGLE_BRANCH_10_10
    -tcpwindow 4M
    -slowassoc 5 -reloadoncrash 1
    -udp 1.8g
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 7
    }
}

UTF::Linux mc18tst11 -sta {sta1 enp1s0} \
    -console "sr1end25:40005" -power {mc18ips1 3} \
    {*}$stacmn

UTF::Linux mc18tst12 -sta {sta2 enp1s0} \
    -console "sr1end25:40004" -power {mc18ips6 7} \
    {*}$stacmn

UTF::Linux mc18tst13 -sta {sta3 enp1s0} \
    -console "sr1end25:40006" -power {mc18ips6 1} \
    {*}$stacmn

UTF::Linux SR1End13 -sta {lan1 eth1}
# sr1end14 is dead
#UTF::Linux SR1End14 -sta {wan1 eth1}

lan1 configure -ipaddr 192.168.1.50

UTF::Linux mc18end2 -sta {lan2 p1p1}
lan2 configure -ipaddr 192.168.1.49

UTF::Linux mc18tst10 -sta {lan3 p1p1}
lan3 configure -ipaddr 192.168.1.48

#############################
#
# AtlasII
#
#############################

UTF::Router atlasII -sta {dummy dummy} \
    -tag BISON04T_BRANCH_7_14 \
    -relay lan3 \
    -lanpeer {lan3 lan2} \
    -wanpeer wan1 \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
    -embeddedimage {4366c} \
    -console "mc18tst10:40000" \
    -power {mc18ips1 1} \
    -nvram {
	# watchdog=2000
	lan_stp=0
	lan1_stp=0
	wl0_ssid=atlasII/0
	wl0_chanspec=11
	wl0_radio=0
	wl0_vht_features=5
	wl1_ssid=atlasII/1
	wl1_chanspec=36
	wl1_radio=0
	wl1_vht_features=6
	wl2_ssid=atlasII/2
	wl2_chanspec=161
	wl2_radio=0
	wl2_vht_features=6
	samba_mode=2
	wl0_dyn160=1
	wl1_dyn160=1
	wl2_dyn160=1
    } \
    -model bcm94709acdcrh_p415_nvram \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g 0-93 -attn2g 18-93 -pad 24} \
    -noradio_pwrsave 1 -perfchans {36/80} -nosamba 1

dummy configure -attngrp B

dummy clone atlasII/0 -sta {atlasII/0 eth1 atlasII/0.%15 wl0.%} \
    -perfchans {3l}
dummy clone atlasII/1 -sta {atlasII/1 eth2 atlasII/1.%15 wl1.%} \
    -perfchans {36/80} -channelsweep {-max 64}
dummy clone atlasII/2 -sta {atlasII/2 eth3 atlasII/2.%15 wl2.%} \
    -perfchans {161/80} -channelsweep {-min 100}

dummy destroy

######

set xargs {
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
    -perfonly 1 -datarate 0 -docpu 1 -nosamba 0
}
lappend xargs -nvram "[atlasII/2 cget -nvram]
    wl1_country_code=Q1
    wl1_country_rev=137
    wl2_country_code=Q1
    wl2_country_rev=137
"

atlasII/0 clone atlasIIx/0 {*}$xargs
atlasII/1 clone atlasIIx/1 {*}$xargs -perfchans {36/80 36/160}
atlasII/2 clone atlasIIx/2 {*}$xargs -perfchans {161/80 128/160}

atlasIIx/2 configure \
    -dualband {atlasIIx/1 -c2 36/80 -c1 161/80 -lan1 lan2 -lan2 lan3}

#####

atlasII/0 clone atlasIIb124/0 -tag BISON04T_TWIG_7_14_124
atlasII/1 clone atlasIIb124/1 -tag BISON04T_TWIG_7_14_124
atlasII/2 clone atlasIIb124/2 -tag BISON04T_TWIG_7_14_124

atlasIIb124/0 clone atlasIIb124x/0 {*}$xargs
atlasIIb124/1 clone atlasIIb124x/1 {*}$xargs
atlasIIb124/2 clone atlasIIb124x/2 {*}$xargs

atlasIIb124x/0 configure \
    -dualband {atlasIIb124x/2 -c1 36/80 -c2 161/80 -lan1 lan2 -lan2 lan3}

#####

atlasII/0 clone atlasIIb131/0 -tag BISON04T_TWIG_7_14_131
atlasII/1 clone atlasIIb131/1 -tag BISON04T_TWIG_7_14_131
atlasII/2 clone atlasIIb131/2 -tag BISON04T_TWIG_7_14_131

atlasIIb131/0 clone atlasIIb131x/0 {*}$xargs
atlasIIb131/1 clone atlasIIb131x/1 {*}$xargs
atlasIIb131/2 clone atlasIIb131x/2 {*}$xargs

atlasIIb131x/0 configure \
    -dualband {atlasIIb131x/2 -c1 36/80 -c2 161/80 -lan1 lan2 -lan2 lan3}

#####

atlasII/0 clone atlasIIg2/0 -tag BISON04T_TWIG_7_14_131_16
atlasII/1 clone atlasIIg2/1 -tag BISON04T_TWIG_7_14_131_16
atlasII/2 clone atlasIIg2/2 -tag BISON04T_TWIG_7_14_131_16

atlasIIg2/0 clone atlasIIg2x/0 {*}$xargs
atlasIIg2/1 clone atlasIIg2x/1 {*}$xargs
atlasIIg2/2 clone atlasIIg2x/2 {*}$xargs

atlasIIg2x/0 configure \
    -dualband {atlasIIg2x/2 -c1 36/80 -c2 161/80 -lan1 lan2 -lan2 lan3}

#####

atlasII/0 clone atlasIIb164/0 -tag BISON04T_TWIG_7_14_164
atlasII/1 clone atlasIIb164/1 -tag BISON04T_TWIG_7_14_164
atlasII/2 clone atlasIIb164/2 -tag BISON04T_TWIG_7_14_164

atlasIIb164/0 clone atlasIIb164x/0 {*}$xargs
atlasIIb164/1 clone atlasIIb164x/1 {*}$xargs
atlasIIb164/2 clone atlasIIb164x/2 {*}$xargs

atlasIIb164x/0 configure \
    -dualband {atlasIIb164x/2 -c1 36/80 -c2 161/80 -lan1 lan2 -lan2 lan3}

#set UTF::PerfCacheMigrate {
#    atlasII atlasIIb164
#}

####

set UTF::RouterNightlyCustom {

    if {[regexp {(.*x)/} $Router - base]} {
	# external
	if {$STA3 ne ""} {

	    package require UTF::Test::TripleBand

	    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
		-c1 3l -c2 44/80 -c3 157/80 -lan1 lan1 -lan2 lan2 -lan3 lan3

	}
    } else {
	# Internal

#	UTF::Try "$Router: Vendor IE" {
#	    package require UTF::Test::vndr_ie
#	    UTF::Test::vndr_ie $Router $STA1
#	}
#   }

#    package require UTF::Test::MiniUplinks
#    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -otherlans {lan1}

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2 -otherlans {lan1}

}


#####
UTF::Q mc18b lan2


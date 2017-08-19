#
#  $Copyright Broadcom Corporation$
#  $Id: 1704f95316b4c0cb7d3d970a01290dd181d83d1f $
#
#  Testbed configuration file for MD05r Testbed
#

set UTF::Use11h 1; # Enable 11h for non-radar channels
set UTF::UseCSA 1; # Enable CSA for ChannelSweep

source "utfconf/md05.tcl"
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/md05r"

G3 configure -default 10

set UTF::SetupTestBed {
    G3 attn default
    return
}

UTF::Power::Synaccess NP15 -lan_ip 172.16.1.15 -relay md05end1 -rev 1
UTF::Power::Synaccess NP35 -lan_ip 172.16.1.35 -relay md05end1 -rev 1

lan configure -ipaddr 192.168.1.100

UTF::Linux md05end2 -sta "wan p16p1" -lan_ip 10.19.61.78
wan configure -ipaddr 192.168.254.244

set stacommon {
    -slowassoc 5
    -reloadoncrash 1
    -tcpwindow 3m
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl bw_cap 2g -1;
	wl country '#a/0';
	wl obss_coex 0;
	wl vht_features 3;
    }
}

UTF::Linux md05tst3 -sta {sta1 enp1s0} \
    -console {md05end1:40001} \
    -power {NP35 1} \
    {*}$stacommon

UTF::Linux md05tst2 -sta {sta2 enp1s0} \
    -console {md05end1:40002} \
    {*}$stacommon

###############

UTF::Router _53574 -name 53574 -sta {
    53574 eth1 53574.%15 wl0.%
} \
    -lanpeer lan \
    -lan_ip 192.168.1.1 \
    -power {NP15 1} \
    -console "md05end1:40000" \
    -wanpeer wan -web 2 \
    -relay md05end1 \
    -brand "linux-2.6.36-arm-up-internal-router-rsdb" \
    -model bcm953574acr_p236 \
    -nvram {
	watchdog=2000
	wl0_ssid=53574
	wl0_chanspec=3
	wl1_ssid=53574a
	wl1_chanspec=36

	# Features
	wl1_vht_features=2
	wl0_vht_features=3

	wl0_txbf_imp=0
	#wl0_txbf_bfe_cap=0
	wl0_txbf_bfr_cap=0

	# leave both radios turned on for RSDB
	wl0_radio=1
	wl1_radio=1
	rsdb_mode=0

	wl_msglevel=0x80901
	wl0_country_code=#a
	wl0_country_rev=0
	wl1_country_code=#a
	wl1_country_rev=0

    } -docpu 1 \
    -datarate {-i 0.5} -udp 800m \
    -noradio_pwrsave 1 -perfchans {36/80 3} \
    -yart {-attn5g {25-83} -attn2g {38-83} -pad 34}

53574 configure -attngrp G3

53574 clone 53574x -docpu 1 \
    -brand linux-2.6.36-arm-up-external-vista-router-full-src

# Skip CPU test, since it may pick the wrong band.
53574 clone 53574g -docpu 0 \
    -sta {53574g eth1 53574g.%7 wl0.%}  \
    -nvram [regsub {rsdb_mode=0} [53574 cget -nvram] {rsdb_mode=1}] \
    -perfchans {3} -channelsweep {-band g}

53574 clone 53574a -docpu 1 \
    -sta {53574a eth2 53574a.%7 wl1.%} \
    -nvram [regsub {rsdb_mode=0} [53574 cget -nvram] {rsdb_mode=1}] \
    -perfchans {36/80} -channelsweep {-band a}

53574a configure -dualband {53574g -c1 36/80}

# WAR for SWWLAN-143231
53574 configure -noaes 1 -notkip 1
53574a configure -noaes 1 -notkip 1
53574g configure -noaes 1 -notkip 1

53574 clone 53574d178 -tag DINGO_TWIG_9_10_178
53574a clone 53574d178a -tag DINGO_TWIG_9_10_178
53574g clone 53574d178g -tag DINGO_TWIG_9_10_178



#set UTF::PerfCacheMigrate {
#    53574b0 53574d178
#    53574b0 53574
#}


set UTF::RouterNightlyCustom {

#    package require UTF::Test::MiniUplinks
#    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 -dodwdsr 1 -dowetr 1

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2

}

UTF::Q md05r md05end1


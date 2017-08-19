#######
#
# lab14hnd-mirabadi Config file for Coex chamber.
#
#######

#
# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/so-coex-1"

# UTF gateway machine.  Must log into this one in order to access the
# other systems.
UTF::Linux lab14hnd-mirabadi -sta {lan eth1} -onall false
lan configure -ipaddr 192.168.1.220

set ::UTF::_llogfile ""

package require UTF::Power

UTF::Power::Synaccess NP22-S50 -relay lab14hnd-mirabadi
UTF::Power::Synaccess NP22-S60 -relay lab14hnd-mirabadi
UTF::Power::Synaccess NP22-S70 -relay lab14hnd-mirabadi -rev 1
UTF::Power::Synaccess NP22-40 -relay lab14hnd-mirabadi -rev 1

package require UTF::Aeroflex


if {[info exists ::env(HOSTNAME)] &&
    $::env(HOSTNAME) eq "STA7.sj.broadcom.com"} {
    # Testing NFS-less access
    set UTF::UseFCP nocheck
    set UTF::BuildFileServer lan
}

#######################
# Aeroflex
UTF::Aeroflex af -lan_ip 172.16.1.200 \
    -relay lab14hnd-mirabadi \
    -group {G1 {1 2 3 4}}

set ::UTF::SetupTestBed {

    G1 attn default

#    foreach s {4360-1 4360-2} {
#	catch {$s iptables -F}
#	catch {$s wl down}
#   }

#    catch {lan iptables -F}

#    foreach s {4360-1 4360-2 lan} {
#	catch {$s unload}
#	catch {$s deinit}
#   }

#    unset s
#    unset ::UTF::SetupTestBed
#    return
}

#######################
# STAs

# -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl dump ampdu}} \
# -pre_perf_hook {{%S wl ampdu_clear_dump}} \

# DUT
# Currently the NPC is outside the enclosures
# Need to re-arrrange and add once PC per enclosure
# NPC plug 1 connected to enclusure holding STA1 and STA4
UTF::Cygwin STA1 -sta {43225Win7} \
    -lan_ip 172.16.1.111 -osver 7 \
    -power {NP22-S60 2} \
    -allowdevconreboot 1
43225Win7 configure -ipaddr 192.168.1.111

43225Win7 clone 43225Win7b -tag BISON05T_BRANCH_7_35
43225Win7b configure -ipaddr 192.168.1.111

# DUT
# Currently the NPC is outside the enclosures
UTF::Cygwin STA2 -sta {4313Win7combo} \
   -lan_ip 172.16.1.112 -osver 7 \
   -power {NP22-S50 1} \
   -allowdevconreboot 1
4313Win7combo configure -ipaddr 192.168.1.112

4313Win7combo clone 4313Win7combob -tag BISON05T_BRANCH_7_35
4313Win7combob configure -ipaddr 192.168.1.112

# DUT
UTF::Cygwin STA3 -user user -sta {43228Win7} \
    -lan_ip 172.16.1.113 -osver 7 \
    -power {NP22-S60 1} \
    -perfchans {3 3l}

43228Win7 configure -ipaddr 192.168.1.113

43228Win7 clone 43228Win7b -tag BISON05T_BRANCH_7_35
43228Win7b configure -ipaddr 192.168.1.113

# DUT
UTF::Cygwin STA4 -user user -sta {4352Win7} \
   -lan_ip 172.16.1.114 -osver 7 \
   -power {NP22-S50 2} \
   -allowdevconreboot 1
4352Win7 configure -ipaddr 192.168.1.114

4352Win7 clone 4352Win7b -tag BISON05T_BRANCH_7_35
4352Win7b configure -ipaddr 192.168.1.114

#################

UTF::Linux STA6 -sta {sta2 enp11s0} \
    -lan_ip 172.16.1.116 \
    -console "lab14hnd-mirabadi:40007" \
    -power "NP22-S70 1" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl vht_features 3} \
    -tcpwindow 3m

UTF::Linux STA7 -sta {sta1 enp11s0} \
    -lan_ip 172.16.1.117 \
    -console "lab14hnd-mirabadi:40008" \
    -power "NP22-S70 2" \
    -wlinitcmds {wl msglevel +assoc;wl down;wl vht_features 3} \
    -tcpwindow 3m

##########################
# AP

#set ::UTF::RcRestartHangWAR 1

UTF::Router 4709 -sta {dummy dummy} \
    -relay lab14hnd-mirabadi \
    -lan_ip 192.168.1.1 \
    -lanpeer lan \
    -power {NP22-40 1} \
    -console "lab14hnd-mirabadi:40041" \
    -brand linux-2.6.36-arm-internal-router \
    -nvram {
	watchdog=2000
	wl0_chanspec=3
	wl0_ssid=43570_1
	wl0_vht_features=3
	wl0_radio=0
	wl1_chanspec=36
	wl1_ssid=43570_2
	wl1_vht_features=3
	wl1_radio=0
    } \
    -coldboot 0 -udp 800m \
    -datarate {-i 0.5} \
    -yart {-attn5g {8-83} -attn2g {30-83}} \
    -noradio_pwrsave 1

dummy configure -attngrp G1

dummy clone 4709/43570a -sta {4709/43570a eth1 4709/43570a.%15 wl0.%} \
    -perfchans {36/80} -channelsweep {-band a}
dummy clone 4709/43570g -sta {4709/43570g eth2 4709/43570g.%15 wl1.%} \
    -perfchans {3} -channelsweep {-band b}

dummy destroy

set xargs {
    -brand linux-2.6.36-arm-external-vista-router-full-src
    -perfonly 1 -datarate 0
}

#####

4709/43570a clone 4709b35/43570a -tag BIS143RC41_{BRANCH,REL}_7_35{,_?}
4709/43570g clone 4709b35/43570g -tag BIS143RC41_{BRANCH,REL}_7_35{,_?}

4709b35/43570a clone 4709b35x/43570a {*}$xargs
4709b35/43570g clone 4709b35x/43570g {*}$xargs

####

set UTF::RouterNightlyCustom {

    catch {
	package require UTF::Test::MiniUplinks
	UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
    }

}

UTF::Q so-coex-1


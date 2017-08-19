# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 8f0ae18e8abb2662d9202823e4a2e853585f08d3 $
#
# Testbed configuration file for mc15a testbed
#

# Common config
source utfconf/mc15.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc15c"

set ::UTF::SetupTestBed {
    C attn default

    foreach S {4360 X51 X21b} {
	catch {$S wl down}
	$S deinit
    }
    return
}

# STAs

UTF::Linux mc15end2 -sta {4360 enp1s0} \
    -power {mc15ips1 7} \
    -console {sr1end20:40002} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl bw_cap 2g -1;wl vht_features 3} \
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1

4360 configure -attngrp C -ipaddr 192.168.1.60 -date 2017.1.4.0

#######################################

UTF::Power::WebRelay wr -lan_ip 192.168.1.14 -relay sr1end11
UTF::Power::Laptop mc41tst4b -button {wr 1}

UTF::MacOS mc40tst6 -sta {X51 en0} -power {mc41tst4b} \
    -brand macos-internal-wl-gala -type Debug_10_11 \
    -wlinitcmds {wl msglevel +assoc;wl assert_type 0; wl down;wl vht_features 3;wl up; wl country '#a/0'} \
    -coreserver AppleCore -kextunload 2 -tcpwindow 3640K \
    -channelsweep {-no2g40 -skip {12 13 14}} \
    -udp 1.2g -perfchans {36/80 3} -slowassoc 5 \
    -yart {-pad 23 -attn5g 25-95 -attn2g 38-95} \
    -post_assoc_hook {%S innetworkwait $AP 23} \
    -nativetools 1 -datarate {-i 0.5}

X51 configure -ipaddr 192.168.1.94

#set UTF::PerfCacheMigrate {BIS715GALA BIS715LOBO}

X51 clone X51v -tag AARDVARK_BRANCH_6_30
X51 clone X51v223 -tag AARDVARK_{TWIG,REL}_6_30_223{,_*}
X51 clone X51b -tag BISON_BRANCH_7_10
X51 clone X51o -tag BIS715LOBO_BRANCH_7_77 \
    -embeddedimage 4352 -nobighammer true

X51 clone X51.1 \
    -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nocal 1 \
    -datarate 0 -yart {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 0; wl vht_features 3; wl dtim 3}

X51.1 clone X51v.1 -tag AARDVARK_BRANCH_6_30
X51.1 clone X51b.1 -tag BISON_BRANCH_7_10

#####

UTF::Power::WebRelay wr2 -lan_ip 192.168.1.3 -relay sr1end11
UTF::Power::Laptop mc15tst5b -button {wr2 4}

# skip mcs32 because MacOS + 11d AP forces autochannel and switches to
# US/0.  US/0 on this HW appears to block mcs32.

UTF::MacOS mc15tst5 -sta {X21b en0} -power {mc15tst5b} \
    -brand macos-internal-wl-gala -type Debug_10_11 \
    -wlinitcmds {wl msglevel +assoc;wl assert_type 0;wl up;wl country '#a/0'} \
    -coreserver AppleCore -kextunload 1 \
    -channelsweep {-no2g40 -skip {12 13 14}} \
    -datarate {-frameburst 1 -skiptx 32} -udp 300m -slowassoc 5 \
    -yart {-pad 23 -attn5g 30-95 -attn2g 40-95 -frameburst 1} \
    -nativetools 1 \
    -post_assoc_hook {%S innetworkwait $AP 20}

X21b configure -ipaddr 192.168.1.95

X21b clone X21bb -tag BISON_BRANCH_7_10
X21b clone X21bo -tag BIS715LOBO_BRANCH_7_77 \
    -kextunload 2 -nobighammer true

####

UTF::Q mc15c


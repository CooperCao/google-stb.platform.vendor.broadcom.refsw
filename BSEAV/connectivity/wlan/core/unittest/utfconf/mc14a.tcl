# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 11c51141f2f8a7677ab307747da84efb5f8c932b $
#
# Testbed configuration file for mc14a testbed
#

source "utfconf/mc14.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc14a"

set UTF::SetupTestBed {
    A attn default
    foreach s {4352 43228xp} {
	catch {$s wl down}
	$s deinit
    }
    return
}

######################################

UTF::Linux mc14tst1 -sta {4352 enp1s0} \
    -console mc14end2:40005 \
    -power {mc14ips1 1} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country US/0;wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -reloadoncrash 1 -tcpwindow 3m -udp 800m

4352 configure -attngrp A -ipaddr 192.168.1.80 -hasdhcpd 1

######################################

UTF::Power::CanaKit ck2 \
    -lan_ip mc14end2:40004 \
    -invert {1 3}

UTF::Cygwin mc14tst2 -sta {43228xp} \
    -osver 5 -udp 300m -node {DEV_4359} \
    -power_sta {ck2 1} -app_tag trunk \
    -power {ck2 2} \
    -yart {-pad 43 -attn5g 15-63 -attn2g 22-63 -frameburst 1}

43228xp clone 43228xpb -tag BISON_BRANCH_7_10
43228xp clone 43228xpb35 -tag BISON05T_BRANCH_7_35
43228xp clone 43228xpg -tag BIS715GALA_BRANCH_7_21 \
    -brand win_mfgtest_wl -app_tag BIS715GALA_BRANCH_7_21 -type Internal

43228xp clone 4352xp -node {DEV_43B1}
4352xp clone 4352xpb35 -tag BISON05T_BRANCH_7_35

# 43236 USB BMac Dongle on XP
UTF::WinDHD mc14tst2d -lan_ip mc14tst2 -sta {43236xp} \
    -node {PID_BD17} -embeddedimage 43236 \
    -console "mc14end2:40003" \
    -brand win_mfgtest_wl \
    -type Bcm/Bcm_DriverOnly_BMac \
    -app_tag "trunk" \
    -power_sta {ck2 3} \
    -power {ck2 2} \
    -nobighammer 0 -tcpwindow 256k \
    -yart {-pad 43 -attn5g 15-63 -attn2g 22-63 -frameburst 1}

43236xp clone 43236xpb -tag "BISON_BRANCH_7_10"
43236xp clone 43236xpb14 -tag "BISON04T_BRANCH_7_14"

####
UTF::Q mc14a

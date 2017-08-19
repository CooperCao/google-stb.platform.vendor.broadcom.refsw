# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 6acbea5e0828b117d73e2d7f374ea662269d89f8 $
#
# Testbed configuration file for mc14b testbed
#

source "utfconf/mc14.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc14c"

set UTF::SetupTestBed {
    C attn default
    foreach s {4352 43228w7 43526w7} {
	catch {$s wl down}
	$s deinit
    }
    return
}

######################################

UTF::Linux mc14tst5 -sta {4352 enp1s0} \
    -console mc14end2:40001 \
    -power {mc14ips1 4} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -reloadoncrash 1 -tcpwindow 3m -udp 800m

4352 configure -attngrp C -ipaddr 192.168.1.80 -hasdhcpd 1 -date 2017.1.4.0

#######################################

UTF::Power::CanaKit ck3 \
    -lan_ip mc14end2:40007 -invert {1 3}

# Win7 4322MC_HP(DE) in WWAN slot
UTF::Cygwin mc14tst6 -sta {43228w7} -osver 7 \
    -node {DEV_4359} \
    -power_sta {ck3 1} \
    -power {ck3 2} -sign 1 \
    -allowdevconreboot 1 -udp 300m \
    -datarate {-i 0.5} -reg {Country ALL} \
    -yart {-pad 43 -attn5g 7-63 -attn2g 27-63} \
    -nosharedwep 1 -nopm1 1 -installer InstallDriver

43228w7 clone 43228w7b   -tag BISON_BRANCH_7_10
43228w7 clone 43228w7b35 -tag BISON05T_BRANCH_7_35 \
    -usecsa 0 -use11h 0 -wlinitcmds {wl down;wl spect 0}

# 43526 USB BMac Dongle on Win7 (same host as above)
UTF::WinDHD mc14tst6b -lan_ip mc14tst6 -sta {43526w7} \
    -node {PID_BD1D} -embeddedimage 43526b \
    -console "mc14end2:40006" \
    -power_sta {ck3 3} \
    -power {ck3 2} -sign 1  \
    -user user -osver 7 -udp 300m \
    -app_tag "trunk" \
    -app_brand "win_internal_wl" \
    -brand win_mfgtest_wl \
    -type Bcm/Bcm_DriverOnly_BMac \
    -reg {Country ALL} -nobighammer 1 -nocal 1 \
    -wlinitcmds {wl down; wl vht_features 3} \
    -datarate {-i 0.5} -perfchans {36/80 3} \
    -yart {-pad 43 -attn5g 7-63 -attn2g 22-63} \
    -installer InstallDriver

43526w7 clone 43526w7b   -tag BISON_BRANCH_7_10
43526w7 clone 43526w7b35 -tag BISON05T_BRANCH_7_35

####
UTF::Q mc14c

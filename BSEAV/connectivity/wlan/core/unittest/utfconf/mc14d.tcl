# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 47ebc0cb9e418b61e3e29964b93fd042f3a34339 $
#
# Testbed configuration file for mc14b testbed
#

source "utfconf/mc14.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc14d"

#B configure -default 0

set UTF::SetupTestBed {
    #B attn default
    foreach s {4352 4352w10} {
	catch {$s wl down}
	$s deinit
    }
    return
}

######################################

UTF::Linux mc14tst7 -sta {4352 enp1s0} \
    -console mc14end2:40002 \
    -power {mc14ips1 6} \
    -tag BISON05T_BRANCH_7_35 \
    -tag trunk \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -reloadoncrash 1 -tcpwindow 3m -udp 800m \
    -yart {-pad 40 -attn5g 10-63+3 -attn2g 25-63+3}

4352 configure -ipaddr 192.168.1.80 -hasdhcpd 1 -attngrp D -date 2017.1.4.0

#######################################
set UTF::TestSigning true

# BindIperf being used as a WAR for SWWLAN-125912
set UTF::BindIperf true

UTF::Cygwin mc14tst3 -sta {4352w10} -osver 1064 \
    -power {mc14ips1 2} -kdpath kd.exe \
    -reg {Country ALL} -sign 1 -installer InstallDriver \
    -wlinitcmds {wl down;wl vht_features 3} \
    -brand win10_internal_wl -usemodifyos 1 -udp 800m -tcpwindow 3m \
    -allowdevconreboot 1 -perfchans {36/80 3} \
    -yart {-pad 43 -attn5g 15-63 -attn2g 22-63}


4352w10 clone 4352w10b -tag BISON_BRANCH_7_10 -embeddedimage 4352 -nokpps 1
4352w10 clone 4352w10b35 -tag BISON05T_BRANCH_7_35 \
    -brand win10_internal_wl \
    -embeddedimage 4352 -nokpps 1 -usecsa 0 -use11h 0 -noibss 1


####
UTF::Q mc14d

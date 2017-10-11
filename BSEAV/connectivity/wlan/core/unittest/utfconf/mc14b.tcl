# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for mc14b testbed
#

source "utfconf/mc14.tcl"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc14b"

B configure -default 0

set UTF::SetupTestBed {
    B attn default
    return
}

######################################

UTF::Linux mc14tst4 -sta {4352 enp1s0} \
    -console mc14end2:40000 \
    -power {mc14ips1 3} \
    -tag BISON05T_BRANCH_7_35 \
    -tag trunk \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -reloadoncrash 1 -tcpwindow 3m -udp 800m

4352 configure -attngrp B -ipaddr 192.168.1.80 -hasdhcpd 1 -date 2017.1.4.0

#######################################
set UTF::TestSigning true

# BindIperf being used as a WAR for SWWLAN-125912
set UTF::BindIperf true

UTF::WinDHD mc14tst8 -sta {4356w10} \
    -app_tag trunk \
    -osver 1064 -installer InstallDriver \
    -power {mc14ips1 5} -kdpath kd.exe -sign 1 \
    -brand win10_dhdpcie_internal_wl \
    -type checked/DriverOnly/x64 \
    -usemodifyos 1 -tcpwindow 3m -noibss 1 \
    -wlinitcmds {wl down;wl vht_features 3;wl bw_cap 2 -1} \
    -allowdevconreboot 1 -perfchans {36/80 3} -nocal 1 \
    -yart {-pad 40 -attn5g 10-63 -attn2g 25-63} -udp 800m

# logman -ets stop LwtNetLog
# netsh trace start wireless_dbg persistent=yes provider={21ba7b61-05f8-41f1-9048-c09493dcfe38} level=0xff keywords=0xff
# netsh trace stop

# clone with cut-down channelsweep for debug
4356w10 clone 4356w10cs -channelsweep {-band a -bw 20 -max 40} -perfchans 36/80

4356w10 clone 4356w10gold -tag DHD_REL_1_435 -app_tag DHD_REL_1_435

4356w10 clone 4356w8x -brand win8x_dhdpcie_internal_wl

UTF::Cygwin pb2atst5n -lan_ip 10.19.84.97 -sta {4356nic} \
    -osver 1064 -embeddedimage 4354 \
    -power {mc14ips1 5} -kdpath kd.exe -sign 1 \
    -tag BISON05T_BRANCH_7_35 \
    -brand win8x_internal_wl -nomaxmem 1 \
    -usemodifyos 1 -udp 800m -tcpwindow 3m \
    -wlinitcmds {wl down;wl ol_disable 1;wl vht_features 3} \
    -yart {-pad 40 -attn5g 10-63 -attn2g 25-63} \
    -allowdevconreboot 1 -perfchans {36/80 3}

#snit::method UTF::WinDHD wl {args} {
#    $self dhd dump
#    $base wl {*}$args
#}

####
UTF::Q mc14b

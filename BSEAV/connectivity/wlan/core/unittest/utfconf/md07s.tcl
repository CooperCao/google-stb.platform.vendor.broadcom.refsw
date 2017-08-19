# -*-tcl-*-
#
#  $Copyright Broadcom Corporation$
#  $Id: 78692e3d429abcfe060b9d5478469b0c1e1a1629 $
#
#  Testbed configuration file for MD07s Testbed

source "utfconf/md07.tcl"

set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/md07s"

set UTF::SetupTestBed {
    G2 attn default
    return
}

UTF::Power::Synaccess NPC17 -lan_ip 172.16.1.17 -relay md07end3 -rev 1

#set UTF::PerfCacheMigrate {
#    4360-4 4360AP
#    4360-3 4360
#
#    4360-4B 4360eAP
#    4360-3B 4360e
#}

UTF::Linux md07tst3 -sta {4360 enp1s0} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl bw_cap 2g -1;wl vht_features 3} \
    -tcpwindow 4M \
    -console {md07end3:40003} \
    -power {NPC17 1} \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -yart {-attn5g {15-63} -attn2g {15-63}}
4360 configure -ipaddr 192.168.1.131 -attngrp G2

4360 clone 4360e -tag "EAGLE_BRANCH_10_10"

UTF::Linux md07tst4 -sta {4360AP enp1s0} \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl bw_cap 2g -1;wl vht_features 3;wl dtim 3} \
    -tcpwindow 4M \
    -console {md07end3:40004} \
    -power {NPC17 2} \
    -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5} -udp 1.2g \
    -yart {-attn5g {15-63} -attn2g {15-63}}
4360AP configure -ipaddr 192.168.1.141 -attngrp G2

4360AP clone 4360eAP -tag "EAGLE_BRANCH_10_10"

UTF::Q md07s md07end3

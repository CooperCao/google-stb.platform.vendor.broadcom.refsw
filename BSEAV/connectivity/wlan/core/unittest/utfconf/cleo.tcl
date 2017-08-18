# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 0485c2e4bd249ab8844b3a457fb0705e3b9bd782 $
#
# Testbed configuration file for DVT "cleo" test rig
#

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/cleo"

# Controller
UTF::Linux cleo

package require UTF::ChanAssy
UTF::ChanAssy ca -relay cleo -group {G {0 1 2 3}}

# Specify the "default" attenuation (default default is zero)
G configure -default 10

# Reset anything that needs to be reset when starting a new nightly test.
set UTF::SetupTestBed {
    G attn default
}


# "enp1s0" may need to be adjusted depending on your HW.
# EAGLE_{TWIG,REL}_10_10_2{,_*}

UTF::Linux cleoref -sta {REF eth1} \
    -tag EAGLE_BRANCH_10_10 \
    -wlinitcmds {wl country "#a/0"; wl dtim 3;wl msglevel +assoc;wl down; wl vht_features 7} \
    -modopts {assert_type=1 nompc=1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl reset_cnts}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl counters}}



#    -wl /usr/bin/wl

REF configure -ipaddr 192.168.1.102


# To enable simple RvR inside StaNightly add something like:
# -yart {-attn5g {3-95} -attn2g {3-95} -pad 20 -frameburst 1}
#
# -yart: YART.test (Yet Another Range Test)
# -attn5g: range of attenuator values for 5g testing (will quit on
#          disconnect, so it's safe to go higher than you need)
# -attn2g: range of attenuator values for 2g testing
# -pad: Estimate of path loss due to fixed components.  This is used
#       to offset results to make it easy to change your rig without
#       -spoiling your contrlchart history.
# -frameburst 1: enable frameburst



UTF::Linux cleodut -sta {DUT eth1} \
    -tag EAGLE_BRANCH_10_10 \
    -perfchans {161/80 36l 3} \
    -nopm1 0 -nopm2 0 -tcpwindow 16M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 2g \
    -wlinitcmds {wl country "#a/0"; wl msglevel +assoc;wl down; wl vht_features 7} \
    -modopts {assert_type=1 nompc=1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl reset_cnts}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl counters}}

DUT configure -ipaddr 192.168.1.101 -attngrp G


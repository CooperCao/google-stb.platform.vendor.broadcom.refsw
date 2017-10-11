# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for DVT "clifford" test rig
#

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/clifford"

# Controller
UTF::Linux clifford

package require UTF::ChanAssy
UTF::ChanAssy ca -relay clifford -group {G {0 1 2}}

# Specify the "default" attenuation (default default is zero)
G configure -default 10

# Reset anything that needs to be reset when starting a new nightly test.
set UTF::SetupTestBed {
    G attn default
}


# "enp1s0" may need to be adjusted depending on your HW.

UTF::Linux cliffordref -sta {REF eth1} \
    -tag EAGLE_{TWIG,REL}_10_10_2{,_*} \
    -wlinitcmds {wl dtim 3;wl msglevel +assoc;wl down; wl vht_features 7} \
    -modopts {assert_type=1 nompc=1}

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



UTF::Linux clifforddut -sta {DUT eth1} \
    -tag EAGLE_{TWIG,REL}_10_10_2{,_*} \
    -perfchans {161/80 36l 3} \
    -nopm1 1 -nopm2 1 -tcpwindow 16M -slowassoc 5 -reloadoncrash 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 2g \
    -wlinitcmds {wl msglevel +assoc;wl down; wl vht_features 7} \
    -modopts {assert_type=1 nompc=1}

DUT configure -ipaddr 192.168.1.101 -attngrp G


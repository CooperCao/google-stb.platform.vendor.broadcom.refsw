# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#
# Testbed configuration file for DVD "scrabble" test rig
#

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/scrabble"

# Controller
UTF::Linux scrabble

package require UTF::ChanAssy
UTF::ChanAssy ca -relay scrabble -group {G {0 1 2}}

UTF::Linux bringup104 -sta {DUT eth1} \
    -slowassoc 5 \
    -tag AARDVARK_BRANCH_6_30 \
    -nowep 1 \
    -perfchans {161/80 36l 3} \
    -channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl btc_mode 0} \
    -tcpwindow 4M -modopts {assert_type=1}

DUT configure -ipaddr 192.168.1.101 -attngrp G

UTF::Linux cubox70 -sta {REF eth1} \
    -slowassoc 5 \
    -tag AARDVARK_BRANCH_6_30 \
    -nowep 1 \
    -perfchans {161/80 36l 3} \
    -channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -wlinitcmds {wl msglevel +assoc; wl btc_mode 0; wl dtim 3} \
    -tcpwindow 4M -modopts {assert_type=1}

REF configure -ipaddr 192.168.1.102

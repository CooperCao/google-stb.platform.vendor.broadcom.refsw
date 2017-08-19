# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 90cccd21212eb10a5ff8bccb20fe3a2c7df93487 $
#
# Testbed configuration file for MC41testbed
#

source utfconf/mc41shared.tcl

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc41e"

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    E attn default
}

UTF::Linux mc41tst5 -sta {4360e eth0} \
    -console {mc41end1:40014} \
    -power {nb1 7} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON05T_BRANCH_7_35 \
    -wlinitcmds {wl msglevel +assoc;wl down;wl country '#a/0';wl dtim 3;wl bw_cap 2g -1;wl vht_features 3} \
    -tcpwindow 4m

4360e configure -ipaddr 192.168.1.95 -attngrp E -hasdhcpd 1 -ap 1

#############################

# Add hook to delay tests until the scan is passed.
# msglevel +scan is enabled to watch out for any scans we missed

UTF::MacOS mc41tst6 -sta {X87 en0} \
    -tag BIS715LOBO_BRANCH_7_77 \
    -brand macos-internal-wl-gala -type Debug_10_11 \
    -kextunload 2 -power {ck 1} \
    -wlinitcmds {wl msglevel +assoc +scan;wl assert_type 0;wl down;wl country '#a/0';wl vht_features 3} \
    -tcpwindow 3640K \
    -udp 1.2g -perfchans {36/80 3} -slowassoc 5 \
    -yart {-pad 30 -attn5g 23-95 -attn2g 32-95} \
    -nativetools 1 \
    -post_assoc_hook {%S innetworkwait $AP 20}

#set UTF::PerfCacheMigrate {BIS715GALA BIS715LOBO}

# Power switched USB ethernet, for recovery from lost IP
UTF::Power::CanaKit ck \
    -lan_ip /dev/serial/by-path/pci-0000:00:1a.0-usb-0:2:1.0 \
    -relay lan -invert 1

X87 clone eth -power {ck 1}

#####
UTF::Q mc41e

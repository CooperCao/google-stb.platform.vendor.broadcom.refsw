#
# Utf configuration for multicast testing with AP/STA Robert
# J. McMahon (setup in my cube)
#

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/mc44"

package require UTF::Power
UTF::Power::Synaccess MC44NPC -lan_ip 10.19.86.114
# -tag "KIRIN_BRANCH_5_100"
# -tag  "KIRIN_REL_5_100_56"
# -date "2010.7.17"


# Controller
UTF::Linux mc44

UTF::Linux MC44Lx12 -lan_ip 10.19.86.120 \
                    -sta {vlan120_if eth0.120}

vlan120_if configure -ipaddr 192.168.1.120

UTF::Linux MC44Lx11 -lan_ip 10.19.86.119 \
                     -sta {43224Lx11 eth1} \
                     -brand "linux-internal-wl" \
                     -wlinitcmds {wl msglevel 0x101; wl msglevel +scan; wl msglevel +rate} \
                     -tag "NIGHTLY" \

UTF::Linux MC44Lx10 -lan_ip 10.19.86.118 \
                     -sta {43224Lx10 eth1} \
                     -brand "linux-internal-wl" \
                     -wlinitcmds {wl msglevel 0x101; wl msglevel +scan; wl msglevel +rate} \
                     -tag "NIGHTLY" \

UTF::Linux MC44Lx9HN -lan_ip 10.19.86.116 \
                     -sta {43224Lx9 eth1} \
                     -brand "linux-internal-wl" \
                     -wlinitcmds {wl msglevel 0x101; wl msglevel +scan; wl msglevel +rate} \
                     -tag "NIGHTLY" \

UTF::Linux MC44Lx8Sniff -lan_ip 10.19.86.117 \
                     -sta {43224Lx8 eth1} \
                     -brand "linux-internal-wl" \
                     -tag "NIGHTLY" \

UTF::Linux MC44Lx13Sniff -lan_ip 10.19.86.121 \
                     -sta {43224Lx13 eth1} \
                     -brand "linux-internal-wl" \
                     -tag "NIGHTLY" \

UTF::Cygwin MC44Win7-1 -lan_ip 10.19.86.122 \
                     -sta {Win7-1} \
                     -osver 7 \
                     -tag NIGHTLY \
                     -user user \
                     -installer inf \
                     -tcpwindow auto

package require UTF::AeroflexDirect
UTF::AeroflexDirect AF -lan_ip 172.16.2.116 -silent 0 -debug 0 -group {L1 {1 2} L2 {3 4} L3 {5 6} ALL {1 2 3 4 5 6}}

UTF::AeroflexDirect AF-M -lan_ip 172.16.2.115 -silent 1 -debug 0 -group {L1M {1 2} L2M {3 4} L3M {5 6} ALLM {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
    ::hidden_node off
    ::UTF::Streamslib::force_cleanup "vlan120_if 43224Lx9 43224Lx10 43224Lx11"
    return
}


#    -tag "MILLAU_REL_5_70_48_1"
#    -tag "MILLAU_BRANCH_5_70"
#    -tag "AKASHI_REL_5_110_1_4"
UTF::Router _4717_Q -name 4717_Q \
    -sta {4717_Q eth1} \
    -brand linux-internal-router \
    -tag "AKASHI_TWIG_5_110_64" \
    -power {MC44NPC 1} \
    -relay "MC44Lx12" \
    -lanpeer vlan120_if \
    -lan_ip 192.168.1.1 \
    -console 10.19.86.120:40000 \
    -nvram {
	wl_msglevel=0x101
	wl0_ssid=QOS_GOLDEN
 	router_disable=0
	antdiv=0
	macaddr=00:90:4C:2F:0B:01
	et1macaddr=00:90:4C:2F:0C:01
        lan_ipaddr=192.168.1.1
    }

4717_Q configure -attngrp "L1"
#
#        AccessPoint
#             |
#             | L1
#             |
#             +S1
#            / \
#        L2 /   \L3
#          /     \
#         +S2     +S3
#        / \     / \
#       /   \   /   \
#      H1   H2  H3  H4
#
# +  = splitter/combiner
# Lx = variableattenuator leg
# Hx = Sta/Host
#
# When hidden node on
#      H1,H2 hidden from H3,H4 via ~(L2+L3+S1) attenuation
#      H1,H2 ~L2 from AP
#      H3,H4 ~L3 from AP
# When hidden node off
#      H1,H2,H3,H4 L1 from AP
#      H1,H2 ~S1 from H3,H4
#
# Test script example: Test/HiddenNode.test
#
proc ::hidden_node {state {db 50}} {
    set state [string tolower $state]
    switch -exact $state {
	"on" {
	    set L2L3pathloss [expr {$db - 10}]
	    L1 attn 0
	    if {$L2L3pathloss < 0} {
		UTF::Message WARN "" "setting L2/L3 path loss to zero"
		L2 attn 0
		L3 attn 0
	    } else {
		L2 attn $L2L3pathloss
		L3 attn $L2L3pathloss
	    }
	    set ::hidden_node_state "ON"
	}
	"off" {
	    set L1pathloss [expr {$db - 10}]
	    if {$L1pathloss < 0} {
		UTF::Message WARN "" "setting L1 path loss to zero"
		L1 attn 0
	    }  else {
		L1 attn $L1pathloss
	    }
	    L2 attn 0
	    L3 attn 0
	    set ::hidden_node_state "OFF"
	}
	default {
	    error "unknown state $state - should be on | off"
	}
    }
}

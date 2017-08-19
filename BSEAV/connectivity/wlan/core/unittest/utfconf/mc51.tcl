# -*-tcl-*-
#
# Testbed configuration file for MC51
#

# Load Packages
package require UTF::Aeroflex

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc51"

# Switches
# 172.16.1.10 (rack)
# 172.16.1.11 (rack)
# 172.16.1.20 (AP enclosure 4717a)
# 172.16.1.40 (Top Enclosure with mc51tst1,2)

# VLANS:
# 1: corp
# 3: private 172
# 10: LAN
# 20: WAN

# Define power controllers on cart
UTF::Power::WebSwitch ws1 -lan_ip 172.16.1.21 -relay mc51end1
#UTF::Power::WebSwitch webswitch1 -lan_ip 172.16.1.41 -relay mc51end1
UTF::Power::Synaccess npc22c -lan_ip 172.16.1.51 -relay mc51end1

# Attenuator - Aeroflex
# (IP is mc51tatt1, but xport is locked out - use serial)
UTF::Aeroflex af -lan_ip mc51end1:40002 -group {
    G1 {1 2}
    G2 {3 4}
}

G1 configure -default 15
G2 configure -default 15

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    # delete myself
    unset ::UTF::SetupTestBed

    # Reset attenuator
    G1 attn default
    G2 attn default

    # Down STAs
    foreach S {43217 43228 4352a 4352b} {
	catch {$S wl down}
	$S deinit
    }
    return
}

# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc51end1 -sta {lan1 eth1}
lan1 configure -ipaddr 192.168.1.50

# UTF Endpoint2 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc51end2 -sta {lan2 eth1}

###############################
#
# Top Ramsey box
#
###############################

############
# SoftAP
UTF::Linux mc51tst2 -sta {4352a enp1s0} \
    -console "mc51end1:40005" \
    -power {npc22c 2} -tag BISON05T_BRANCH_7_35 \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl country '#a/0'}

4352a configure -ipaddr 192.168.1.50 -hasdhcpd 1 -attngrp G2

#############

UTF::Linux mc51tst1 -sta {4352b enp2s0} \
    -console "mc51end1:40003" \
    -power {ws1 1} -tag BISON05T_BRANCH_7_35 \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl country '#a/0'}

4352b configure -ipaddr 192.168.2.50 -hasdhcpd 1 -attngrp G1

###############################
#
# Bottom Ramsey box
#
###############################

UTF::Linux mc51tst3 -sta {43228 enp1s0} \
    -console "mc51end1:40001" \
    -power {npc22c 1} -slowassoc 5 -udp 250m -reloadoncrash 1 \
    -yart {-attn5g 32-103 -attn2g 43-103 -pad 26} \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl country '#a/0'}

43228 clone 43228p2p -type debug-apdef-stadef-p2p-mchan-tdls

43228 clone 43228b -tag BISON_BRANCH_7_10 -sta {43228b enp1s0 _4312b enp4s0}
_4312b clone 4312b -nocustom 1

43228 clone 43228b35 -tag BISON05T_BRANCH_7_35
4312b clone 4312b35 -tag BISON05T_BRANCH_7_35

43228 clone 43228e -tag EAGLE_BRANCH_10_10
43228 clone 43228n -tag NANDP_BRANCH_1_2
43228 clone 43228h -tag HORNET_BRANCH_12_10

###############################

UTF::Linux mc51tst4 -sta {43217 enp1s0} \
    -console "mc51end1:40009" \
    -power {ws1 2} \
    -slowassoc 5 -apmode 1 -udp 250m \
    -yart {-attn2g 45-103 -pad 26} \
    -wlinitcmds {wl down;wl bw_cap 2 -1;wl country '#a/0'} \
    -nocustom 1

43217 clone 43217d -tag DINGO_TWIG_9_10_178
43217 clone 43217b -tag BISON04T_BRANCH_7_14 \
    -brand linux-internal-media \
    -type debug-apdef-stadef-p2p-mchan-tdls-wowl-mfp


###############################

set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	package require UTF::Test::MultiSTANightly
	MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
	    -nosetuptestbed -nostaload -nostareload -nosetup \
	    -noapload -norestore -nounload
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA
   }
}

#####
UTF::Q mc51 lan1


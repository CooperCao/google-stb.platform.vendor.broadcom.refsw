#
# Utf configuration for Apple Power Throttling testing
# Robert J. McMahon (setup in my cube)
#

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext2/rmcmahon/applpower"

package require UTF::Power
UTF::Power::Synaccess MC_Enc_NPC5 -lan_ip 10.19.86.111
UTF::Power::Synaccess MC_Enc_NPC6 -lan_ip 10.19.86.112

package require UTF::WebRelay
UTF::WebRelay RamseyAP_BRCM -lan_ip 10.19.86.112 -port 1
UTF::WebRelay RamseyAP_AirPort -lan_ip 10.19.86.112 -port 2

#
# Power the appropriate APs by setting power to the entire Ramsey enclosure
#
set ::UTF::SetupTestBed {
    UTF::Message SETUP "" "Setting up testbed, AP = $(ap)"
    if {$(ap) != ""} {
	RamseyAP_AirPort power off
	RamseyAP_BRCM power on
	MC_Enc_NPC5 power on 1
    } else {
	RamseyAP_AirPort power on
	RamseyAP_BRCM power off
    }
    ::G2 attn 20
    UTF::Sleep 30.0
    return
}

package require UTF::AeroflexDirect
UTF::AeroflexDirect AF -lan_ip 10.19.86.109 -concurrent 0 -group {G1 {3 4} A1 {1} A2 {2} A3 {3} A4 {4} G2 {1 2 3 4} ALL {1 2 3 4}}

UTF::Linux MCrjmLx5 -lan_ip 10.19.86.108 \
                    -sta {vlan119_if eth0.119}

#                   -tag PBR_REL_5_10_131_32
UTF::MacOS MacBook -lan_ip 10.19.84.253 \
                   -brand "macos-internal-wl-snowleopard" \
                   -type Debug_10_6 \
                   -tag KIRIN_BRANCH_5_100 \
                   -sta {MacBook4353 en1} \
                   -kextload false \
                   -wlinitcmds {wl msglevel 0x101} \
                   -coreserver AppleCore \
                   -tcpwindow 512k

MacBook4353 configure -attngrp G2

vlan119_if configure -ipaddr 192.168.1.119

UTF::Router _4717_D -name 4717_D \
    -sta {4717_D eth1} \
    -brand linux-internal-router \
    -tag "AKASHI_BRANCH_5_110" \
    -power {MC_Enc_NPC5 1} \
    -relay "MCrjmLx5" \
    -lanpeer vlan119_if \
    -lan_ip 192.168.1.10 \
    -console 10.19.86.108:40000 \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
	wl0_ssid=PowerThrottle
 	router_disable=0
        lan_ipaddr=192.168.1.10
	wl0_antdiv=0
        wl0_rateset=all
	wl0_phytype=n
	macaddr=00:90:4C:2F:0B:04
	et1macaddr=00:90:4C:2F:0C:04
	antswitch=0
    }


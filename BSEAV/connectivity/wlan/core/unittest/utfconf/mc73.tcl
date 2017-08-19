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
UTF::Power::WebRelay MacsPB -lan_ip 10.19.85.3
UTF::Power::Laptop x28dpwr -button {MacsPB 1}

package require UTF::WebRelayDirect
UTF::WebRelayDirect x28d_pb -relaynum 2 -lan_ip 10.19.85.3

#
# Power the appropriate APs by setting power to the entire Ramsey enclosure
#

proc ::powerenc {} {
    if {$(ap) != ""} {
	RamseyAP_AirPort power off
	RamseyAP_BRCM power on
	MC_Enc_NPC5 power on 1
    } else {
	RamseyAP_AirPort power on
	RamseyAP_BRCM power off
    }
}
set ::UTF::SetupTestBed {
    UTF::Message SETUP "" "Setting up testbed, AP = $(ap)"
    ::AF configure -debug 1
    ::G1 attn 25
    ::A4 attn 103
    ::AF configure -debug 0
    catch {vlan119_if rexec pkill iperf}
    UTF::Sleep 2.0
    return
}

package require UTF::AeroflexDirect
UTF::AeroflexDirect AF -lan_ip 10.19.86.109 -concurrent 0 -group {G1 {1 2 3} A1 {1} A2 {2} A3 {3} A4 {4} ALL {1 2 3 4}}

UTF::Linux MCrjmLx5 -lan_ip 10.19.86.108 \
                    -sta {vlan119_if eth0.119}

UTF::Linux MCrjmUTF -lan_ip 10.19.85.172

UTF::MacOS MacBookPro -lan_ip 10.19.84.253 \
                  -brand  "macos-internal-wl-zin" \
	          -type Debug_10_8 \
                  -kextload true -datarate 0 \
                  -tag NIGHTLY \
                  -sta {x28d en1} \
                  -kextload true \
                  -coreserver AppleCore \
                  -tcpwindow 512k \
                  -power {x28dpwr}\
                  -power_button {auto}\
                  -wlinitcmds {wl msglevel 0x101; wl msglevel +scan; wl msglevel +rate; wl phy_cal_disable 1}

UTF::MacOS MacBookAir -lan_ip 10.19.85.2 \
                  -brand  "macos-internal-wl-zin" \
	          -type Debug_10_8 \
                  -kextload true -datarate 0 \
                  -tag NIGHTLY \
                  -sta {x29d en0} \
                  -kextload true \
                   -wlinitcmds {wl msglevel 0x101; wl msglevel +scan; wl msglevel +rate; wl phy_cal_disable 1} \
                  -coreserver AppleCore \
                  -tcpwindow 512k

x29d configure -attngrp G1
x29d clone x29d_ruby -tag RUBY_BRANCH_6_20
x29d clone x29d_tot -tag NIGHTLY
x29d clone x29d_tob -tag AARDVARK_BRANCH_6_30
x29d clone x29d_k -tag KIRIN_BRANCH_5_100
x29d clone x29d_rel -tag AARDVARK_REL_6_30_*
#x29d clone x29d_r -tag KIRIN_REL_5_106_98_95*
x29d clone x29d_r2 -tag KIRIN_REL_5_106_98_96*
x29d clone x29d_r3 -tag KIRIN_REL_5_106_98_97*
x29d clone x29d_kl -tag KIRIN_REL_5_106_98_*
x29d clone x29d_krl -tag KIRIN_REL_5_106_98_105
x29d clone x29d_r4 -tag AARDVARK_REL_6_30*
x29d clone x29d_r5 -tag AARDVARK_REL_6_30_101
x29d clone x29d_r6 -tag AARDVARK_REL_6_30_112
x29d clone x29c_r -tag AARDVARK_REL_6_30_118_53

x28d configure -attngrp G1
x28d clone x28d_ruby -tag RUBY_BRANCH_6_20
x28d clone x28d_tot -tag NIGHTLY
x28d clone x28d_tob -tag AARDVARK_BRANCH_6_30
x28d clone x28d_k -tag KIRIN_BRANCH_5_100
x28d clone x28d_rel -tag AARDVARK_REL_6_30_*
x28d clone x28d_r -tag KIRIN_REL_5_106_98_95*
x28d clone x28d_r -tag KIRIN_REL_5_106_98_95*
x28d clone x28d_r4 -tag AARDVARK_REL_6_30*
x28d clone x28d_r5 -tag AARDVARK_REL_6_30_101
x28d clone x28d_r6 -tag AARDVARK_REL_6_30_112
x28d clone x28d_r -tag AARDVARK_REL_6_30_118_28

vlan119_if configure -ipaddr 192.168.1.119

UTF::Router _E4200 -name E4200 \
    -sta {E4200_2G eth1 E4200_5G eth2} \
    -brand linux-external-router\
    -tag "AKASHI_REL_5_110_35" \
    -power rtrpower \
    -relay "MCrjmLx5" \
    -lanpeer vlan119_if \
    -console 10.19.86.108:40000 \
    -nvram {
	boot_hw_model=E4200
        et0macaddr=00:90:4c:0b:0b:b1
        macaddr=00:90:4c:0b:0b:b2
        sb/1/macaddr=00:90:4c:0b:0b:b3
        pci/1/1/macaddr=00:90:4c:0b:0b:b4
	wan_hwaddr=00:90:4c:0b:0b:b5
        wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
	wan_ifnames=vlan2
	wl_msglevel=0x101
	fw_disable=1
	wl0_ssid=MC73_2G
	wl1_ssid=MC73_5G
	wan_ifname=vlan2
	wan0_ifname=vlan2
	wan_proto=static
	wan0_proto=static
	wan_ipaddr=192.168.4.2
	wan0_ipaddr=192.168.4.2
	wan_netmask=255.255.255.0
	wan0_netmask=255.255.255.0
	router_disable=0
	wl1_nbw_cap=2
    }

E4200_5G configure -attngrp G1

package require UTF::Airport

UTF::Airport AirPort -name Airport \
    -lanpeer vlan119_if \
    -relay MCrjmLx5 \
    -sta {AirPortSta eth1}

AirPortSta configure -lan_ip 192.168.1.1

set numsteps 36
set dwell 1
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting) "1 $dwell"
}
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting) "-1 $dwell"
}

set numsteps 33
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting36l) "1 $dwell"
}
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting36l) "-1 $dwell"
}


set numsteps 39
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting36lb) "1 $dwell"
}
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting36lb) "-1 $dwell"
}

set numsteps 30
set dwell 1
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting2x2) "1 $dwell"
}
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting2x2) "-1 $dwell"
}

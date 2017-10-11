#
# UTF configuration for IPTV 3x3  4706/4331nrh testing
# Robert J. McMahon (setup next to my cube) MC65
#

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/mc65"

package require UTF::Power
package require UTF::Streamslib

UTF::Power::Synaccess NPC-NRH-A -lan_ip 172.16.2.160 -rev 1
UTF::Power::Synaccess NPC-NRH-B -lan_ip 172.16.2.161 -rev 1
UTF::Power::Synaccess NPC-NRH-C -lan_ip 172.16.2.162
UTF::Power::Synaccess NPC-NRH-D -lan_ip 172.16.2.163

UTF::Linux NRH-UTF -lan_ip 10.19.85.133

UTF::Linux NRH-Lx1 -lan_ip 10.19.85.134 \
                    -sta {vlan160_if eth0.160}
UTF::Linux NRH-Lx2 -lan_ip 10.19.85.135 \
                    -sta {vlan161_if eth0.161 wan eth0.1000}
UTF::Linux NRH-Lx3 -lan_ip 10.19.85.136 \
                    -sta {vlan162_if eth0.162}
UTF::Linux NRH-Lx4 -lan_ip 10.19.85.137 \
                    -sta {vlan163_if eth0.163 vlan169_if eth0.169}

vlan160_if configure -ipaddr 192.168.1.160 -peer iptv_ap_nrh
vlan161_if configure -ipaddr 192.168.1.161 -peer iptv_br1_nrh
vlan162_if configure -ipaddr 192.168.1.162 -peer iptv_br2_nrh
vlan163_if configure -ipaddr 192.168.1.163 -peer iptv_br3_nrh
vlan169_if configure -ipaddr 192.168.1.169

UTF::Linux NRH-Sniff -lan_ip 10.19.85.138 \
                     -sta {4331Lx6 eth0} \
                     -brand "linux-internal-wl" \
                     -tag "NIGHTLY"

package require UTF::AeroflexDirect

UTF::AeroflexDirect AF -lan_ip 172.16.2.132 -retries 0 -concurrent 0 -silent 1 -group {L1 {1 2 3} L2 {4 5 6} L3 {7 8 9} ALL {1 2 3 4 5 6 7 8 9 10 11 12} C1 {1} C2 {2} C3 {3}}

set ::UTF::SetupTestBed {
    # run a simple AF command just to synch its serial
    set ends "vlan160_if vlan161_if vlan162_if vlan163_if 4331Lx6"
    ::AF configure -debug 1
    ::L1 attn?
    ::AF configure -debug 0
    ::hidden_node off
    ::UTF::Streamslib::force_cleanup $ends
    foreach end $ends {
	catch {$end initctl start consolelogger}
    }
    return
}

UTF::Router _iptv_ap_nrh -name iptv_ap_nrh \
    -sta {iptv_ap_nrh eth1} \
    -brand linux26-internal-router \
    -tag "AARDVARK_BRANCH_6_30" \
    -power {NPC-NRH-A 1} \
    -relay "NRH-Lx1" \
    -lanpeer vlan160_if \
    -lan_ip 192.168.1.2 \
    -console 10.19.85.134:40000 \
    -nvram {
	watchdog=3000
	wl_msglevel=0x101
	fw_disable=1
	wl0_radio=1
	wl0_channel=157
	wl0_nbw_cap=1
        wl0_phytype=n
        wl0_nbw=40
        wl0_nband=1
        wl0_nctrlsb=lower
	wl0_wfi_enable=1
	wl0_ssid=NRH3x3
	wl0_wet_tunnel=1
 	router_disable=0
	emf_enable=1
	ctf_disable=0
	aci_daemon=down
	acs_daemon=down
	macaddr=00:90:4C:2F:0D:31
	et1macaddr=00:90:4C:2F:0E:31
        lan_ipaddr=192.168.1.2
	lan_stp=0
    }

UTF::Router _iptv_br1_nrh -name iptv_br1_nrh \
    -sta {iptv_br1_nrh eth1} \
    -brand linux26-internal-router \
    -tag "AARDVARK_BRANCH_6_30" \
    -power {NPC-NRH-B 1} \
    -relay "NRH-Lx2" \
    -lanpeer vlan161_if \
    -lan_ip 192.168.1.3 \
    -console 10.19.85.135:40000 \
    -nvram {
	watchdog=3000
	wl_msglevel=0x101
	fw_disable=1
	wl0_radio=1
	wl0_channel=157
        wl0_nbw_cap=1
        wl0_phytype=n
        wl0_nbw=40
        wl0_nband=1
        wl0_nctrlsb=lower
	wl0_wfi_enable=1
	wl0_mode=wet
	wl0_ssid=NRH3x3
	wl0_wet_tunnel=1
	router_disable=1
	emf_enable=1
	aci_daemon=down
	acs_daemon=down
	macaddr=00:90:4C:2F:0D:32
	et1macaddr=00:90:4C:2F:0E:32
        lan_ipaddr=192.168.1.3
	lan_stp=0
    }

UTF::Router _iptv_br2_nrh -name iptv_br2_nrh \
    -sta {iptv_br2_nrh eth1} \
    -brand linux26-internal-router \
    -tag "AARDVARK_BRANCH_6_30" \
    -power {NPC-NRH-C 1} \
    -relay "NRH-Lx3" \
    -lanpeer vlan162_if \
    -lan_ip 192.168.1.4 \
    -console 10.19.85.136:40000 \
    -nvram {
	watchdog=3000
	wl_msglevel=0x101
	fw_disable=1
	wl0_radio=1
	wl0_channel=157
	wl0_nbw_cap=1
        wl0_phytype=n
        wl0_nbw=40
        wl0_nband=1
        wl0_nctrlsb=lower
	wl0_wfi_enable=1
	wl0_mode=wet
	wl0_ssid=NRH3x3
	wl0_wet_tunnel=1
	router_disable=1
	emf_enable=1
	aci_daemon=down
	acs_daemon=down
	macaddr=00:90:4C:2F:0D:33
	et1macaddr=00:90:4C:2F:0E:33
        lan_ipaddr=192.168.1.4
	lan_stp=0
    }

UTF::Router _iptv_br3_nrh -name iptv_br3_nrh \
    -sta {iptv_br3_nrh eth1} \
    -brand linux26-internal-router \
    -tag "AARDVARK_BRANCH_6_30" \
    -power {NPC-NRH-D 1} \
    -relay "NRH-Lx4" \
    -lanpeer vlan163_if \
    -lan_ip 192.168.1.5 \
    -console 10.19.85.137:40000 \
    -nvram {
	watchdog=3000
	wl_msglevel=0x101
	fw_disable=1
	wl0_radio=1
	wl0_channel=157
	wl0_nbw_cap=1
        wl0_phytype=n
        wl0_nbw=40
        wl0_nband=1
        wl0_nctrlsb=lower
	wl0_wfi_enable=1
	wl0_mode=wet
	wl0_wet_tunnel=1
	wl0_ssid=NRH3x3
	router_disable=1
	emf_enable=1
	aci_daemon=down
	acs_daemon=down
	macaddr=00:90:4C:2F:0D:34
	et1macaddr=00:90:4C:2F:0E:34
        lan_ipaddr=192.168.1.5
	lan_stp=0
    }


iptv_ap_nrh configure -peer vlan160_if -attngrp L1
iptv_br1_nrh configure -peer vlan161_if -attngrp L2
iptv_br2_nrh configure -peer vlan162_if -attngrp L2
iptv_br3_nrh configure -peer vlan163_if -attngrp L3

proc ::hidden_node {state {db 40}} {
    set state [string tolower $state]
    switch -exact $state {
	"on" {
	    set L2L3pathloss [expr {$db - 28}]
	    L1 attn 0
	    if {$L2L3pathloss < 0} {
		UTF::Message WARN "" "setting L2/L3 path loss to zero"
		L2 attn 0
		L3 attn 0
	    } else {
		L2 attn $L2L3pathloss
		L3 attn [expr {int($L2L3pathloss + 5)}]
	    }
	    set ::hidden_node_state "ON"
	}
	"off" {
	    # set L1pathloss [expr {$db - 27}]
	    set L1pathloss [expr {$db - 5}]
	    if {$L1pathloss < 0} {
		UTF::Message WARN "" "setting L1 path loss to zero"
		L1 attn 0
	    }  else {
		L1 attn $L1pathloss
	    }
	    L2 attn 0
	    L3 attn 5
	    set ::hidden_node_state "OFF"
	}
	default {
	    error "unknown state $state - should be on | off"
	}
    }
}
set ::UTF::StepProfile(MCS) "{25 1} {3 1} {2 1} {3 1} {5 1} {1 1} {3 1} {3 1} {-45 1} {47 1} {-1 1}"
set ::UTF::StepProfile(MCS2) "{3 1} {2 1} {3 1} {5 1} {1 1} {3 1} {3 1} {-20 1} {20 1} {-1 1}"
set numsteps 32
set dwell 1
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(marketting) "1 $dwell"
}
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(full) "1 $dwell"
}
for {set ix 1} {$ix < $numsteps} {incr ix} {
    lappend ::UTF::StepProfile(full) "-1 $dwell"
}
set ::UTF::StepProfile(44) [list {44 1} {-44 1} {44 2} {-44 2} {44 4} {-44 4} {44 10} {-44 10}]
set ::UTF::StepProfile(test) [list {44 1} {-44 1} {48 2} {-33 3}]

4331Lx6 clone main -tag RUBY_BRANCH_6_20
iptv_ap_nrh clone ap -tag AARDVARK_REL_6_30_78 -brand linux26-external-router-media-partial-src
iptv_br1_nrh clone br1 -tag AARDVARK_REL_6_30_78 -brand linux26-external-router-media-partial-src
iptv_br2_nrh clone br2 -tag AARDVARK_REL_6_30_78 -brand linux26-external-router-media-partial-src
iptv_br3_nrh clone br3 -tag AARDVARK_REL_6_30_78 -brand linux26-external-router-media-partial-src

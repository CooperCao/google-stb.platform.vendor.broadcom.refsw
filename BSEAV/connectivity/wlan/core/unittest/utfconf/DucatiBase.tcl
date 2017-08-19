#
# Utf configuration for Ducati testing
# Robert J. McMahon (setup in my cube)
#

# SummaryDir sets the location for test results in nightly testing.
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/rmcmahon/multimedia"

package require UTF::Power

#
# Keep Windows STA at released level as it's not under test
#
UTF::Cygwin MCrjmTst1 -lan_ip 10.19.85.173 -sta {4312Vista} -user user \
     -osver 6 -installer inf -brand win_internal_wl -tag "BASS_REL_5_60_18_8"

UTF::Power::Synaccess MCrjmNPC1 -lan_ip 10.19.85.170
UTF::Power::Synaccess MCrjmNPC2 -lan_ip 172.16.2.14
UTF::Power::Synaccess MCrjmNPC3 -lan_ip 172.16.2.16
UTF::Power::Synaccess MCrjmNPC4 -lan_ip 172.16.2.17

UTF::Linux MCrjmLx1 -lan_ip 10.19.85.172 \
                    -sta {vlan113_if eth1.113}
UTF::Linux MCrjmLx2 -lan_ip 10.19.85.176 \
                    -sta {vlan114_if eth0.114}
UTF::Linux MCrjmLx3 -lan_ip 10.19.85.175 \
                    -sta {vlan116_if eth0.116}
UTF::Linux MCrjmLx4 -lan_ip 10.19.84.250 \
                    -sta {vlan117_if eth0.117}
UTF::Linux MCrjmLx14 -lan_ip 10.19.86.123 \
                    -sta {vlan118_if eth0.118}
UTF::Linux MCrjmLx6 -lan_ip 10.19.86.110 \
                     -sta {4322Linux eth1} \
                     -brand "linux-internal-wl" \
                     -tag "BASS_BRANCH_5_60"
UTF::Linux MCrjmSniff -lan_ip 10.19.85.169 \
                     -sta {4324Sniff eth1} \
                     -brand "linux-internal-wl" \
                     -tag "BASS_BRANCH_5_60"

vlan113_if configure -ipaddr 192.168.1.172
vlan114_if configure -ipaddr 192.168.1.176
vlan116_if configure -ipaddr 192.168.1.116
vlan117_if configure -ipaddr 192.168.1.117
vlan118_if configure -ipaddr 192.168.1.118
4322Linux configure  -ipaddr 192.168.1.120
vlan117_if configure -peer 4717_A
vlan114_if configure -peer 4717_B
vlan116_if configure -peer 4717_C
vlan118_if configure -peer 4717_D

package require UTF::AeroflexDirect
UTF::AeroflexDirect AF -lan_ip 10.19.85.174 -concurrent 0 -group {G1 {1 2} G2 {3 4} G3 { 5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
    set linuxhosts "MCrjmLx2 MCrjmLx3 MCrjmLx4 MCrjmLx14"
    foreach host $linuxhosts {
	set sta [$host cget -sta]
	UTF::Message INFO WET_LxSetup "$sta"
	set ifdev [lindex $sta 1]
	set ipaddr [[lindex $sta 0] cget -ipaddr]
	$host ifconfig $ifdev $ipaddr
    }
    #
    # Do the attenuator last, sometimes it get hosed with
    # utf error couldn't open socket.
    #
    G1 attn 20; G2 attn 20; G3 attn 0;
    return
}

UTF::Router _4717_A -name 4717_A \
    -sta {4717_A eth1} \
    -brand linux-internal-router \
    -tag "DUCATI_BRANCH_5_24" \
    -power {MCrjmNPC1 1} \
    -relay "MCrjmLx4" \
    -lanpeer vlan117_if \
    -lan_ip 192.168.1.2 \
    -console 10.19.84.250:40000 \
    -nvram {
	wl_msglevel=0x101
	fw_disable=1
	wl0_ssid=DucatiVideoA
 	router_disable=0
	macaddr=00:90:4C:2F:0B:01
	et1macaddr=00:90:4C:2F:0C:01
        lan_ipaddr=192.168.1.2
    }

UTF::Router _4717_B -name 4717_B \
    -sta {4717_B eth1} \
    -brand linux-internal-router \
    -tag "DUCATI_BRANCH_5_24" \
    -power {MCrjmNPC2 1} \
    -relay "MCrjmLx2" \
    -lanpeer vlan114_if \
    -lan_ip 192.168.1.3 \
    -console 10.19.85.176:40000 \
    -nvram {
	wl_msglevel=0x101
	fw_disable=0
	wl0_ssid=DucatiVideoB
	router_disable=1
	macaddr=00:90:4C:2F:0B:02
	et1macaddr=00:90:4C:2F:0C:02
        lan_ipaddr=192.168.1.3
    }


UTF::Router _4717_C -name 4717_C \
    -sta {4717_C eth1} \
    -brand linux-internal-router \
    -tag "DUCATI_BRANCH_5_24" \
    -power {MCrjmNPC3 1} \
    -relay "MCrjmLx3" \
    -lanpeer vlan116_if \
    -lan_ip 192.168.1.4 \
    -console 10.19.85.175:40000 \
    -nvram {
	wl_msglevel=0x101
	fw_disable=0
	wl0_ssid=DucatiVideoC
	router_disable=1
	macaddr=00:90:4C:2F:0B:03
	et1macaddr=00:90:4C:2F:0C:03
        lan_ipaddr=192.168.1.4
    }

UTF::Router _4717_D -name 4717_D \
    -sta {4717_D eth1} \
    -brand linux-internal-router \
    -tag "DUCATI_BRANCH_5_24" \
    -power {MCrjmNPC4 1} \
    -relay "MCrjmLx14" \
    -lanpeer vlan118_if \
    -lan_ip 192.168.1.5 \
    -console 10.19.86.123:40000 \
    -nvram {
	wl_msglevel=0x101
	fw_disable=0
	wl0_ssid=DucatiVideoD
	router_disable=1
	macaddr=00:90:4C:2F:0B:04
	et1macaddr=00:90:4C:2F:0C:04
        lan_ipaddr=192.168.1.5
    }

4717_A configure -peer vlan117_if
4717_B configure -peer vlan114_if -attngrp G1
4717_C configure -peer vlan116_if -attngrp G2
4717_D configure -peer vlan118_if -attngrp G3

4717_A clone 4717_A_MILLAU -tag "MILLAU_BRANCH_5_70"
# 4717_B clone 4717_B_MILLAU -tag "MILLAU_BRANCH_5_70"
# 4717_C clone 4717_C_MILLAU -tag "MILLAU_BRANCH_5_70"

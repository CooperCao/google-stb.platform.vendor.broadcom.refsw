# -*-tcl-*-
#
# testbed configuration for Steve Liang MC47testbed
#
# Packages
package require UTF::Aeroflex
package require UTF::Linux

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext7/$::env(LOGNAME)/mc47"

# Define power controllers on cart
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.11
UTF::Power::Synaccess npc4 -lan_ip 172.16.1.15

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.16.1.3 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
    if {[catch {4313IPAFC11 wl down} ret]} {
        error "SetupTestBed: 4313IPAFC11 wl down failed with $ret"
    }
    4313IPAFC11 wl phymsglevel tra
    4313IPAFC11 wl msglevel trace
    if {[catch {4312FC11 wl down} ret]} {
        error "SetupTestBed: 4312FC11 wl down failed with $ret"
    }
    AP3 restart wl0_radio=0
    af setGrpAttn G3 0
    unset ::UTF::SetupTestBed
    return
}

# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux mc47end2 \
    -sta {lan2 eth1}

# STA Laptop DUT Dell E6400 with 4312 NIC
# assumes nightly top-of-tree build
UTF::Linux mc47tst6 \
        -sta {4312FC11 eth1} \
        -tcpwindow 512k \
        -power {npc4 1} \
        -console "mc47end2:40001" \
        -power_button {auto} \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

# Clones of STA 4312FC11 with Different Options for Test
4312FC11 clone 4312FC11-AARDVARK -tag AARDVARK_BRANCH_6_30
4312FC11 clone 4312FC11-RUBY -tag RUBY_BRANCH_6_20
4312FC11 clone 4312FC11-PSTAMERGE -tag PSTAMERGE_BRANCH_6_28
4312FC11 clone 4312FC11-KIRIN -tag KIRIN_BRANCH_5_100
4312FC11 clone 4312FC11-KIRIN-REL -tag  KIRIN_REL_5_100_??
4312FC11 clone 4312FC11-BASS -tag BASS_BRANCH_5_60
4312FC11 clone 4312FC11-BASS-REL -tag  BASS_REL_5_60_48_*

# STA Laptop DUT Dell E6400 with 4312 NIC
# assumes nightly top-of-tree build
UTF::Linux mc47tst3 \
        -sta {4313IPAFC11 eth1} \
        -tcpwindow 512k \
        -power {npc4 2} \
        -console "mc47end2:40002" \
        -power_button {auto} \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

# Clones of STA 4313IPAFC11 with Different Options for Test
4313IPAFC11 clone 4313IPAFC11-AARDVARK -tag AARDVARK_BRANCH_6_30
4313IPAFC11 clone 4313IPAFC11-RUBY -tag RUBY_BRANCH_6_20
4313IPAFC11 clone 4313IPAFC11-PSTAMERGE -tag PSTAMERGE_BRANCH_6_28
4313IPAFC11 clone 4313IPAFC11-KIRIN -tag KIRIN_BRANCH_5_100
4313IPAFC11 clone 4313IPAFC11-KIRIN-TWIG -tag KIRIN_REL_5_100_82_*
4313IPAFC11 clone 4313IPAFC11-BASS -tag BASS_BRANCH_5_60
4313IPAFC11 clone 4313IPAFC11-BASS-REL -tag  BASS_REL_5_60_48_*

# Linksys 320N 4717/4322 wireless router.
UTF::Router AP3 \
    -sta "4717-3 eth1" \
    -relay "mc47end2" \
    -lan_ip 192.168.1.3 \
    -lanpeer lan2 \
    -console "mc47end2:40000" \
    -power {npc5 1} \
    -brand linux-internal-router \
    -tag "MILLAU_REL_5_70_48_*" \
    -nvram {
        et0macaddr=00:21:29:03:00:01
        macaddr=00:21:29:03:00:02
        lan_ipaddr=192.168.1.3
        lan_gateway=192.168.1.3
        dhcp_start=192.168.1.210
        dhcp_end=192.168.1.249
        lan1_ipaddr=192.168.2.3
        lan1_gateway=192.169.2.3
        dhcp1_start=192.168.2.200
        dhcp1_end=192.168.2.249
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=mc47LinuxSTAtest
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
}

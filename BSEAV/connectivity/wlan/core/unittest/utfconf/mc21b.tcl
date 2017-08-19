# -*-tcl-*-
#
# testbed configuration for Steve Liang MC21B testbed
#
# -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

# Packages
package require UTF::Aeroflex
package require UTF::Linux

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext7/$::env(LOGNAME)/mc21b"

# Define power controllers on cart
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.24
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.26

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.16.1.32 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
    af setGrpAttn G1 0
    af setGrpAttn G2 0
    AP1 restart wl0_radio=0
    if {[catch {43224Win7 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 43224Win7 radio"}
    if {[catch {43224FC9 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 43224FC9 radio"}
    unset ::UTF::SetupTestBed
    return
}

# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux mc21bend3 \
    -sta {lan eth1}

# STA Laptop DUT Dell E6400 with 43224 NIC
# assumes nightly top-of-tree build
UTF::Cygwin mc21btst5 -user user\
        -sta {43224Win7} \
        -installer inf \
        -osver 7 \
        -tcpwindow 512k \
        -power {npc1 1} \
        -power_button {auto} \
        -allowdevconreboot 1 \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

# STA Laptop DUT Dell E6410 with 43224 NIC
# assumes nightly top-of-tree build
UTF::Cygwin mc50tst3 -user user\
        -sta {43224Vista} \
        -installer inf \
        -osver 7 \
        -tcpwindow 512k \
        -power {npc1 1} \
        -power_button {auto} \
        -allowdevconreboot 1 \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

# STA Laptop DUT Dell E6400 with 43224 NIC
# assumes nightly top-of-tree build
UTF::Linux mc21btst6 \
        -sta {43224FC9 eth1} \
        -tcpwindow 512k \
        -power {npc1 2} \
        -power_button {auto} \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

43224FC9 clone 43224FC9-KIRIN -tag KIRIN_BRANCH_5_100
43224Win7 clone 43224Win7-KIRIN -tag KIRIN_BRANCH_5_100

# Linksys 320N 4717/4322 wireless router.
UTF::Router AP1 \
    -sta "AP1-4717 eth1" \
    -relay "mc21bend3" \
    -lan_ip 192.168.1.1 \
    -lanpeer lan \
    -console "mc21bend3:40000" \
    -power {npc1 1} \
    -brand linux-internal-router \
    -tag "MILLAU_REL_5_70_48_*" \
    -nvram {
        et0macaddr=00:21:29:01:00:01
        macaddr=00:21:29:01:00:02
        lan_ipaddr=192.168.1.1
        lan_gateway=192.168.1.1
        dhcp_start=192.168.1.100
        dhcp_end=192.168.1.120
        lan1_ipaddr=192.168.2.1
        lan1_gateway=192.169.2.1
        dhcp1_start=192.168.2.100
        dhcp1_end=192.168.2.149
       	fw_disable=1
       	wl_msglevel=0x101
        wl0_ssid=mc21btest
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
}

# -*-tcl-*-
#
# Configuration file for Milosz Zielinski's MC31 testbed
#

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
 

# Enable Windows TestSigning
set UTF::TestSigning 1


# UTFD support
set ::env(UTFDPORT) 9978
package require UTFD

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc31"

# Power controllers
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.2
#UTF::Power::Synaccess npc2 -lan_ip 172.16.1.11
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.12
UTF::Power::Synaccess npc4 -lan_ip 172.16.1.15
UTF::Power::Synaccess npc5 -lan_ip 172.16.1.16

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.16.1.3 -group {G1 {1 2 5} G2 {3 4}}
# UTF::Aeroflex af -lan_ip 172.16.1.3 -group {4706 {1 2 5} 4717 {3 4} ALL {1 2 3 4 5 6}}


set ::UTF::SetupTestBed {
    af setGrpAttn G1 0
    af setGrpAttn G2 0

    UTF::Sleep 30
#    AP3 power off
#    AP2 power off

# AP3 restart wl0_radio=0
#    AP2 restart wl0_radio=0
    AP1 restart wl0_radio=0
#    if {[catch {4313EPA-Win7 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 4313EPA-Win7 radio"}
    if {[catch {4360FC19 wl down} ret]} {UTF::Message ERROR "" "Error shutting down 4360FC19 radio"}
        unset ::UTF::SetupTestBed
    
    return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# FC11 - new system installed 2015.1.22
UTF::Linux mc31end1 \
    -sta {lan eth1}

if {0} {
# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux mc31end2 \
    -sta {lan2 eth1}
}

#device does not exist
if {0} {
UTF::Sniffer mc31snf1 \
    -lan_ip 10.19.85.239 \
    -user root \
    -sta {43224FC9Sniffer eth1}
}


# STA Laptop DUT Dell E6400 with 4331 NIC
# assumes nightly top-of-tree build
# 10.22.23.67
# -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan}    0x101 is +err +assoc
UTF::Linux mc31tst1 -user root \
	-sta {4331fc19p2pwl enp4s0 4331fc19p2p wl0.1} \
	-tag trunk \
	-type debug-apdef-stadef-p2p-mchan-tdls \
        -console "mc31end1:40002" \
        -tcpwindow 512k \
        -power {npc4 2} \
        -brand linux-internal-wl \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
	-post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl phy_cal_disable 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}}

4331fc19p2pwl configure -ipaddr 192.168.1.5

#4331fc19p2pwl clone 4331fc19p2pwlaa -tag AARDVARK_BRANCH_6_30
#4331fc19p2p clone 4331fc19p2paa -tag AARDVARK_BRANCH_6_30

4331fc19p2pwl clone 4331fc19p2pwlBISON -tag BISON_BRANCH_7_10
4331fc19p2pwl clone 4331fc19p2pwlEAGLE -tag EAGLE_BRANCH_10_10
4331fc19p2p   clone 4331fc19p2pBISON -tag BISON_BRANCH_7_10
4331fc19p2p   clone 4331fc19p2pEAGLE -tag EAGLE_BRANCH_10_10

#4313IPAFC11 clone 4313IPAFC11-AARDVARK -tag AARDVARK_BRANCH_6_30


# STA Laptop DUT Dell E6400
# BCM: 43602
# 2015.3.31 update installed on 2015.4.16
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.18
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.6.30
# 2015.7.06 update installed on 2015.7.08
UTF::Cygwin mc31tst2 -user user -sta {43602Win10} \
        -osver 1064 \
        -sign true \
        -node DEV_43BA \
        -usemodifyos 1 \
        -kdpath kd.exe \
        -tcpwindow 4M \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -brand win8x_internal_wl \
        -power "npc4 1"

43602Win10 clone 43602Win10-TOT
43602Win10 clone 43602Win10-BISON735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400 with 4352 NIC
# 2015.3.31 update installed on 2015.4.16
# 2015.7.02 update installed on 2015.7.08
UTF::Cygwin mc31tst3 -user user -sta {4352Win81x64} \
        -osver 8164 \
        -sign true \
        -node DEV_43B1 \
        -installer inf \
        -tcpwindow 2M \
        -power {npc3 2} \
        -brand win8x_internal_wl \
        -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3}  \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -post_perf_hook {{$S wl rssi} {%S wl rate} {%Swl nrate} {%S wl dump ampdu}} \
        -pre_perf_hook {{%S wl  ampdu_clear_dump}}

# Clones of STA 4352Win81x64
4352Win81x64 clone 4352Win81x64-TOT
4352Win81x64 clone 4352Win81x64-BISON735   -tag BISON05T_BRANCH_7_35


# STA Linux FC19 PC 4360 NIC
# assumes nightly top-of-tree build
UTF::Linux mc31tst4 -user root -sta {4360FC19p2pwl enp11s0 4360FC19p2p wl0.1} \
        -tcpwindow 512k \
        -type debug-apdef-stadef-p2p-mchan-tdls \
        -brand linux-internal-wl \
        -perfchans {157l 36l} \
        -power {npc5 1} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -nobighammer 0 \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl phy_cal_disable 0}}
        

# Clones of STA 4360FC19 with Different Options for Test
4360FC19p2pwl configure -ipaddr 192.168.1.6
4360FC19p2p clone 4360FC19p2paa -tag AARDVARK_BRANCH_6_30
4360FC19p2pwl clone 4360FC19p2pwlaa -tag AARDVARK_BRANCH_6_30
4360FC19p2p clone 4360FC19p2pBISON -tag BISON_BRANCH_7_10
4360FC19p2pwl clone 4360FC19p2pwlBISON -tag BISON_BRANCH_7_10


# Dell E6400 laptop with 43142 chip
# in the lower large Ramsey
# Windows 10x64
# 2015.5.15 re-installed on 2015.6.4 and installed as new system
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.6.30
# 2015.7.06 update installed on 2015.7.09
UTF::Cygwin mc31tst5 -sta {43142Win10x64} \
    -lan_ip 10.22.23.71 \
    -osver 1064 \
    -node DEV_4365 \
    -sign true \
    -user user \
    -power {npc3 1} \
    -installer inf \
    -ssh ssh \
    -brand win8x_internal_wl \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -tcpwindow 4M \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

43142Win10x64 clone 43142Win10x64-TOT
43142Win10x64 clone 43142Win10x64-BISON735 -tag BISON05T_BRANCH_7_35



# 4708 wireless router.
UTF::Router AP1 \
	 -sta "47061 eth1 47062 eth2" \
    -power {npc1 1} \
    -relay "mc31end1" \
    -lan_ip 192.168.1.1 \
    -brand linux26-internal-router \
    -lanpeer lan \
    -console "mc31end1:40001" \
	-txt_override {
	watchdog=6000
    } \
   -nvram {
	watchdog=6000; # PR#90439
	wl_msglevel=0x101
	wl0_ssid=4706/4331-mc31
	wl0_chanspec=1
	wl0_radio=0
	wl0_obss_coex=0
	wl1_ssid=4706/4360-mc31
	wl1_chanspec=36
	wl1_radio=0
	wl1_obss_coex=0
	boardtype=0x05b2; # 4706nr
   }

47061 clone 47061b -tag BISON_BRANCH_7_10
47061 clone 47061-good -date 2015.5.18.0

if {0} {
# Linksys 320N 4717/4322 wireless router.
# JGP - no ping as of 2014.12.8.0
UTF::Router AP2 \
    -sta "4717-2 eth1" \
    -relay "mc31end1" \
    -lan_ip 192.168.1.2 \
    -lanpeer lan \
    -console "mc31end1:40002" \
    -power {npc2 1} \
    -brand linux26-internal-router \
    -tag "BISON_BRANCH_7_10" \
    -nvram {
        et0macaddr=00:21:29:02:00:01
        macaddr=00:21:29:02:00:02
        lan_ipaddr=192.168.1.2
        lan_gateway=192.168.1.2
        dhcp_start=192.168.1.121
        dhcp_end=192.168.1.149
        lan1_ipaddr=192.168.2.2
        lan1_gateway=192.169.2.2
        dhcp1_start=192.168.2.122
        dhcp1_end=192.168.2.149
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=mc31test
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
}
4717-2 configure -attngrp G2

# Linksys 320N 4717/4322 wireless router.
# JGP - no ping as of 2014.12.8.0
UTF::Router AP3 \
    -sta "4717-3 eth1" \
    -relay "mc31end2" \
    -lan_ip 192.168.1.3 \
    -lanpeer lan2 \
    -console "mc31end2:40000" \
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
        wl0_ssid=mc31LinuxSTAtest
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
}
}


# -*-tcl-*-
#
# Testbed configuration file for Milosz Zielinski MC87 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTFD

# UTFD
set ::env(UTFDPORT) 9980

# SummaryDir sets the location for test results
set UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc87"
set UTF::Logfile "~/utf_test.log"
set UTF::TestSigning true

# optional items for controlchart.test to run each iteration
# set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl nrate; %S wl antdiv; %S wl btc_mode; %S wl txant}}}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    # Set attenuator to 0
    G1 attn 20
    G2 attn 20
    G3 attn 20
    
    return
}

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1
set UTF::RouterDataRatePlot 1
set UTF::CountedErrors 1

# Enable Iperf 2.0.8
set UTF::IPerfBeta 1
set UTF::TcpReadStats 1

# Define power controllers in testbed
UTF::Power::Synaccess npc22a -lan_ip 172.16.1.1 -relay mc87end1 -rev 1
UTF::Power::Synaccess npc22b -lan_ip 172.16.1.2 -relay mc87end1 -rev 1
UTF::Power::WebRelay  web10relay1 -lan_ip 172.16.1.11 -relay mc87end1

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc87end1 -sta {lan eth1}

# mc87tst2 windows 8.1x86 update on Dell E6420 laptop
# 10.19.60.119, user: user, pass: hrun*10
UTF::Cygwin mc87tst2 \
        -sta {43228WINSTA2} \
        -user user \
        -node {DEV_4359} \
        -osver 81 -tag BISON05T_BRANCH_7_35 \
        -brand win8x_internal_wl \
        -kdpath kd.exe \
        -usemodifyos 1 \
        -ssh ssh \
        -installer InstallDriver \
        -sign 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up ; wl assert_type 1 }
#    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \


# mc87tst4 windows 8x86 update on blade system.
# 10.19.61.137, user: user, pass: hrun*10
UTF::Cygwin mc87tst4 \
        -sta {43228WINSTA4} \
        -user user \
        -node {DEV_4359} \
        -osver 1064 -tag BISON05T_BRANCH_7_35 \
        -udp 800m \
        -brand winthresh_internal_wl \
        -kdpath kd.exe \
        -usemodifyos 1 \
        -ssh ssh \
        -installer InstallDriver \
        -sign 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up ; wl assert_type 1 }
#    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \



# -osver 81 -tag BISON04T_BRANCH_7_14 \
#-brand win8x_internal_wl \
#
# mc87tst5 windows 8.1x86 update on blade system.
# 10.19.61.138, user: user, pass: hrun*10
UTF::Cygwin mc87tst5 \
        -sta {4352WINSTA5} \
        -user user \
        -node {DEV_43B1} \
        -osver 10 -tag BISON05T_BRANCH_7_35 \
        -udp 800m \
        -brand winthresh_internal_wl \
        -kdpath kd.exe \
        -usemodifyos 1 \
        -ssh ssh \
        -installer InstallDriver \
        -sign 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up ; wl assert_type 1 }
#    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \



# mc87tst6 windows 8.1x64 update on blade system.
# 10.19.61.139, user: user, pass: hrun*10
UTF::WinDHD mc87tst6 \
        -sta {4356WINSTA6} \
        -user user \
        -node {DEV_43EC} \
        -osver 1064 -tag NIGHTLY \
        -udp 800m \
        -brand winthresh_dhdpcie_internal_wl \
        -type checked/DriverOnly/x64 \
        -kdpath kd.exe \
        -usemodifyos 1 \
        -ssh ssh \
        -installer InstallDriver \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up ; wl assert_type 1 }
#    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
#	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \



# Clones for mc87tst2
#43228WIN8 clone 43228WIN8-AARDVARK -tag AARDVARK_BRANCH_6_30
#43228WIN8 clone 43228WIN8-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_???
#43228WIN8 clone 43228WIN8-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_*
#43228WIN8 clone 43228WIN8-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*

# Clones for NWiFi testing
4356WINSTA6 clone 4356WINSTA6-NWiFi -brand win8x_dhdpcie_internal_wl -osver 1064
4352WINSTA5 clone 4352WINSTA5-NWiFi -brand win8x_internal_wl -osver 10
43228WINSTA4 clone 43228WINSTA4-NWiFi -brand win8x_internal_wl -osver 1064

# AP Section
#Linksys E4200 4718/43228WIN8 Router AP1
UTF::Router E4200 \
        -sta {4718/4321 eth1 4718/4331 eth2}\
        -lan_ip 192.168.1.1 \
        -brand "linux-external-router" \
        -tag "AKASHI_REL_5_110_??" \
        -power "npc22a 1" \
        -relay "mc87end1" \
        -lanpeer lan \
        -console "mc87end1:40001" \
        -nvram {
            boot_hw_model=E4200
            et0macaddr=00:90:4c:01:cc:1e
            macaddr=00:90:4c:01:cc:1d
            lan_ipaddr=192.168.1.1
            lan_gateway=192.168.1.1
            dhcp_start=192.168.1.101
            dhcp_end=192.168.1.149
            lan1_ipaddr=192.168.2.1
            lan1_gateway=192.169.2.1
            dhcp1_start=192.168.2.100
            dhcp1_end=192.168.2.149
            wl_msglevel=0x101
            wl0_ssid=4718
            wl0_radio=0
            wl1_ssid=4718/4331
            wl1_channel=1
            wl1_nbw_cap=0
            wl1_obss_coex=0
            wl0_obss_coex=0
            wandevs=et0
            {lan_ifnames=vlan1 eth1 eth2}
        }



set ::UTFD::intermediate_sta_list(0)  4352WINSTA5-NWiFi
set ::UTFD::intermediate_sta_OS(0)    Win8x
set ::UTFD::intermediate_last_svn(0)  0
set ::UTFD::intermediate_sta_list(1)  43228WINSTA4-NWiFi
set ::UTFD::intermediate_sta_OS(1)    Win8x
set ::UTFD::intermediate_last_svn(1)  0
set ::UTFD::intermediate_ap           "4718/4331"
set ::UTFD::intermediate_ap_name      "4718/4331 AP"
set ::UTFD::max_STAindex_count        2
set ::UTFD::rigname                   "mc87"
set ::UTFD::driver_build_path         "/projects/hnd_sig_ext14/jpalte/testbuildlist"



# Attenuator - Aeroflex (run 'af setup' if changing to UTF::AeroflexDirect)
# Syntax "-lan_ip 172.16.1.51:20000/udp" is to support parallel test using UDP
#
UTF::Aeroflex af1 -lan_ip 172.16.1.51:20000/udp \
        -relay "mc87end1" -group {G1 {1 2 3} G2 {4 5 6} G3 {4 5 6}}

proc nightly_sta_wdi {} {
    UTFD::metascript %AUTO% -watch 4356WINSTA6 -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4356-Win10} -ap {4718/4331} -sta {4356WINSTA6} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4352WINSTA5 -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4352-Win10} -ap {4718/4331} -sta {4352WINSTA5} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 43228WINSTA4 -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 43228-Win10} -ap {4718/4331} -sta {43228WINSTA4} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
}

proc nightly_rvr_wdi {} {
    UTFD::metascript %AUTO% -watch 4356WINSTA6 -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4356-Win10} -ap {4718/4331} -sta {4356WINSTA6} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4352WINSTA5 -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4352-Win10} -ap {4718/4331} -sta {4352WINSTA5} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 43228WINSTA4 -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {43228-Win10} -ap {4718/4331} -sta {43228WINSTA4} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
}

proc nightly_sta_nwifi {} {
    UTFD::metascript %AUTO% -watch 4356WINSTA6-NWiFi -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4356-Win10} -ap {4718/4331} -sta {4356WINSTA6-NWiFi} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4352WINSTA5-NWiFi -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4352-Win10} -ap {4718/4331} -sta {4352WINSTA5-NWiFi} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 43228WINSTA4-NWiFi -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 43228-Win10} -ap {4718/4331} -sta {43228WINSTA4-NWiFi} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
}

proc nightly_rvr_nwifi {} {
    UTFD::metascript %AUTO% -watch 4356WINSTA6-NWiFi -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4356-Win10} -ap {4718/4331} -sta {4356WINSTA6-NWiFi} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4352WINSTA5-NWiFi -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4352-Win10} -ap {4718/4331} -sta {4352WINSTA5-NWiFi} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 43228WINSTA4-NWiFi -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {43228-Win10} -ap {4718/4331} -sta {43228WINSTA4-NWiFi} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
}

proc loadtestscripts {} {
    UTFD::metascript %AUTO% -watch 4356WINSTA6 -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4356-Win10} -ap {4718/4331} -sta {4356WINSTA6} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4352WINSTA5 -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4352-Win10} -ap {4718/4331} -sta {4352WINSTA5} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 43228WINSTA4 -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 43228-Win10} -ap {4718/4331} -sta {43228WINSTA4} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    #UTFD::metascript %AUTO% -watch 4356WINSTA6-NWiFi -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4356-Win10} -ap {4718/4331} -sta {4356WINSTA6-NWiFi} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    #UTFD::metascript %AUTO% -watch 4352WINSTA5-NWiFi -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 4352-Win10} -ap {4718/4331} -sta {4352WINSTA5-NWiFi} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    #UTFD::metascript %AUTO% -watch 43228WINSTA4-NWiFi -script "Test/StaNightly.test -utfconf utfconf/mc87.tcl -title {mc87:StaNightly 43228-Win10} -ap {4718/4331} -sta {43228WINSTA4-NWiFi} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4356WINSTA6 -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4356-Win10} -ap {4718/4331} -sta {4356WINSTA6} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 4352WINSTA5 -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4352-Win10} -ap {4718/4331} -sta {4352WINSTA5} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    UTFD::metascript %AUTO% -watch 43228WINSTA4 -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {43228-Win10} -ap {4718/4331} -sta {43228WINSTA4} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    #UTFD::metascript %AUTO% -watch 4356WINSTA6-NWiFi -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4356-Win10} -ap {4718/4331} -sta {4356WINSTA6-NWiFi} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    #UTFD::metascript %AUTO% -watch 4352WINSTA5-NWiFi -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {4352-Win10} -ap {4718/4331} -sta {4352WINSTA5-NWiFi} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
    #UTFD::metascript %AUTO% -watch 43228WINSTA4-NWiFi -script "Test/RvRNightly1.test -utfconf utfconf/mc87.tcl -title {43228-Win10} -ap {4718/4331} -sta {43228WINSTA4-NWiFi} -va {af1} -attngrp {G2} -email hnd-utf-list@broadcom.com" -type triggered -watchinterval 600
}
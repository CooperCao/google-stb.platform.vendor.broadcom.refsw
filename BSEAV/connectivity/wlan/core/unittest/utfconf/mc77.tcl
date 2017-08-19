# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum MC77 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package require UTF::TclReadLines

UTF::Power::WebRelay usbwr -relay mc77end1 -lan_ip 172.19.12.33

# default devicemode, default=0
global env
set nvramAppendList {}
if {[info exist env(DEVICEMODE)] && ($env(DEVICEMODE) != "0")} {
    append nvramAppendList "devicemode=$env(DEVICEMODE) "
}

UTF::Cygwin VWstation -lan_ip 172.19.14.239 -user user -sta {}

set UTF::DataRatePlot 1
set UTF::WebTree 1

# Added by kshum
#set UTF::TestSigning 1

# Parse logs

# Define Sniffer
UTF::Sniffer mc69tst3 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30

#./UTF.tcl 4360w8x64 load /projects/hnd/swbuild/build_windows/trunk/win8x_internal_wl/2014.10.27.0/release/Win8X/checked/DriverOnly/bcmwl63.inf
 
#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl

#If you want EmbeddedNightly.test to do log parsing, you need to add the statement below to your config file. This is in addition to the above statement.

set ::UTF::PostTestHook {
    package require UTF::utils
    UTF::do_post_test_analysis [info script] ""}

#To use LSF compute farm to offload your endpoint, add:

set ::aux_lsf_queue sj-hnd



# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# ChannelSweep with up/dn traffic
set UTF::ChannelPerf 1

# SummaryDir sets the location for test results

set ::UTF::SummaryDir "/projects/hnd_svt_ap5/$::env(LOGNAME)/mc77"

# Define power controllers on cart
#UTF::Power::Synaccess mc77npc1 -lan_ip 172.19.14.23 -relay mc77end1 -rev 1
#UTF::Power::Synaccess mc77npc2 -lan_ip 172.19.14.25 -relay mc77end1 -rev 1
UTF::Power::Synaccess mc77npc1 -lan_ip 172.19.14.23
UTF::Power::Synaccess mc77npc2 -lan_ip 172.19.14.25 -rev 1
UTF::Power::Synaccess mc77npc3 -lan_ip 172.19.14.26

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.14.24 \
    -relay "mc77end1" \
    -group {G1 {1 2 3} ALL {1 2 3 4 5 6}}

#UTF::Aeroflex 172.19.14.24 -group {G1 {1 2 3} G2 {4 5 6}} -relay "mc77end1"
G1 configure -default 5
ALL configure -default 5
#G2 configure -default 25

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default;
    #G1 attn default
    #G2 attn default

    foreach S {4360f15 4360w8x64} {
	catch {$S wl down}
	$S deinit
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {4708/43602MCH2 4708/43602MCH5 4709/4360MCH2 4709/4360MCH5 4709/43602MCH2 4709/43602MCH5 Atlas/MCH5h Atlas/MCH5l Atlas/MCH2} {
	catch {\
        $S apshell wl -i [$S cget -device] down
        if 0 {
        $S apshell nvram set wl0_taf_enable=1 
        $S apshell nvram set wl1_taf_enable=1 
        $S apshell nvram set wl_taf_enable=1
        $S apshell nvram commit
        };# if 0
    }
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #4708 restart wl0_radio=0
    #4709 restart wl0_radio=0
    #Atlas restart wl0_radio=0
    foreach dev "mc77end1 mc77tst2" {
        catch {
        $dev sysctl -w net.core.rmem_max="16000000"
        $dev sysctl -w net.core.wmem_max="16000000"
        $dev sysctl -w net.ipv4.tcp_rmem="4096 87380 16000000"
        $dev sysctl -w net.ipv4.tcp_wmem="4096 87380 16000000"
        $dev sysctl -w net.core.netdev_max_backlog="3000"
        $dev echo bic > /proc/sys/net/ipv4/tcp_congestion_control
        }
    }

    # delete myself
    #unset ::UTF::SetupTestBed

    return
}
    #-pre_perf_hook {{4706/4360 wl 5g_rate -v 6x3 -l -g} {4706/4360 wl nrate}}



 set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl offloads 0} {%S wl up} {%S wl mpc 0}\
            {%S wl isup} {%S wl msglevel +error +assoc} {%S services stop}\
            {%S wl stbc_rx 1} {%S wl stbc_tx 1} {%S wl nphy_percal 1}\
            {%S wl sgi_rx 3} {%S wl sgi_tx -1} {%S wl PM 0}\
            {%S wl roam_trigger -100 all}}


# UTF Endpoint1 f11 - Traffic generators (no wireless cards)
UTF::Linux mc77end1 \
    -sta {lan eth1}
lan configure -ipaddr 192.168.1.90

UTF::Linux mc77end2 \
    -sta {lan2 enp0s29u1u4c2}
lan2 configure -ipaddr 192.168.1.96

if {[info exist env(ACTIVE_AP)]} {
    set active_ap $env(ACTIVE_AP)
} else {
    set active_ap 4709
}

UTF::Cygwin mc77tst1 \
        -user user \
        -sta {4360w8x64} \
        -node {DEV_43A0} \
        -perfchans {36/80 36l 3} \
        -power {mc77npc2 1} \
        -power_button "auto" \
        -wlinitcmds {wl down; wl msglevel +error +assoc; wl vht_features 3; wl up} \
        -tcpwindow 4M \
        -installer inf\
        -tag "BISON05T_REL_7_35_182" \
        -brand win8x_internal_wl \
        -osver 8164 \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S ifconfig}} \
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl scansuppress 0} \
        {%S wl dump scb} {%S wl counters} {%S wl dump ampdu} {%S ifconfig}}

4360w8x64 configure -attngrp G1

UTF::Linux mc77tst2 \
        -lan_ip mc77tst2 \
        -sta {4360f15 eth0} \
        -tcpwindow 4M \
        -perfchans {36/80 36l 3} \
        -wlinitcmds {wl down; wl msglevel +error +assoc; wl vht_features 3; wl up} \
        -tag {BISON05T_REL_7_35_100} \
        -power {mc77npc2 2} \
        -power_button "auto" \
        -nobighammer 0 \
        -brand linux-internal-wl \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S ifconfig}} \
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl scansuppress 0} \
        {%S wl dump scb} {%S wl counters} {%S wl dump ampdu} {%S ifconfig}\
        {%S wl ratedump} {%S wl dump phycal}}

4360f15 configure -attngrp G1

set nvramAtlas {
    "fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
    gmac3_enable=1
}

set nvram4708 {
    watchdog=3000
    wl_msglevel=0x101
    console_loglevel=7
    wl0_ssid=Broadcom
    wl0_chanspec=3
    wl0_obss_coex=0
    wl0_bw_cap=-1
    wl0_radio=0
    wl1_ssid=Broadcom
    wl1_chanspec=36
    wl1_obss_coex=0
    wl1_bw_cap=-1                                                                                                  
    wl1_radio=0
    lan_ipaddr=192.168.1.10
    lan_gateway=192.168.1.10
}

if {[llength nvramAppendList]} {
    append nvram4708 $nvramAppendList
}

UTF::Router 4708 -sta {4708/4360MCH2 eth1 4708/4360MCH2.%15 wl0.% 4708/4360MCH5 eth2 4708/4360MCH5.%15 wl1.%} \
    -relay lan \
    -lan_ip 192.168.1.10 \
    -power {mc77npc3 1} \
    -lanpeer lan \
    -console "mc77end1:40003" \
    -tag "BISON04T_{REL,BRANCH}_7_14{,_??,_???}" \
    -brand "linux-2.6.36-arm-external-vista-router-full-src" \
    -nvram $nvram4708 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 15-120 -attn2g 40-120 -pad 10} \
    -noradio_pwrsave 1 -perfchans {36/80 3} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl reset_cnts} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} {%S wl pktq_stats C:00:10:18:E2:E9:99}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info 00:10:18:E2:F2:BE} {%S wl rssi 00:10:18:E2:F2:BE} \
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} \
    {%S wl sta_info 00:10:18:E2:E9:99} {%S wl rssi 00:10:18:E2:E9:99} \
    {%S wl pktq_stats C:00:10:18:E2:E9:99}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

set nvram4709 {
    watchdog=3000
    wl_msglevel=0x101
    console_loglevel=7
    wl0_ssid=4709/43602MCH2
    wl0_chanspec=36
    wl0_obss_coex=0
    wl0_bw_cap=-1
    wl0_radio=0
    wl1_ssid=4709/43602MCH5
    wl1_chanspec=11
    wl1_obss_coex=0
    wl1_bw_cap=-1
    wl1_radio=0
    lan_ipaddr=192.168.1.20
    lan_gateway=192.168.1.20
    wl0_vht_features=3
    wl1_vht_features=3
}

if {[llength nvramAppendList]} {
    append nvram4709 $nvramAppendList
}
UTF::Router 4709 -sta {4709/43602MCH2 eth1 4709/43602MCH2.%15 wl0.% 4709/43602MCH5 eth2 4709/43602MCH5.%15 wl1.%} \
    -relay lan \
    -lan_ip 192.168.1.20 \
    -power {mc77npc1 2} \
    -lanpeer lan \
    -console "mc77end1:40002" \
    -tag "BISON04T_{REL,BRANCH}_7_14{,_??,_???}" \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" \
    -nvram $nvram4709 \
    -datarate {-i 0.5 -frameburst 1} -udp 2.0G \
    -yart {-frameburst 1 -attn5g 20-120 -attn2g 30-120 -pad 10} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl reset_cnts} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} {%S wl pktq_stats C:00:10:18:E2:E9:99}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl reset_cnts}\
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl txmaxpkts} {%S wl dump ampdu}\
    {%S wl assoclist} {%S wl sta_info 00:10:18:E2:F2:BE} {%S wl rssi 00:10:18:E2:F2:BE} \
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} \
    {%S wl sta_info 00:10:18:E2:E9:99} {%S wl rssi 00:10:18:E2:E9:99} \
    {%S wl pktq_stats C:00:10:18:E2:E9:99}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

set nvram4709A {
    watchdog=3000
    wl_msglevel=0x101                                                                                               
    console_loglevel=7                                                                                              
    wl0_ssid=4709A/4360MCH2                                                                                          
    wl0_chanspec=36                                                                                                 
    wl0_obss_coex=0                                                                                                 
    wl0_bw_cap=-1                                                                                                   
    wl0_radio=0                                                                                                     
    wl1_ssid=4709A/4360MCH5                                                                                          
    wl1_chanspec=11                                                                                                 
    wl1_obss_coex=0                                                                                                 
    wl1_bw_cap=-1                                                                                                   
    wl1_radio=0                                                                                                     
    lan_ipaddr=192.168.1.40                                                                                         
    lan_gateway=192.168.1.40      
}
if {[llength nvramAppendList]} {
    append nvram4709A $nvramAppendList
}

UTF::Router 4709A -sta {4709/4360MCH2 eth1 4709/4360MCH2.%15 wl1.% 4709/4360MCH5 eth2 4709/4360MCH5.%15 wl0.%} \
    -relay lan \
    -lan_ip 192.168.1.40 \
    -power {mc77npc3 2} \
    -lanpeer lan \
    -console "mc77end1:40004" \
    -tag "BISON04T_{REL,BRANCH}_7_14{,_??,_???}" \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" \
    -nvram $nvram4709A \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 15-120 -attn2g 40-120 -pad 10} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl reset_cnts} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} {%S wl pktq_stats C:00:10:18:E2:E9:99}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl rrm}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info 00:10:18:E2:F2:BE} {%S wl rssi 00:10:18:E2:F2:BE} \
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} \
    {%S wl sta_info 00:10:18:E2:E9:99} {%S wl rssi 00:10:18:E2:E9:99} \
    {%S wl pktq_stats C:00:10:18:E2:E9:99} {%S wl rrm}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

set nvramAtlas {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=Atlas/MCH2
	wl0_chanspec=11
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=Atlas/MCH5l
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
	wl2_ssid=Atlas/MCH5h
	wl2_chanspec=161
	wl2_obss_coex=0
	wl2_bw_cap=-1
	wl2_radio=0
    lan_ipaddr=192.168.1.30
    lan_gateway=192.168.1.30
}
if {[llength nvramAppendList]} {
    append nvramAtlas $nvramAppendList
}
UTF::Router Atlas -sta {Atlas/MCH5h eth3 Atlas/MCH5h.%15 wl2.% Atlas/MCH2 eth1 Atlas/MCH2.%15 wl0.% Atlas/MCH5l eth2 Atlas/MCH5l.%15 wl1.%} \
    -relay lan \
    -lan_ip 192.168.1.30 \
    -power {mc77npc1 1} \
    -lanpeer lan \
    -console "mc77end1:40001" \
    -tag "BISON04T_BRANCH_7_14{,_?*}" \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src" \
    -nvram $nvramAtlas \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 15-120 -attn2g 40-120 -pad 10} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl reset_cnts} {%S wl assoc} {%S wl ampdu_clear_dump} \
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} {%S wl pktq_stats C:00:10:18:E2:E9:99}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info 00:10:18:E2:F2:BE} {%S wl rssi 00:10:18:E2:F2:BE} \
    {%S wl pktq_stats C:00:10:18:E2:F2:BE} \
    {%S wl sta_info 00:10:18:E2:E9:99} {%S wl rssi 00:10:18:E2:E9:99} \
    {%S wl pktq_stats C:00:10:18:E2:E9:99}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

# Aardvark STA clones
if {[info exist env(TAG_A)]} {
    set tag_a $env(TAG_A)
} else {
    set tag_a AARDVARK01T_REL_6_37_14_119
}
puts "#### tag_a = $tag_a ####"

4708/4360MCH5 clone 4708/4360MCH5a -tag $tag_a \
    -brand "linux-2.6.36-arm-external-vista-router-full-src" \
    -perfchans {36/80 36l 36}
4708/4360MCH2 clone 4708/4360MCH2a -tag $tag_a \
    -brand "linux-2.6.36-arm-external-vista-router-full-src" \
    -perfchans {6l 6}
4709/43602MCH5 clone 4709/43602MCH5a -tag $tag_a \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" \
    -perfchans {36/80 36l 36}
4709/43602MCH2 clone 4709/43602MCH2a -tag $tag_a \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" \
    -perfchans {6l 6}

# Bison STA clones
if {[info exist env(AP_IMAGE)]} {
    4708/4360MCH5 clone 4708/4360MCH5b \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4708/4360MCH2 clone 4708/4360MCH2b \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    4708/4360MCH5 configure -dualband {4708/4360MCH2 -c1 161/80 -c2 1l}

    4709/4360MCH5 clone 4709/4360MCH5b \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4709/4360MCH2 clone 4709/4360MCH2b \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    4709/4360MCH5 configure -dualband {4709/4360MCH2 -c1 161/80 -c2 1l}

    4709/43602MCH5 clone 4709/43602MCH5b \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4709/43602MCH2 clone 4709/43602MCH2b \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    4709/43602MCH5 configure -dualband {4709/43602MCH2 -c1 161/80 -c2 1l \
        -lan1 lan -lan2 lan2}
    
    Atlas/MCH5h clone Atlas/MCH5h-aspm-default \
        -image $env(AP_IMAGE) \
        -perfchans {161/80 161u 161}
    Atlas/MCH5l clone Atlas/MCH5l-aspm-default \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    Atlas/MCH2 clone Atlas/MCH2-aspm-default \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    Atlas/MCH5h clone Atlas/MCH5h-aspm-always\
        -image $env(AP_IMAGE) \
        -nvram "$nvramAtlas aspmd_alltime_active=1" \
        -perfchans {161/80 161u 161}
    Atlas/MCH5l clone Atlas/MCH5l-aspm-always\
        -image $env(AP_IMAGE) \
        -nvram "$nvramAtlas aspmd_alltime_active=1" \
        -perfchans {36/80 36l 36}
    Atlas/MCH2 clone Atlas/MCH2-aspm-always\
        -image $env(AP_IMAGE) \
        -nvram "$nvramAtlas aspmd_alltime_active=1" \
        -perfchans {6l 6}
} else {
if {[info exist env(TAG_B)]} {
    set tag_b $env(TAG_B)
} else {
    set tag_b BISON04T_REL_7_14_131_{??}
}
puts "#### tag_b = $tag_b ####"

4708/4360MCH5 clone 4708/4360MCH5b1 -tag $tag_b \
    -brand "linux-2.6.36-arm-external-vista-router-full-src" \
    -perfchans {36/80 36l 36}\
    -nvram {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=Broadcom
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=Broadcom
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
    lan_ipaddr=192.168.1.10
    lan_gateway=192.168.1.10
    } \
    -postboot {
    sysctl -w kernel.panic=1 kernel.panic_on_oops=1
    nvram set devicemode=1
    }

if {[info exist env(BRAND)]} {
    set brand $env(BRAND)
} else {
    set brand "linux-2.6.36-arm-external-vista-router-full-src"
}

4708/4360MCH2 clone 4708/4360MCH2b1 -tag $tag_b \
    -brand $brand \
    -perfchans {6l 6}
4708/4360MCH5 clone 4708/4360MCH5b -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
4708/4360MCH2 clone 4708/4360MCH2b -tag $tag_b \
    -brand $brand \
    -perfchans {6l 6}
4708/4360MCH5 configure -dualband {4708/4360MCH2 -c1 161/80 -c2 1l \
    -lan1 lan -lan2 lan2}

if {[info exist env(BRAND)]} {
    set brand $env(BRAND)
} else {
    set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
}
puts "#### tag_b = $tag_b ####"
4709/43602MCH5 clone 4709/43602MCH5b -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709" \
    -perfchans {36/80 36l 36}
4709/43602MCH2 clone 4709/43602MCH2b -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709" \
    -perfchans {6l 6}
4709/43602MCH5 configure -dualband {4709/43602MCH2 -c1 161/80 -c2 1l \
    -lan1 lan -lan2 lan2}

if {[info exist env(BRAND)]} {
    set brand $env(BRAND)
} else {
    set brand "linux-2.6.36-arm-external-vista-router-full-src"
}

4709/4360MCH5 clone 4709/4360MCH5b -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
4709/4360MCH2 clone 4709/4360MCH2b -tag $tag_b \
    -brand $brand \
    -perfchans {6l 6}
4709/4360MCH5 configure -dualband {4709/4360MCH2 -c1 161/80 -c2 1l}

if {[info exist env(BRAND)]} {
    set brand $env(BRAND)
} else {
    set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
}
Atlas/MCH5h clone Atlas/MCH5hb -tag $tag_b \
    -brand $brand \
    -perfchans {161 161u 161/80} -channelsweep {-min 100}
Atlas/MCH5l clone Atlas/MCH5lb -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36} -channelsweep {-max 64}
Atlas/MCH2 clone Atlas/MCH2b -tag $tag_b \
    -brand $brand \
    -perfchans {6l 6}

Atlas/MCH5h configure -dualband {Atlas/MCH2 -c1 161/80 -c2 1l}
Atlas/MCH5l configure -dualband {Atlas/MCH2 -c1 36/80 -c2 1l}

if {[info exist env(BRAND)]} {
    set brand $env(BRAND)
} else {
    set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
}
Atlas/MCH5h clone Atlas/MCH5hbi -tag $tag_b \
    -brand $brand \
    -perfchans {161 161u 161/80} -channelsweep {-min 100}
Atlas/MCH5l clone Atlas/MCH5lbi -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36} -channelsweep {-max 64}
Atlas/MCH2 clone Atlas/MCH2bi -tag $tag_b \
    -brand $brand \
    -perfchans {6l 6}
Atlas/MCH5h configure -dualband {Atlas/MCH2 -c1 161/80 -c2 1l}
Atlas/MCH5l configure -dualband {Atlas/MCH2 -c1 36/80 -c2 1l}
}

# Dingo STA clones
if {[info exist env(TAG_D)]} {
    set tag_d $env(TAG_D)
} else {
    set tag_d "DINGO_{BRANCH,REL}_9_10{,_?*}"
}
puts "#### tag_d = $tag_d ####"

#set brand "linux-2.6.36-arm-internal-router"
set brand "linux-2.6.36-arm-external-vista-router-full-src"
4708/4360MCH5 clone 4708/4360MCH5d -tag $tag_d \
    -brand $brand \
    -perfchans {36/80 36l 36}
4708/4360MCH2 clone 4708/4360MCH2d -tag $tag_d \
    -brand $brand \
    -perfchans {6l 6}
set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src"
4709/43602MCH5 clone 4709/43602MCH5d -tag $tag_d \
    -brand $brand \
    -perfchans {36/80 36l 36}
4709/43602MCH2 clone 4709/43602MCH2d -tag $tag_d \
    -brand $brand \
    -perfchans {6l 6}
#set brand "linux-2.6.36-arm-external-vista-router-full-src"
set brand linux-2.6.36-arm-up-internal-router
4709/4360MCH5 clone 4709/4360MCH5d -tag $tag_d \
    -brand $brand \
    -perfchans {36/80 36l 36}
4709/4360MCH2 clone 4709/4360MCH2d -tag $tag_d \
    -brand $brand \
    -perfchans {6l 6}
set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
Atlas/MCH5h clone Atlas/MCH5hd -tag $tag_d \
    -brand $brand \
    -perfchans {161 161u 161/80} -channelsweep {-min 100}
Atlas/MCH5l clone Atlas/MCH5ld -tag $tag_d \
    -brand $brand \
    -perfchans {36/80 36l 36} -channelsweep {-max 64}
Atlas/MCH2 clone Atlas/MCH2d -tag $tag_d \
    -brand $brand \
    -perfchans {6l 6}

# Eagle STA clones
if {[info exist env(TAG_E)]} {
    set tag_e $env(TAG_E)
} else {
    set tag_e EAGLE_{BRANCH,REL}_10_10{,_?*}
}
puts "#### tag_e = $tag_e ####"

set brand "linux-2.6.36-arm-external-vista-router-full-src"
4708/4360MCH5 clone 4708/4360MCH5e -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}
4708/4360MCH2 clone 4708/4360MCH2e -tag $tag_e \
    -brand $brand \
    -perfchans {6l 6}
set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src"
4709/43602MCH5 clone 4709/43602MCH5e -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}
4709/43602MCH2 clone 4709/43602MCH2e -tag $tag_e \
    -brand $brand \
    -perfchans {6l 6}
set brand "linux-2.6.36-arm-external-vista-router-full-src"
4709/4360MCH5 clone 4709/4360MCH5e -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}
4709/4360MCH2 clone 4709/4360MCH2e -tag $tag_e \
    -brand $brand \
    -perfchans {6l 6}
set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
Atlas/MCH5h clone Atlas/MCH5he -tag $tag_e \
    -brand $brand \
    -perfchans {161 161u 161/80} -channelsweep {-min 100}
Atlas/MCH5l clone Atlas/MCH5le -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36} -channelsweep {-max 64}
Atlas/MCH2 clone Atlas/MCH2e -tag $tag_e \
    -brand $brand \
    -perfchans {6l 6}


set UTF::RouterNightlyCustom {

    if {$STA3 ne "" && [regexp {(.*x)/0} $Router - base]} {

	package require UTF::Test::TripleBand

    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
        -c1 44/80 -c2 3l -c3 157/80 -lan1 lan -lan2 lan2 -lan3 lan3

    }
}

set UTF::ChannelPerf 1
set UTF::KPPSSweep 1
UTF::Q mc77

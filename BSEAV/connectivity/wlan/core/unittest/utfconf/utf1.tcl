# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum utf1 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package require UTF::TclReadLines

#UTF::Cygwin VWstation -lan_ip 172.19.14.239 -user user -sta {}

set UTF::DataRatePlot 1
set UTF::WebTree 1
set UTF::TestSigning true

set nvramMU "wl0_mu_features=0x8000"

#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl

#If you want EmbeddedNightly.test to do log parsing, you need to add the statement below to your config file. This is in addition to the above statement.

set ::UTF::PostTestHook {
    package require UTF::utils
    UTF::do_post_test_analysis [info script] ""}

#To use LSF compute farm to offload your endpoint, add:

set ::aux_lsf_queue sj-hnd

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results

set ::UTF::SummaryDir "/projects/hnd_svt_ap8/$::env(LOGNAME)/utf1"
 

# Define power controllers on cart
UTF::Power::Synaccess fz01npc1 -lan_ip 172.19.12.4 -relay svt-utf1-end1 -rev 1
UTF::Power::Synaccess fz01npc2 -lan_ip 172.19.12.6 -relay svt-utf1-end1 -rev 1
 
 
# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.12.55 \
    -relay "lan" \
    -group {G1 {1 2 3 4} ALL {1 2 3 4 5 6}}

G1 configure -default 20 

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default;

    foreach S {4360f19s3 4366f19s4} {
	catch {$S wl down}
	$S deinit
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {53574r1g 53574r1a 4366MCM5 47189_2G} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP1 restart wl0_radio=0
# Made AP2 => AP1 since we have only one AP
 

    
    # delete myself
    unset ::UTF::SetupTestBed

    return
}
    #-pre_perf_hook {{4706/4360 wl 5g_rate -v 6x3 -l -g} {4706/4360 wl nrate}}


set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl offloads 0} {%S wl up} {%S wl mpc 0}\
            {%S wl isup} {%S wl msglevel +error +assoc} {%S services stop}\
            {%S wl stbc_rx 1} {%S wl stbc_tx 1} {%S wl nphy_percal 1}\
            {%S wl sgi_rx 3} {%S wl sgi_tx -1} {%S wl PM 0}\
            {%S wl roam_trigger -100 all}}

if {0} {
 set ::rvr_sta_init {{%S wl stbc_rx 1} {%S wl stbc_tx 1}}

 set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl offloads 0} {%S wl up} {%S wl mpc 0}\
            {%S wl isup} {%S wl msglevel +error +assoc} {%S services stop}\
            {%S wl stbc_rx 1} {%S wl stbc_tx 1} {%S wl nphy_percal 1}\
            {%S wl sgi_rx 3} {%S wl sgi_tx -1} {%S wl PM 0}\
            {%S wl roam_trigger -100 all}}
};# if 0


# UTF Endpoint1 fc15 - Traffic generators ( 
UTF::Linux svt-utf1-end1 \
    -sta {lan eth1} 
lan configure -ipaddr 192.168.1.97
 

if {[info exist env(ACTIVE_AP)]} {
    set active_ap $env(ACTIVE_AP)
} else {
    set active_ap 47189 
}

UTF::Linux svt-utf1-st1  \
        -lan_ip svt-utf1-st1 \
        -sta {4360f19s3 enp2s0} \
        -tcpwindow 4M \
        -perfchans {36/80 36l 3l} \
        -wlinitcmds {wl msglevel +assoc +error} \
        -power {fz01npc2 1} \
        -power_button "auto" \
        -nobighammer 0 -tcpwindow 4M \
        -tag BISON05T_REL_7_35_100 \
        -brand linux-internal-wl \
        -iperfdaemon false \
        -pre_perf_hook {{%S wl reset_cnts} {%S wl scansuppress 1} \
         {%S wl ampdu_clear_dump} {%S wl reset_cnts}}\
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl scansuppress 0}\
         {%S wl dump scb} {%S wl counters} {%S wl dump ampdu}}

4360f19s3 clone 4360f19s2 \
        -wlinitcmds {wl msglevel +assoc +error; wl txchain 3; wl rxchain 3} 
4360f19s3 configure -attngrp G1
4360f19s2 configure -attngrp G1
4360f19s2 clone 4360f19s2a \
        -wlinitcmds {wl msglevel +assoc +error; wl txchain 5; wl rxchain 5} 
4360f19s2a configure -attngrp G1

UTF::Linux svt-utf1-st2  \
        -lan_ip svt-utf1-st2 \
        -sta {4366f19s4 enp2s0} \
        -tcpwindow 4M \
        -perfchans {36/80 36 3l} \
        -wlinitcmds {wl msglevel +error +assoc} \
        -power {fz01npc2 2} \
        -power_button "auto" \
        -nobighammer 0 -tcpwindow 4M \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -date {2016.9.29.0} \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl scansuppress 0}\
         {%S wl dump scb} {%S wl counters} {%S wl dump ampdu}}
4366f19s4 clone 4366f19s2 \
        -wlinitcmds {wl msglevel +assoc +error; wl txchain 3; wl rxchain 3} 
4366f19s4 clone 4366f19s3 \
        -wlinitcmds {wl msglevel +assoc +error; wl txchain 7; wl rxchain 7} 
4366f19s4 configure -attngrp G1
4366f19s3 configure -attngrp G1
4366f19s2 configure -attngrp G1

if 0 {
UTF::Linux svt-utf1-st3  \
        -lan_ip svt-utf1-st3 \
        -sta {4360f19s3 enp2s0} \
        -tcpwindow 4M \
        -perfchans {36/80 36 3l} \
        -wlinitcmds {wl msglevel +assoc +errorrr}\
        -power {fz01npc2 2} \
        -power_button "auto" \
        -nobighammer 0 -tcpwindow 4M \
        -tag BISON05T_REL_7_35_100 \
        -brand linux-internal-wl \
        -iperfdaemon false \
        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}\
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl scansuppress 0}\
         {%S wl dump scb} {%S wl counters} {%S wl dump ampdu}}

4360f19s3 configure -attngrp G1
4360f19s3 clone 4360f19s2 \
    -wlinitcmds {wl msglevel +assoc +error; wl txchain 3; wl rxchain 3}
};#if 0

set nvram53574r1 {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=53574-2G
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
    wl0_country_code=Q1
    wl0_country_rev=27
	wl1_ssid=53574-5G
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
    wl1_country_code=Q1
    wl1_country_rev=27
    lan_ipaddr=192.168.1.10
    lan_gateway=192.168.1.10
}
set nvram53574r0 {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=53574
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
    wl0_country_code=Q1
    wl0_country_rev=27
    lan_ipaddr=192.168.1.10
    lan_gateway=192.168.1.10
}

set nvramRSDB {
    rsdb_mode=-1
}
if {[info exists nvramAppendList]} {
    append nvram53574r0 $nvramAppendList
    append nvram53574r1 $nvramAppendList
}

UTF::Router 53574 -sta {53574r0 eth1 53574r0.%15 wl0.%} \
    -relay svt-utf1-end1 \
    -lan_ip 192.168.1.10 \
    -power {fz01npc1 1} \
    -lanpeer lan \
    -console "svt-utf1-end1:40001" \
    -tag "DINGO_REL_9_10{,_?*}" \
    -brand "linux-2.6.36-arm-up-internal-router-rsdb" \
    -nvram $nvram53574r0 \
    -wlinitcmds {sleep 10} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 20-120 -attn2g 40-120 -pad 30} \
    -noradio_pwrsave 1 -perfchans {6 6l 36 36l 36/80} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl pktq_stats C:00:10:18:e2:e9:78} {%S wl pktq_stats C:00:10:18:EE:CC:14}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S et counters} {%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info 00:10:18:e2:e9:78} {%S wl rssi 00:10:18:e2:e9:78} \                     
    {%S wl pktq_stats C:00:10:18:e2:e9:78} {%S wl pktq_stats C:00:10:18:e2:e9:78}\
    {%S wl sta_info 00:10:18:EE:CC:14} {%S wl rssi 00:10:18:EE:CC:14} \                     
    {%S wl pktq_stats C:00:10:18:EE:CC:14}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}
53574r0 configure -dualband {-c1 161/80 -c2 6l}

UTF::Router 53574r1 -sta {53574r1g eth1 53574r1g.%15 wl0.% 53574r1a eth2 53574r1a.%15 wl1.%} \
    -relay svt-utf1-end1 \
    -lan_ip 192.168.1.10 \
    -power {fz01npc1 1} \
    -lanpeer lan \
    -console "svt-utf1-end1:40001" \
    -tag "DINGO_REL_9_10{,_?*}" \
    -brand "linux-2.6.36-arm-up-internal-router-rsdb" \
    -nvram "$nvram53574r1 $nvramRSDB" \
    -wlinitcmds {sleep 5} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 20-120 -attn2g 40-120 -pad 30} \
    -noradio_pwrsave 1 -perfchans {6 6l 36 36l 36/80} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl pktq_stats C:00:10:18:e2:e9:78} {%S wl pktq_stats C:00:10:18:EE:CC:14}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S et counters} {%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info 00:10:18:e2:e9:78} {%S wl rssi 00:10:18:e2:e9:78} \                     
    {%S wl pktq_stats C:00:10:18:e2:e9:78} {%S wl pktq_stats C:00:10:18:e2:e9:78}\
    {%S wl sta_info 00:10:18:EE:CC:14} {%S wl rssi 00:10:18:EE:CC:14} \                     
    {%S wl pktq_stats C:00:10:18:EE:CC:14}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}
53574r1a configure -dualband {53574r1g -c1 161/80 -c2 6l}

set nvram47189 {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=47189-5G
	wl0_chanspec=36
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
    wl0_country_code=Q1
    wl0_country_rev=27
    wl0_mu_features=0x8000
	wl1_ssid=47189-2G
	wl1_chanspec=6
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
    wl1_country_code=Q1
    wl1_country_rev=27
    lan_ipaddr=192.168.1.20
    lan_gateway=192.168.1.20
}

if {[info exists nvramAppendList]} {
    append nvram47189 $nvramAppendList
}

UTF::Router 47189 -sta {4366MCM5 eth2 4366MCM5.%15 wl0.% 47189_2G eth3 47189_2G.%15 wl1.%} \
    -relay svt-utf1-end1 \
    -lan_ip 192.168.1.20 \
    -power {fz01npc1 2} \
    -lanpeer lan \
    -console "svt-utf1-end1:40002" \
    -tag "DINGO_{REL,TWIG}_9_10_178{,_?*}" \
    -brand "linux-2.6.36-arm-external-vista-router-full-src" \
    -nvram $nvram47189 \
    -wlinitcmds {sleep 4} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 40-100 -attn2g 40-100 -pad 30} \
    -noradio_pwrsave 1 -perfchans {6 6l 36 36l 36/80} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl pktq_stats C:[4366f19s4 macaddr]} {%S wl pktq_stats C:[4360f19s2 macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb} \
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S et counters} {%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info [4366f19s4 macaddr]} {%S wl rssi [4366f19s4 macaddr]} \                     
    {%S wl pktq_stats C:[4366f19s4 macaddr]}\
    {%S wl sta_info [4360f19s2 macaddr]} {%S wl rssi [4360f19s2 macaddr]} \                     
    {%S wl pktq_stats C:[4360f19s2 macaddr]}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

4366MCM5 configure -dualband {47189_2G -c1 161/80 -c2 6l}

set nvram53573 {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=53573-5G
	wl0_chanspec=36
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=53573-2G
	wl1_chanspec=3
	wl1_obss_coex=0
	wl1_bw_cap=-1
    wl0_country_code=Q1
    wl0_country_rev=27
    wl1_country_code=Q1
    wl1_country_rev=27
    lan_ipaddr=192.168.1.30
    lan_gateway=192.168.1.30
}

if {[info exists nvramAppendList]} {
    append nvram53573 $nvramAppendList
}

UTF::Router 53573 -sta {53573r0g eth1 53574r0g.%15 wl0.% 53573r0a eth2 53574r0a.%15 wl1.%} \
    -relay svt-utf1-end1 \
    -lan_ip 192.168.1.30 \
    -power {fz01npc1 1} \
    -lanpeer lan \
    -console "svt-utf1-end1:40001" \
    -tag "DINGO_REL_9_10{,_?*}" \
    -brand "linux-2.6.36-arm-up-internal-router-rsdb" \
    -nvram $nvram53573 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 20-120 -attn2g 40-120 -pad 30} \
    -noradio_pwrsave 1 -perfchans {6 6l 36 36l 36/80} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl pktq_stats C:00:10:18:e2:e9:78} {%S wl pktq_stats C:00:10:18:EE:CC:14}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S et counters} {%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl assoclist} {%S wl sta_info 00:10:18:e2:e9:78} {%S wl rssi 00:10:18:e2:e9:78} \                     
    {%S wl pktq_stats C:00:10:18:e2:e9:78} {%S wl pktq_stats C:00:10:18:e2:e9:78}\
    {%S wl sta_info 00:10:18:EE:CC:14} {%S wl rssi 00:10:18:EE:CC:14} \                     
    {%S wl pktq_stats C:00:10:18:EE:CC:14}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}
53573r0a configure -dualband {53573r0g -c1 161/80 -c2 6l}

if {[info exist env(AP_IMAGE)]} {
    4366MCM5 clone 4366MCM5i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    47189_2G clone 47189_2G-i \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    53574r0 clone 53574r0i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36 6l 6}
    53574r1a clone 53574r1ai \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    53574r1g clone 53574r1gi \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    53573r0a clone 53573r0ai \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    53573r0g clone 53573r0gi \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    53573r0a clone 53573r1ai -nvram "$nvram53573 $nvramRSDB"
    53573r0g clone 53573r1gi -nvram "$nvram53573 $nvramRSDB"
    53573r0ai configure -dualband {53573r0gi -c1 161/80 -c2 6l}
    53573r1ai configure -dualband {53573r1gi -c1 161/80 -c2 6l}
    53574r0i configure -dualband {53574r0i -c1 161/80 -c2 6l}
    53574r1ai configure -dualband {53574r1gi -c1 161/80 -c2 6l}
    4366MCM5-i configure -dualband {47189_2G-i -c1 161/80 -c2 6l}
} else {
    # Dingo STA clones
    if {[info exist env(TAG_D)]} {
        set tag_d $env(TAG_D)
    } else {
        set tag_d "DINGO_{REL,BRANCH}_9_10_{?*,_?*}"
    }
    puts "#### tag_d = $tag_d ####"

    set brand linux-2.6.36-arm-up-internal-router 
    4366MCM5 clone 4366MCM5-i -tag $tag_d \
        -brand $brand \
        -perfchans {36/80 36l 36}
    47189_2G clone 47189_2G-i -tag $tag_d \
        -brand $brand \
        -perfchans {6l 6}

    set brand linux-2.6.36-arm-up-internal-router-rsdb 
    53574r0 clone 53574r0di -tag $tag_d \
        -brand $brand \
        -perfchans {36/80 36l 36 6l 6}
    53574r0di clone 53574ai-mimo -wlinitcmds {\
        sleep 10; drsdbd_cli -m mimo; sleep 5} \
        -perfchans {36/80 36l 36}
    53574ai-mimo clone 53574gi-mimo -perfchans {6l 6}

    53574r0di clone 53574ai-rsdb -wlinitcmds {sleep 10;\
        drsdbd_cli -m mimo; sleep 5; drsdbd_cli -m rsdb; sleep 5} \
        -perfchans {36/80 36l 36}
    53574ai-rsdb clone 53574gi-rsdb -perfchans {6l 6}

    53574r1a clone 53574ai-dyn-rsdb -tag $tag_d \
        -brand $brand \
        -perfchans {36/80 36l 36}
    53574ai-dyn-rsdb clone 53574gi-dyn-rsdb -perfchans {6l 6}

    53574r1a clone 53574r1adi -tag $tag_d \
        -brand $brand \
        -perfchans {36/80 36l 36}
    53574r1g clone 53574r1gdi -tag $tag_d \
        -brand $brand \
        -perfchans {6l 6}
    53573r0a clone 53573r0adi \
        -brand $brand \
        -perfchans {36/80 36l 36}
    53573r0g clone 53573r0gdi \
        -brand $brand \
        -perfchans {6l 6}
    53573r0a clone 53573r1adi -nvram "$nvram53573 $nvramRSDB"
    53573r0g clone 53573r1gdi -nvram "$nvram53573 $nvramRSDB"

    set brand linux-2.6.36-arm-up-external-vista-router-dhdap-moca-full-src
    4366MCM5-i clone 4366MCM5-e -brand $brand
    4366MCM5-e clone 4366MCM5-mu-e \
        -nvram "$nvram47189 $nvramMU"

    47189_2G-i clone 47189_2G-e -brand $brand

    53573r0adi configure -dualband {53573r0gdi -c1 161/80 -c2 6l}
    53573r1adi configure -dualband {53573r1gdi -c1 161/80 -c2 6l}
    53574r0di configure -dualband {53574r0di -c1 161/80 -c2 6l}
    53574r1adi configure -dualband {53574r1gdi -c1 161/80 -c2 6l}
    4366MCM5-i configure -dualband {47189_2G-i -c1 161/80 -c2 6l}
    4366MCM5-e configure -dualband {47189_2G-e -c1 161/80 -c2 6l}
}

set UTF::RouterNightlyCustom {
    if {$STA3 ne "" && [regexp {(.*x)/0} $Router - base]} {

	package require UTF::Test::TripleBand

    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
        -c1 44/80 -c2 3l -c3 157/80 -lan1 lan -lan2 lan2 -lan3 lan3
    }
}

# ChannelSweep with up/dn traffic                                                          
set UTF::ChannelPerf 1

UTF::Q utf1 

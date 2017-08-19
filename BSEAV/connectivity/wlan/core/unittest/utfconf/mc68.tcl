# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum MC68 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

set UTF::WebTree 1
set UTF::DataRatePlot 1

set nvramMUauto "wl0_mu_features=0x8000 wl1_mu_features=0x8000 wl2_mu_features=0x8000"
set nvramMU "wl0_mu_features=1 wl1_mu_features=1 wl2_mu_features=1"
set nvramSU "wl0_mu_features=0 wl1_mu_features=0 wl2_mu_features=0"
set nvramDM1 "devicemode=1"
set conf1x1 "wl rxchain 1; wl txchain 1"
set conf2x2 "wl rxchain 3; wl txchain 3"
set confSU "wl txbf_bfe_cap 1"
set confSUi "wl txbf_bfe_cap 0"

# Parse logs

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

set ::UTF::SummaryDir "/projects/hnd_svt_ap8/$::env(LOGNAME)/mc68"

# Define power controllers on cart
UTF::Power::Synaccess mc68npc1 -lan_ip mc68npc1 -rev 1 
UTF::Power::Synaccess mc68npc2 -lan_ip mc68npc2 -rev 1
UTF::Power::Synaccess staNPC1 -lan_ip 172.2.1.45 -rev 1
UTF::Power::Synaccess staNPC2 -lan_ip 172.2.1.46 -rev 1
UTF::Power::Synaccess staNPC3 -lan_ip 172.2.1.55 -rev 1
UTF::Power::Synaccess staNPC4 -lan_ip 172.2.1.56 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af1 -lan_ip 172.2.1.80 \
    -relay "mc68end2" \
    -group {G1 {1 2} G2 {3 4} G3 {5 6} G4 {7 8} ALL1 {1 2 3 4 5 6 7 8}}

UTF::Aeroflex af2 -lan_ip 172.2.1.81 \
    -relay "mc68end2" \
    -group {G5 {1 2} G6 {3 4} G7 {5 6} G8 {7 8} ALL2 {1 2 3 4 5 6 7 8}}

G1 configure -default 20
G2 configure -default 20
G3 configure -default 20
G4 configure -default 20
G5 configure -default 20
G6 configure -default 20
G7 configure -default 20
G8 configure -default 20
ALL1 configure -default 20 
ALL2 configure -default 20 

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL1 attn default;
    ALL2 attn default;

    foreach n {1 2 3 4 5 6 7 8} {
	catch {4366c0f19-$n wl down}
    catch {4366c0f19-$n wl down}
    catch {4366c0f19-$n wl status}
	4366c0f19-$n deinit
    catch {
        4366c0f19-$n sysctl -w net.core.rmem_max="16000000"
        4366c0f19-$n sysctl -w net.core.wmem_max="16000000" 
        4366c0f19-$n sysctl -w net.ipv4.tcp_rmem="4096 87380 16000000" 
        4366c0f19-$n sysctl -w net.ipv4.tcp_wmem="4096 87380 16000000" 
        4366c0f19-$n sysctl -w net.core.netdev_max_backlog="3000" 
        4366c0f19-$n echo bic > /proc/sys/net/ipv4/tcp_congestion_control
    }
    }
    foreach S {mc69end1 mc68end2} {
    catch {
        $S sysctl -w net.core.rmem_max="16000000"
        $S sysctl -w net.core.wmem_max="16000000" 
        $S sysctl -w net.ipv4.tcp_rmem="4096 87380 16000000" 
        $S sysctl -w net.ipv4.tcp_wmem="4096 87380 16000000" 
        $S sysctl -w net.core.netdev_max_backlog="3000" 
        $S echo bic > /proc/sys/net/ipv4/tcp_congestion_control
    }
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {Atlas2MCH5h Atlas2MCH2 Atlas2MCH5l} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    foreach S {4908a 4908b 4908c} {
	catch {$S dslshell wl -i [$S cget -device] down}
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #

    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# UTF Endpoint1 
UTF::Linux mc68end1 \
    -sta {lan p4p1} \
    -iperf iperf208c64 \
    -lan_ip mc68end1

lan configure -ipaddr 192.168.1.96

set wic "wl bw_cap 2g -1; wl vht_features 7; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0;\
wl msglevel +assoc +error"
# UTF Endpoint1 f19 - Traffic generators (no wireless cards)
UTF::Linux mc68end2 \
    -sta {lan2 p4p1} \
    -iperf iperf208c64 \
    -lan_ip mc68end2

lan configure -ipaddr 192.168.1.97

UTF::Linux mc68tst1  \
    -lan_ip mc68tst1 \
    -sta {4366c0f19-1 enp2s0} \
    -power {staNPC1 1} \
    -power_button "auto" \
    -brand linux-internal-wl \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1 " \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-1 configure -attngrp G1

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-1 clone 1x1MUsta1 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-1 clone 1x1MUsta1
}

UTF::Linux mc68tst2  \
    -lan_ip mc68tst2 \
    -sta {4366c0f19-2 enp2s0} \
    -power {staNPC1 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1 " \
    -brand linux-internal-wl \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-2 configure -attngrp G2

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-2 clone 1x1MUsta2 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-2 clone 1x1MUsta2
}

UTF::Linux mc68tst3  \
    -lan_ip mc68tst3 \
    -sta {4366c0f19-3 enp2s0} \
    -power {staNPC2 1} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1 " \
    -brand linux-internal-wl \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-3 configure -attngrp G3

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-3 clone 1x1MUsta3 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-3 clone 1x1MUsta3
}

UTF::Linux mc68tst4  \
    -lan_ip mc68tst4 \
    -sta {4366c0f19-4 enp2s0} \
    -power {staNPC2 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1 " \
    -brand linux-internal-wl \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-4 configure -attngrp G4

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-4 clone 1x1MUsta4 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-4 clone 1x1MUsta4
}

UTF::Linux mc68tst5  \
    -lan_ip mc68tst5 \
    -sta {4366c0f19-5 enp2s0} \
    -power {staNPC3 1} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -brand linux-internal-wl \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-5 configure -attngrp G5

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-5 clone 1x1MUsta5 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-5 clone 1x1MUsta5
}

UTF::Linux mc68tst6  \
    -lan_ip mc68tst6 \
    -sta {4366c0f19-6 enp2s0} \
    -power {staNPC3 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -brand linux-internal-wl \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-6 configure -attngrp G6

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-6 clone 1x1MUsta6 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-6 clone 1x1MUsta6 
}

UTF::Linux mc68tst7  \
    -lan_ip mc68tst7 \
    -sta {4366c0f19-7 enp1s0} \
    -power {staNPC4 1} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -brand linux-internal-wl \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
    -tcpwindow 2m \
    -iperf iperf208c64
4366c0f19-7 configure -attngrp G7                                                                                                                                                                                                           
if {[info exist env(STA_IMAGE)]} {                                                                                                                                                                                                          
    4366c0f19-7 clone 1x1MUsta7 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-7 clone 1x1MUsta7
}               

UTF::Linux mc68tst8 \
    -lan_ip mc68tst8 \
    -sta {4366c0f19-8 enp1s0} \
    -power {staNPC4 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -brand linux-internal-wl \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-8 configure -attngrp G8

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-8 clone 1x1MUsta8 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-8 clone 1x1MUsta8
}

foreach n "1 2 3 4 5 6 7 8" {
    1x1MUsta${n} clone 1x1SUsta${n} -wlinitcmds "wl down; $wic ;$confSU; $conf1x1; wl up "
    1x1SUsta${n} clone 2x2SUsta${n} -wlinitcmds "wl down; $wic; $confSU; $conf2x2; wl up "
    1x1SUsta${n} clone 2x2SUIsta${n} -wlinitcmds "wl down; $wic; $confSUi; $conf2x2; wl up "
    1x1SUsta${n} clone 1x1SUIsta${n} -wlinitcmds "wl down; $wic; $confSUi; $conf1x1; wl up "
    1x1MUsta${n} clone 2x2MUsta${n} -wlinitcmds "wl down; $wic; $conf2x2; wl up "
}

set nvramAtlas2 {
    watchdog=3000
    wl_msglevel=0x101
    console_loglevel=7
    wl0_ssid=Atlas2MCH2
    wl0_chanspec=11
    wl0_obss_coex=0
    wl0_bw_cap=-1
    wl0_radio=0
    wl0_country_code=Q1
    wl0_country_rev=137
    wl1_ssid=Atlas2MCH5l
    wl1_chanspec=36
    wl1_obss_coex=0
    wl1_bw_cap=-1
    wl1_radio=0 
    wl1_country_code=Q1
    wl1_country_rev=137
    wl2_ssid=Atlas2MCH5h
    wl2_chanspec=161
    wl2_obss_coex=0
    wl2_bw_cap=-1
    wl2_radio=0
    wl2_country_code=Q1
    wl2_country_rev=137
    lan_ipaddr=192.168.1.10   
	lan_gateway=192.168.1.10
	dhcp_start=192.168.1.100
  	dhcp_end=192.168.1.150
}

UTF::Router Atlas2 \
    -sta {Atlas2MCH5h eth3 Atlas2MCH5h.%15 wl2.% Atlas2MCH2 eth1 Atlas2MCH2.%15 wl0.% Atlas2MCH5l eth2 Atlas2MCH5l.%15 wl1.%} \
	-lan_ip 192.168.1.10 \
	-lanpeer {lan lan2} \
	-relay "mc68end2" \
    -power {mc68ncp2 1} \
    -console "mc68end2:40000" \
    -tag BISON04T_REL_7_14_131{_??} \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src" \
	-nvram $nvramAtlas2 \
    -nopm1 1 -nopm2 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 20-95 -attn2g 20-95 -pad 0} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -embeddedimage {436{5,6}c} \
    -slowassoc 5 \
    -iperf iperf208c64 \
    -pre_perf_hook {{%S wl dump txbf}}\
    -post_perf_hook {{%S wl dump txbf}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

if {[info exist env(AP_IMAGE)]} {
    puts "image is $env(AP_IMAGE)"

    Atlas2MCH2 clone Atlas2MCH2-p \
        -image $env(AP_IMAGE) \
        -nvram "$nvramAtlas2" \
        -perfchans {6l 6}\
        -pre_perf_hook {{%S wl curpower}}

    Atlas2MCH5l clone Atlas2MCH5l-p \
        -image $env(AP_IMAGE) \
        -nvram "$nvramAtlas2" \
        -perfchans {36/80 36l 36}\
        -pre_perf_hook {{%S wl curpower}}

    Atlas2MCH5h clone Atlas2MCH5h-p \
        -image $env(AP_IMAGE) \
        -nvram "$nvramAtlas2" \
        -perfchans {157/80 157l 157}\
        -pre_perf_hook {{%S wl curpower}}

    foreach objName "Atlas2MCH5l Atlas2MCH5h" {
        ${objName}-p clone ${objName}-mu-p \
        -nvram "$nvramAtlas2 $nvramMU"\
        -rvrnightly {-mumode mu}

        ${objName}-mu-p clone ${objName}_g3-mu-p 
        ${objName}-mu-p clone ${objName}-mu_auto-p \
        -nvram "$nvramAtlas2 $nvramMUauto"
        ${objName}-mu_auto-p clone ${objName}_g3-mu_auto-p 
        ${objName}-p clone ${objName}-su-p \
        -nvram "$nvramAtlas2 $nvramSU"
        ${objName}-su-p clone ${objName}_g3-su-p
    }
    Atlas2MCH5l-mu-p clone Atlas2MCH5l-mu-pf_mon0 \
    -wlinitcmds "sleep 5; wl -i eth2 muclient_pfmon 0"
    Atlas2MCH5l-mu-p clone Atlas2MCH5l-mu-sched0 \
    -wlinitcmds "sleep 5; wl -i eth2 muclient_scheduler 0"
} 

# Bison STA clones
if {[info exist env(TAG_B)]} {
    set tag_b $env(TAG_B)
} else {
    set tag_b BISON04T_REL_7_14_131{_??}
}
puts "#### tag_b = $tag_b ####"

if {[info exist env(TAG_G3)]} {
    set tag_g3 $env(TAG_G3)
} else {
    set tag_g3 BISON04T_REL_7_14_164{_?*}
}
puts "#### tag_g3 = $tag_g3 ####"

### for internal images
set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
Atlas2MCH2 clone Atlas2MCH2-i -tag $tag_b \
    -brand $brand \
    -perfchans {6l 6}
Atlas2MCH2-i clone Atlas2MCH2-dm1-i \
-nvram "$nvramAtlas2 $nvramDM1"
Atlas2MCH2-i clone Atlas2MCH2_g3-i -tag $tag_g3
Atlas2MCH2_g3-i clone Atlas2MCH2_g3-dm1-i \
-nvram "$nvramAtlas2 $nvramDM1"

Atlas2MCH5l clone Atlas2MCH5l-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
Atlas2MCH5l-i clone Atlas2MCH5l_g3-i -tag $tag_g3\
    -perfchans {36/80 36l 36}

Atlas2MCH5h clone Atlas2MCH5h-i -tag $tag_b \
    -brand $brand \
    -perfchans {157/80 157l 157}
Atlas2MCH5h-i clone Atlas2MCH5h_g3-i -tag $tag_g3\
    -perfchans {157/80 157l 157}

set nvramTAF {wl0_taf_enable=1 wl1_taf_enable=1 wl2_taf_enable=1\
toa-sta-1=00:10:18:fb:d1:78 prio=1 type=video" "toa-bss-1=0 type=data"}

foreach obj "Atlas2MCH5l Atlas2MCH5h Atlas2MCH5l_g3 Atlas2MCH5h_g3" {
    ${obj}-i clone ${obj}-su-i \
        -nvram "$nvramAtlas2 $nvramSU"
    ${obj}-i clone ${obj}-su-taf-i \
        -nvram "$nvramAtlas2 $nvramSU $nvramTAF" \
        -pre_perf_hook {
        {%S rexec "ps | grep toad"}
        {%S wl taf atos list}
        {%S wl taf ebos list}
        }\
        -post_perf_hook {
        {%S wl taf atos list}
        {%S wl taf ebos list}
        }
    ${obj}-su-i clone ${obj}-su-dm1-i \
        -nvram "$nvramAtlas2 $nvramSU $nvramDM1"

    ${obj}-i clone ${obj}-mu-i \
        -nvram "$nvramAtlas2 $nvramMU"\
        -rvrnightly {-mumode mu}
    ${obj}-mu-i clone ${obj}-mu_auto-i \
        -nvram "$nvramAtlas2 $nvramMUauto"
    ${obj}-mu-i clone ${obj}-mu-dm1-i \
        -nvram "$nvramAtlas2 $nvramMU $nvramDM1"
    ${obj}-mu-i clone ${obj}-mu_auto-dm1-i \
        -nvram "$nvramAtlas2 $nvramMUauto $nvramDM1"

    ${obj}-mu-i clone ${obj}-mu-taf-i\
        -nvram "$nvramAtlas2 $nvramMU $nvramTAF" \
        -pre_perf_hook {
        {%S rexec "ps | grep toad"}
        {%S wl taf atos list}
        {%S wl taf ebos list}
        }\
        -post_perf_hook {
        {%S wl taf atos list}
        {%S wl taf ebos list}
        }
    ${obj}-mu_auto-i clone  ${obj}-mu_auto-atf1-i \
        -nvram "$nvramAtlas2 $nvramMUauto wl2_atf=1"
    ${obj}-mu_auto-i clone  ${obj}-mu_auto-atf0-i \
        -nvram "$nvramAtlas2 $nvramMUauto wl2_atf=0"
}

Atlas2MCH5h-mu-i configure -dualband {Atlas2MCH2-i -c1 36/80 -c2 6l}
Atlas2MCH5h-mu-i configure -dualband {Atlas2MCH2-i -c1 157/80 -c2 6l}
Atlas2MCH5h-mu-i configure -dualband {Atlas2MCH5l-mu-i -c1 157/80 -c2 36/80}

Atlas2MCH5h-mu-dm1-i configure -dualband {Atlas2MCH2-dm1-i -c1 36/80 -c2 6l}
Atlas2MCH5h-mu-dm1-i configure -dualband {Atlas2MCH2-dm1-i -c1 157/80 -c2 6l}
Atlas2MCH5h-mu-dm1-i configure -dualband {Atlas2MCH5l-mu-dm1-i -c1 157/80 -c2 36/80}

Atlas2MCH5l_g3-mu-i configure -dualband {Atlas2MCH2_g3-i -c1 36/80 -c2 6l}
Atlas2MCH5h_g3-mu-i configure -dualband {Atlas2MCH2_g3-i -c1 157/80 -c2 6l}
Atlas2MCH5h_g3-mu-i configure -dualband {Atlas2MCH5l_g3-mu-i -c1 157/80 -c2 36/80}

Atlas2MCH5l_g3-mu-dm1-i configure -dualband {Atlas2MCH2_g3-dm1-i -c1 36/80 -c2 6l}
Atlas2MCH5h_g3-mu-dm1-i configure -dualband {Atlas2MCH2_g3-dm1-i -c1 157/80 -c2 6l}
Atlas2MCH5h_g3-mu-dm1-i configure -dualband {Atlas2MCH5l_g3-mu-dm1-i -c1 157/80 -c2 36/80}

### for external images
set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
Atlas2MCH2-i clone Atlas2MCH2_e -brand $brand
Atlas2MCH5l-mu-i clone Atlas2MCH5l-mu-e -brand $brand
Atlas2MCH5h-mu-i clone Atlas2MCH5h-mu-e -brand $brand
Atlas2MCH5l-mu_auto-i clone Atlas2MCH5l-mu_auto-e -brand $brand
Atlas2MCH5h-mu_auto-i clone Atlas2MCH5h-mu_auto-e -brand $brand
Atlas2MCH5l-su-i clone Atlas2MCH5l-su-e -brand $brand
Atlas2MCH5l-su-i clone Atlas2MCH5l_g3-su-e -brand $brand
Atlas2MCH5h-su-i clone Atlas2MCH5h-su-e -brand $brand
Atlas2MCH5h_g3-su-i clone Atlas2MCH5h_g3-su-e -brand $brand

Atlas2MCH5l-mu-dm1-i clone Atlas2MCH5l-mu-dm1-e -brand $brand
Atlas2MCH5h-mu-dm1-i clone Atlas2MCH5h-mu-dm1-e -brand $brand
Atlas2MCH5l-mu_auto-dm1-i clone Atlas2MCH5l-mu_auto-dm1-e -brand $brand
Atlas2MCH5h-mu_auto-dm1-i clone Atlas2MCH5h-mu_auto-dm1-e -brand $brand
Atlas2MCH5l-su-dm1-i clone Atlas2MCH5l-su-dm1-e -brand $brand
Atlas2MCH5h-su-dm1-i clone Atlas2MCH5h-su-dm1-e -brand $brand

Atlas2MCH5l_g3-mu-i clone Atlas2MCH5l_g3-mu-e -brand $brand
Atlas2MCH5h_g3-mu-i clone Atlas2MCH5h_g3-mu-e -brand $brand
Atlas2MCH5l_g3-mu_auto-i clone Atlas2MCH5l_g3-mu_auto-e -brand $brand
Atlas2MCH5h_g3-mu_auto-i clone Atlas2MCH5h_g3-mu_auto-e -brand $brand

Atlas2MCH5l_g3-mu-dm1-i clone Atlas2MCH5l_g3-mu-dm1-e -brand $brand
Atlas2MCH5h_g3-mu-dm1-i clone Atlas2MCH5h_g3-mu-dm1-e -brand $brand
Atlas2MCH5l_g3-mu_auto-dm1-i clone Atlas2MCH5l_g3-mu_autoi-dm1-e -brand $brand
Atlas2MCH5h_g3-mu_auto-dm1-i clone Atlas2MCH5h_g3-mu_autoi-dm1-e -brand $brand
Atlas2MCH5l_g3-su-dm1-i clone Atlas2MCH5l_g3-su-dm1-e -brand $brand
Atlas2MCH5h_g3-su-dm1-i clone Atlas2MCH5h_g3-su-dm1-e -brand $brand

Atlas2MCH5l-mu-e configure -dualband {Atlas2MCH2_e -c1 36/80 -c2 6l}
Atlas2MCH5h-mu-e configure -dualband {Atlas2MCH2_e -c1 157/80 -c2 6l}
Atlas2MCH5h-mu-e configure -dualband {Atlas2MCH5l-mu-e -c1 157/80 -c2 36/80}

Atlas2MCH5l_g3-mu-e configure -dualband {Atlas2MCH2_g3-e -c1 36/80 -c2 6l}
Atlas2MCH5h_g3-mu-e configure -dualband {Atlas2MCH2_g3-e -c1 157/80 -c2 6l}
Atlas2MCH5h_g3-mu-e configure -dualband {Atlas2MCH5l_g3-mu-e -c1 157/80 -c2 36/80}

set nvram4908 {
	wl2_ssid=4908_4366Emcm5h
	wl1_ssid=4908_4366Emcm5
	wl0_ssid=4908_4366mcm2
	wl0_chanspec=161
	wl1_chanspec=36
	wl2_chanspec=3
	wl0_radio=0
	wl1_radio=1
	wl2_radio=0
	wl0_vht_features=6
	wl1_vht_features=6
	wl2_vht_features=7
    wl0_country_code=US/0
    wl1_country_code=US/0
    wl2_country_code=US/0
	wan_ifname=
	wan_ifnames=
	{lan_ifnames=eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7}
}

package require UTF::DSL
UTF::DSL 4908 -sta {4908a eth5 4908a.%8 wl0.% 4908b eth6 4908b.%8 wl1.% 4908c eth7 4908c.%8 wl2.%} \
    -model 4908 \
    -jenkins "" \
	-relay "mc68end1" \
    -tag BISON04T_BRANCH_7_14 \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_int \
    -type bcm94908HND_nand_fs_image_128_ubi.w \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan lan2} \
    -console "mc68end2:40001" \
    -relay lan2 \
    -power {mc68ncp2 2} \
    -nvram $nvram4908 \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g {0-90} -attn2g {48-90} -pad 29} \
    -noradio_pwrsave 1

4908a clone 4908_4366mcm2 -sta {4908_4366mcm2 eth5 4908_4366mcm2.%8 wl0.%} \
    -channelsweep {-max 64} -perfchans {6/40}
4908b clone 4908_4366Emcm5 -sta {4908_4366Emcm5 eth6 4908_4366Emcm5.%8 wl1.%} \
    -channelsweep {-max 100} -perfchans {36/80}
4908c clone 4908_4366Emcm5h -sta {4908_4366Emcm5h eth7 4908_4366Emcm5h.%8 wl2.%} \
    -channelsweep {-min 100} -perfchans {161/80}

set external {
    -jenkins "" \
    -tag BISON04T_TWIG_7_14_131 \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_external_full \
    -type bcm94908HND_nand_cferom_fs_image_128_ubi*.w \
}

4908_4366Emcm5h clone 4908_4366Emcm5h-e {*}$external -perfonly 1
4908_4366Emcm5 clone 4908_4366Emcm5-e {*}$external -perfonly 1


set internal {
    -jenkins "" \
    -tag REL502L02_BISON04T_TWIG_7_14_164\
    -brand BISON04T_TWIG_7_14_164__LINUX_4.1.0_ROUTER_DHDAP/94908HND_int \
    -type bcm94908HND_nand_cferom_fs_image_128_ubi.w \
}

4908_4366Emcm5h clone 4908_4366Emcm5h-i {*}$internal
4908_4366Emcm5 clone 4908_4366Emcm5-i {*}$internal
4908_4366mcm2 clone 4908_4366mcm2-i  {*}$internal

if {[info exists env(IMAGE_4908)]} {
    set image $env(IMAGE_4908)
} else {
    set image /projects/bcawlan_builds/Irvine/REL502L02_BISON04T_TWIG_7_14_164/BISON04T_TWIG_7_14_164__LINUX_4.1.0_ROUTER_DHDAP/94908HND_int/2016.8.24.121/images/bcm94908HND_nand_cferom_fs_image_128_ubi.w 
    #set image /projects/bcawlan_builds/Irvine/502HND02rc4_BISON04T_REL_7_14_164/images/internal/bcm94908HND_nand_cferom_fs_image_128_ubi.w 
}

foreach obj "4908_4366Emcm5h 4908_4366Emcm5" {
    ${obj}-i clone ${obj}-mu-i\
        -nvram "$nvram4908 $nvramMUauto" -rvrnightly {-mumode mu}\
        -image $image 

    ${obj}-i clone ${obj}-su-i\
        -nvram "$nvram4908 $nvramSU"\
        -image $image 

    ${obj}-e clone ${obj}-mu-e\
        -nvram "$nvram4908 $nvramMUauto" -rvrnightly {-mumode mu}\
        -image $image 

    ${obj}-e clone ${obj}-su-e\
        -nvram "$nvram4908 $nvramSU"\
        -image $image 
}

if {[info exists env(AP_IMAGE)]} {
    4908_4366Emcm5h clone 4908_4366Emcm5h-p -image $env(AP_IMAGE)
    4908_4366Emcm5 clone 4908_4366Emcm5-p -image $env(AP_IMAGE)
    4908_4366mcm2 clone 4908_4366mcm2-p -image $env(AP_IMAGE)
    foreach obj "4908_4366Emcm5h 4908_4366Emcm5" {
        ${obj}-p clone ${obj}-mu-p\
            -nvram "$nvram4908 $nvramMUauto" -rvrnightly {-mumode mu}
        ${obj}-p clone ${obj}-su-p\
            -nvram "$nvram4908 $nvramSU"
    }
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
set UTF::KPPSSweep 1
set UTF::UTFStatus 1
UTF::Q utf4

# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum MC69 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

#package require UTF::TclReadLines
#UTF::Power::WebRelay pb6awr -relay mc46end1 -lan_ip 172.19.12.32 -invert {2 3}
#Linux endpoints WebRelay - 10.19.86.134
UTF::Power::WebRelay usbwr -relay mc46end1 -lan_ip 172.19.12.32

set UTF::WebTree 1
set UTF::DataRatePlot 1

set nvramDM "devicemode=1 "
set nvramCTDMA "devpath1=pcie/1/1/ 1:ctdma=1 devpath2=pcie/2/3/ 2:ctdma=1 "
set nvramMU "wl0_mu_features=1"

UTF::Cygwin VWstation -lan_ip 172.19.12.239 -user user -sta {}

if 0 {
# Define Sniffer
UTF::Sniffer mc46tst3 -user root \
        -sta {43224SNF1 eth3} \
        -tag RUBY_REL_6_20_42 
};# if 0

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

set ::UTF::SummaryDir "/projects/hnd_svt_ap5/$::env(LOGNAME)/mc46"

# Define power controllers on cart
UTF::Power::Synaccess mc46npc1 -lan_ip 172.19.12.5 -rev 1
UTF::Power::Synaccess mc46npc2 -lan_ip 172.19.12.27 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip mc46att1 \
    -group {G1 {1 2 3 4} ALL {1 2 3 4 5 6}}

G1 configure -default 20

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #ALL attn default;
    G1 attn default;

    foreach S {4366mcb1f19 4366mcc0f19} {
	catch {$S wl down}
	$S deinit
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {47094R/4366MCB1 4709C0/4366MCC0} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #4708 restart wl0_radio=0
    #AP2 restart wl0_radio=0
    #AP5 restart wl0_radio=0

    #mc46tst3 add_netconsole mc46end1 6661
    #mc46tst3 service netconsole start
    foreach dev "mc46end1 mc46end2 mc46tst2 mc46tst3" {
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
    unset ::UTF::SetupTestBed

    return
}

# UTF Endpoint1 f11 - Traffic generators (no wireless cards)
UTF::Linux mc46end1 \
    -sta {lan eth1} 

lan configure -ipaddr 192.168.1.97

UTF::Linux mc46end2 \
    -sta {lan2 p4p1} 

lan2 configure -ipaddr 192.168.1.96

UTF::Linux mc46tst2  \
        -lan_ip mc46tst2 \
        -sta {4366mcb1f19 enp1s0} \
        -power {mc46npc2 1} \
        -power_button "auto" \
        -tcpwindow 4M \
        -wlinitcmds {wl msglevel +assoc +error; wl down; wl vht_features 7; wl up} \
        -date {} \
        -tag EAGLE_BRANCH_10_10 \
        -brand linux-internal-wl \
        -power_button "auto" \
        -nobighammer 0 \
        -perfchans {36/80 36l 3l} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S ifconfig}} \
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl dump scb} {%S wl counters}\
        {%S ifconfig} {%S wl dump ampdu}}

4366mcb1f19 configure -attngrp G1

UTF::Linux mc46tst3  \
        -lan_ip mc46tst3 \
        -sta {4366mcc0f19 enp1s0} \
        -power {mc46npc2 2} \
        -power_button "auto" \
        -tcpwindow 4M \
        -wlinitcmds {wl msglevel +assoc +error; wl down; wl vht_features 7; wl up} \
        -date {} \
        -tag EAGLE_BRANCH_10_10 \
        -brand linux-internal-wl \
        -power_button "auto" \
        -nobighammer 0 \
        -perfchans {36/80 36l 3l} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S ifconfig}} \
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl dump scb} {%S wl counters}\
        {%S ifconfig} {%S wl dump ampdu}}

4366mcc0f19 configure -attngrp G1

4366mcc0f19 clone 4366softap -tag EAGLE_TWIG_10_10_69
4366softap configure -ipaddr 192.168.1.25 -attngrp G1 -ap 1 

set nvram47094R { 
watchdog=3000
wl_msglevel=0x101
console_loglevel=7
wl0_ssid=Broadcom
wl0_chanspec=36
wl0_obss_coex=0
wl0_bw_cap=-1
wl0_radio=0
wl0_vht_features=7
wl0_country_code=
lan_ipaddr=192.168.1.1
lan_gateway=192.168.1.1
dhcp_start=192.168.1.100
dhcp_end=192.168.1.149
lan1_ipaddr=192.168.2.1
lan1_gateway=192.169.2.1
dhcp1_start=192.168.2.100
dhcp1_end=192.168.2.149
}   

set nvram4709C0 { 
watchdog=3000
wl_msglevel=0x101
console_loglevel=7
wl0_ssid=Broadcom
wl0_chanspec=36
wl0_obss_coex=0
wl0_bw_cap=-1
wl0_radio=0
wl0_vht_features=7
wl0_country_code=
lan_ipaddr=192.168.1.2
lan_gateway=192.168.1.2
dhcp_start=192.168.1.100
dhcp_end=192.168.1.149
lan1_ipaddr=192.168.2.1
lan1_gateway=192.169.2.1
dhcp1_start=192.168.2.100
dhcp1_end=192.168.2.149
}   

if {[info exists nvramAppendList]} {
    append nvram47094R $nvramAppendList
    append nvram4709C0 $nvramAppendList
}

set nvramAtlas {
    "fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
    gmac3_enable=1
}

UTF::Router 47094R \
	-sta {47094R/4366MCB1 eth1 47094R/4366MCB1.%15 wl0.% 47094R/43602MCH2 eth2 47094R/43602MCH2.%15 wl1.%} \
	-lan_ip 192.168.1.1 \
	-lanpeer {lan lan2} \
	-relay "mc46end1" \
    -power {mc46npc1 1} \
    -console "mc46end1:40001" \
    -tag BISON04T_REL_7_14_124{,_??} \
    -brand "linux-2.6.36-arm-internal-router-dhdap-atlas" \
	-nvram $nvram47094R \
    -nopm1 1 -nopm2 1 \
    -embeddedimage {436{5,6}b} \
    -wlinitcmds {sleep 2} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8G \
    -yart {-frameburst 1 -attn5g 20-100 -attn2g 20-100 -pad 21} \
    -noradio_pwrsave 1 -perfchans {161/80 161u 161 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} {%S wl pktq_stats C:[4366mcc0f19 macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl reset_cnts} \
    {%S wl dump ampdu} {%S wl ratedump} {%S wl dump phycal} \
    {%S wl dump scb} {%S et counters} {%S wl counters} \
    {%S wl assoclist} {%S wl sta_info [4366mcb1f19 macaddr]} {%S wl rssi [4366mcb1f19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} \
    {%S wl sta_info [4366mcc0f19 macaddr]} {%S wl rssi [4366mcc0f19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcc0f19 macaddr]}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

UTF::Router 4709C0 \
	-sta {4709C0/4366MCC0 eth1 4709C0/4366MCC0.%15 wl0.% 4709C0/43602MCH2 eth2 4709C0/43602MCH2.%15 wl1.%} \
	-lan_ip 192.168.1.2 \
	-lanpeer {lan lan2} \
	-relay "mc46end1" \
    -power {mc46npc1 2} \
    -console "mc46end1:40002" \
    -tag BISON04T_REL_7_14_124{,_??} \
    -brand "linux-2.6.36-arm-internal-router-dhdap-atlas" \
	-nvram $nvram4709C0 \
    -nopm1 1 -nopm2 1 \
    -embeddedimage {436{5,6}c} \
    -wlinitcmds {sleep 2} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8G \
    -yart {-frameburst 1 -attn5g 20-100 -attn2g 20-100 -pad 21} \
    -noradio_pwrsave 1 -perfchans {161/80 161u 161 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} {%S wl pktq_stats C:[4366mcc0f19 macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl reset_cnts} \
    {%S wl dump ampdu} {%S wl ratedump} {%S wl dump phycal} \
    {%S wl dump scb} {%S et counters} {%S wl counters} \
    {%S wl assoclist} {%S wl sta_info [4366mcb1f19 macaddr]} {%S wl rssi [4366mcb1f19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} \
    {%S wl sta_info [4366mcc0f19 macaddr]} {%S wl rssi [4366mcc0f19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcc0f19 macaddr]}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

if {[info exist env(AP_IMAGE)]} {
    foreach obj "47094R\/4366MCB1 4709C0\/4366MCC0" obj1 "47094R\/43602MCH2 4709C0\/43602MCH2" \
    cln "4366MCb 4366MCc" cln1 "43602MCH2b 43602MCH2c" nr "[list $nvram47094R] [list $nvram4709C0]" {
    ${obj} clone ${cln}_dhda \
        -image $env(AP_IMAGE) \
        -nvram "$nr $nvramAtlas" \
        -perfchans {161/80 161u 161 6l 6}

    ${cln}_dhda clone ${cln}_dhda-ctdma0-dm0 \
        -nvram "$nr $nvramAtlas"
    ${cln}_dhda clone ${cln}_dhda-ctdma0-dm1 \
        -nvram "$nr $nvramAtlas $nvramDM"
    ${cln}_dhda clone ${cln}_dhda-ctdma1-dm0 \
        -nvram "$nr $nvramAtlas $nvramCTDMA"
    ${cln}_dhda clone ${cln}_dhda-ctdma1-dm1 \
        -nvram "$nr $nvramAtlas $nvramCTDMA $nvramDM"
    ${cln}_dhda clone ${cln}_dhda-ctdma1-dm1-mu1 \
        -nvram "$nr $nvramAtlas $nvramCTDMA $nvramDM $nvramMU"

    ${obj} clone ${cln}_dhd \
        -image $env(AP_IMAGE) \
        -perfchans {161/80 161u 161 6l 6}

    ${cln}_dhd clone ${cln}_dhd-aspm-default \
        -nvram "$nr"
    ${cln}_dhd clone ${cln}_dhd-aspm-always \
        -nvram "$nr aspmd_alltime_active=1"
    ${cln}_dhd clone ${cln}_dhd-aspm-disabled \
        -nvram "$nr aspmd_disable=1"

    ${obj1} clone ${cln1}_dhda \
        -image $env(AP_IMAGE) \
        -nvram "$nr $nvramAtlas" \
        -perfchans {6l 6}

    ${cln1}_dhda clone ${cln1}_dhda-dm0 \
        -nvram "$nr $nvramAtlas"
    ${cln1}_dhda clone ${cln1}_dhda-dm1 \
        -nvram "$nr $nvramAtlas $nvramDM"

    ${cln}_dhda-ctdma0-dm0 configure -dualband "${cln1}_dhda-dm0 -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    ${cln}_dhda-ctdma0-dm1 configure -dualband "${cln1}_dhda-dm1 -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    ${cln}_dhda-ctdma1-dm0 configure -dualband "${cln1}_dhda-dm0 -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    ${cln}_dhda-ctdma1-dm1 configure -dualband "${cln1}_dhda-dm1 -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    };#foreach obj
} else {
    # Bison STA clones
    if {[info exist env(TAG_B)]} {
        set tag_b $env(TAG_B)
    } else {
        set tag_b BISON04T_REL_7_14_131{,_??}
    }
    puts "#### tag_b = $tag_b ####"
    foreach obj "47094R\/4366MCB1 4709C0\/4366MCC0" obj1 "47094R\/43602MCH2 4709C0\/43602MCH2" \
    cln "4366MCb 4366MCc" cln1 "43602MCH2b 43602MCH2c" nr "[list $nvram47094R] [list $nvram4709C0]" {

    ### for internal images
    set brand "linux-2.6.36-arm-internal-router"

    ${obj} clone ${obj}b-i -tag $tag_b \
        -brand $brand \
        -perfchans {161/80 161u 161 6l 6}

    ${obj1} clone ${obj1}b-i -tag $tag_b \
        -brand $brand \
        -perfchans {6l 6}

    set brand linux-2.6.36-arm-internal-router-dhdap
    ${obj1} clone ${obj1}b_dhd-i -tag $tag_b \
        -brand $brand \
        -perfchans {161/80 161u 161 6l 6}

    ${obj1} clone ${obj1}b_dhd-i -tag $tag_b \
        -brand $brand \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-internal-router-dhdap-atlas" 
    ${obj} clone ${cln}_dhda-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -nvram "$nr $nvramAtlas" \
        -perfchans {161/80 161u 161 6l 6}
    ${cln}_dhda-ctdma0-dm0-i clone ${cln}_dhda-ctdma0-dm0-1603 \
        -tag BISON04T_REL_7_14_131_1603 \
        -perfchans {161/80 161u 161}
    ${cln}_dhda-ctdma0-dm0-1603 clone ${cln}_dhda-ctdma0-dm0-9 \
        -tag BISON04T_REL_7_14_131_9 

    ${cln}_dhda-ctdma0-dm0-i clone ${cln}_dhda-ctdma0-dm1-i -tag $tag_b \
        -nvram "$nr $nvramAtlas $nvramDM"
    ${cln}_dhda-ctdma0-dm0-i clone ${cln}_dhda-ctdma1-dm0-i -tag $tag_b \
        -nvram "$nr $nvramAtlas $nvramCTDMA"
    ${cln}_dhda-ctdma0-dm0-i clone ${cln}_dhda-ctdma1-dm1-i -tag $tag_b \
        -nvram "$nr $nvramAtlas $nvramCTDMA $nvramDM"
    ${cln}_dhda-ctdma0-dm0-i clone ${cln}_dhda-ctdma1-dm1-mu1-i -tag $tag_b \
        -nvram "$nr $nvramAtlas $nvramCTDMA $nvramDM $nvramMU"

    ${obj1} clone ${cln1}_dhda-dm0-i -tag $tag_b \
        -brand $brand \
        -nvram "$nr $nvramAtlas" \
        -perfchans {6l 6}
    ${cln1}_dhda-dm0-i clone ${cln1}_dhda-dm1-i -tag $tag_b \
        -nvram "$nr $nvramAtlas $nvramDM"

    ### for external images
    #set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
    #set brand "linux-2.6.36-arm-external-vista-router-full-src" 
    #set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
    #${cln}_dhda-ctdma0-dm0-i configure -dualband {${cln}_dhda-ctdma0-dm0-i -c2 161/80 -c1 6l}
    #${cln}_dhda-ctdma0-dm0-i configure -dualband {${cln}_dhda-ctdma0-dm0-i -c2 161/80 -c1 6l \
    #-b1 0 -b2 0}
    ${cln}_dhda-ctdma0-dm0-i configure -dualband "${cln1}_dhda-dm0-i -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    ${cln}_dhda-ctdma0-dm1-i configure -dualband "${cln1}_dhda-dm1-i -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    ${cln}_dhda-ctdma1-dm0-i configure -dualband "${cln1}_dhda-dm0-i -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    ${cln}_dhda-ctdma1-dm1-i configure -dualband "${cln1}_dhda-dm1-i -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2"
    };# foreach obj

}

# Eagle STA clones    
if {[info exist env(AP_IMAGE)]} {
    47094R/4366MCB1 clone 4366MCe-i \
        -image $env(AP_IMAGE) \
        -perfchans {161/80 161u 161 6l 6}
} else {
    ### for internal images
    if {[info exist env(TAG_E)]} {
        set tag_e $env(TAG_E)
    } else {
        set tag_e EAGLE_BRANCH_10_10{,_?*}
    }
    puts "#### tag_e = $tag_e ####"

    set brand "linux-2.6.36-arm-internal-router"
    47094R/4366MCB1 clone 4366MCe-i -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161 6l 6}

    set brand linux-2.6.36-arm-internal-router-dhdap
    47094R/4366MCB1 clone 4366MCe_dhd-i -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161 6l 6}

    ### for external images
    set brand "linux-2.6.36-arm-external-vista-router-full-src" 
    47094R/4366MCB1 clone 4366MCe -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161 6l 6}

    set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
    47094R/4366MCB1 clone 4366MCe_dhd -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161 6l 6}

    4366MCe-i configure -dualband {4366MCe-i -c1 161/80 -c2 6l \
    -lan1 lan -lan2 lan2}
 
    4366MCe-i clone 4366MCe-lesi0-i \
    -pre_perf_hook {{%S wl phy_lesi 0} {%S wl phy_lesi} {%S et clear_dump}\
    {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} {%S wl pktq_stats C:[4366mcc0f19 macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl phy_lesi} {%S wl dump rssi} {%S wl nrate} {%S wl reset_cnts} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl assoclist}\
    {%S wl ratedump} {%S wl dump phycal} \
    {%S wl sta_info [4366mcb1f19 macaddr]} {%S wl rssi [4366mcb1f19 macaddr]} \
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} {%S wl sta_info [4366mcc0f19 macaddr]}\
    {%S wl rssi [4366mcc0f19 macaddr]} {%S wl pktq_stats C:[4366mcc0f19 macaddr]}}

    4366MCe-i clone 4366MCe-lesi1-i \
    -pre_perf_hook {{%S wl phy_lesi 1} {%S wl phy_lesi} {%S et clear_dump}\
    {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} {%S wl pktq_stats C:[4366mcc0f19 macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}} \
    -post_perf_hook {{%S wl phy_lesi} {%S wl dump rssi} {%S wl nrate} {%S wl reset_cnts} \
    {%S wl ratedump} {%S wl dump phycal} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl assoclist}\
    {%S wl sta_info [4366mcb1f19 macaddr]} {%S wl rssi [4366mcb1f19 macaddr]} \
    {%S wl pktq_stats C:[4366mcb1f19 macaddr]} {%S wl sta_info [4366mcc0f19 macaddr]}\
    {%S wl rssi [4366mcc0f19 macaddr]} {%S wl pktq_stats C:[4366mcc0f19 macaddr]}}
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

UTF::Q mc46

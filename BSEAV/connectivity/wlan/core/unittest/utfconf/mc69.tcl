# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum MC69 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package require UTF::TclReadLines
#UTF::Power::WebRelay pb6awr -relay mc69end1 -lan_ip 172.19.12.32 -invert {2 3}
UTF::Power::WebRelay usbwr -relay mc69end1 -lan_ip 172.19.12.32

set UTF::WebTree 1
set UTF::DataRatePlot 1

set nvramDM "devicemode=1 "
set nvramCTDMA "devpath1=pcie/1/1/ 1:ctdma=1 devpath2=pcie/2/3/ 2:ctdma=1 3:ctdma=1" 
set nvramMUauto "wl0_mu_features=0x8000 wl1_mu_features=0x8000 wl2_mu_features=0x8000"
set nvramMU "wl0_mu_features=1 wl1_mu_features=1 wl2_mu_features=1"

UTF::Cygwin VWstation -lan_ip 172.19.12.239 -user user -sta {}

# Define Sniffer
UTF::Sniffer mc69tst3 -user root \
        -sta {43224SNF1 eth3} \
        -tag RUBY_REL_6_20_42 


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

set ::UTF::SummaryDir "/projects/hnd_svt_ap5/$::env(LOGNAME)/mc69"

#AP1: Router WRT320N/4717
#STA1: Linux 4322HM8LD_Dell and 43224_A203 
#STA2: 43228w7 


# Define power controllers on cart
UTF::Power::Synaccess mc69npc1 -lan_ip 172.19.12.23 -relay mc69end2
UTF::Power::Synaccess mc69npc2 -lan_ip 172.19.12.27 -relay mc69end2 -rev 1
UTF::Power::Synaccess mc69npc3 -lan_ip 172.19.12.28 -relay mc69end2 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.12.24 \
    -relay "mc69end2" \
    -group {G1 {1 2 3 4} ALL {1 2 3 4 5 6}}

G1 configure -default 10

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #ALL attn default;
    G1 attn default;

    foreach S {4366mcf19a 4366mcf19} {
	catch {$S wl down}
	$S deinit
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {4908a 4908b 4908c} {
	catch {$S dslshell wl -i [$S cget -device] down}
    }
    foreach S {A2p502MCH5h A2p502MCH2 A2p502MCH5l \
    A2p416MCH5h A2p416MCH2 A2p416MCH5l} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #4708 restart wl0_radio=0
    #AP2 restart wl0_radio=0
    #AP5 restart wl0_radio=0

    foreach dev "mc69end1 mc69end2 mc69tst1 mc69tst2" {
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
UTF::Linux mc69end1 \
    -sta {lan eth1} 

lan configure -ipaddr 192.168.1.97

UTF::Linux mc69end2 \
    -sta {lan2 p2p1} 

lan2 configure -ipaddr 192.168.1.96


UTF::Linux mc69tst1  \
        -lan_ip mc69tst1 \
        -sta {4366mcf19 enp2s0} \
        -power {mc69npc2 1} \
        -power_button "auto" \
        -tcpwindow 4M \
        -wlinitcmds {wl msglevel +assoc +error; wl down; wl vht_features 7; wl up} \
        -date {2016.5.9.1} \
        -tag EAGLE_BRANCH_10_10 \
        -brand linux-internal-wl \
        -power_button "auto" \
        -nobighammer 0 \
        -perfchans {36/80 36l 3l} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S ifconfig}} \
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl dump scb} {%S wl counters}\
        {%S ifconfig} {%S wl dump ampdu}}

4366mcf19 configure -attngrp G1
        #-tag EAGLE_REL_10_10_69_74 \

UTF::Linux mc69tst2  \
        -lan_ip mc69tst2 \
        -sta {4366mcf19a enp2s0} \
        -power {mc69npc2 2} \
        -power_button "auto" \
        -tcpwindow 4M \
        -wlinitcmds {wl msglevel +assoc +error; wl down; wl vht_features 7; wl up;} \
        -date {2016.5.9.1} \
        -tag EAGLE_BRANCH_10_10 \
        -brand linux-internal-wl \
        -power_button "auto" \
        -nobighammer 0 \
        -perfchans {36/80 36l 3l} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S ifconfig}} \
        -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl dump scb} {%S wl counters}\
        {%S ifconfig} {%S wl dump ampdu}}
4366mcf19a configure -attngrp G1

set nvram4709 {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
    wl0_ssid=Broadcom
    wl0_chanspec=36
    wl0_obss_coex=0
    wl0_bw_cap=-1
    wl0_radio=1
    wl0_vht_features=7
    wl0_country_code=US/0
    wl1_ssid=Broadcom
    wl1_chanspec=3
    wl1_obss_coex=0
    wl1_bw_cap=-1
    wl1_radio=0
    wl1_vht_features=7
    wl1_country_code=US/0
	lan_ipaddr=192.168.1.10
	lan_gateway=192.168.1.10
	dhcp_start=192.168.1.150
  	dhcp_end=192.168.1.179
	lan1_ipaddr=192.168.2.1
	lan1_gateway=192.169.2.1
	dhcp1_start=192.168.2.100
    dhcp1_end=192.168.2.149
}
if {[info exists nvramAppendList]} {
    append nvram4709 $nvramAppendList
}

set nvramAtlas {
    #"fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1" 
    #gmac3_enable=1
}

UTF::Router 4709 \
	-sta {4709/4366EMCM5 eth1 4709/4366EMCM5.%15 wl0.% 4709/4366MCH5l eth2 4709/4366MCH5l.%15 wl1.%} \
	-lan_ip 192.168.1.10 \
	-lanpeer {lan lan2} \
	-relay lan2 \
    -power {mc69npc1 2} \
    -console "mc69end2:40001" \
    -tag BISON04T_{BRANCH,REL}_7_14{,_?*} \
    -brand "linux-2.6.36-arm-external-vista-router-full-src" \
	-nvram $nvram4709 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8G \
    -embeddedimage {436{5,6}c} \
    -yart {-frameburst 1 -attn5g 15-100 -attn2g 30-100 -pad 10} \
    -noradio_pwrsave 1 -perfchans {161/80 161u 161} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump} {%S wl reset_cnts} \
    {%S wl pktq_stats C:[4366mcf19 macaddr]} {%S wl pktq_stats [4366mcf19a macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1} } \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl dump ampdu} \
    {%S wl ratedump} {%S wl dump phycal} \
    {%S wl assoclist} {%S wl sta_info [4366mcf19 macaddr]} {%S wl rssi [4366mcf19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcf19 macaddr]} \
    {%S wl sta_info [4366mcf19a macaddr]} {%S wl rssi [4366mcf19a macaddr]} \                     
    {%S wl pktq_stats [4366mcf19a macaddr]}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

set nvramAtlas2 {
    watchdog=3000
    wl_msglevel=0x101
    console_loglevel=7
    wl0_ssid=Atlas2MCH2
    wl0_chanspec=11
    wl0_obss_coex=0
    wl0_bw_cap=-1
    wl0_radio=0
    wl0_country_code=US/0
    wl1_ssid=Atlas2MCH5l
    wl1_chanspec=36
    wl1_obss_coex=0
    wl1_bw_cap=-1
    wl1_radio=0 
    wl1_country_code=US/0
    wl2_ssid=Atlas2MCH5h
    wl2_chanspec=161
    wl2_obss_coex=0
    wl2_bw_cap=-1
    wl2_radio=0
    wl2_country_code=US/0
    lan_ipaddr=192.168.1.20   
	lan_gateway=192.168.1.20
	dhcp_start=192.168.1.150
  	dhcp_end=192.168.1.179
	lan1_ipaddr=192.168.2.1
	lan1_gateway=192.169.2.1
	dhcp1_start=192.168.2.100
    dhcp1_end=192.168.2.149
}
    #wl0_pspretend_retry_limit=0
    #wl1_pspretend_retry_limit=0
    #wl2_pspretend_retry_limit=0

if {[info exists nvramAppendList]} {
    append nvramAtlas2 $nvramAppendList
}

    #-nopm1 1 -nopm2 1 \

UTF::Router Atlas2p416 \
    -sta {A2p416MCH5h eth3 A2p416MCH5h.%15 wl2.% A2p416MCH2 eth1 A2p416MCH2.%15 wl0.% A2p416MCH5l eth2 A2p416MCH5l.%15 wl1.%} \
    -relay lan2 \
    -lan_ip 192.168.1.20 \
    -power {mc69npc1 1} \
	-lanpeer {lan lan2} \
    -console "mc69end2:40001" \
    -tag "BISON04T_BRANCH_7_14{,_?*}" \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src" \
    -nvram $nvramAtlas2 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8G \
    -embeddedimage {436{5,6}b} \
    -yart {-frameburst 1 -attn5g 15-100 -attn2g 46-100 -pad 10} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl reset_cnts} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:[4366mcf19 macaddr]} {%S wl pktq_stats [4366mcf19a macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl ratedump} {%S wl dump phycal} \
    {%S wl assoclist} {%S wl sta_info [4366mcf19 macaddr]} {%S wl rssi [4366mcf19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcf19 macaddr]} \
    {%S wl sta_info [4366mcf19a macaddr]} {%S wl rssi [4366mcf19a macaddr]} \                     
    {%S wl pktq_stats [4366mcf19a macaddr]}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

set nvramAtlas2p502 {
    watchdog=3000
    wl_msglevel=0x101
    console_loglevel=7
    wl0_ssid=Atlas2MCH2
    wl0_chanspec=11
    wl0_obss_coex=0
    wl0_bw_cap=-1
    wl0_radio=0
    wl0_country_code=US/0
    wl1_ssid=Atlas2MCH5l
    wl1_chanspec=36
    wl1_obss_coex=0
    wl1_bw_cap=-1
    wl1_radio=0 
    wl1_country_code=US/0
    wl2_ssid=Atlas2MCH5h
    wl2_chanspec=161
    wl2_obss_coex=0
    wl2_bw_cap=-1
    wl2_radio=0
    wl2_country_code=US/0
    lan_ipaddr=192.168.1.30   
	lan_gateway=192.168.1.30
	dhcp_start=192.168.1.150
  	dhcp_end=192.168.1.179
	lan1_ipaddr=192.168.2.1
	lan1_gateway=192.169.2.1
	dhcp1_start=192.168.2.100
    dhcp1_end=192.168.2.149
}
UTF::Router Atlas2p502 \
    -sta {A2p502MCH5h eth3 A2p502MCH5h.%15 wl2.% A2p502MCH2 eth1 A2p502MCH2.%15 wl0.% A2p502MCH5l eth2 A2p502MCH5l.%15 wl1.%} \
    -relay lan2 \
    -lan_ip 192.168.1.30 \
    -power {mc69npc3 1} \
	-lanpeer {lan lan2} \
    -console "mc69end2:40000" \
    -tag "BISON04T_BRANCH_7_14{,_?*}" \
    -brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src" \
    -nvram $nvramAtlas2p502 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8G \
    -embeddedimage {436{5,6}c} \
    -yart {-frameburst 1 -attn5g 15-100 -attn2g 46-100 -pad 10} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl reset_cnts} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:[4366mcf19 macaddr]} {%S wl pktq_stats [4366mcf19a macaddr]}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl dump shmem} {%S wl dump shmemx} \
    {%S wl dump txbf} {%S wl dump mutx}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} \
    {%S wl dump scb} {%S et counters} {%S wl counters} {%S wl dump ampdu} {%S wl txmaxpkts} \
    {%S wl ratedump} {%S wl dump phycal} \
    {%S wl assoclist} {%S wl sta_info [4366mcf19 macaddr]} {%S wl rssi [4366mcf19 macaddr]} \                     
    {%S wl pktq_stats C:[4366mcf19 macaddr]} \
    {%S wl sta_info [4366mcf19a macaddr]} {%S wl rssi [4366mcf19a macaddr]} \                     
    {%S wl pktq_stats [4366mcf19a macaddr]} \
    {%S wl dump txbf} {%S wl dump mutx}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}


package require UTF::DSL
UTF::DSL 4908 -sta {4908a eth5 4908a.%8 wl0.% 4908b eth6 4908b.%8 wl1.% 4908c eth7 4908c.%8 wl2.%} \
    -model 4908 \
    -jenkins http://bcacpe-hudson.broadcom.com/hnd \
    -relay lan2 \
    -tag HND_502L01_94908GW_GMAC_HND \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_DEBUG_int \
    -type bcm94908HND_DEBUG_nand_fs_image_128_ubi.w \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan lan2} \
    -console "mc69end2:40002" \
    -relay lan2 \
    -power {mc69npc1 2} \
    -nvram {
	wl0_ssid=4908_4366mch5l
	wl1_ssid=4908_4366Emcm5
	wl2_ssid=4908_4366mcm2
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
    } \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g {0-90} -attn2g {48-90} -pad 29} \
    -noradio_pwrsave 1 \
    -wlinitcmds {
    wl -i eth6 phy_lesi
    }

#dummy configure -attngrp G1

4908a clone 4908_4366mch5l -sta {4908_4366mch5l eth5 4908_4366mch5l.%8 wl0.%} \
    -channelsweep {-min 100} -perfchans {161/80}
4908b clone 4908_4366Emcm5 -sta {4908_4366Emcm5 eth6 4908_4366Emcm5.%8 wl1.%} \
    -channelsweep {-max 64} -perfchans {36/80}
4908c clone 4908_4366mcm2 -sta {4908_4366mcm2 eth7 4908_4366mcm2.%8 wl2.%} \
    -perfchans {3}

#dummy destroy

set external {
    -jenkins "" \
    -tag BISON04T_TWIG_7_14_131 \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_external_full \
    -type bcm94908HND_nand_cferom_fs_image_128_ubi*.w
}

4908_4366mch5l clone 4908_4366mch5l-x {*}$external -perfonly 1

set internal {
    -jenkins "" \
    -tag BISON04T_TWIG_7_14_131 \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_DEBUG_int \
    -type bcm94908HND_DEBUG_nand_fs_image_128_ubi.w \
}

4908_4366mch5l clone 4908_4366mch5l-i {*}$internal
4908_4366Emcm5 clone 4908_4366Emcm5-i {*}$internal
4908_4366mcm2 clone 4908_4366mcm2-i  {*}$internal

4908_4366Emcm5-i clone 4908_4366Emcm5-LESIoff-TDCSoff-i \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth6 phy_lesi;
    wl -i eth6 phy_lesi 0;
    wl -i eth6 phy_lesi;
    wl -i eth6 smth_enable;
    wl -i eth6 smth_enable 0;
    wl -i eth6 smth_enable;
    }  
4908_4366Emcm5-i clone 4908_4366Emcm5-LESI-TDCSoff-i \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth6 phy_lesi;
    wl -i eth6 phy_lesi 1;
    wl -i eth6 phy_lesi;
    wl -i eth6 smth_enable;
    wl -i eth6 smth_enable 0;
    wl -i eth6 smth_enable;
    }  
4908_4366Emcm5-i clone 4908_4366Emcm5-LESI-TDCS-i \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth6 smth_enable;
    wl -i eth6 smth_enable 1;
    wl -i eth6 smth_enable;
    wl -i eth6 phy_lesi;
    wl -i eth6 phy_lesi 1;
    wl -i eth6 phy_lesi;
    }  
4908_4366Emcm5-i clone 4908_4366Emcm5-LESIoff-TDCS-i \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth6 phy_lesi;
    wl -i eth6 phy_lesi 0;
    wl -i eth6 phy_lesi;
    wl -i eth6 smth_enable;
    wl -i eth6 smth_enable 1;
    wl -i eth6 smth_enable;
    }  

if {[info exists env(AP_IMAGE)]} {
    4908_4366mch5l clone 4908_4366mch5l-p -image $env(AP_IMAGE)
    4908_4366Emcm5 clone 4908_4366Emcm5-p -image $env(AP_IMAGE)
    4908_4366mcm2 clone 4908_4366mcm2-p -image $env(AP_IMAGE)
    4908_4366Emcm5-LESIoff-TDCSoff-i clone 4908_4366Emcm5-LESIoff-TDCSoff-p -image $env(AP_IMAGE)
    4908_4366Emcm5-LESIoff-TDCS-i clone 4908_4366Emcm5-LESIoff-TDCS-p -image $env(AP_IMAGE)
    4908_4366Emcm5-LESI-TDCSoff-i clone 4908_4366Emcm5-LESI-TDCSoff-p -image $env(AP_IMAGE)
    4908_4366Emcm5-LESI-TDCS-i clone 4908_4366Emcm5-LESI-TDCS-p -image $env(AP_IMAGE)
}

if {[info exist env(AP_IMAGE)]} {
    4709/4366EMCM5 clone 4366EMCM5_dhda-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -nvram "$nvram4709" \
        -perfchans {161/80 161u 161}
    4366EMCM5_dhda-ctdma0-dm0 clone 4366EMCM5_dhda-ctdma0-dm1 \
        -nvram "$nvram4709 $nvramDM"
    4366EMCM5_dhda-ctdma0-dm0 clone 4366EMCM5_dhda-ctdma1-dm0 \
        -nvram "$nvram4709 $nvramCTDMA"
    4366EMCM5_dhda-ctdma0-dm0 clone 4366EMCM5_dhda-ctdma1-dm1 \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    4366EMCM5_dhda-ctdma0-dm0 clone 4366EMCM5_dhda-dm1-mu1 \
        -nvram "$nvram4709 $nvramDM $nvramMUauto"

    4709/4366MCH5l clone 4366MCH5l_dhda-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -nvram "$nvram4709 $nvramAtlas" \
        -perfchans {36/80 36l 36}
    4366MCH5l_dhda-ctdma0-dm0 clone 4366MCH5l_dhda-ctdma0-dm1 \
        -nvram "$nvram4709 $nvramDM"
    4366MCH5l_dhda-ctdma0-dm0 clone 4366MCH5l_dhda-ctdma1-dm0 \
        -nvram "$nvram4709 $nvramCTDMA"
    4366MCH5l_dhda-ctdma0-dm0 clone 4366MCH5l_dhda-ctdma1-dm1 \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    4366MCH5l_dhda-ctdma0-dm0 clone 4366MCH5l_dhda-dm1-mu1 \
        -nvram "$nvram4709 $nvramDM $nvramMUauto"

    4366EMCM5_dhda-ctdma0-dm0 configure -dualband {4366MCH5l_dhda-ctdma0-dm0 -c1 161/80 -c2 6l \
        -lan1 lan -lan2 lan2}
    4366EMCM5_dhda-ctdma0-dm1 configure -dualband {4366MCH5l_dhda-ctdma0-dm1 -c1 161/80 -c2 6l \
        -lan1 lan -lan2 lan2}
    4366EMCM5_dhda-ctdma1-dm0 configure -dualband {4366MCH5l_dhda-ctdma1-dm0 -c1 161/80 -c2 6l \
        -lan1 lan -lan2 lan2}
    4366EMCM5_dhda-ctdma1-dm1 configure -dualband {4366MCH5l_dhda-ctdma1-dm1 -c1 161/80 -c2 6l \
        -lan1 lan -lan2 lan2}
    4366EMCM5_dhda-dm1-mu1 configure -dualband {4366MCH5l_dhda-dm1 -c1 161/80 -c2 6l \
        -lan1 lan -lan2 lan2}

    A2p416MCH5h clone Atlas2MCH5h-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -perfchans {161 161u 161/80} -channelsweep {-min 100}
    Atlas2MCH5h-ctdma0-dm0 clone Atlas2MCH5h-ctdma0-dm1 \
        -nvram "$nvram4709 $nvramDM"
    Atlas2MCH5h-ctdma0-dm0 clone Atlas2MCH5h-ctdma1-dm0 \
        -nvram "$nvram4709 $nvramCTDMA"
    Atlas2MCH5h-ctdma0-dm0 clone Atlas2MCH5h-ctdma1-dm1 \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    Atlas2MCH5h-ctdma0-dm0 clone Atlas2MCH5h-dm0-mu1 \
        -nvram "$nvram4709 $nvramMUauto"

    A2p416MCH5l clone Atlas2MCH5l-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -perfchans {36 36l 36/80} -channelsweep {-max 64}
    Atlas2MCH5l-ctdma0-dm0 clone Atlas2MCH5l-ctdma0-dm1 \
        -nvram "$nvram4709 $nvramDM"
    Atlas2MCH5l-ctdma0-dm0 clone Atlas2MCH5l-ctdma1-dm0 \
        -nvram "$nvram4709 $nvramCTDMA"
    Atlas2MCH5l-ctdma0-dm0 clone Atlas2MCH5l-dm0-mu1 \
        -nvram "$nvram4709 $nvramMUauto"
    
    A2p416MCH2 clone Atlas2MCH2-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -perfchans {6 6l}
    Atlas2MCH2-ctdma0-dm0 clone Atlas2MCH2-ctdma0-dm1 \
         -nvram "$nvram4709 $nvramDM"
    Atlas2MCH2-ctdma0-dm0 clone Atlas2MCH2-ctdma1-dm0 \
         -nvram "$nvram4709 $nvramCTDMA"
    Atlas2MCH2-ctdma0-dm0 clone Atlas2MCH2-ctdma1-dm1 \
         -nvram "$nvram4709 $nvramCTDMA $nvramDM"

    A2p502MCH5h clone A2p502MCH5h-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -perfchans {161 161u 161/80} -channelsweep {-min 100}
    A2p502MCH5h-ctdma0-dm0 clone A2p502MCH5h-ctdma0-dm1 \
        -nvram "$nvram4709 $nvramDM"
    A2p502MCH5h-ctdma0-dm0 clone A2p502MCH5h-ctdma1-dm0 \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p502MCH5h-ctdma0-dm0 clone A2p502MCH5h-ctdma1-dm1 \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p502MCH5h-ctdma0-dm0 clone A2p502MCH5h-dm0-mu1 \
        -nvram "$nvram4709 $nvramMUauto"

    A2p502MCH5l clone A2p502MCH5l-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -perfchans {36 36l 36/80} -channelsweep {-max 64}
    A2p502MCH5l-ctdma0-dm0 clone A2p502MCH5l-ctdma0-dm1 \
        -nvram "$nvram4709 $nvramDM"
    A2p502MCH5l-ctdma0-dm0 clone A2p502MCH5l-ctdma1-dm0 \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p502MCH5l-ctdma0-dm0 clone A2p502MCH5l-ctdma1-dm1 \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p502MCH5l-ctdma0-dm0 clone A2p502MCH5l-dm0-mu1 \
        -nvram "$nvram4709 $nvramMUauto"
    
    A2p502MCH2 clone A2p502MCH2-ctdma0-dm0 \
        -image $env(AP_IMAGE) \
        -perfchans {6 6l}
    A2p502MCH2-ctdma0-dm0 clone A2p502MCH2-ctdma0-dm1 \
         -nvram "$nvram4709 $nvramDM"
    A2p502MCH2-ctdma0-dm0 clone A2p502MCH2-ctdma1-dm0 \
         -nvram "$nvram4709 $nvramCTDMA"
    A2p502MCH2-ctdma0-dm0 clone A2p502MCH2-ctdma1-dm1 \
         -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p502MCH2 configure -dualband {A2p502MCH5h -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p502MCH5h configure -dualband {A2p502MCH5l -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p502MCH5l configure -dualband {A2p502MCH2 -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
} else {
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
        set tag_g3 BISON04T_REL_7_14_1{??}
    }
    puts "#### tag_b = $tag_g3 ####"

    ### for internal images
    set brand "linux-2.6.36-arm-internal-router"
    4709/4366EMCM5 clone 4366EMCM5b-i -tag $tag_b \
        -brand $brand \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5lb-i -tag $tag_b \
        -brand $brand \
        -perfchans {36/80 36l 36}

    set brand linux-2.6.36-arm-internal-router-dhdap
    4709/4366EMCM5 clone 4366EMCM5b_dhd-i -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709 $nvramAtlas" \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5lb_dhd-i -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709 $nvramAtlas" \
        -perfchans {36/80 36l 36}

    4366EMCM5b_dhd-i configure -dualband {4366MCH5lb_dhd-i -c1 161/80 -c2 6l \
        -lan1 lan -lan2 lan2}

    set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
    foreach obj "4366EMCM5b_dhda 4366EMCM5b_dhda-g3" tag "$tag_b $tag_g3" {
        4709/4366EMCM5 clone ${obj}-ctdma0-dm0-i -tag $tag \
            -brand $brand \
            -nvram "$nvram4709 $nvramAtlas" \
            -perfchans {161/80 161u 161}
        ${obj}-ctdma0-dm0-i clone ${obj}-ctdma0-dm1-i \
            -nvram "$nvram4709 $nvramDM"
        ${obj}-ctdma0-dm0-i clone ${obj}-ctdma1-dm0-i \
            -nvram "$nvram4709 $nvramCTDMA"
        ${obj}-ctdma0-dm0-i clone ${obj}-ctdma1-dm1-i \
            -nvram "$nvram4709 $nvramCTDMA $nvramDM"
        ${obj}-ctdma0-dm0-i clone ${obj}-dm0-mu1-i \
            -nvram "$nvram4709 $nvramMUauto"
        ${obj}-ctdma0-dm0-i clone ${obj}-dm1-mu1-i \
            -nvram "$nvram4709 $nvramDM $nvramMUauto"
    }

    4709/4366MCH5l clone 4366MCH5lb_dhda-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709 $nvramAtlas" \
        -perfchans {36/80 36l 36}
    4366MCH5lb_dhda-ctdma0-dm0-i clone 4366MCH5lb_dhda-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    4366MCH5lb_dhda-ctdma0-dm0-i clone 4366MCH5lb_dhda-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    4366MCH5lb_dhda-ctdma0-dm0-i clone 4366MCH5lb_dhda-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    4366MCH5lb_dhda-ctdma0-dm0-i clone 4366MCH5lb_dhda-dm0-mu1-i \
        -nvram "$nvram4709 $nvramMUauto"
    4366MCH5lb_dhda-ctdma0-dm0-i clone 4366MCH5lb_dhda-dm1-mu1-i \
        -nvram "$nvram4709 $nvramDM $nvramMUauto"

    4366EMCM5b_dhda-ctdma0-dm0-i clone 4366EMCM5b_dm0-LESIoff-TDCSoff-i \
    -wlinitcmds {
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 0;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 0;
    wl -i eth1 smth_enable;
    }
    4366EMCM5b_dhda-ctdma0-dm0-i clone 4366EMCM5b_dm0-LESI-TDCSoff-i \
    -wlinitcmds {
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 1;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 0;
    wl -i eth1 smth_enable;
    }
    4366EMCM5b_dhda-ctdma0-dm0-i clone 4366EMCM5b_dm0-LESI-TDCS-i \
    -wlinitcmds {
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 1;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 1;                                                                                                                                                                                                                
    wl -i eth1 smth_enable;                                                                                                                                                                                                                  
    }                                                                                                                                                                                                                                        
    4366EMCM5b_dhda-ctdma0-dm0-i clone 4366EMCM5b_dm0-LESIoff-TDCS-i \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth1 phy_lesi;                                                                                                                                                                                                                     
    wl -i eth1 phy_lesi 0;                                                                                                                                                                                                                   
    wl -i eth1 phy_lesi;                                                                                                                                                                                                                     
    wl -i eth1 smth_enable;                                                                                                                                                                                                                  
    wl -i eth1 smth_enable 1;                                                                                                                                                                                                                
    wl -i eth1 smth_enable;                                                                                                                                                                                                                  
    }                                                                                                                                                                                                                                        
           
    A2p416MCH5h clone A2p416MCH5hb-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -perfchans {161 161u 161/80} -channelsweep {-min 100}
    A2p416MCH5hb-ctdma0-dm0-i clone A2p416MCH5hb-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    A2p416MCH5hb-ctdma0-dm0-i clone A2p416MCH5hb-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p416MCH5hb-ctdma0-dm0-i clone A2p416MCH5hb-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p416MCH5hb-ctdma0-dm0-i clone A2p416MCH5hb-dm0-mu1-i \
        -nvram "$nvram4709 $nvramMUauto"

    A2p416MCH5l clone A2p416MCH5lb-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -perfchans {36 36l 36/80} -channelsweep {-max 64}
    A2p416MCH5lb-ctdma0-dm0-i clone A2p416MCH5lb-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    A2p416MCH5lb-ctdma0-dm0-i clone A2p416MCH5lb-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p416MCH5lb-ctdma0-dm0-i clone A2p416MCH5lb-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p416MCH5lb-ctdma0-dm0-i clone A2p416MCH5lb-dm0-mu1-i \
        -nvram "$nvram4709 $nvramMUauto"

    A2p416MCH2 clone A2p416MCH2b-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -perfchans {6 6l}
    A2p416MCH2b-ctdma0-dm0-i clone A2p416MCH2b-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    A2p416MCH2b-ctdma0-dm0-i clone A2p416MCH2b-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p416MCH2b-ctdma0-dm0-i clone A2p416MCH2b-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"

    A2p502MCH5h clone A2p502MCH5hb-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -perfchans {161 161u 161/80} -channelsweep {-min 100}
    A2p502MCH5hb-ctdma0-dm0-i clone A2p502MCH5hb-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    A2p502MCH5hb-ctdma0-dm0-i clone A2p502MCH5hb-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p502MCH5hb-ctdma0-dm0-i clone A2p502MCH5hb-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p502MCH5hb-ctdma0-dm0-i clone A2p502MCH5hb-dm0-mu1-i \
        -nvram "$nvram4709 $nvramMUauto"
    A2p502MCH5hb-dm0-mu1-i clone A2p502MCH5hb-g3-dm0-mu1-i \
        -tag $tag_g3
    A2p502MCH5hb-ctdma0-dm0-i clone A2p502MCH5hb-g3-ctdma0-dm0-i \
        -tag $tag_g3

    A2p502MCH5l clone A2p502MCH5lb-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -perfchans {36 36l 36/80} -channelsweep {-max 64}
    A2p502MCH5lb-ctdma0-dm0-i clone A2p502MCH5lb-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    A2p502MCH5lb-ctdma0-dm0-i clone A2p502MCH5lb-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p502MCH5lb-ctdma0-dm0-i clone A2p502MCH5lb-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p502MCH5lb-ctdma0-dm0-i clone A2p502MCH5lb-dm0-mu1-i \
        -nvram "$nvram4709 $nvramMUauto"
    A2p502MCH5lb-dm0-mu1-i clone A2p502MCH5lb-g3-dm0-mu1-i \
        -tag $tag_g3
    A2p502MCH5lb-ctdma0-dm0-i clone A2p502MCH5lb-g3-ctdma0-dm0-i \
        -tag $tag_g3

    A2p502MCH2 clone A2p502MCH2b-ctdma0-dm0-i -tag $tag_b \
        -brand $brand \
        -perfchans {6 6l}
    A2p502MCH2b-ctdma0-dm0-i clone A2p502MCH2b-ctdma0-dm1-i \
        -nvram "$nvram4709 $nvramDM"
    A2p502MCH2b-ctdma0-dm0-i clone A2p502MCH2b-ctdma1-dm0-i \
        -nvram "$nvram4709 $nvramCTDMA"
    A2p502MCH2b-ctdma0-dm0-i clone A2p502MCH2b-ctdma1-dm1-i \
        -nvram "$nvram4709 $nvramCTDMA $nvramDM"
    A2p502MCH2b-ctdma0-dm0-i clone A2p502MCH2b-g3-ctdma0-dm0-i \
        -tag $tag_g3
    A2p502MCH2b-ctdma0-dm0-i clone A2p502MCH2b-g3-ctdma0-dm0-i \
        -tag $tag_g3

    ### for external images
    set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
    4709/4366EMCM5 clone 4366EMCM5b_dhd-ctdma0-dm0 -tag $tag_b   \
        -brand $brand \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5lb_dhd-ctdma0-dm0 -tag $tag_b   \
        -brand $brand \
        -perfchans {36/80 36l 36}

    set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
    4709/4366EMCM5 clone 4366EMCM5b_dhda-ctdma0-dm0 -tag $tag_b   \
        -brand $brand \
        -perfchans {161/80 161u 161}
    4366EMCM5b_dhda-ctdma0-dm0 clone 4366EMCM5b_dm0-LESIoff-TDCSoff-e \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 0;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 0;
    wl -i eth1 smth_enable;
    }  
    4366EMCM5b_dhda-ctdma0-dm0 clone 4366EMCM5b_dm0-LESI-TDCSoff-e \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 1;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 0;
    wl -i eth1 smth_enable;
    }  
    4366EMCM5b_dhda-ctdma0-dm0 clone 4366EMCM5b_dm0-LESI-TDCS-e \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 1;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 1;
    wl -i eth1 smth_enable;
    }  
    4366EMCM5b_dhda-ctdma0-dm0 clone 4366EMCM5b_dm0-LESIoff-TDCS-e \
    -wlinitcmds {                                                                                                                                                                                                                            
    wl -i eth1 phy_lesi;
    wl -i eth1 phy_lesi 0;
    wl -i eth1 phy_lesi;
    wl -i eth1 smth_enable;
    wl -i eth1 smth_enable 1;
    wl -i eth1 smth_enable;
    }  

    4709/4366MCH5l clone 4366MCH5lb_dhda-ctdma0-dm0 -tag $tag_b \
        -brand $brand \
        -perfchans {36/80 36l 36}

    A2p416MCH2b-ctdma0-dm0-i configure -dualband {A2p416MCH5hb-ctdma0-dm0-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p416MCH2b-ctdma0-dm1-i configure -dualband {A2p416MCH5hb-ctdma0-dm1-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p416MCH2b-ctdma1-dm0-i configure -dualband {A2p416MCH5hb-ctdma1-dm0-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p416MCH2b-ctdma1-dm1-i configure -dualband {A2p416MCH5hb-ctdma1-dm1-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p416MCH2b-ctdma1-dm0-i configure -dualband {A2p416MCH5hb-dm0-mu1-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p416MCH5hb-ctdma0-dm0-i configure -dualband {A2p416MCH5lb-ctdma0-dm0-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p416MCH5hb-ctdma0-dm1-i configure -dualband {A2p416MCH5lb-ctdma0-dm1-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p416MCH5hb-ctdma1-dm0-i configure -dualband {A2p416MCH5lb-ctdma1-dm0-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p416MCH5hb-ctdma1-dm1-i configure -dualband {A2p416MCH5lb-ctdma1-dm1-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p416MCH5hb-dm0-mu1-i configure -dualband {A2p416MCH5lb-dm0-mu1-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p416MCH5lb-ctdma0-dm0-i configure -dualband {A2p416MCH2b-ctdma0-dm0-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p416MCH5lb-ctdma0-dm1-i configure -dualband {A2p416MCH2b-ctdma0-dm1-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p416MCH5lb-ctdma1-dm0-i configure -dualband {A2p416MCH2b-ctdma1-dm0-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p416MCH5lb-ctdma1-dm1-i configure -dualband {A2p416MCH2b-ctdma1-dm1-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p416MCH5lb-dm0-mu1-i configure -dualband {A2p416MCH2b-ctdma1-dm0-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}

    A2p502MCH2b-ctdma0-dm0-i configure -dualband {A2p502MCH5hb-ctdma0-dm0-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p502MCH2b-ctdma0-dm1-i configure -dualband {A2p502MCH5hb-ctdma0-dm1-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p502MCH2b-ctdma1-dm0-i configure -dualband {A2p502MCH5hb-ctdma1-dm0-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p502MCH2b-ctdma1-dm1-i configure -dualband {A2p502MCH5hb-ctdma1-dm1-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p502MCH2b-ctdma1-dm0-i configure -dualband {A2p502MCH5hb-dm0-mu1-i -c1 1l -c2 161/80 -lan1 lan -lan2 lan2}
    A2p502MCH5hb-ctdma0-dm0-i configure -dualband {A2p502MCH5lb-ctdma0-dm0-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p502MCH5hb-ctdma0-dm1-i configure -dualband {A2p502MCH5lb-ctdma0-dm1-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p502MCH5hb-ctdma1-dm0-i configure -dualband {A2p502MCH5lb-ctdma1-dm0-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p502MCH5hb-ctdma1-dm1-i configure -dualband {A2p502MCH5lb-ctdma1-dm1-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p502MCH5hb-dm0-mu1-i configure -dualband {A2p502MCH5lb-dm0-mu1-i -c1 161/80 -c2 36/80 -lan1 lan -lan2 lan2}
    A2p502MCH5lb-ctdma0-dm0-i configure -dualband {A2p502MCH2b-ctdma0-dm0-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p502MCH5lb-ctdma0-dm1-i configure -dualband {A2p502MCH2b-ctdma0-dm1-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p502MCH5lb-ctdma1-dm0-i configure -dualband {A2p502MCH2b-ctdma1-dm0-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p502MCH5lb-ctdma1-dm1-i configure -dualband {A2p502MCH2b-ctdma1-dm1-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p502MCH5lb-dm0-mu1-i configure -dualband {A2p502MCH2b-ctdma1-dm0-i -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
}

# Eagle STA clones    
if {[info exist env(AP_IMAGE)]} {
    4709/4366EMCM5 clone 4366EMCM5e-i \
        -image $env(AP_IMAGE) \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5le-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l i36}

    4709/4366EMCM5 clone 4366EMCM5e \
        -image $env(AP_IMAGE) \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5le \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
} else {
    ### for internal images
    if {[info exist env(TAG_E)]} {
        set tag_e $env(TAG_E)
    } else {
        set tag_e EAGLE_REL_10_10_63{,_?*}
    }
    puts "#### tag_e = $tag_e ####"

    set brand "linux-2.6.36-arm-internal-router"
    4709/4366EMCM5 clone 4366EMCM5e-i -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161}
    4709/4366MCH5l clone 4366MCH5le-i -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    set brand linux-2.6.36-arm-internal-router-dhdap
    4709/4366EMCM5 clone 4366EMCM5e_dhd-i -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5le_dhd-i -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    ### for external images
    set brand "linux-2.6.36-arm-external-vista-router-full-src" 
    4709/4366EMCM5 clone 4366EMCM5e -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5le -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
    4709/4366EMCM5 clone 4366EMCM5e_dhd -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161}

    4709/4366MCH5l clone 4366MCH5le_dhd -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
    A2p416MCH5h clone A2p416MCH5he -tag $tag_e \
        -brand $brand \
        -perfchans {161/80 161u 161} -channelsweep {-min 100}
    A2p416MCH5l clone A2p416MCH5le -tag $tag_e \
        -brand $brand \
        -perfchans {36/80 36l 36} -channelsweep {-max 64}
    A2p416MCH2 clone A2p416MCH2e -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}
    A2p416MCH5he configure -dualband {A2p416MCH2e -c1 161/80 -c2 1l -lan1 lan -lan2 lan2}
    A2p416MCH5le configure -dualband {A2p416MCH2e -c1 36/80 -c2 1l -lan1 lan -lan2 lan2}
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

UTF::Q mc69

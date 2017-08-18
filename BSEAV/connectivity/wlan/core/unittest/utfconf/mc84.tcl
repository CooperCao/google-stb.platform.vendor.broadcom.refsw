

# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum MC69 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::AeroflexDirect
package require UTF::Linux
package require UTF::Sniffer

#package require UTF::TclReadLines
#UTF::Power::WebRelay pb6awr -relay svt-utf2-end1 -lan_ip 172.19.12.32 -invert {2 3}
#Linux endpoints WebRelay - 10.19.86.134
#UTF::Power::WebRelay usbwr -relay svt-utf2-end1 -lan_ip 172.19.12.32

set UTF::WebTree 1
set UTF::DataRatePlot 1

# set devicemode; default=0
global env
if {[info exist env(DEVICEMODE)] && ($env(DEVICEMODE) != "0")} {
    append nvramAppendList "devicemode=$env(DEVICEMODE) "
} 

#set nvramMU "wl0_mu_features=1 wl1_mu_features=1"
set nvramMU "wl0_mu_features=-1"
set confRSDB "wl rsdb_mode rsdb"
set confSU "wl txbf_bfe_cap 1"
set nvramCTDMA ""
#set nvramCTDMA "devpath1=pcie/1/1/ 1:ctdma=1 devpath2=pcie/2/3/ 2:ctdma=1 "

#if {[info exist env(DCACHE)] && ($env(DCACHE) != "0")} {
#    append nvramAppendList "1:cache_en=$env(DCACHE) "
#    append nvramAppendList "1:b0war_en=$env(DCACHE) "
#    append nvramAppendList "devpath1=pcie/1/1/ "
#}
if {[info exist env(NVRAM)] && ($env(NVRAM) != "0")} {
    append nvramAppendList $env(NVRAM)
}
if {[info exist env(CPUCLK)] && [string equal $env(CPUCLK) "default"] == 0} {
    append nvramAppendList "1:cpuclk=$env(CPUCLK) "
}

#UTF::Cygwin VWstation -lan_ip 172.19.12.239 -user user -sta {}


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

set ::UTF::SummaryDir "/projects/hnd_sig_ext10/sunnyc/mc84"

# Set default to use wl from trunk, Use -app_tag to modify
#set UTF::TurnkApps 1

# Define power controllers on cart
UTF::Power::Synaccess mc33npc1 -lan_ip 172.16.1.4 -rev 1
UTF::Power::Synaccess mc33npc2 -lan_ip 172.16.1.5
UTF::Power::Synaccess mc33npc3 -lan_ip 172.16.1.7

# Attenuator - Aeroflex
UTF::AeroflexDirect af -lan_ip 172.16.1.210 \
    -group {G1 {1 2 3 4} G2 {5 6 7 8} ALL {1 2 3 4 5 6 7 8 9}}

ALL configure -default 0


# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default;
 
    foreach S {4359-1 4359-2 4359-3 4359-4} {
        catch {$S wl down}
        $S deinit
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {4709C0/4366MC 4709C0/43602MCH2} {
        catch {$S apshell wl -i [$S cget -device] down}
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #4708 restart wl0_radio=0
    #AP2 restart wl0_radio=0
    #AP5 restart wl0_radio=0

    #svt-utf2-tst2 add_netconsole svt-utf2-end1 6661
    #svt-utf2-tst2 service netconsole start

    # delete myself
    unset ::UTF::SetupTestBed

    return
}


# UTF Endpoint1 f19 - Traffic generators (no wireless cards)
UTF::Linux mc84end1 -lan_ip 10.19.85.71 \
    -sta {lan p4p1}

#UTF::Linux svt-utf2end2 \
#    -sta {lan2 enp0s20u1c2} 

#lan2 configure -ipaddr 192.168.1.96

# Define Sniffer
UTF::Sniffer SNIF -user root \
        -lan_ip mc33snf1 \
        -sta {4360snif enp5s0} \
        -power {mc33npc4 1} \
        -power_button "auto" \
        -tag BISON_BRANCH_7_10 \
        -tcpwindow 16M \
        -wlinitcmds {}

# set to 1x1
set wic "wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0"

UTF::DHD STA-1  \
    -lan_ip mc33tst1 \
    -sta {4359-1 eth0} \
    -power {mc33npc1 2} \
    -power_button "auto" \
    -hostconsole "mc84end1:40000" \
    -dhd_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -app_tag trunk \
    -tag DIN07T48RC50_BRANCH_9_75 \
    -clm_blob "ss_mimo.clm_blob" \
    -nocal 1 -slowassoc 10 \
    -nvram "bcm943593fcpagbss.txt" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
    -udp 800m -tcpwindow 2m \
    -yart {-attn5g 0-95 -attn2g 0-95 -pad 0} 


4359-1 configure -attngrp G1

if {[info exist env(STA_IMAGE)]} {
    4359-1 clone 4359musta1 \
    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -image $env(STA_IMAGE)
} else {
    4359-1 clone 4359susta1 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin 
    4359susta1 clone 2x2susta1 -wlinitcmds "wl down; $wic; $confSU; wl up"

    4359-1 clone 4359musta1 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin 
    4359musta1 clone 2x2musta1 -wlinitcmds "wl down; $wic; wl up"
}


#4359-1 clone 4359susta1 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic; $confSU; $confRSDB; wl up" \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin 
    
#4359susta1 clone 4359susta1-2x2 -wlinitcmds "wl down; $wic; $confSU; wl up"


#4359-1 clone 4359musta1 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin

#4359musta1 clone 4359musta1-2x2 \
#    -wlinitcmds  -wlinitcmds "wl down; $wic; wl up"
 

UTF::DHD STA-2  \
    -lan_ip mc33tst2 \
    -sta {4359-2 eth0} \
    -power {mc33npc2 1} \
    -power_button "auto" \
    -hostconsole "mc84end1:40003" \
    -dhd_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -app_tag trunk \
    -tag DIN07T48RC50_BRANCH_9_75 \
    -clm_blob "ss_mimo.clm_blob" \
    -nocal 1 -slowassoc 10 \
    -nvram "bcm943593fcpagbss.txt" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
    -udp 800m -tcpwindow 2m \
    -yart {-attn5g 0-95 -attn2g 0-95 -pad 0}
  

4359-2 configure -attngrp G1

if {[info exist env(STA_IMAGE)]} {
    4359-2 clone 4359musta2 \
	-wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -image $env(STA_IMAGE)
} else {
    4359-2 clone 4359susta2 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin

    4359susta2 clone 2x2susta2 -wlinitcmds "wl down; $wic; $confSU; wl up"

    4359-2 clone 4359musta2 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin

    4359musta2 clone 2x2musta2 -wlinitcmds "wl down; $wic; wl up"
}


#    4359-2 clone 4359susta2 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#	-wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl up" \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin

#    4359susta2 clone 4359susta2-2x2 -wlinitcmds "wl down; $wic; $confSU; wl up"

#4359-2 clone 4359musta2 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin


UTF::DHD STA-3  \
    -lan_ip mc33tst3 \
    -sta {4359-3 eth0} \
    -power {mc33npc2 2} \
    -power_button "auto" \
    -hostconsole "mc84end1:40002" \
    -dhd_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -app_tag trunk \
    -tag DIN07T48RC50_BRANCH_9_75 \
    -nocal 1 -slowassoc 10 \
    -nvram "bcm943593fcpagbss.txt" \
    -clm_blob "ss_mimo.clm_blob" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode rsdb;} \
    -udp 800m -tcpwindow 2m \
    -yart {-attn5g 0-95 -attn2g 0-95 -pad 0} 


4359-3 configure -attngrp G1

# for 4359C0
if {[info exist env(STA_IMAGE)]} {
    4359-3 clone 4359musta3 \
        -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -image $env(STA_IMAGE)
} else {
    4359-3 clone 4359susta3 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin 

    4359susta3 clone 2x2susta3 -wlinitcmds "wl down; $wic; $confSU; wl up"

    4359-3 clone 4359musta3 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin

    4359musta3 clone 2x2musta3 -wlinitcmds "wl down; $wic; wl up"
}


# for 4359C0
#4359-3 clone 4359musta3 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin


UTF::DHD STA-4  \
    -lan_ip mc33tst4 \
    -sta {4359-4 eth0} \
    -power {mc33npc3 1} \
    -power_button "auto" \
    -hostconsole "mc84end1:40004" \
    -dhd_tag trunk \
    -dhd_brand linux-internal-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -app_tag trunk \
    -tag DIN07T48RC50_BRANCH_9_75 \
    -nocal 1 -slowassoc 10 \
    -nvram "bcm943593fcpagbss.txt" \
    -clm_blob "ss_mimo.clm_blob" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl infra 1; wl rsdb_mode rsdb;} \
    -udp 800m -tcpwindow 2m \
    -yart {-attn5g 0-95 -attn2g 0-95 -pad 0} 


4359-4 configure -attngrp G1

# for 4359C0
if {[info exist env(STA_IMAGE)]} {
    4359-4 clone 4359musta4 \
    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -image $env(STA_IMAGE)
} else {
    4359-4 clone 4359susta4 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \

    4359susta4 clone 2x2susta4 -wlinitcmds "wl down; $wic; $confSU; wl up"

    4359-4 clone 4359musta4 -tag DIN07T48RC50_REL_9_75{,_?*} \
    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \

    4359musta4 clone 2x2musta4 -wlinitcmds "wl down; $wic; wl up"
}

# for 4359C0
#4359-4 clone 4359musta4 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin


# router config

set nvram4709C0 { 
watchdog=3000
wl_msglevel=0x101
console_loglevel=7
wl0_ssid=BroadcomA
wl0_chanspec=36
wl0_obss_coex=0
wl0_bw_cap=-1
wl0_radio=0
wl0_vht_features=7
wl0_country_code=US/0
wl1_ssid=BroadcomG
wl1_chanspec=3
wl1_obss_coex=0
wl1_bw_cap=-1
wl1_radio=0
wl1_vht_features=7
wl1_country_code=US/0
lan_ipaddr=192.168.1.1
lan_gateway=192.168.1.1
dhcp_start=192.168.1.100
dhcp_end=192.168.1.149
lan1_ipaddr=192.168.2.1
lan1_gateway=192.169.2.1
dhcp1_start=192.168.2.100
}   
set nvramAtlas {
    "fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
    gmac3_enable=1
}

if {[info exists nvramAppendList]} {
    append nvram4709C0 $nvramAppendList
}

UTF::Router AP \
    -sta {4709C0/4366MC eth1 4709C0/4366MC.%15 wl0.% 4709C0/43602MCH2 eth2 4709C0/43602MCH2.%15 wl1.%} \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan} \
    -relay "mc84end1" \
    -power {mc33npc1 1} \
    -console "mc84end1:40001" \
    -tag BISON04T_TWIG_7_14_131 \
    -brand "linux-2.6.36-arm-internal-router-dhdap-atlas" \
    -nvram $nvram4709C0 \
    -nopm1 1 -nopm2 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 0-95 -attn2g 0-95 -pad 0} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -embeddedimage 4365 \
    -pre_perf_hook {{%S et clear_dump} {%S et counters} {%S wl assoc} {%S wl ampdu_clear_dump}\
    {%S wl pktq_stats C:00:90:4c:74:c1:d8} {%S wl pktq_stats C:00:90:4c:74:c1:84}\
    {%S wl authe_sta_list} {%S wl autho_sta_list} {%S wl dump scb}\
    {%S wl assoclist} {%S wl ssid} {%S wl tempsense_disable 1}\
    {%S wl assert_type 0}} \
    -post_perf_hook {{%S wl dump rssi} {%S wl nrate} {%S wl reset_cnts} \
    {%S wl dump scb} {%S et counters} {%S wl counters} \
    {%S wl assoclist} {%S wl sta_info 00:90:4c:74:c1:d8} {%S wl rssi 00:90:4c:74:c1:d8} \
    {%S wl pktq_stats C:00:90:4c:74:c1:d8} \
    {%S wl sta_info 00:90:4c:74:c1:84} {%S wl rssi 00:90:4c:74:c1:84} \                     
    {%S wl pktq_stats C:00:90:4c:74:c1:84} {%S wl dump mutx}} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}
4709C0/4366MC configure -dualband {4709C0/4366MC -c1 161/80 -c2 6l}

if {[info exist env(AP_IMAGE)]} {
    4709C0/4366MC clone 4709C0/4366MCb_dhd-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36 6l 6}
    4709C0/4366MCb_dhd-i clone 4709C0/4366MCb_dhda-i

    4709C0/43602MCH2 clone 4709C0/43602MCH2b_dhd-i \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}

    4709C0/4366MC clone 4709C0/4366MCb-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36 6l 6}

    4709C0/4366MC clone 4709C0/4366MCb-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36 6l 6}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b-i \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}
    4709C0/43602MCH2b-i clone 4709C0/43602MCH2b_dhda-i
} else {
    # Bison STA clones
    if {[info exist env(TAG_B)]} {
        set tag_b $env(TAG_B)
    } else {
	#set tag_b EAGLE_BRANCH_10_10
        set tag_b BISON04T_TWIG_7_14_131
    }
    puts "#### tag_b = $tag_b ####"

    ### for internal images
    #set brand "linux-2.6.36-arm-internal-router"
    set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
    4709C0/4366MC clone 4709C0/4366MCb-i -tag $tag_b \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b-i -tag $tag_b \
        -brand $brand \
        -perfchans {6l 6}

    set brand linux-2.6.36-arm-internal-router-dhdap-atlas
    4709C0/4366MC clone 4709C0/4366MCb_dhd-i -tag $tag_b \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b_dhd-i -tag $tag_b \
        -brand $brand \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
    4709C0/4366MC clone 4709C0/4366MCb_dhda-i -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709C0 $nvramAtlas" \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b_dhda-i -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709C0 $nvramAtlas" \
        -perfchans {6l 6}

    4709C0/4366MCb_dhda-i clone 4709C0/4366MCb_dhda1-i
    4709C0/4366MCb_dhda-i clone 4709C0/4366MCb_dhda2-i

    ### for external images
    set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
    4709C0/4366MC clone 4709C0/4366MCb_dhda -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709C0 $nvramAtlas" \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b_dhda -tag $tag_b \
        -brand $brand \
        -nvram "$nvram4709C0 $nvramAtlas" \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-external-vista-router-full-src" 
    4709C0/4366MC clone 4709C0/4366MCb -tag $tag_b \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b -tag $tag_b \
        -brand $brand \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
    4709C0/4366MC clone 4709C0/4366MCb_dhd -tag $tag_b \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2b_dhd -tag $tag_b \
        -brand $brand \
        -perfchans {6l 6}
}

set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
4709C0/4366MCb_dhd-i clone 4366C0-fdmutx1 \
    -nvram "$nvram4709C0 $nvramMU $nvramCTDMA $nvramAtlas"
4709C0/4366MCb_dhd-i clone 4366C0-fdmutx0 \
    -nvram "$nvram4709C0 $nvramAtlas"

4366C0-fdmutx1 clone 4366C0-fdmutx1-tob \
     -tag BISON04T_BRANCH_7_14
4366C0-fdmutx0 clone 4366C0-fdmutx0-tob \
     -tag BISON04T_BRANCH_7_14
    

4709C0/4366MCb_dhd-i configure -dualband {4709C0/4366MCb_dhd-i -c1 161/80 -c2 6l}
4709C0/4366MCb_dhda-i configure -dualband {4709C0/4366MCb_dhda-i -c1 161/80 -c2 6l}
4709C0/43602MCH2b_dhd-i configure -dualband {4709C0/4366MCb_dhd-i -c2 161/80 -c1 6l}

# Eagle STA clones    
if {[info exist env(AP_IMAGE)]} {
    4709C0/4366MC clone 4709C0/4366MCe-i \
        -image $env(AP_IMAGE) \
        -nvram $nvram4709C0 \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2e-i \
        -image $env(AP_IMAGE) \
        -perfchans {6l 6}

} else {
    ### for internal images
    if {[info exist env(TAG_E)]} {
        set tag_e $env(TAG_E)
    } else {
        set tag_e EAGLE_TWIG_10_10_69{,_?*}
    }
    puts "#### tag_e = $tag_e ####"

    set brand "linux-2.6.36-arm-internal-router"
    4709C0/4366MC clone 4709C0/4366MCe-i -tag $tag_e \
        -brand $brand \
        -nvram $nvram4709C0 \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2e-i -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    set brand linux-2.6.36-arm-internal-router-dhdap
    4709C0/4366MC clone 4709C0/4366MCe_dhd-i -tag $tag_e \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/4366MCe_dhd-i clone 4709C0/4366MCe_dhd1
    4709C0/4366MCe_dhd-i clone 4709C0/4366MCe_dhd2

    4709C0/43602MCH2 clone 4709C0/43602MCH2e_dhd-i -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    ### for external images
    set brand "linux-2.6.36-arm-external-vista-router-full-src" 
    4709C0/4366MC clone 4709C0/4366MCe -tag $tag_e \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2e -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}

    set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
    4709C0/4366MC clone 4709C0/4366MCe_dhd -tag $tag_e \
        -brand $brand \
        -perfchans {36/80 36l 36}

    4709C0/43602MCH2 clone 4709C0/43602MCH2e_dhd -tag $tag_e \
        -brand $brand \
        -perfchans {6l 6}
}

4709C0/4366MCe-i configure -dualband {4709C0/4366MCe-i -c1 161/80 -c2 6l}
4709C0/43602MCH2e-i configure -dualband {4709C0/4366MCe-i -c2 161/80 -c1 6l}

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
UTF::Q mc84 lan

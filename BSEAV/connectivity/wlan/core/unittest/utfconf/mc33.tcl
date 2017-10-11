

# -*-tcl-*-
#
# Testbed configuration file for MC33 testbed
#

############################################################
# Load Packages
# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
}

package require UTF::Linux
package require UTF::AeroflexDirect
package require UTFD
package require UTF::Aeroflex
package require UTF::Sniffer

#---------------------------------------
# To setup UTFD port for this rig
set ::env(UTFDPORT) 9988

#-------------------------------------------------
# Set attenuator ranges (needed for RvRNightly1)
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"

set UTF::WebTree 1
set UTF::DataRatePlot 1

# set devicemode; default=0
global env
if {[info exist env(DEVICEMODE)] && ($env(DEVICEMODE) != "0")} {
    append nvramAppendList "devicemode=$env(DEVICEMODE) "
}

#set nvramMU "wl0_mu_features=1 wl1_mu_features=1"
set nvramMU "wl0_mu_features=-1"
# for 1x1
set confRSDB "wl rsdb_mode rsdb"
set confSU "wl txbf_bfe_cap 1"
set nvramCTDMA ""
#for 2x2
set confMIMO "wl rsdb_mode mimo"

# Parse logs

#set ::UTF::PostTestAnalysis /home/$::env(LOGNAME)/src/tools/parse_wl_logs/post_test_analysis.tcl

#If you want EmbeddedNightly.test to do log parsing, you need to add the statement below to your config file. This is in addition to the above statement.

set ::UTF::PostTestHook {
    package require UTF::utils
    UTF::do_post_test_analysis [info script] ""}

#To use LSF compute farm to offload your endpoint, add:

set ::aux_lsf_queue sj-hnd

#-----------------------------------------------------------------
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/mc33"

#---------------------------------------
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    # Make Sure Attenuators are set to 0 value
    ALL attn default;
    foreach S {4366e-1 4366e-2 4366e-3 4366e-4} {
        catch {$S wl down}
        $S deinit
    }

    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {4709C0/4366MC 4709C0/43602MCH2} {
        catch {$S apshell wl -i [$S cget -device] down}
    }

    # delete myself
    unset ::UTF::SetupTestBed
    return
}

#--------------------------------------------------------
# power controllers
UTF::Power::Synaccess mc33npc1 -lan_ip 172.16.1.4 -rev 1
UTF::Power::Synaccess mc33npc2 -lan_ip 172.16.1.5
UTF::Power::Synaccess mc33npc3 -lan_ip 172.16.1.7

#----------------------------------------------------------------
# Attenuator - Aeroflex
UTF::AeroflexDirect af -lan_ip 172.16.1.210 \
    -group {G1 {1 2 3 4} G2 {5 6 7 8} ALL {1 2 3 4 5 6 7 8 9}}
ALL configure -default 0

#--------------------------------------------------------
# UTF Endpoint1 f19 - Traffic generators
UTF::Linux mc33end1 -lan_ip 10.19.85.249 -sta {lan p4p1}

#-----------------------------------------------------------
# Common configuration for all STA
set stacmn {
    -tag EAGLE_BRANCH_10_10
    -brand "linux-internal-wl" \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M
    -slowassoc 10 -reloadoncrash 1
    -udp 1.2G
    -wlinitcmds {
	wl msglevel +assoc;
	wl msglevel +error;
	wl down;
	wl txchain 3; wl rxchain 3;
	wl country '#a/0';
	wl vht_features 7
    }
}

#replaced 4359 mc with 4366e mc on all stas
#------------------------------------------------------
# FC19 STA-1: 4366e-1
UTF::Linux STA-1 -lan_ip mc33tst1 -sta {4366e-1 enp1s0} \
    -power {mc33npc1 2} -power_button "auto" \
    -console "mc33end1:40001" \
    {*}$stacmn

4366e-1 configure -attngrp G1

#-------------------------------------------------------
# FC19 STA-2: 4366e-2
UTF::Linux STA-2 -lan_ip mc33tst2 -sta {4366e-2 enp1s0} \
    -power {mc33npc2 1} -power_button "auto" \
    -console "mc33end1:40002" \
    {*}$stacmn

4366e-2 configure -attngrp G1

#-------------------------------------------------------
# FC19 STA-3: 4366e-3
UTF::Linux STA-3 -lan_ip mc33tst3 -sta {4366e-3 enp3s0} \
    -power {mc33npc2 2} -power_button "auto" \
    -console "mc33end1:40003" \
    {*}$stacmn

4366e-3 configure -attngrp G1

#------------------------------------------------------
# FC19 STA-4: 4366e-4
UTF::Linux STA-4 -lan_ip mc33tst4 -sta {4366e-4 enp3s0} \
    -power {mc33npc3 1} -power_button "auto" \
    -console "mc33end1:40004" \
    {*}$stacmn

4366e-4 configure -attngrp G1

#------------------------------------------------------
#4366e MU STAs#
set num {a b c d e f}
set i 0
foreach STA {4366e-1 4366e-2 4366e-3 4366e-4} {
    set suffix [lindex $num $i]
    $STA clone 4366e-${suffix}1x1 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 1; wl txchain 1;}

    $STA clone 4366e-${suffix}1x1su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 1; wl txchain 1;}

    $STA clone 4366e-${suffix}2x2 \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl rxchain 3; wl txchain 3;}

    $STA clone 4366-e${suffix}2x2su \
        -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl country US/0; wl down; wl bw_cap 2g -1; wl vht_features 7; wl txbf_bfr_cap 0; wl txbf_bfe_cap 1; wl rxchain 3; wl txchain 3;}
    incr i
}




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

#--------------------------------------------------
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
    -relay "mc33end1" \
    -power {mc33npc1 1} \
    -console "mc33end1:40000" \
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
#------------------------------------------------
# BISON04T_TWIG_7_14_131 (Golden2.x)
#------------------------------------------------
4709C0/4366MCb_dhd-i clone 4366C0-131-mutx1 \
    -nvram "$nvram4709C0 $nvramMU $nvramCTDMA $nvramAtlas" \
    -rvrnightly {-mumode mu}
4709C0/4366MCb_dhd-i clone 4366C0-131-mutx0 \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -rvrnightly {-mumode su}
#------------------------------------------------
# BISON04T_TWIG_7_14_164 (Golden3.x)
#------------------------------------------------
4366C0-131-mutx1 clone 4366C0-164-mutx1 \
     -tag BISON04T_TWIG_7_14_164 -rvrnightly {-mumode mu}
4366C0-131-mutx0 clone 4366C0-164-mutx0 \
     -tag BISON04T_TWIG_7_14_164 -rvrnightly {-mumode su}
#------------------------------------------------
# BISON04T_BRANCH_7_14 (17.1)
#------------------------------------------------
4366C0-131-mutx1 clone 4366C0-714-mutx1 \
     -tag BISON04T_BRANCH_7_14 -rvrnightly {-mumode mu}
4366C0-131-mutx0 clone 4366C0-714-mutx0 \
     -tag BISON04T_BRANCH_7_14 -rvrnightly {-mumode su}

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

#---------------------------------------------------
set comments {
UTF::DHD STA-1  \
    -lan_ip mc33tst1 \
    -sta {4359-1 eth0} \
    -power {mc33npc1 2} \
    -power_button "auto" \
    -hostconsole "mc33end1:40001" \
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

#if {[info exist env(STA_IMAGE)]} {
#    4359-1 clone 4359musta1 \
#    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -image $env(STA_IMAGE)
#} else {
#    4359-1 clone 4359susta1 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl rxchain 1; wl txchain 1; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin
#    4359susta1 clone 2x2susta1 -wlinitcmds "wl down; $wic; $confSU; wl up"
#
#    4359-1 clone 4359musta1 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin
#    4359musta1 clone 2x2musta1 -wlinitcmds "wl down; $wic; wl up"
#}
#

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
    -hostconsole "mc33end1:40002" \
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

#if {[info exist env(STA_IMAGE)]} {
#    4359-2 clone 4359musta2 \
#	-wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -image $env(STA_IMAGE)
#} else {
#    4359-2 clone 4359susta2 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin
#
#    4359susta2 clone 2x2susta2 -wlinitcmds "wl down; $wic; $confSU; wl up"
#
#    4359-2 clone 4359musta2 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confRSDB;wl rxchain 1; wl txchain 1; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin
#
#    4359musta2 clone 2x2musta2 -wlinitcmds "wl down; $wic; wl up"
#}
#

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
    -hostconsole "mc33end1:40003" \
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
#if {[info exist env(STA_IMAGE)]} {
#    4359-3 clone 4359musta3 \
#        -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -image $env(STA_IMAGE)
#} else {
#    4359-3 clone 4359susta3 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl rxchain 1; wl txchain 1; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin 
#
#    4359susta3 clone 2x2susta3 -wlinitcmds "wl down; $wic; $confSU; wl up"
#
#    4359-3 clone 4359musta3 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin
#
#    4359musta3 clone 2x2musta3 -wlinitcmds "wl down; $wic; wl up"
#}
#

# for 4359C0
#4359-3 clone 4359musta3 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds {wl msglevel +assoc; wl down; wl vht_features 3; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0; wl rsdb_mode rsdb; wl up} \
#    -type 43596a0-roml/config_pcie_features2/rtecdc.bin


UTF::DHD STA-4  \
    -lan_ip mc33tst4 \
    -sta {4359-4 eth0} \
    -power {mc33npc3 1} \
    -power_button "auto" \
    -hostconsole "mc33end1:40004" \
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
#if {[info exist env(STA_IMAGE)]} {
#    4359-4 clone 4359musta4 \
#    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -image $env(STA_IMAGE)
#} else {
#    4359-4 clone 4359susta4 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl rxchain 1; wl txchain 1; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
#
#    4359susta4 clone 2x2susta4 -wlinitcmds "wl down; $wic; $confSU; wl rxchain 3; wl txchain 3; wl up"
#
#    4359-4 clone 4359musta4 -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
#    -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
#    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
#
#    4359musta4 clone 2x2musta4 -wlinitcmds "wl down; $wic; wl rxchain 3; wl txchain 3; wl up"
#}
#

set i 1
foreach STA {4359-1 4359-2 4359-3 4359-4} {
    if {[info exist env(STA_IMAGE)]} {
        $STA clone 4359musta${i} \
            -wlinitcmds "wl down; $wic ;$confRSDB; wl up" \
            -image $env(STA_IMAGE)
    } else {
	$STA clone 4359susta${i} -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
	    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
	    -wlinitcmds "wl down; $wic ;$confSU; $confRSDB; wl rxchain 1; wl txchain 1; wl up"
	$STA clone 2x2susta${i} -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
	    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
	    -wlinitcmds "wl down; $wic; $confSU; wl rxchain 3; wl txchain 3; wl up"
	$STA clone 4359musta${i} -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
	    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
	    -wlinitcmds "wl down; $wic ;$confRSDB; wl rxchain 1; wl txchain 1; wl up"
	$STA clone 2x2musta${i} -tag DIN07T48RC50_BRANCH_9_75{,_?*} \
	    -type 43596a0-roml/config_pcie_utf/rtecdc.bin \
	    -wlinitcmds "wl down; $wic; $confMIMO; wl rxchain 3; wl txchain 3; wl up"
    }
    incr i
}
}
#UTF::Q mc33 lan

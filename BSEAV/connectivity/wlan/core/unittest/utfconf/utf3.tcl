# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum UTF3 testbed
#


# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

#package require UTF::TclReadLines
#UTF::Power::WebRelay pb6awr -relay svt-utf3-end1 -lan_ip 172.19.12.32 -invert {2 3}
#Linux endpoints WebRelay - 10.19.86.134
#UTF::Power::WebRelay usbwr -relay svt-utf3-end1 -lan_ip 172.19.12.32

set UTF::WebTree 1
set UTF::DataRatePlot 1

set nvramMU "wl0_mu_features=0x8000 wl1_mu_features=0x8000"
set nvramSU "wl0_mu_features=0 wl1_mu_features=0"
set confRSDB "wl rsdb_mode rsdb"
set confMIMO "wl rsdb_mode mimo"
set confSU "wl txbf_bfe_cap 1"
set confSUi "wl txbf_bfe_cap 0"
set nvramDM1 "devicemode=1"
set conf1x1 "wl rxchain 1; wl txchain 1"
set conf2x2 "wl rxchain 3; wl txchain 3"

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

set ::UTF::SummaryDir "/projects/hnd_svt_ap8/$::env(LOGNAME)/utf3"

# Define power controllers on cart
UTF::Power::Synaccess svt-utf3-npc1 -lan_ip 10.19.95.209 
UTF::Power::Synaccess svt-utf3-npc2 -relay svt-utf3-end1 -lan_ip 172.16.1.101 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip svt-utf3-tst8 \
    -group {G1 {1 2} G2 {3 4} G3 {5 6} G4 {7 8} ALL {1 2 3 4 5 6 7 8}}

G1 configure -default 6 
G2 configure -default 6 
G3 configure -default 6
G4 configure -default 6
ALL configure -default 6 

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default;
    #G1 attn default;
    #G2 attn default;

    foreach S {4366c0f19-1 4366c0f19-2 4366c0f19-3 4366c0f19-4} {
	catch {$S wl disassoc}
	catch {$S wl down}
	catch {$S wl status}
	$S deinit
    }
    foreach S {svt-utf3-end1 svt-utf3-end2 4366c0f19-1 4366c0f19-2 4366c0f19-3 4366c0f19-4} {
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
    foreach S {4709C0/4366MC 4709C0/4366MCH5} {
	catch {$S apshell wl -i [$S cget -device] down}
    }
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #4708 restart wl0_radio=0
    #AP2 restart wl0_radio=0
    #AP5 restart wl0_radio=0

    #svt-utf3-tst2 add_netconsole svt-utf3-end1 6661
    #svt-utf3-tst2 service netconsole start
    
    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# UTF Endpoint1 f11 - Traffic generators (no wireless cards)
UTF::Linux svt-utf3-end1\
    -sta {lan p2p1} \
    -iperf iperf208c64 \
    -lan_ip svt-utf3-end1

lan configure -ipaddr 192.168.1.96

UTF::Linux svt-utf3-end2 \
    -sta {lan2 p4p1}  \
    -iperf iperf208c64 \
    -lan_ip svt-utf3-end2

lan2 configure -ipaddr 192.168.1.97

# Define Sniffer
UTF::Sniffer svt-utf3-sniffer -user root \
        -lan_ip svt-utf3-tst5 \
        -sta {4366mc enp5s0} \
        -power {svt-utf3-npc3 1} \
        -power_button "auto" \
        -date {} \
        -tag EAGLE_REL_10_10_57{,_*} \
        -brand linux-internal-wl \
        -tcpwindow 16M \
        -wlinitcmds {}

set wic "wl bw_cap 2g -1; wl vht_features 7; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0;\
wl msglevel +assoc +error"

UTF::Linux svt-utf3-tst1  \
    -lan_ip svt-utf3-tst1 \
    -sta {4366c0f19-1 enp1s0} \
    -power {svt-utf3-npc1 2} \
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

UTF::Linux svt-utf3-tst2  \
    -lan_ip svt-utf3-tst2 \
    -sta {4366c0f19-2 enp1s0} \
    -power {svt-utf3-npc1 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
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

UTF::Linux svt-utf3-tst3  \
    -lan_ip svt-utf3-tst3 \
    -sta {4366c0f19-3 enp1s0} \
    -power {svt-utf3-npc1 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
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

UTF::Linux svt-utf3-tst4  \
    -lan_ip svt-utf3-tst4 \
    -sta {4366c0f19-4 enp1s0} \
    -power {svt-utf3-npc1 2} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up " \
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

foreach n "1 2 3 4" {
    1x1MUsta${n} clone 1x1SUsta${n} -wlinitcmds "wl down; $wic ;$confSU; $conf1x1; wl up "
    1x1SUsta${n} clone 2x2SUsta${n} -wlinitcmds "wl down; $wic; $confSU; $conf2x2; wl up "
    1x1SUsta${n} clone 2x2SUIsta${n} -wlinitcmds "wl down; $wic; $confSUi; $conf2x2; wl up "
    1x1SUsta${n} clone 1x1SUIsta${n} -wlinitcmds "wl down; $wic; $confSUi; $conf1x1; wl up "
    1x1MUsta${n} clone 2x2MUsta${n} -wlinitcmds "wl down; $wic; $conf2x2; wl up "
}

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
wl0_country_code=Q1
wl0_country_rev=137
wl1_ssid=BroadcomAG
wl1_chanspec=161
wl1_obss_coex=0
wl1_bw_cap=-1
wl1_radio=0
wl1_vht_features=7
wl1_country_code=Q1
wl1_country_rev=137
lan_ipaddr=192.168.1.1
lan_gateway=192.168.1.1
dhcp_start=192.168.1.151
dhcp_end=192.168.1.199
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

UTF::Router 4709C0 \
	-sta {4709C0/4366MC eth2 4709C0/4366MC.%15 wl1.% 4709C0/4366MCH5 eth1 4709C0/4366MCH5.%15 wl0.%} \
	-lan_ip 192.168.1.1 \
	-lanpeer {lan lan2} \
	-relay "svt-utf3-end1" \
    -power {svt-utf3-npc2 1} \
    -console "svt-utf3-end1:40000" \
    -tag EAGLE_TWIG_10_10_69 \
    -brand "linux-2.6.36-arm-internal-router-dhdap" \
	-nvram $nvram4709C0 \
    -wlinitcmds "" \
    -nopm1 1 -nopm2 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 30-95 -attn2g 30-95 -pad 0} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 6l 6} -nosamba 1 -nocustom 1 \
    -embeddedimage {436{5,6}b} \
    -slowassoc 5 \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}
4709C0/4366MC configure -dualband {4709C0/4366MC -c1 161/80 -c2 6l}

if {[info exist env(AP_IMAGE)]} {
    4709C0/4366MC clone 4366MCh-su-p \
        -image $env(AP_IMAGE) \
        -perfchans {157/80 157l 157 6l 6}

    4366MCh-su-p clone 4366MCb-su-p \
        -perfchans {36/80 36l 36 6l 6}\
        -nvram "$nvram4709C0 $nvramSU"

    4709C0/4366MCH5 clone 4366MCH5b-su-p \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}\
        -nvram "$nvram4709C0 $nvramSU"

    4366MCH5b-su-p clone 4366MCH5h-su-p \
        -perfchans {157/80 157l 157}

    foreach objName "4366MCH5h 4366MCh 4366MCH5b 4366MCb" {
        ${objName}-su-p clone ${objName}-su-dm1-p \
            -nvram "$nvram4709C0 $nvramDM1"
        ${objName}-su-p clone ${objName}-mu-p \
            -nvram "$nvram4709C0 $nvramMU"\
            -rvrnightly {-mumode mu}\
            -pre_perf_hook {
                {%S wl phy_mu_nss4enable 1}
            }
        ${objName}-su-p clone ${objName}-mu-dm1-p \
            -nvram "$nvram4709C0 $nvramMU $nvramDM1"\
            -rvrnightly {-mumode mu}
    }
}
# Bison STA clones
if {[info exist env(TAG_B)]} {
    set tag_b $env(TAG_B)
} else {
    #set tag_b BISON04T_{REL,TWIG}_7_14_131{_2?,}
    #set tag_b BISON04T_REL_7_14_131{_2?}
    set tag_b BISON04T_TWIG_7_14_131
}
puts "#### tag_b = $tag_b ####"

### for internal images
set brand "linux-2.6.36-arm-internal-router"
#set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
4709C0/4366MC clone 4366MCb-i -tag $tag_b \
    -brand $brand \
    -embeddedimage {436{5,6}c} \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5b-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
4366MCH5b-i clone 4366MCH5hb-i \
    -perfchans {157/80 157l 157}

set brand linux-2.6.36-arm-internal-router-dhdap
4709C0/4366MC clone 4366MCb_dhd-i -tag $tag_b \
    -brand $brand \
    -embeddedimage {436{5,6}c} \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5b_dhd-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
4366MCH5b_dhd-i clone 4366MCH5hb_dhd-i \
    -perfchans {157/80 157l 157}

set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
4709C0/4366MC clone 4366MCb_dhda-i -tag $tag_b \
    -brand $brand \
    -embeddedimage {436{5,6}c} \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5b_dhda-i -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {36/80 36l 36}
4366MCH5b_dhda-i clone 4366MCH5hb_dhda-i \
    -perfchans {157/80 157l 157}

4366MCb_dhda-i clone 4366MCb_dhda1-i
4366MCb_dhda-i clone 4366MCb_dhda2-i

### for external images
set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
4709C0/4366MC clone 4366MCb_dhda -tag $tag_b \
    -brand $brand \
    -embeddedimage {436{5,6}c} \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5b_dhda -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {36/80 36l 36}
4366MCH5b_dhda clone 4366MCH5hb_dhda \
    -perfchans {157/80 157l 157}

set brand "linux-2.6.36-arm-external-vista-router-full-src" 
4709C0/4366MC clone 4366MCb -tag $tag_b \
    -brand $brand \
    -embeddedimage {436{5,6}c} \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5b -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
4366MCH5b clone 4366MCH5hb \
    -perfchans {157/80 157l 157}

set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
4709C0/4366MC clone 4366MCb_dhd -tag $tag_b \
    -brand $brand \
    -embeddedimage {436{5,6}c} \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5b_dhd -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}
4366MCH5b_dhd clone 4366MCH5hb_dhd \
    -perfchans {157/80 157l 157}

4366MCH5b_dhd-i configure -dualband {4366MCb_dhd-i -c1 157/80 -c2 6l}
4366MCH5b_dhda-i configure -dualband {4366MCb_dhda-i -c1 157/80 -c2 6l}
4366MCH5b_dhd-i configure -dualband {4366MCb_dhd-i -c2 157/80 -c1 6l}
4366MCH5hb_dhd-i configure -dualband {4366MCb_dhd-i -c1 157/80 -c2 6l}
4366MCH5hb_dhda-i configure -dualband {4366MCb_dhda-i -c1 157/80 -c2 6l}
4366MCH5hb_dhd-i configure -dualband {4366MCb_dhd-i -c2 157/80 -c1 6l}


### Gold 3.0 ###
if {[info exist env(TAG_G3)]} {
    set tag_g3 $env(TAG_G3)
} else {
    set tag_g3 BISON04T_REL_7_14_164{_?*} 
}   
puts "#### tag_g3 = $tag_g3 ####"

foreach tag "$tag_b $tag_g3" {
foreach objName "4366MCH5b 4366MCH5hb 4366MCb" {
    ${objName}_dhd-i clone ${objName}_dhda-mu \
        -nvram "$nvram4709C0 $nvramMU"\
        -rvrnightly {-mumode mu}
    ${objName}_dhd-i clone ${objName}_dhda-su \
        -nvram "$nvram4709C0 $nvramSU"
    ${objName}_dhda-i clone ${objName}_dhda-mu-i \
        -nvram "$nvram4709C0 $nvramMU $nvramAtlas"\
        -rvrnightly {-mumode mu}
    ${objName}_dhda-i clone ${objName}_dhda-mu-dm1-i \
        -nvram "$nvram4709C0 $nvramMU $nvramDM1"\
        -rvrnightly {-mumode mu}
    ${objName}_dhda-i clone ${objName}_dhda-su-i \
        -nvram "$nvram4709C0 $nvramSU"
    ${objName}_dhda-i clone ${objName}_dhda-su-dm1-i \
        -nvram "$nvram4709C0 $nvramSU $nvramDM1"
    ${objName}_dhda clone ${objName}_dhda-mu-e \
        -nvram "$nvram4709C0 $nvramMU $nvramAtlas"\
        -rvrnightly {-mumode mu}
    ${objName}_dhda clone ${objName}_dhda-su-e \
        -nvram "$nvram4709C0 $nvramSU"
    ${objName}_dhda clone ${objName}_dhda-mu-dm1-e \
        -nvram "$nvram4709C0 $nvramMU $nvramDM1"\
        -rvrnightly {-mumode mu}
    ${objName}_dhda clone ${objName}_dhda-su-dm1-e \
        -nvram "$nvram4709C0 $nvramSU $nvramDM1"
    ### Gold 3.0
    ${objName}_dhda-mu-i clone ${objName}_dhda-g3-mu-i -tag $tag_g3\
        -rvrnightly {-mumode mu}
    ${objName}_dhda-su-i clone ${objName}_dhda-g3-su-i -tag $tag_g3
    ${objName}_dhda-mu-dm1-i clone ${objName}_dhda-g3-mu-dm1-i -tag $tag_g3\
        -rvrnightly {-mumode mu}
    ${objName}_dhda-su-dm1-i clone ${objName}_dhda-g3-su-dm1-i -tag $tag_g3
    ${objName}_dhda-mu-e clone ${objName}_dhda-g3-mu-e -tag $tag_g3\
        -rvrnightly {-mumode mu}
    ${objName}_dhda-su-e clone ${objName}_dhda-g3-su-e -tag $tag_g3
    ${objName}_dhda-mu-dm1-e clone ${objName}_dhda-g3-mu-dm1-e -tag $tag_g3\
        -rvrnightly {-mumode mu}
    ${objName}_dhda-su-dm1-e clone ${objName}_dhda-g3-su-dm1-e -tag $tag_g3
}
}
4366MCH5hb_dhda-mu-i clone 4366MCH5hb_dhda-mu-atf1-i\
-nvram "$nvram4709C0 $nvramMU $nvramAtlas wl1_atf=1"\
-rvrnightly {-mumode mu}
4366MCH5hb_dhda-mu-i clone 4366MCH5hb_dhda-mu-atf0-i\
-nvram "$nvram4709C0 $nvramMU $nvramAtlas wl1_atf=0"\
-rvrnightly {-mumode mu}
4366MCH5hb_dhda-g3-mu-i clone 4366MCH5hb_dhda-g3-mu-atf1-i\
-nvram "$nvram4709C0 $nvramMU $nvramAtlas wl1_atf=1"\
-rvrnightly {-mumode mu}
4366MCH5hb_dhda-g3-mu-i clone 4366MCH5hb_dhda-g3-mu-atf0-i\
-nvram "$nvram4709C0 $nvramMU $nvramAtlas wl1_atf=0"\
-rvrnightly {-mumode mu}

# Eagle STA clones    
if {[info exist env(AP_IMAGE)]} {
    4709C0/4366MC clone 4366MCe-i \
        -image $env(AP_IMAGE) \
        -nvram $nvram4709C0 \
        -perfchans {157/80 157l 157}

    4709C0/4366MCH5 clone 4366MCH5e-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4366MCe-i clone 4366MCe-mu -nvram "$nvram4709C0 $nvramMU"\
    -rvrnightly {-mumode mu}
    4366MCe-i clone 4366MCe-su -nvram "$nvram4709C0 $nvramSU"
    4366MCe-mu configure -dualband {4366MCe-mu -c1 161/80 -c2 6l}
}
### for internal images
if {[info exist env(TAG_E)]} {
    set tag_e $env(TAG_E)
} else {
    set tag_e EAGLE_TWIG_10_10_69{,_?*}
}
puts "#### tag_e = $tag_e ####"

set brand "linux-2.6.36-arm-internal-router"
4709C0/4366MC clone 4366MCe-i -tag $tag_e \
    -brand $brand \
    -nvram $nvram4709C0 \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5e-i -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4366MCe-i clone 4366MCe-mu -nvram "$nvram4709C0 $nvramMU"\
-rvrnightly {-mumode mu}
4366MCe-i clone 4366MCe-su -nvram "$nvram4709C0 $nvramSU"
4366MCe-mu configure -dualband {4366MCe-mu -c1 161/80 -c2 6l}

set brand linux-2.6.36-arm-internal-router-dhdap
4709C0/4366MC clone 4366MCe_dhd-i -tag $tag_e \
    -brand $brand \
    -perfchans {157/80 157l 157}

4366MCe_dhd-i clone 4366MCe_dhd1
4366MCe_dhd-i clone 4366MCe_dhd2

4709C0/4366MCH5 clone 4366MCH5e_dhd-i -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

### for external images
set brand "linux-2.6.36-arm-external-vista-router-full-src" 
4709C0/4366MC clone 4366MCe -tag $tag_e \
    -brand $brand \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5e -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
4709C0/4366MC clone 4366MCe_dhd -tag $tag_e \
    -brand $brand \
    -perfchans {157/80 157l 157}

4709C0/4366MCH5 clone 4366MCH5e_dhd -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4366MCe-i configure -dualband {4366MCe-i -c1 161/80 -c2 6l}
4366MCH5e-i configure -dualband {4366MCe-i -c2 161/80 -c1 6l}

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
UTF::Q utf3

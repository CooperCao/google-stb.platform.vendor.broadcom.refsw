# -*-tcl-*-
#
# Testbed configuration file for Kelvin Shum UTF2 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

set UTF::WebTree 1
set UTF::DataRatePlot 1

# set devicemode; default=0
global env
set nvramDM1 "devicemode=1"
set nvramMU "wl0_mu_features=0x8000 wl1_mu_features=0x8000"
set nvramSU "wl0_mu_features=0 wl1_mu_features=0"
set conf1x1 "wl rxchain 1; wl txchain 1"
set conf2x2 "wl rxchain 3; wl txchain 3"
set confSU "wl txbf_bfe_cap 1"
set confSUi "wl txbf_bfe_cap 0"

if {[info exist env(NVRAM)] && ($env(NVRAM) != "0")} {
    append nvramAppendList $env(NVRAM) 
}
if {[info exist env(CPUCLK)] && [string equal $env(CPUCLK) "default"] == 0} {
    append nvramAppendList "1:cpuclk=$env(CPUCLK) "
}


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

set ::UTF::SummaryDir "/projects/hnd_svt_ap8/$::env(LOGNAME)/utf2"

# Define power controllers on cart
UTF::Power::Synaccess svt-utf2-npc1 -relay svt-utf2-end1 -lan_ip 172.16.1.20 -rev 1
UTF::Power::Synaccess svt-utf2-npc2 -relay svt-utf2-end1 -lan_ip 172.16.1.21 -rev 1
UTF::Power::Synaccess svt-utf2-npc3 -relay svt-utf2-end1 -lan_ip svt-utf2-end2 -rev 1
UTF::Power::Synaccess svt-utf2-npc4 -relay svt-utf2-end1 -lan_ip 172.16.1.23 

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip svt-utf2-tst8 \
    -group {G1 {1 2} G2 {3 4} G3 {5 6} G4 {7 8} ALL {1 2 3 4 5 6 7 8}}

G1 configure -default 5
G2 configure -default 5
G3 configure -default 5
G4 configure -default 5
ALL configure -default 5

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn default;

    set MAX 12582912
    set DEF 2097152
    set MIN 10240
    set XMEM "10240 2097152 12582912"
    set MEM "12582912 12582912 12582912"

    foreach S {4908a 4908b 4908c} {
	catch {$S dslshell wl -i [$S cget -device] down}
    }
    foreach S {4366c0f19-1 4366c0f19-2 4366c0f19-3 4366c0f19-4} {
	catch {$S wl disassoc}
	catch {$S wl down}
	catch {$S wl status}
	$S deinit
    }
    foreach S {svt-utf2-end1 svt-utf2-end3 4366c0f19-1 4366c0f19-2 4366c0f19-3 4366c0f19-4} {
    catch {
        $n sysctl -w net.core.rmem_max=$MAX
        $S sysctl -w net.core.wmem_max=$MAX
        $S sysctl -w net.core.rmem_default=$DEF
        $S sysctl -w net.core.wmem_default=$DEF
        $S sysctl -w net.ipv4.tcp_rmem=$XMEM
        $S sysctl -w net.ipv4.tcp_wmem=$XMEM
        $S sysctl -w net.ipv4.tcp_mem=$MEM
        $S sysctl -w net.ipv4.route.flush=1
        $S sysctl -w net.ipv4.tcp_timestamps=1
        $S sysctl -w net.ipv4.tcp_sack=1
        $S sysctl -w net.core.netdev_max_backlog=30000
        $S sysctl -w net.ipv4.tcp_limit_output_bytes=1310720
        $S sysctl -w net.ipv4.tcp_congestion_control=bic
        #$S sysctl -w net.core.rmem_max="16000000"
        #$S sysctl -w net.core.wmem_max="16000000" 
        #$S sysctl -w net.ipv4.tcp_rmem="4096 87380 16000000" 
        #$S sysctl -w net.ipv4.tcp_wmem="4096 87380 16000000" 
        #$S sysctl -w net.core.netdev_max_backlog="3000" 
        #$S echo bic > /proc/sys/net/ipv4/tcp_congestion_control
    }
    }
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {4709C0/4366MC 4709C0/4366MCH5E} {
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

# UTF Endpoint1 f11 - Traffic generators (no wireless cards)
UTF::Linux svt-utf2-end1 \
    -sta {lan p4p1} \
    -power {svt-utf2-npc3 1} \
    -lan_ip svt-utf2-end1 

lan configure -ipaddr 192.168.1.97

UTF::Linux svt-utf2-end3 \
    -sta {lan2 p4p1} \
    -power {svt-utf2-npc3 1} \
    -lan_ip svt-utf2-end3 

lan2 configure -ipaddr 192.168.1.96

# Define Sniffer
UTF::Sniffer svt-utf2-sniffer -user root \
        -lan_ip svt-utf2-tst3 \
        -sta {4366mc enp5s0} \
        -power {svt-utf2-npc3 2} \
        -power_button "auto" \
        -date {} \
        -tag EAGLE_REL_10_10_57{,_*} \
        -brand linux-internal-wl \
        -tcpwindow 16M \
        -wlinitcmds {}

set wic "wl bw_cap 2g -1; wl vht_features 7; wl mpc 1; wl infra 1; wl txbf_bfr_cap 0;\
wl msglevel +assoc +error"

UTF::Linux svt-utf2-tst1  \
    -lan_ip svt-utf2-tst1 \
    -sta {4366c0f19-1 enp1s0} \
    -power {svt-utf2-npc2 1} \
    -power_button "auto" \
    -brand linux-internal-wl \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up" \
    -tcpwindow 2m \
    -iperf iperf208c64

4366c0f19-1 configure -attngrp G1

if {[info exist env(STA_IMAGE)]} {
    4366c0f19-1 clone 1x1MUsta1 \
    -image $env(STA_IMAGE)
} else {
    4366c0f19-1 clone 1x1MUsta1
}

UTF::Linux svt-utf2-tst2  \
    -lan_ip svt-utf2-tst2 \
    -sta {4366c0f19-2 enp1s0} \
    -power {svt-utf2-npc2 1} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up" \
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

UTF::Linux svt-utf2-tst4  \
    -lan_ip svt-utf2-tst4 \
    -sta {4366c0f19-3 enp1s0} \
    -power {svt-utf2-npc4 1} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up" \
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

UTF::Linux svt-utf2-tst5  \
    -lan_ip svt-utf2-tst5 \
    -sta {4366c0f19-4 enp1s0} \
    -power {svt-utf2-npc4 1} \
    -power_button "auto" \
    -tag EAGLE_BRANCH_10_10 \
    -date {2016.9.29.0} \
    -wlinitcmds "wl down; $wic; $conf1x1; wl up" \
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
wl0_ssid=4366MCH5E
wl0_chanspec=157
wl0_obss_coex=0
wl0_bw_cap=-1
wl0_radio=0
wl0_vht_features=7
wl0_country_code=Q1
wl0_country_rev=137
wl1_ssid=4366MC
wl1_chanspec=36
wl1_obss_coex=0
wl1_bw_cap=-1
wl1_radio=0
wl1_vht_features=7
wl1_country_code=Q1
wl1_country_rev=139
wl1_nband=1
lan_ipaddr=192.168.1.10
lan_gateway=192.168.1.10
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

    #-channelsweep {-band b -bw 40} \

UTF::Router 4709C0 \
	-sta {4709C0/4366MC eth2 4709C0/4366MC.%15 wl1.% 4709C0/4366MCH5E eth1 4709C0/4366MCH5E.%15 wl0.%} \
	-lan_ip 192.168.1.10 \
	-lanpeer {lan lan2} \
	-relay "svt-utf2-end1" \
    -power {svt-utf2-npc1 1} \
    -console "svt-utf2-end1:40000" \
    -tag EAGLE_TWIG_10_10_69 \
    -brand "linux-2.6.36-arm-internal-router-dhdap" \
	-nvram $nvram4709C0 \
    -nopm1 1 -nopm2 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2G \
    -yart {-frameburst 1 -attn5g 30-95 -attn2g 30-95 -pad 0} \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 157/80 157l 157} -nosamba 1 -nocustom 1 \
    -embeddedimage {436{5,6}c} \
    -slowassoc 5 \
    -bootwait 15 \
    -msgactions { 
        {txerr valid (1) reason 0020} FAIL
    } \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

4709C0/4366MC configure -dualband {4709C0/4366MC -c1 161/80 -c2 6l}

if {[info exist env(AP_IMAGE)]} {
    4709C0/4366MC clone 4366MCb_dhd-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4366MCb_dhd-i clone 4366MCb_dhda-i

    4709C0/4366MCH5E clone 4366MCH5E_3x3_dhd-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}

    4709C0/4366MC clone 4366MCb-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}

    4709C0/4366MC clone 4366MCb-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}

    4709C0/4366MCH5E clone 4366MCH5E_3x3-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4366MCH5E_3x3-i clone 4366MCH5E_3x3_dhda-i
}

# Bison STA clones
if {[info exist env(TAG_B)]} {
    set tag_b $env(TAG_B)
} else {
    set tag_b BISON04T_REL_7_14_131{_??}
    #set tag_b BISON04T_{TWIG,REL}_7_14_131{,_??}
    #set tag_b BISON04T_TWIG_7_14_131
}
puts "#### tag_b = $tag_b ####"

### for internal images
set brand "linux-2.6.36-arm-internal-router"
4709C0/4366MC clone 4366MCb-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5E_3x3-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

set brand linux-2.6.36-arm-internal-router-dhdap
4709C0/4366MC clone 4366MCb_dhd-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5E_3x3_dhd-i -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

set brand "linux-2.6.36-arm-internal-router-dhdap-atlas"
4709C0/4366MC clone 4366MCb_dhda-i -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5E_3x3_dhda-i -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {36/80 36l 36}

4366MCb_dhda-i clone 4366MCb_dhda1-i
4366MCb_dhda-i clone 4366MCb_dhda2-i

### for external images
set brand "linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src"
4709C0/4366MC clone 4366MCb_dhda -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5E_3x3_dhda -tag $tag_b \
    -brand $brand \
    -nvram "$nvram4709C0 $nvramAtlas" \
    -perfchans {36/80 36l 36}

set brand "linux-2.6.36-arm-external-vista-router-full-src" 
4709C0/4366MC clone 4366MCb -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5E_3x3 -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
4709C0/4366MC clone 4366MCb_dhd -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5E_3x3_dhd -tag $tag_b \
    -brand $brand \
    -perfchans {36/80 36l 36}

4366MCb_dhda-i clone 4366MC_dhda-mu-i \
    -nvram "$nvram4709C0 $nvramMU $nvramAtlas"\
    -rvrnightly {-mumode mu}
4366MCH5E_3x3_dhda-i clone 4366MCH5E_3x3_dhda-mu-i \
    -nvram "$nvram4709C0 $nvramMU $nvramAtlas"\
    -rvrnightly {-mumode mu}
4366MCb_dhda-i clone 4366MC_dhda-su-i \
    -nvram "$nvram4709C0 $nvramSU $nvramAtlas"
4366MCH5E_3x3_dhda-i clone 4366MCH5E_3x3_dhda-su-i\
    -nvram "$nvram4709C0 $nvramSU $nvramAtlas"
4366MCb_dhda-i clone 4366MC_dhda-mu-dm1-i \
    -nvram "$nvram4709C0 $nvramMU $nvramDM1"\
    -rvrnightly {-mumode mu}
4366MCH5E_3x3_dhda-i clone 4366MCH5E_3x3_dhda-mu-dm1-i\
    -nvram "$nvram4709C0 $nvramMU $nvramDM1"\
    -rvrnightly {-mumode mu}
4366MCb_dhda-i clone 4366MC_dhda-su-dm1-i \
    -nvram "$nvram4709C0 $nvramSU $nvramDM1"
4366MCH5E_3x3_dhda-i clone 4366MCH5E_3x3_dhda-su-dm1-i\
    -nvram "$nvram4709C0 $nvramSU $nvramDM1"
4366MCb_dhd-i configure -dualband {4366MCH5E_3x3_dhd-i -c2 157/80 -c1 6l}
4366MCb_dhda-i configure -dualband {4366MCH5E_3x3_dhda-i -c2 157/80 -c1 6l}


### Gold 3.0 ###
if {[info exist env(TAG_G3)]} {
    set tag_g3 $env(TAG_G3)
} else {
    set tag_g3 BISON04T_REL_7_14_164{_?*} 
}
puts "#### tag_g3 = $tag_g3 ####"

4366MC_dhda-mu-i clone 4366MC_dhda-g3-mu-i -tag $tag_g3
4366MC_dhda-su-i clone 4366MC_dhda-g3-su-i -tag $tag_g3
4366MC_dhda-mu-dm1-i clone 4366MC_dhda-g3-mu-dm1-i -tag $tag_g3
4366MC_dhda-su-dm1-i clone 4366MC_dhda-g3-su-dm1-i -tag $tag_g3
4366MCH5E_3x3_dhda-mu-i clone 4366MCH5E_3x3_dhda-g3-mu-i -tag $tag_g3
4366MCH5E_3x3_dhda-su-i clone 4366MCH5E_3x3_dhda-g3-su-i -tag $tag_g3
4366MCH5E_3x3_dhda-mu-dm1-i clone 4366MCH5E_3x3_dhda-g3-mu-dm1-i -tag $tag_g3
4366MCH5E_3x3_dhda-su-dm1-i clone 4366MCH5E_3x3_dhda-g3-su-dm1-i -tag $tag_g3

set nvramASPM "aspm_config=1"
4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-aspm-i \
    -nvram "$nvram4709C0 $nvramMU $nvramAtlas"\
    -wlinitcmds {echo l1_powersave > /sys/module/pcie_aspm/parameters/policy} \
    -pre_perf_hook {
        {%S dhd -i eth2 aspm}
    }

4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-bad\
    -image /projects/hnd_swbuild/PRESERVED/build_linux/BISON04T_BRANCH_7_14/linux-2.6.36-arm-internal-router-dhdap-atlas/2016.7.2.0/build/image/linux.trx.gz
4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-good \
    -image /projects/hnd_swbuild/PRESERVED/build_linux/BISON04T_BRANCH_7_14/linux-2.6.36-arm-internal-router-dhdap-atlas/2016.7.1.0/build/image/linux.trx.gz
4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-p1 -tag $tag_g3 \
        -pre_perf_hook {
            {%S wl svmp_mem 0x22f00 1 0} 
            {%S wl svmp_mem 0x22f01 1 16}
            {%S wl svmp_mem 0x22f02 1 24} 
            {%S wl svmp_mem 0x22f03 1 48}
            {%S wl svmp_mem 0x22f04 1 56}
            {%S wl svmp_mem 0x22f05 1 72} 
            {%S wl svmp_mem 0x22f06 1 80}
            {%S wl svmp_mem 0x22f07 1 88} 
            {%S wl svmp_mem 0x22f08 1 96} 
            {%S wl svmp_mem 0x22f09 1 104} 
        } 
4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-p2 -tag $tag_g3 \
        -pre_perf_hook {
            {%S wl svmp_mem 0x22f00 1 0} 
            {%S wl svmp_mem 0x22f01 1 16}
            {%S wl svmp_mem 0x22f02 1 24} 
            {%S wl svmp_mem 0x22f03 1 52}
            {%S wl svmp_mem 0x22f04 1 60}
            {%S wl svmp_mem 0x22f05 1 72} 
            {%S wl svmp_mem 0x22f06 1 84}
            {%S wl svmp_mem 0x22f07 1 92} 
            {%S wl svmp_mem 0x22f08 1 100} 
            {%S wl svmp_mem 0x22f09 1 108} 
        } 
4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-force4MU -tag $tag_g3 \
        -pre_perf_hook {
            {%S wl mu_group}
            {%S wl mu_group -g 0 0x009 0x109 0x209 0x309 -f 0} 
            {%S wl mu_group}
        }
4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-f0 -tag $tag_g3 \
        -pre_perf_hook {
            {%S wl curpower} 
            {%S wl mu_group -g 0 0x009 0x109 0x209 0x309 -f 0} 
            {%S wl mu_group}
        } 
4366MC_dhda-g3-mu-f0 clone 4366MC_dhda-g3-mu-f1 \
        -pre_perf_hook {
            {%S wl curpower} 
            {%S wl mu_group -g 0 0x009 0x109 0x209 0x309 -f 1} 
            {%S wl mu_group}
        } 
     
        #-pre_perf_hook {
        #    {%S wl curpower} 
        #    {%S wl bus:flr_lfrag_txpkts_adjust 1} 
        #    {UTF::Every 1 %S wl bus:dumptxrings}                                                                                   
        #} \
        #-post_perf_hook {{UTF::Every cancel all}} 
if {[info exist env(AP_IMAGE)]} {
    4366MC_dhda-mu-i clone 4366MC_dhda-mu-p -image $env(AP_IMAGE)
    4366MC_dhda-su-i clone 4366MC_dhda-su-p -image $env(AP_IMAGE)
    4366MC_dhda-g3-mu-i clone 4366MC_dhda-g3-mu-p -image $env(AP_IMAGE)\
        -pre_perf_hook {
            {%S wl phy_mu_nss4enable 1}
        }
    
    4366MC_dhda-g3-su-i clone 4366MC_dhda-g3-su-p -image $env(AP_IMAGE)
    4366MCH5E_3x3_dhda-g3-mu-i clone 4366MCH5E_3x3_dhda-g3-mu-p -image $env(AP_IMAGE)
    4366MCH5E_3x3_dhda-g3-su-i clone 4366MCH5E_3x3_dhda-g3-su-p -image $env(AP_IMAGE)
    4366MC_dhda-g3-mu-dm1-i clone 4366MC_dhda-g3-mu-dm1-p -image $env(AP_IMAGE)
}

# Eagle STA clones    
if {[info exist env(AP_IMAGE)]} {
    4709C0/4366MC clone 4366MCe-i \
        -image $env(AP_IMAGE) \
        -nvram $nvram4709C0 \
        -perfchans {36/80 36l 36}

    4709C0/4366MCH5E clone 4366MCH5Ee_3x3-i \
        -image $env(AP_IMAGE) \
        -perfchans {36/80 36l 36}
    4366MCe-i clone 4366MCe-mu-i -nvram "$nvram4709C0 $nvramMU "\
    -rvrnightly {-mumode mu}

    4366MCe-i clone 4366MCe-su-i
    4366MCe-mu-i configure -dualband {4366MCe-mu-i -c1 161/80 -c2 6l}
}
### for internal images
if {[info exist env(TAG_E)]} {
    set tag_e $env(TAG_E)
} else {
    set tag_e EAGLE_REL_10_10_122{,_?*}
}
puts "#### tag_e = $tag_e ####"

set brand "linux-2.6.36-arm-internal-router"
4366MC_dhda-g3-su-i clone 4366MC_dhda-e-su-i -tag $tag_e
4366MC_dhda-g3-mu-i clone 4366MC_dhda-e-mu-i -tag $tag_e

4709C0/4366MC clone 4366MCe-i -tag $tag_e \
    -brand $brand \
    -nvram $nvram4709C0 \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5Ee_3x3-i -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4366MCe-i clone 4366MCe-mu-i -nvram "$nvram4709C0 $nvramMU "\
-rvrnightly {-mumode mu}
4366MCe-i clone 4366MCe-su-i
4366MCe-mu-i configure -dualband {4366MCe-mu-i -c1 161/80 -c2 6l}

set brand linux-2.6.36-arm-internal-router-dhdap
4709C0/4366MC clone 4366MCe_dhd-i -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4366MCe_dhd-i clone 4366MCe_dhd1-i
4366MCe_dhd-i clone 4366MCe_dhd2-i

4709C0/4366MCH5E clone 4366MCH5Ee_3x3_dhd-i -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

### for external images
set brand "linux-2.6.36-arm-external-vista-router-full-src" 
4709C0/4366MC clone 4366MCe -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5Ee_3x3 -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

set brand "linux-2.6.36-arm-external-vista-router-dhdap-full-src" 
4709C0/4366MC clone 4366MCe_dhd -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4709C0/4366MCH5E clone 4366MCH5Ee_3x3_dhd -tag $tag_e \
    -brand $brand \
    -perfchans {36/80 36l 36}

4366MCe-i configure -dualband {4366MCe-i -c1 161/80 -c2 6l}
4366MCH5Ee_3x3-i configure -dualband {4366MCe-i -c2 161/80 -c1 6l}

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
	-relay "svt-utf2-end1" \
    -tag BISON04T_BRANCH_7_14 \
    -brand LINUX_4.1.0_ROUTER_DHDAP/94908HND_int \
    -type bcm94908HND_nand_fs_image_128_ubi.w \
    -lan_ip 192.168.1.1 \
    -lanpeer {lan lan2} \
    -console "svt-utf2-end1:40001" \
    -relay lan2 \
    -power {svt-utf2-npc1 2} \
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
        -nvram "$nvram4908 $nvramMU" -rvrnightly {-mumode mu}\
        -image $image 

    ${obj}-i clone ${obj}-su-i\
        -nvram "$nvram4908 $nvramSU"\
        -image $image 

    ${obj}-e clone ${obj}-mu-e\
        -nvram "$nvram4908 $nvramMU" -rvrnightly {-mumode mu}\
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
            -nvram "$nvram4908 $nvramMU" -rvrnightly {-mumode mu}
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
UTF::Q utf2

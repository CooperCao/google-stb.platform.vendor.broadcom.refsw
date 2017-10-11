# -*-tcl-*-
#
# Testbed configuration file for Rodney Baba MC70testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext/$::env(LOGNAME)/mc75"

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# this is a tool given by Tim to allow running private scripts ontop of StaNightly
set ::UTF::StaNightlyCustom {
	# Custom code here
	package require UTF::Test::throttle
	if {[$STA cget -custom] != ""} {
		throttle $Router $STA -chanspec 3 -nonthrottlecompare 1
	}
	UTF::Message INFO $STA "throttle test skipped"
}

# Define power controllers on cart
UTF::Power::Synaccess npc45 -lan_ip 172.4.1.45 -rev 1
UTF::Power::Synaccess npc46 -lan_ip 172.4.1.46 -rev 1
UTF::Power::WebRelay  web47 -lan_ip 172.4.1.47
UTF::Power::Synaccess npc55 -lan_ip 172.4.1.55 -rev 1
UTF::Power::Synaccess npc56 -lan_ip 172.4.1.56 -rev 1
UTF::Power::WebRelay  web57 -lan_ip 172.4.1.57
UTF::Power::Synaccess npc65 -lan_ip 172.4.1.65 -rev 1
UTF::Power::Synaccess npc75 -lan_ip 172.4.1.75 -rev 1
UTF::Power::Synaccess npc85 -lan_ip 172.4.1.85 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.4.1.200 \
    -relay "lan" \
    -group {
	    G1 {1 2 3}
	    G2 {4 5 6}
	    G3 {7 8 9}
	    }
	    
	    G1 configure -default 0
	    G2 configure -default 0
	    G3 configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default
    G2 attn default
    G3 attn default

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    AP1 restart wl0_radio=0
    AP1 restart wl1_radio=0
    AP2 restart wl0_radio=0
    AP2 restart wl1_radio=0
 

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP 4331FC15 Mac4360BU 4360STA MacX29d} {
	    UTF::Try "$S Down" {
		    $S wl down
	    }
	    $S deinit
	}
    # unset S so it doesn't interfere
    unset S

          
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}


# Define Sniffer
UTF::Sniffer mc75snf1 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc85 2} \
        -power_button {auto} \
        -console "mc75end1:40006"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc75end1 \
    -sta {lan eth1} 

# UTF Endpoint2 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc75end2 \
    -sta {lan1 eth1}

# FC15 Linux Dell DQ76 Small Form Factor PC
UTF::Linux mc75tst1 -sta {4360SOFTAP eth0} \
    -power {npc45 1} \
    -console {mc75end1:40007} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
    }
    
4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
	}

# Needed for some performance improvements
# wl ack_ratio 6; wl ampdu_mpdu 64; wl txmaxpkts 512;

# Clones for mc64tst1
4360SOFTAP clone 4360SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP clone 4360SOFTAP1-AARDVARK -tag AARDVARK_BRANCH_6_28
4360SOFTAP clone 4360SOFTAP1-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_* -brand "linux-external-wl"
4360SOFTAP clone 4360SOFTAP-BISON -tag BIS120RC4_BRANCH_7_15


# softap settting for AARDVARK
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
	}
	
# softap settting for BISON
4360SOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
	}
	
4360SOFTAP1-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP1-AARDVARK-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_?? -brand "linux-internal-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"

# softap settting for AARDVARK
4360SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-TAG-EXT configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-10 configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-10-EXT configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP clone 4360SOFTAP-RUBY -tag RUBY_BRANCH_6_20
4360SOFTAP clone 4360SOFTAP-KIRIN -tag KIRIN_BRANCH_5_100
4360SOFTAP clone 4360SOFTAP-KIRIN-REL -tag KIRIN_REL_5_100_*
4360SOFTAP clone 4360SOFTAP-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360SOFTAP clone 4360SOFTAP-BASS -tag BASS_BRANCH_5_60   
4360SOFTAP clone 4360SOFTAP-BASS-REL -tag BASS_REL_5_60_???

# STA Laptop DUT Dell E6400 FC11 - 4331hm P152
# need to ensure no dhcp running before tests in wl init as this is used as a softAp on some tests
UTF::Linux mc75tst2 -sta {4331FC15 eth0} \
  	-power {npc45 2} \
	-power_button {auto} \
    -console "mc75end1:40008" \
    -wlinitcmds {wl msglevel 0x101  ; wl mpc 0; service dhcpd stop;:} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -perfonly 1  -custom 1


# Clones for mc64tst3
4331FC15 clone 4331FC15-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1 -custom 1
4331FC15 clone 4331FC15-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_118_??
4331FC15 clone 4331FC15-BISON -tag  BIS120RC4_BRANCH_7_15 -custom 1
4331FC15 clone 4331FC15-RUBY -tag RUBY_BRANCH_6_20
4331FC15 clone 4331FC15-KIRIN -tag KIRIN_BRANCH_5_100
4331FC15 clone 4331FC15-KIRIN-REL -tag KIRIN_REL_5_100_*
4331FC15 clone 4331FC15-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4331FC15 clone 4331FC15-BASS -tag BASS_BRANCH_5_60   
4331FC15 clone 4331FC15-BASS-REL -tag BASS_REL_5_60_???         

# SoftAP Version of the same host 
# need to ensure that DHCP is started on the host with a static address
4331FC15 clone 4331FC15ap
4331FC15ap configure -ipaddr 192.168.1.20 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4331FC15ap \
    -wlinitcmds {wl mpc 0; wl stbc_rx 1; service dhcpd stop;:} 
4331FC15ap clone 4331FC15ap-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_118_??
4331FC15ap-AARDVARK-REL-TAG configure -ipaddr 192.168.1.20 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4331FC15ap \
    -wlinitcmds {wl mpc 0; wl stbc_rx 1; service dhcpd stop;:} 

# load clones for SoftAP
4331FC15ap clone 4331FC15ap-AARDVARK -tag AARDVARK_BRANCH_6_30
4331FC15ap clone 4331FC15ap-RUBY -tag RUBY_BRANCH_6_20
4331FC15ap clone 4331FC15ap-KIRIN -tag KIRIN_BRANCH_5_100
4331FC15ap clone 4331FC15ap-KIRIN-REL -tag KIRIN_REL_5_100_*
4331FC15ap clone 4331FC15ap-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*


# 4360BU MacBook Air
# Needed to get around MAC Power Cycle to Debugger
# The {%S wl btc_mode 0} entires in pre-perf hooks is to work around a 2.G 20 RvR issue with BT Coex
# Even with BT enabled on the Host, we still see a lower RvR RX Graph
UTF::Power::Laptop 4360BUbpower -button {web57 1}
UTF::MacOS mc75tst3 -sta {Mac4360BU en1} \
	-power {4360BUbpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl btc_mode} {%S ifconfig} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 3640K  -custom 1



# used for when connecting to softap
# Mac4360BU configure -ipaddr 192.168.1.25

# used when testing against an non-ac router, so we don't want to tet AC rates there
Mac4360BU clone Mac4360BUnonly -custom 1 \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1 }
Mac4360BUnonly clone Mac4360BUnonly-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1
Mac4360BUnonly clone Mac4360BUnonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
Mac4360BUnonly clone Mac4360BUnonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 
Mac4360BUnonly clone Mac4360BUnonly-BISON -tag BIS120RC4_BRANCH_7_15 -custom 1
Mac4360BUnonly clone Mac4360BUnonly-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1 

# Clones for mc64tst6
Mac4360BU clone Mac4360BU-TOT -nochannels 1 -custom 1 \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1 }
Mac4360BU clone Mac4360BU-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1 -custom 1
Mac4360BU clone Mac4360BU-AARDVARK-RAVI \
	-image /projects/hnd_software_ext6/work/richapur/AARDVARK_TWIG_6_30_223/src/wl/macos/build/Debug_10_8/AirPortBroadcom43XX.kext
Mac4360BU clone Mac4360BU-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*
Mac4360BU clone Mac4360BU1-AARDVARK -tag AARDVARK_BRANCH_6_28
Mac4360BU clone Mac4360BU1-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
Mac4360BU clone Mac4360BU-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
Mac4360BU clone Mac4360BU-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_*
Mac4360BU clone Mac4360BU-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand
Mac4360BU clone Mac4360BU-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand
Mac4360BU clone Mac4360BU-BISON -tag BIS120RC4_BRANCH_7_15 -custom 1
Mac4360BU clone Mac4360BU-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
Mac4360BU clone Mac4360BU-RUBY -tag RUBY_BRANCH_6_20
Mac4360BU clone Mac4360BU-KIRIN -tag KIRIN_BRANCH_5_100
Mac4360BU clone Mac4360BU-KIRIN-REL -tag KIRIN_REL_5_100_*
Mac4360BU clone Mac4360BU-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
Mac4360BU clone Mac4360BU-BASS -tag BASS_BRANCH_5_60   
Mac4360BU clone Mac4360BU-BASS-REL -tag BASS_REL_5_60_???      

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
Mac4360BU clone Mac4360BU-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl country US ; wl vht_features 3; wl dtim 3}
Mac4360BU-SOFTAP-TOT configure -ipaddr 192.168.1.110 -ssid test4360MACap

Mac4360BU clone Mac4360BU-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl country US ; wl vht_features 3; wl dtim 3}
Mac4360BU-SOFTAP-AARDVARK configure -ipaddr 192.168.1.110 -ssid test4360MACap
    
Mac4360BU-SOFTAP-AARDVARK clone Mac4360BU-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl country US ; wl vht_features 3; wl dtim 3}
Mac4360BU-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.110 -ssid test4360MACap

Mac4360BU clone Mac4360BU-SOFTAP-BISON -tag BIS120RC4_BRANCH_7_15 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl country US ; wl vht_features 3; wl dtim 3}
Mac4360BU-SOFTAP-BISON configure -ipaddr 192.168.1.110 -ssid test4360MACap

Mac4360BU clone Mac4360BU-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl country US ; wl vht_features 3; wl dtim 3}
Mac4360BU-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.110 -ssid test4360MACap
    
# FC15 Linux Dell DQ76 Small Form Factor PC - X29d
UTF::Linux mc75tst4 -sta {4360STA eth0} \
    -power {npc55 2} \
    -console {mc75end1:40012} \
    -slowassoc 5 -reloadoncrash 1 \
    -brand "linux-internal-wl" \
    -type obj-debug-p2p-mchan \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-post_assoc_hook {%S wl phy_forcecal 1} \
	-channelsweep {-count 15} \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 4M  -custom 1 -perfonly 1 \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl btc_mode 0 ; wl mpc 0; wl mimo_bw_cap 1; wl vht_features 3; \
        wl obss_coex 0; wl country US ; wl assert_type 1;: 
    }
# used when testing against an non-ac router, so we don't want to tet AC rates there
4360STA clone 4360STAnonly -custom 1
4360STAnonly clone 4360STAnonly-AARDVARK -tag AARDVARK_BRANCH_6_30  -modopts passivemode=1 -perfonly 1
4360STAnonly clone 4360STAnonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*  -modopts passivemode=1
4360STAnonly clone 4360STAnonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_*  -modopts passivemode=1
4360STAnonly clone 4360STAnonly-BISON -tag BIS120RC4_BRANCH_7_15 -modopts passivemode=1
4360STAnonly clone 4360STAnonly-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*  -modopts passivemode=1


# Clones for mc75tst4
4360STA clone 4360STA-TOT -nochannels 1
4360STA clone 4360STA-AARDVARK -tag AARDVARK_BRANCH_6_30 -modopts passivemode=1 -perfonly 1 -custom 1
4360STA clone 4360STA-AARDVARK-RAVI -modopts passivemode=1 \
	-image /projects/hnd_software_ext6/work/richapur/AARDVARK_TWIG_6_30_223/src/wl/linux/obj-debug-p2p-mchan-2.6.38.6-26.rc1.fc15.i686.PAE/
4360STA clone 4360STA-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*  -modopts passivemode=1
4360STA clone 4360STA-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl" -yart {-attn "10-75" -mirror -frameburst 1}
4360STA clone 4360STA-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
4360STA clone 4360STA-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
4360STA clone 4360STA-BISON -tag BIS120RC4_BRANCH_7_15 -modopts passivemode=1
4360STA clone 4360STA-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -modopts passivemode=1
4360STA clone 4360STA-BISON-NONP2P -tag BISON_BRANCH_7_10 -type obj-debug-apdef-stadef
4360STA clone 4360STA-RUBY -tag RUBY_BRANCH_6_20
4360STA clone 4360STA-KIRIN -tag KIRIN_BRANCH_5_100
4360STA clone 4360STA-KIRIN-REL -tag KIRIN_REL_5_100_*
4360STA clone 4360STA-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360STA clone 4360STA-BASS -tag BASS_BRANCH_5_60   
4360STA clone 4360STA-BASS-REL -tag BASS_REL_5_60_???      


# MC75tst5 - MacBook Air with Zin OS 10.10 with X29c P200
# Needed to get around MAC Power Cycle to Debugger    
UTF::Power::Laptop X29dpower -button {web47 1}
UTF::MacOS mc75tst5 -sta {MacX29c en0} \
	-power {X29dpower} \
	-power_button {auto} \
	-wlinitcmds {wl msglevel 0x101  ; wl btc_mode 0 ; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl dump phycal} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl dump phycal} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-external-wl-syr" \
	-type Debug_10_10 \
	-coreserver AppleCore \
	-kextload true -datarate 0

# Clones for mc68tst5
MacX29c clone MacX29c-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1 -perfonly 1
MacX29c clone MacX29c-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1 \
	-yart {-attn "10-75" -mirror -frameburst 1}
MacX29c clone MacX29c-BISON -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX29c clone MacX29c-BISON-REL-TAG -tag BIS120RC4_REL_7_15_195 -custom 1
MacX29c clone MacX29c-RUBY -tag RUBY_BRANCH_6_20
MacX29c clone MacX29c-KIRIN -tag KIRIN_BRANCH_5_100
MacX29c clone MacX29c-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX29c clone MacX29c-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX29c clone MacX29c-BASS -tag BASS_BRANCH_5_60   
MacX29c clone MacX29c-BASS-REL -tag BASS_REL_5_60_???            


# AP Section

#Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "43311 eth2" \
	-lan_ip 192.168.1.1 \
    -relay "mc75end1" \
    -lanpeer lan \
    -console "mc75end1:40002" \
    -power {npc65 2} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
	-nvram {
		boot_hw_model=E4200
        wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
		et0macaddr=00:90:4c:08:00:8c
		macaddr=00:90:4c:08:00:9c
		sb/1/macaddr=00:90:4c:08:04:00
		pci/1/1/macaddr=00:90:4c:08:04:01
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
		fw_disable=1
		#Only 1 AP can serve DHCP Addresses
		#router_disable=1
		#simultaneous dual-band router with 2 radios
		wl0_radio=0
		wl1_radio=0
		wl1_nbw_cap=0
        wl_msglevel=0x101
		wl0_ssid=test43311-ant0
		wl1_ssid=test43311-ant1
		wl0_channel=1
		wl1_channel=1
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	    wl1_obss_coex=0
	}

# Clones for 43311
43311 clone 43311-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43311 configure -attngrp G1


# Lynksys E4200 4718/4331 Router AP2
UTF::Router AP2 \
    -sta "43312 eth2" \
	-lan_ip 192.168.1.2 \
    -relay "mc75end1" \
    -lanpeer lan \
    -console "mc75end1:40004" \
    -power {npc75 2} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
	-nvram {
		boot_hw_model=E4200
        wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
		et0macaddr=00:90:4c:09:00:8c
		macaddr=00:90:4c:09:00:9c
        sb/1/macaddr=00:90:4c:09:05:00
        pci/1/1/macaddr=00:90:4c:09:05:01
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
		fw_disable=1
		# Only 1 AP can serve DHCP Addresses
		router_disable=1
		wl0_radio=0
		wl1_radio=0
		wl1_nbw_cap=0
        wl_msglevel=0x101
		wl0_ssid=test43312-ant0
		wl1_ssid=test43312-ant1
		wl0_channel=1
		wl1_channel=1
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	    wl1_obss_coex=0
	}
# Clones for 43312
43311 clone 43312-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43312 configure -attngrp G2

#### ADD UTF::Q for this rig
#####
UTF::Q mc75

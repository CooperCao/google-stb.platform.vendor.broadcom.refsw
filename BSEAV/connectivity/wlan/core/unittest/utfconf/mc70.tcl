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

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext/$::env(LOGNAME)/mc70"


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
UTF::Power::Synaccess npc45 -lan_ip 172.3.1.45 -relay mc70end1 -rev 1
UTF::Power::Synaccess npc46 -lan_ip 172.3.1.46 -relay mc70end1
UTF::Power::WebRelay  web47 -lan_ip 172.3.1.47 -relay mc70end1
UTF::Power::Synaccess npc55 -lan_ip 172.3.1.55 -relay mc70end1 -rev 1
UTF::Power::Synaccess npc56 -lan_ip 172.3.1.56 -relay mc70end1 -rev 1
UTF::Power::WebRelay  web57 -lan_ip 172.3.1.57 -relay mc70end1
UTF::Power::Synaccess npc65 -lan_ip 172.3.1.65 -relay mc70end1 -rev 1
UTF::Power::Synaccess npc75 -lan_ip 172.3.1.75 -relay mc70end1 -rev 1
UTF::Power::Synaccess npc85 -lan_ip 172.3.1.85 -relay mc70end1 -rev 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.3.1.200 \
    -relay "lan" \
    -group {
	    G1 {1 2 3}
	    G2 {4 5 6}
	    G3 {7 8 9}
	    G4 {10 11 12}
	    }
	    
	    G1 configure -default 0
	    G2 configure -default 0
	    G3 configure -default 0
	    G4 configure -default 0

# Define Sniffer
UTF::Sniffer mc70snf1 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc85 2} \
        -power_button {auto} \
        -console "mc70end1:40006"


# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default
    G2 attn default
    G3 attn default
    G4 attn default

    
    
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    AP1 restart wl0_radio=0
    AP1 restart wl1_radio=0
    AP2 restart wl0_radio=0
    AP2 restart wl1_radio=0
 
    
    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP MacX28 4331FC15 MacX52c MacX28b MacX51} {
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

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc70end1 \
    -sta {lan eth1}  -iperf /projects/hnd/tools/linux/bin/iperf

# UTF Endpoint2 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc70end2 \
    -sta {lan1 eth1}

# This configuration uses HSIC as its own endpoint
# 4330OlympicUNO3 P301(XX)
#UTF::HSIC mc70tst1 -sta {4330h eth1} \
#    -relay lan \
#    -power {npc55 1} \
#    -console "mc70end1:40011" \
#    -nvram "src/shared/nvram/bcm94330OlympicUNO3.txt" \
#    -nvram_add "muxenab=0x1" \
#    -tag "FALCON_BRANCH_5_90" \
#    -type "4330b2-roml/usb-g-ccx-btamp-p2p-idsup-idauth-pno.bin.trx" \
#    -host_tag "RTRFALCON_REL_5_130_*" \
#    -host_nvram {
#		lan_ipaddr=10.19.87.185
#		lan_netmask=255.255.252.0
#		lan_gateway=10.19.84.1
#		wandevs=dummy
#		clkfreq=530,176,88
#		watchdog=6000
#   } \
#    -nocal 1

UTF::Linux mc70tst1 -sta {4360SOFTAP eth0} \
    -power {npc46 1} \
    -console {mc70end1:40009} \
    -slowassoc 5 -reloadoncrash 1 \
    -tag AARDVARK_{REL,BRANCH}_6_30{,*} \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
    -wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl dtim 3; wl btc_mode 0 ; wl mimo_bw_cap 1 ; \
        wl vht_features 3; wl obss_coex 0; wl assert_type 1; service dhcpd stop;: 
    }

4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl dtim 3; wl btc_mode 0 ; wl mimo_bw_cap 1; \
        wl vht_features 3; wl obss_coex 0; wl assert_type 1; wl country US ; service dhcpd stop;: 
    }
	

# Clones for mc64tst1
4360SOFTAP clone 4360SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP clone 4360SOFTAP-BISON -tag BISON_BRANCH_7_10
4360SOFTAP clone 4360SOFTAP1-AARDVARK -tag AARDVARK_BRANCH_6_28
4360SOFTAP clone 4360SOFTAP1-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_* -brand "linux-external-wl"

# softap settting for AARDVARK
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl dtim 3; wl btc_mode 0 ; wl mimo_bw_cap 1; \
        wl vht_features 3; wl obss_coex 0; wl assert_type 1; wl country US ; service dhcpd stop;: 
    }
    
# softap settting for AARDVARK
4360SOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl dtim 3; wl btc_mode 0 ; wl mimo_bw_cap 1; \
        wl vht_features 3; wl obss_coex 0; wl assert_type 1; wl country US ; service dhcpd stop;: 
    }
    
4360SOFTAP1-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP1-AARDVARK-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -brand "linux-internal-wl"
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


# Needed to get around MAC Power Cycle to Debugger    
UTF::Power::Laptop X28power -button {web57 2}
UTF::MacOS mc70tst2 -sta {MacX28 en1} \
	-power {X28power} \
	-power_button {auto} \
	-wlinitcmds {wl msglevel 0x101; wl oppr_roam_off 1} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl scansuppress 1} {%S wl phy_cal_disable 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true -datarate 0


# Clones for mc70tst2
MacX28 clone MacX28-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1 -perfonly 1
MacX28 clone MacX28-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1
MacX28 clone MacX28-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX28 clone MacX28-RUBY -tag RUBY_BRANCH_6_20
MacX28 clone MacX28-KIRIN -tag KIRIN_BRANCH_5_100
MacX28 clone MacX28-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX28 clone MacX28-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX28 clone MacX28-BASS -tag BASS_BRANCH_5_60   
MacX28 clone MacX28-BASS-REL -tag BASS_REL_5_60_???


# STA Laptop DUT Dell E6400 FC11 - 4331hm P152
# need to ensure no dhcp running before tests in wl init as this is used as a softAp on some tests
UTF::Linux mc70tst3 -sta {4331FC15 eth0} \
  	-power {npc45 1} \
	-power_button {auto} \
    -console "mc70end1:40007" \
    -wlinitcmds {wl msglevel 0x101  ; wl mpc 0; service dhcpd stop;:} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -perfonly 1 -yart {-attn "10-75" -mirror -frameburst 1 -b 450m}

# Clones for mc70tst3
4331FC15 clone 4331FC15-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1
4331FC15 clone 4331FC15-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_??? -perfonly 1
4331FC15 clone 4331FC15-BISON -tag BISON_BRANCH_7_10 -custom 1 -perfonly 1
4331FC15 clone 4331FC15-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1 -perfonly 1
4331FC15 clone 4331FC15-RUBY -tag RUBY_BRANCH_6_20
4331FC15 clone 4331FC15-KIRIN -tag KIRIN_BRANCH_5_100
4331FC15 clone 4331FC15-KIRIN-REL -tag KIRIN_REL_5_100_*
4331FC15 clone 4331FC15-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_* -perfonly 1
4331FC15 clone 4331FC15-BASS -tag BASS_BRANCH_5_60   
4331FC15 clone 4331FC15-BASS-REL -tag BASS_REL_5_60_???         

# SoftAP Version of the same host 
# need to ensure that DHCP is started on the host with a static address
4331FC15 clone 4331FC15ap
4331FC15ap configure -ipaddr 192.168.1.20 -hasdhcpd 1 -ap 1 -ssid test4331FC15ap \
    -wlinitcmds {wl mpc 0; wl stbc_rx 1; service dhcpd stop;:} 

# load clones for SoftAP
4331FC15ap clone 4331FC15ap-AARDVARK -tag AARDVARK_BRANCH_6_30
4331FC15ap-AARDVARK configure -ipaddr 192.168.1.20 -hasdhcpd 1 -ap 1 -ssid test4331FC15ap \
    -wlinitcmds {wl mpc 0; wl stbc_rx 1; service dhcpd stop;:}
4331FC15ap clone 4331FC15ap-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_???
4331FC15ap-AARDVARK-REL-TAG configure -ipaddr 192.168.1.20 -hasdhcpd 1 -ap 1 -ssid test4331FC15ap \
    -wlinitcmds {wl mpc 0; wl stbc_rx 1; service dhcpd stop;:}
4331FC15ap clone 4331FC15ap-RUBY -tag RUBY_BRANCH_6_20
4331FC15ap clone 4331FC15ap-KIRIN -tag KIRIN_BRANCH_5_100
4331FC15ap clone 4331FC15ap-KIRIN-REL -tag KIRIN_REL_5_100_*
4331FC15ap clone 4331FC15ap-KIRIN-REL-TAG -tag KIRIN_REL_5_100_82_???

# X21b MacBook Air
# Needed to get around MAC Power Cycle to Debugger
# The {%S wl btc_mode 0} entires in pre-perf hooks is to work around a 2.4G 20 RvR issue with BT Coex
# Even with BT enabled on the Host, we still see a lower RvR RX Graph
UTF::Power::Laptop X21bpower -button {web57 1}
UTF::MacOS mc70tst4 -sta {MacX52c en0} \
	-power {X21bpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl btc_mode 0} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true -datarate 0
 
# Clones for mc70tst4
MacX52c clone MacX52c-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1 -perfonly 1
MacX52c clone MacX52c-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1
MacX52c clone MacX52c-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX52c clone MacX52c-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX52c clone MacX52c-RUBY -tag RUBY_BRANCH_6_20
MacX52c clone MacX52c-KIRIN -tag KIRIN_BRANCH_5_100
MacX52c clone MacX52c-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX52c clone MacX52c-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX52c clone MacX52c-BASS -tag BASS_BRANCH_5_60   
MacX52c clone MacX52c-BASS-REL -tag BASS_REL_5_60_???      

# X19 that came with MacBook Pro
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop X28bpower -button {web47 1}
UTF::MacOS mc70tst5 -sta {MacX28b en1} \
	-power {X28bpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true -datarate 0

# Clones for mc70tst5
MacX28b clone MacX28b-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1 -perfonly 1
MacX28b clone MacX28b-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1
MacX28b clone MacX28b-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX28b clone MacX28b-RUBY -tag RUBY_BRANCH_6_20
MacX28b clone MacX28b-KIRIN -tag KIRIN_BRANCH_5_100
MacX28b clone MacX28b-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX28b clone MacX28b-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX28b clone MacX28b-BASS -tag BASS_BRANCH_5_60   
MacX28b clone MacX28b-BASS-REL -tag BASS_REL_5_60_???      


# 4360BU MacBook Air , Now MBP
# Needed to get around MAC Power Cycle to Debugger
# The {%S wl btc_mode 0} entires in pre-perf hooks is to work around a 2.G 20 RvR issue with BT Coex
# Even with BT enabled on the Host, we still see a lower RvR RX Graph
UTF::Power::Laptop 4360BUbpower -button {web57 3}
UTF::MacOS mc70tst6 -sta {MacX51 en0} \
	-power {4360BUbpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101 ; wl msglevel +wsec; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl btc_mode} {%S wl PM} {%S wl phy_tempsense} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1


# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX51 clone MacX51nonly -perfonly 1
MacX51nonly clone MacX51nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1
MacX51nonly clone MacX51nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX51nonly clone MacX51nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_*
MacX51nonly clone MacX51nonly-BISON -tag BISON_BRANCH_7_10



# Clones for mc64tst6
MacX51 clone MacX51-TOT -nochannels 1 -custom 1 \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US; wl assert_type 1; wl oppr_roam_off 1 }
MacX51 clone MacX51-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfonly 1 -custom 1
MacX51 clone MacX51-AARDVARK-RAVI \
	-image /projects/hnd_software_ext6/work/richapur/AARDVARK_TWIG_6_30_223/src/wl/macos/build/Debug_10_8/AirPortBroadcom43XX.kext
MacX51 clone MacX51-AARDVARK-THROTTLE -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl btc_mode} {%S wl PM} {%S wl phy_tempthresh 10} {%S wl phy_tempsense} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}
MacX51 clone MacX51-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-post_assoc_hook {{%S wl msglevel +ps} {%S wl phy_watchdog 0} {%S wl pm2_sleep_ret 500}}
MacX51 clone MacX51-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 -custom 1 \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl PM 2}} 
MacX51 clone MacX51-AARDVARK-PM3 -tag AARDVARK_BRANCH_6_30 -custom 1 \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl PM 3}} 
MacX51 clone MacX51-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*
MacX51 clone MacX511-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX51 clone MacX511-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX51 clone MacX51-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX51 clone MacX51-AARDVARK-REL-TAG-THROTTLE -tag AARDVARK_REL_6_30_223_* \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl btc_mode} {%S wl PM} {%S wl phy_tempthresh 10} {%S wl phy_tempsense} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}
MacX51 clone MacX51-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_*
MacX51 clone MacX51-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_*
MacX51 clone MacX51-AARDVARK-REL-TWIG-EXT -tag AARDVARK_TWIG_6_30_*
MacX51 clone MacX51-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX51 clone MacX51-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX51 clone MacX51-RUBY -tag RUBY_BRANCH_6_20
MacX51 clone MacX51-KIRIN -tag KIRIN_BRANCH_5_100
MacX51 clone MacX51-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX51 clone MacX51-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX51 clone MacX51-BASS -tag BASS_BRANCH_5_60   
MacX51 clone MacX51-BASS-REL -tag BASS_REL_5_60_???      

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX51 clone MacX51-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl country US ; wl assert_type 1; wl vht_features 1; wl dtim 3}
MacX51-SOFTAP-TOT configure -ipaddr 192.168.1.110 -ssid test4360MACap

MacX51 clone MacX51-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl country US ; wl assert_type 1; wl vht_features 1; wl dtim 3}
MacX51-SOFTAP-AARDVARK configure -ipaddr 192.168.1.110 -ssid test4360MACap

MacX51-SOFTAP-AARDVARK clone MacX51-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0  -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl country US ; wl assert_type 1; wl vht_features 1; wl dtim 3}
MacX51-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.110 -ssid test4360MACap

MacX51 clone MacX51-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl country US ; wl assert_type 1; wl vht_features 1; wl dtim 3}
MacX51-SOFTAP-BISON configure -ipaddr 192.168.1.110 -ssid test4360MACap

MacX51 clone MacX51-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl country US ; wl assert_type 1; wl vht_features 1; wl dtim 3}
MacX51-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.110 -ssid test4360MACap

#
# AP Section
#

#Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "43311 eth2" \
	-lan_ip 192.168.1.1 \
    -relay "mc70end1" \
    -lanpeer lan \
    -console "mc70end1:40002" \
    -power {npc65 2} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
	-nvram {
		boot_hw_model=E4200
		et0macaddr=00:90:4c:09:00:8a
		macaddr=00:90:4c:09:00:9a
		sb/1/macaddr=00:90:4c:09:10:00
		pci/1/1/macaddr=00:90:4c:09:11:00
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
	    wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
	}

# Clones for 43311
43311 clone 43311-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router
43311 clone 43311-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43311 configure -attngrp G1

#Linksys E4200 4718/4331 Router AP2
UTF::Router AP2 \
    -sta "43312 eth2" \
	-lan_ip 192.168.1.2 \
    -relay "mc70end1" \
    -lanpeer lan \
    -console "mc70end1:40004" \
    -power {npc75 2} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
	-nvram {
		boot_hw_model=E4200
		et0macaddr=00:90:4c:09:00:8b
		macaddr=00:90:4c:09:00:9b
		sb/1/macaddr=00:90:4c:09:12:00
		pci/1/1/macaddr=00:90:4c:09:13:00
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
		fw_disable=1
		#Only 1 AP can serve DHCP Addresses
		router_disable=1
		#simultaneous dual-band router with 2 radios
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
	    wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
	}

# Clones for 43312
43312 clone 43312-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router
43312 clone 43312-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43312 configure -attngrp G2


#Netgear WNDR4500 Experimental 4706/4331 Router AP2
UTF::Router AP3 \
    -sta "43313 eth2" \
	-lan_ip 192.168.1.2 \
    -relay "mc70end1" \
    -lanpeer lan \
    -console "mc70end1:40008" \
    -power {npc75 2} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
	-nvram {
		#boot_hw_model=wndr4500
		#et0macaddr=00:90:4c:09:00:8b
		#macaddr=00:90:4c:09:00:9b
		#sb/1/macaddr=00:90:4c:09:12:00
		#pci/1/1/macaddr=00:90:4c:09:13:00
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
		fw_disable=1
		#Only 1 AP can serve DHCP Addresses
		router_disable=1
		#simultaneous dual-band router with 2 radios
		wl0_radio=0
		wl1_radio=1
		wl1_nbw_cap=0
        wl_msglevel=0x101
		wl0_ssid=test43313-ant0
		wl1_ssid=test43313-ant1
		wl0_channel=1
		wl1_channel=1
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	    wl1_obss_coex=0
	    wandevs=et0
        {lan_ifnames=vlan1 eth1 eth2}
	}

# Clones for 43312
43313 clone 43313-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router
43313 clone 43313-EXT -brand linux-internal-router

# Used for RvRFastSweep.test
43313 configure -attngrp G2

#### ADD UTF::Q for this rig
#####
UTF::Q mc70

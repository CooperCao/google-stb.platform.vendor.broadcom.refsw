# -*-tcl-*-
#
# Testbed configuration file for Rodney Baba md02testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/md02b"

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
UTF::Power::Synaccess npc206 -lan_ip 192.168.1.206 -rev 1
UTF::Power::Synaccess npc207 -lan_ip 192.168.1.207 -rev 1
UTF::Power::Synaccess npc208 -lan_ip 192.168.1.208 -rev 1
UTF::Power::Synaccess npc209 -lan_ip 192.168.1.209 -rev 1
UTF::Power::Synaccess web211 -lan_ip 192.168.1.211
UTF::Power::Synaccess npc210 -lan_ip 192.168.1.210 -rev 1
UTF::Power::WebRelay  web212 -lan_ip 192.168.1.212


# Attenuator - Aeroflex
# power cycler is 192.168.1.206/npc206 port 2
# console port on 192.168.1.206/2002 or md02end1:40011
#uses udp as that is the way to share the AF simultaneously across a split rig
UTF::Aeroflex af -lan_ip 192.168.1.200:20000/udp \
    -relay "lan" \
    -group {
	    G3 {7 8 9}
	    G4 {10 11 12}
	    }
	    G3 configure -default 0
	    G4 configure -default 0
	    

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G3 attn default
    G4 attn default
    
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360STA MacX238 MacX87} {
	    UTF::Try "$S Down" {
		    $S wl down
	    }
	    $S deinit
    }
    # unset S so it doesn't interfere
    unset S

    # Now APs
    foreach S {4708/4331 4708/4360} {
		catch {$S apshell ifconfig [$S cget -device] down}
    }
    # unset S so it doesn't interfere
    unset S
    
  # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}


# Define Sniffer
# FC15 Linux PC
UTF::Sniffer md02tst8 -sta {4360SNF1 eth0} \
    -power {npc210 2} \
    -console {md02end1:40019} \
    -brand "linux-internal-wl" \
    -tag AARDVARK_BRANCH_6_30 \
    -preinstall_hook {{%S dmesg -n 7}} \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl country US/0 ; wl assert_type 1;:
    }


# UTF Endpoint1 FC15 - Traffic generators (no wireless cards)
# console is md02end1:40010
# power is npc206 1
UTF::Linux md02end1 \
    -sta {lan p1p1} \
    -power {npc206 1}

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
# experimental endpoing to see if F11 vs. F15 is cause of TP variations on 4708/4360
UTF::Linux md02end2 \
    -sta {lan1 eth1} \

    
           
# FC15 Linux PC
UTF::Linux md02tst6 -sta {4360STA eth0} \
    -power {npc210 1} \
    -console {md02end1:40018} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
 	-type obj-debug-p2p-mchan \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 4M  -custom 1 -perfonly 1 \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl mimo_bw_cap 1; \
        wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

# Clones
4360STA clone 4360STA-BISON -tag BISON_BRANCH_7_10
4360STA clone 4360STA-AARDVARK -tag AARDVARK_BRANCH_6_30
4360STA clone 4360STA-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}
4360STA clone 4360STA-AARDVARK-RAMP -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	-post_assoc_hook {UTF::Sleep 25}
4360STA clone 436STA-AARDVARK01T -tag AARDVARK01T_BRANCH_6_37 -custom 1
4360STA clone 4360STA-BISON -tag BISON_BRANCH_7_10 -custom 1
4360STA clone 4360STA-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
4360STA clone 4360STA-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
4360STA clone 4360STA-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
4360STA clone 4360STA-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4360STA clone 4360STA-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"
4360STA clone 4360STA-RUBY -tag RUBY_BRANCH_6_20
4360STA clone 4360STA-KIRIN -tag KIRIN_BRANCH_5_100
4360STA clone 4360STA-KIRIN-REL -tag KIRIN_REL_5_100_*
4360STA clone 4360STA-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360STA clone 4360STA-BASS -tag BASS_BRANCH_5_60   
4360STA clone 4360STA-BASS-REL -tag BASS_REL_5_60_???   

# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX238power -button {web212 1}
UTF::MacOS md02tst7 -sta {MacX238 en0} \
	-power {MacX238power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1 -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX238 clone MacX238nonly
MacX238nonly clone MacX238nonly-BISON -tag BISON_BRANCH_7_10
MacX238nonly clone MacX238nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX238nonly clone MacX238nonly-AARDVARK-REL-TAG -tag  AARDVARK_REL_6_30_223_*
MacX238nonly clone MacX238nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 
MacX238nonly clone MacX238nonly-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*

# Clones
MacX238 clone MacX238 -nochannels 1 -custom 1
MacX238 clone MacX238-BISON -tag BISON_BRANCH_7_10
MacX238 clone MacX238-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX238 clone MacX238-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-post_assoc_hook {{wl msglevel +ps}}
MacX238 clone MacX238-AARDVARK-RAMP -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	-post_assoc_hook {UTF::Sleep 25}
MacX238 clone MacX2381-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX238 clone MacX2381-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX238 clone MacX238-AARDVARK-REL-TAG -tag  AARDVARK_REL_6_30_223_*
MacX238 clone MacX238-AARDVARK-REL-TAG-EXT -tag BIS120RC4_REL_7_15_* 
MacX238 clone MacX238-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX238 clone MacX238-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX238 clone MacX238-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX238 clone MacX238-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX238 clone MacX238-RUBY -tag RUBY_BRANCH_6_20
MacX238 clone MacX238-KIRIN -tag KIRIN_BRANCH_5_100
MacX238 clone MacX238-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX238 clone MacX238-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX238 clone MacX238-BASS -tag BASS_BRANCH_5_60   
MacX238 clone MacX238-BASS-REL -tag BASS_REL_5_60_???

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX238 clone MacX238-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-TOT configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP

MacX238 clone MacX238-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-AARDVARK configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP
    
MacX238-SOFTAP-AARDVARK clone MacX238-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP

MacX238 clone MacX238-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-BISON configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP

MacX238-SOFTAP-AARDVARK clone MacX238-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP

# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX87power -button {web212 2}
UTF::MacOS md02kvm1 -sta {MacX87 en2} \
	-power {MacX87power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL  ; wl assert_type 1 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1  -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX87 clone MacX87nonly
MacX87nonly clone MacX87nonly-BISON -tag BISON_BRANCH_7_10
MacX87nonly clone MacX87nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX87nonly clone MacX87nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*  
MacX87nonly clone MacX87nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 
MacX87nonly clone MacX87nonly-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1

# Clones
MacX87 clone MacX87 -nochannels 1 -custom 1
MacX87 clone MacX87-BISON -tag BISON_BRANCH_7_10
MacX87 clone MacX87-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX87 clone MacX87-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-post_assoc_hook {{wl msglevel +ps}}
MacX87 clone MacX87-AARDVARK-RAMP -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
	-post_assoc_hook {UTF::Sleep 25}
MacX87 clone MacX871-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX87 clone MacX871-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX87 clone MacX87-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*  
MacX87 clone MacX87-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*  
MacX87 clone MacX87-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX87 clone MacX87-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX87 clone MacX87-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX87 clone MacX87-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX87 clone MacX87-RUBY -tag RUBY_BRANCH_6_20
MacX87 clone MacX87-KIRIN -tag KIRIN_BRANCH_5_100
MacX87 clone MacX87-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX87 clone MacX87-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX87 clone MacX87-BASS -tag BASS_BRANCH_5_60   
MacX87 clone MacX87-BASS-REL -tag BASS_REL_5_60_???

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX87 clone MacX87-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-TOT configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP

MacX87 clone MacX87-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-AARDVARK configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP
    
MacX87-SOFTAP-AARDVARK clone MacX87-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP

MacX87 clone MacX87-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-BISON configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP

MacX87-SOFTAP-AARDVARK clone MacX87-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP


#
#
# AP Section

# FC15 Linux PC
UTF::Linux md02kvm2 -sta {4360SOFTAP eth0} \
    -power {npc208 1} \
    -console {md02end1:40014} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G} \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

4360SOFTAP configure -ipaddr 192.168.1.98 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

# Clones for md02tst1
4360SOFTAP clone 4360SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP clone 4360SOFTAP-BISON -tag BISON_BRANCH_7_10


# softap settting for AARDVARK
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}
# softap setting for BISON
4360SOFTAP-BISON configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}
	
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -brand "linux-internal-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"


# softap settting for AARDVARK
4360SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-TAG-EXT configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-10 configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-10-EXT configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP clone 4360SOFTAP-RUBY -tag RUBY_BRANCH_6_20
4360SOFTAP clone 4360SOFTAP-KIRIN -tag KIRIN_BRANCH_5_100
4360SOFTAP clone 4360SOFTAP-KIRIN-REL -tag KIRIN_REL_5_100_*
4360SOFTAP clone 4360SOFTAP-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360SOFTAP clone 4360SOFTAP-BASS -tag BASS_BRANCH_5_60   
4360SOFTAP clone 4360SOFTAP-BASS-REL -tag BASS_REL_5_60_???


# Real AP
UTF::Router 4708 -sta {4708/4360 eth2} \
    -relay lan \
    -lan_ip 192.168.1.1 \
    -power {npc208 2} \
    -lanpeer lan \
    -console "m02end1:40015" \
    -tag "BISON_BRANCH_7_10" \
    -brand "linux-2.6.36-arm-internal-router" \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -nvram {
	lan_ipaddr=192.168.1.1
	lan1_ipaddr=192.168.2.1
	watchdog=2000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=47081/4331
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=47081/4360
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
	fw_disable=1
	#Only 1 AP can serve DHCP Addresses
	#router_disable=1
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1 \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

# Clone for 4331 on 2G
4708/4360 clone 4708/4331 -sta {4708/4331 eth1} \
    -channelsweep {-band b} \
    -datarate {-b 400M -i 0.5 -frameburst 1} \
    -noradio_pwrsave 0

# Used for RvRFastSweep.test
4708/4360 configure -attngrp G4
4708/4331 configure -attngrp G4

        
    
#### ADD UTF::Q for this rig
#####
UTF::Q md02b

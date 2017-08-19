# -*-tcl-*-
#
# Testbed configuration file for Frank Fang md11testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/md11"

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
UTF::Power::Synaccess npc45 -lan_ip 192.168.1.45 -rev 1
UTF::Power::Synaccess npc55 -lan_ip 192.168.1.55 -rev 1
UTF::Power::WebRelay web46 -lan_ip 192.168.1.46

# Attenuator - Aeroflex
# power cycler is 192.168.1.55/npc11 port 2
#uses udp as that is the way to share the AF simultaneously across a split rig
UTF::Aeroflex af -lan_ip 192.168.1.200:20000/udp \
    -relay "lan" \
    -group {
	   	G1 {1 2 3 4}
	    G2 {5 6 7 8 9}
	    }
	    G1 configure -default 0
	    G2 configure -default 0



# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default
    G2 attn default    


    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP 4360STA MacX87 MacX14 MacX238 MacX51} {
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


# UTF Endpoint1 FC19 - Traffic generators (no wireless cards)
# console is md11end1:40010
UTF::Linux md11end1 \
    -sta {lan p1p1} \
    -power {npc45 1}



# FC19 Linux PC
UTF::Linux md11tst2 -sta {4360SOFTAP enp1s0} \
    -power {npc55 1} \
    -console {md11end1:40012} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
    }

4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
	}

# Clones for md11tst1
4360SOFTAP clone 4360SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
4360SOFTAP clone 4360SOFTAP-BISON -tag BISON_BRANCH_7_10
4360SOFTAP clone 4360SOFTAP-BISON15 -tag BIS120RC4_BRANCH_7_15
4360SOFTAP clone 4360SOFTAP-BISON16 -tag BIS120RC4PHY_BRANCH_7_16
4360SOFTAP clone 4360SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*

# softap settting for AARDVARK
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
	}
	
4360SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
	}	
# softap settting for BISON
4360SOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
	}


#softap settting for BISON
4360SOFTAP-BISON15 configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
        }

#softap settting for BISON
4360SOFTAP-BISON16 configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
        }
        
4360SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
	}
	


# FC19 Linux PC
UTF::Linux md11tst1 -sta {4360STA enp1s0} \
    -power {npc45 2} \
    -console {md11end1:40011} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-tcpwindow 4M  -custom 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 \
		-datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl mimo_bw_cap 1; \
        wl obss_coex 0; wl vht_features 3; wl country US ;:
    }

# Clones
4360STA clone 4360STA -nochannels 1 -custom 1
4360STA clone 4360STA-AARDVARK -tag AARDVARK_BRANCH_6_30
4360STA clone 4360STA-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}
4360STA clone 4360STA-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
4360STA clone 4360STA-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
4360STA clone 4360STA-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4360STA clone 4360STA-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"
4360STA clone 4360STA-BISON -tag BISON_BRANCH_7_10
4360STA clone 4360STA-BISON15 -tag BIS120RC4_BRANCH_7_15
4360STA clone 4360STA-BISON16 -tag BIS120RC4PHY_BRANCH_7_16
4360STA clone 4360STA-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
4360STA clone 4360STA-RUBY -tag RUBY_BRANCH_6_20
4360STA clone 4360STA-KIRIN -tag KIRIN_BRANCH_5_100
4360STA clone 4360STA-KIRIN-REL -tag KIRIN_REL_5_100_*
4360STA clone 4360STA-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360STA clone 4360STA-BASS -tag BASS_BRANCH_5_60   
4360STA clone 4360STA-BASS-REL -tag BASS_REL_5_60_???   


# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX87power -button {web46 1}
UTF::MacOS md11tst3 -sta {MacX87 en0 MacX87-awdl awdl0} \
	-power {MacX87power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl roam_off 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl roam_off 1} {%S wl country}} \
	-brand  "macos-internal-wl-syr" \
	-type Debug_10_10 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1 -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX87 clone MacX87nonly
MacX87nonly clone MacX87nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX87nonly clone MacX87nonly-BISON -tag BISON_BRANCH_7_10
MacX87nonly clone MacX87nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
MacX87nonly clone MacX87nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 


# Clones
MacX87 clone MacX87-TOT -nochannels 1 -custom 1 
MacX87 clone MacX87-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX87 clone MacX87-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*	
MacX87 clone MacX871-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX87 clone MacX871-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX87 clone MacX87-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX87 clone MacX87-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX87 clone MacX87-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX87 clone MacX87-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX87 clone MacX87-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX87 clone MacX87-BISON15 -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX87 clone MacX87-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 -custom 1
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
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3;}
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

MacX87 clone MacX87-SOFTAP-BISON15 -tag BIS120RC4_BRANCH_7_15 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-BISON15 configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP


MacX87 clone MacX87-SOFTAP-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-BISON16 configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP

MacX87 clone MacX87-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX87-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX87AP

# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX14power -button {web46 2}
UTF::MacOS md11tst4 -sta {MacX14 en0} \
	-power {MacX14power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl roam_off 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl roam_off 1} {%S wl country}} \
	-brand  "macos-internal-wl-syr" \
	-type Debug_10_10 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1 -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX14 clone MacX14nonly
MacX14nonly clone MacX14nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX14nonly clone MacX14nonly-BISON -tag BISON_BRANCH_7_10
MacX14nonly clone MacX14nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
MacX14nonly clone MacX14nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 


# Clones
MacX14 clone MacX14-TOT -nochannels 1 -custom 1
MacX14 clone MacX14-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX14 clone MacX14-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*	
MacX14 clone MacX141-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX14 clone MacX141-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX14 clone MacX14-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX14 clone MacX14-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX14 clone MacX14-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX14 clone MacX14-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX14 clone MacX14-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX14 clone MacX14-BISON15 -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX14 clone MacX14-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 -custom 1
MacX14 clone MacX14-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX14 clone MacX14-RUBY -tag RUBY_BRANCH_6_20
MacX14 clone MacX14-KIRIN -tag KIRIN_BRANCH_5_100
MacX14 clone MacX14-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX14 clone MacX14-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX14 clone MacX14-BASS -tag BASS_BRANCH_5_60   
MacX14 clone MacX14-BASS-REL -tag BASS_REL_5_60_???

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX14 clone MacX14-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3;}
MacX14-SOFTAP-TOT configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

MacX14 clone MacX14-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-AARDVARK configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP
    
MacX14-SOFTAP-AARDVARK clone MacX14-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

MacX14 clone MacX14-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-BISON configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

MacX14 clone MacX14-SOFTAP-BISON15 -tag BIS120RC4_BRANCH_7_15 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-BISON15 configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP


MacX14 clone MacX14-SOFTAP-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-BISON16 configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

MacX14 clone MacX14-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX238power -button {web46 3}
UTF::MacOS md11tst5 -sta {MacX238 en0 MacX238-awdl awdl0} \
	-power {MacX238power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0} {%S wl roam_off 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl roam_off 1} {%S wl country}} \
	-brand  "macos-internal-wl-syr" \
	-type Debug_10_10 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1 -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX238 clone MacX238nonly
MacX238nonly clone MacX238nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX238nonly clone MacX238nonly-BISON -tag BISON_BRANCH_7_10
MacX238nonly clone MacX238nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
MacX238nonly clone MacX238nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 


# Clones
MacX238 clone MacX238-TOT -nochannels 1 -custom 1
MacX238 clone MacX238-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX238 clone MacX238-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*	
MacX238 clone MacX2381-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX238 clone MacX2381-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX238 clone MacX238-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX238 clone MacX238-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX238 clone MacX238-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX238 clone MacX238-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX238 clone MacX238-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX238 clone MacX238-BISON15 -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX238 clone MacX238-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 -custom 1
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
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3;}
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

MacX238 clone MacX238-SOFTAP-BISON15 -tag BIS120RC4_BRANCH_7_15 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-BISON15 configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP


MacX238 clone MacX238-SOFTAP-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-BISON16 configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP

MacX238 clone MacX238-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX238-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX238AP


# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
# This MBA is for use as a "Canary" in OS updates and isn't used in daily testing
# It also temporarily has an X238 chip inside
# Also it was used for AWDL experiments
UTF::Power::Laptop MacX51power -button {web46 4}
UTF::MacOS md11tst6 -sta {MacX51 en0 MacX238-awdl awdl0} \
	-power {MacX51power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
	-brand  "macos-internal-wl-syr" \
	-type Debug_10_10 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1 -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX51 clone MacX51nonly
MacX51nonly clone MacX51nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX51nonly clone MacX51nonly-BISON -tag BISON_BRANCH_7_10
MacX51nonly clone MacX51nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
MacX51nonly clone MacX51nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 


# Clones
MacX51 clone MacX51-TOT -nochannels 1 -custom 1 \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ALL ; wl ol_disable 1; wl assert_type 1; wl oppr_roam_off 1 }
MacX51 clone MacX51-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX51 clone MacX51-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-post_assoc_hook {{wl msglevel +ps}}
MacX51 clone MacX51-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*	
MacX51 clone MacX511-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX51 clone MacX511-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX51 clone MacX51-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX51 clone MacX51-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX51 clone MacX51-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX51 clone MacX51-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX51 clone MacX51-BISON -tag BISON_BRANCH_7_10 -custom 1
MacX51 clone MacX51-BISON15 -tag BIS120RC4_BRANCH_7_15 -custom 1
MacX51 clone MacX51-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 -custom 1
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
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3;}
MacX51-SOFTAP-TOT configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP

MacX51 clone MacX51-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-AARDVARK configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP
    
MacX51-SOFTAP-AARDVARK clone MacX51-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP

MacX51 clone MacX51-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-BISON configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP

MacX51 clone MacX51-SOFTAP-BISON15 -tag BIS120RC4_BRANCH_7_15 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-BISON15 configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP


MacX51 clone MacX51-SOFTAP-BISON16 -tag BIS120RC4PHY_BRANCH_7_16 \
        -apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-BISON16 configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP

MacX51 clone MacX51-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP

#### ADD UTF::Q for this rig
#####
UTF::Q md11


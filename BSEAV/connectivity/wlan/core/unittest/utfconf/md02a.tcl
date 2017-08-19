# -*-tcl-*-
#
# Testbed configuration file for Frank Fang md02testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/md02a"

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
UTF::Power::WebRelay web211 -lan_ip 192.168.1.211
UTF::Power::Synaccess npc210 -lan_ip 192.168.1.210 -rev 1
UTF::Power::WebRelay  web212 -lan_ip 192.168.1.212


# Attenuator - Aeroflex
# power cycler is 192.168.1.206/npc206 port 2
# console port on 192.168.1.206/2002 or md02end1:40011
UTF::Aeroflex af -lan_ip 192.168.1.200 \
    -relay "lan" \
    -group {
	    G1 {1 2 3}
	    G2 {4 5 6}
	    }
	    G1 configure -default 0
	    G2 configure -default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn default
    G2 attn default

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP 4360SOFTAP1 4360STA MacX51 MacX14} {
	    UTF::Try "$S Down" {
		    $S wl down
	    }
    
	    $S deinit
    }
    # unset S so it doesn't interfere
    unset S

    # X14 special treatment due to not running Right OS/System that support dynamic card reboot per Irene Shen
    UTF::Try "MacX14 Power Cycle" {
	    MacX14 power cycle
    }
    
	# delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}

# Define Sniffer
# FC15 Linux PC
UTF::Sniffer md02snf1 -sta {4360SNF1 eth0} \
    -power {npc209 2} \
    -console {md02end1:40017} \
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



# FC15 Linux PC
UTF::Linux md02tst1 -sta {4360SOFTAP eth0} \
    -power {npc207 1} \
    -console {md02end1:40012} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

# Clones for md02tst1
4360SOFTAP clone 4360SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP clone 4360SOFTAP-BISON -tag BISON_BRANCH_7_10

# softap settting for AARDVARK
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}
# softap settting for BISON
4360SOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -brand "linux-internal-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4360SOFTAP clone 4360SOFTAP-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"


# softap settting for AARDVARK
4360SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-TAG-EXT configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-10 configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP-AARDVARK-REL-10-EXT configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP clone 4360SOFTAP-RUBY -tag RUBY_BRANCH_6_20
4360SOFTAP clone 4360SOFTAP-KIRIN -tag KIRIN_BRANCH_5_100
4360SOFTAP clone 4360SOFTAP-KIRIN-REL -tag KIRIN_REL_5_100_*
4360SOFTAP clone 4360SOFTAP-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360SOFTAP clone 4360SOFTAP-BASS -tag BASS_BRANCH_5_60   
4360SOFTAP clone 4360SOFTAP-BASS-REL -tag BASS_REL_5_60_???


# FC15 Linux PC
UTF::Linux md02tst2 -sta {4360SOFTAP1 eth0} \
    -power {npc207 2} \
    -console {md02end1:40013} \
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

4360SOFTAP1 configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

# Clones for md02tst1
4360SOFTAP1 clone 4360SOFTAP1-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP1 clone 4360SOFTAP1-BISON -tag BISON_BRANCH_7_10


# softap settting for AARDVARK
4360SOFTAP1-AARDVARK configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}
# softap settting for BISON
4360SOFTAP1-BISON configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}
4360SOFTAP1 clone 4360SOFTAP1-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -brand "linux-internal-wl"
4360SOFTAP1 clone 4360SOFTAP1-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
4360SOFTAP1 clone 4360SOFTAP1-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4360SOFTAP1 clone 4360SOFTAP1-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"


# softap settting for AARDVARK
4360SOFTAP1-AARDVARK-REL-TAG configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP1-AARDVARK-REL-TAG-EXT configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP1-AARDVARK-REL-10 configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP1-AARDVARK-REL-10-EXT configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
4360SOFTAP1 clone 4360SOFTAP1-BISON -tag BISON_BRANCH_7_10
4360SOFTAP1 clone 4360SOFTAP1-RUBY -tag RUBY_BRANCH_6_20
4360SOFTAP1 clone 4360SOFTAP1-KIRIN -tag KIRIN_BRANCH_5_100
4360SOFTAP1 clone 4360SOFTAP1-KIRIN-REL -tag KIRIN_REL_5_100_*
4360SOFTAP1 clone 4360SOFTAP1-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360SOFTAP1 clone 4360SOFTAP1-BASS -tag BASS_BRANCH_5_60   
4360SOFTAP1 clone 4360SOFTAP1-BASS-REL -tag BASS_REL_5_60_???

       
# FC15 Linux PC
UTF::Linux md02tst3 -sta {4360STA eth0} \
    -power {npc209 1} \
    -console {md02end1:40016} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
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
4360STA clone 4360STA-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
4360STA clone 4360STA-RUBY -tag RUBY_BRANCH_6_20
4360STA clone 4360STA-KIRIN -tag KIRIN_BRANCH_5_100
4360STA clone 4360STA-KIRIN-REL -tag KIRIN_REL_5_100_*
4360STA clone 4360STA-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360STA clone 4360STA-BASS -tag BASS_BRANCH_5_60   
4360STA clone 4360STA-BASS-REL -tag BASS_REL_5_60_???   


# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX51power -button {web211 1}
UTF::MacOS md02tst4 -sta {MacX51 en0} \
	-power {MacX51power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 3640K  -custom 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX51 clone MacX51nonly
MacX51nonly clone MacX51nonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
MacX51nonly clone MacX51nonly-BISON -tag BISON_BRANCH_7_10
MacX51nonly clone MacX51nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
MacX51nonly clone MacX51nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 


# Clones
MacX51 clone MacX51 -custom 1 \
	-wlinitcmds { wl msglevel 0x101  ; wl msglevel +wsec; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country ALL ; wl assert_type 1; wl oppr_roam_off 1 }
MacX51 clone MacX51-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX51 clone MacX51-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} } \
	-post_assoc_hook {{wl msglevel +ps}}
MacX51 clone MacX51-AARDVARK-227 -tag BU4360B1_REL_6_30_227_*	
MacX51 clone MacX511-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX51 clone MacX511-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX51 clone MacX51-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX51 clone MacX51-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX51 clone MacX51-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX51 clone MacX51-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX51 clone MacX51-BISON -tag BISON_BRANCH_7_10
MacX51 clone MacX51-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
MacX51 clone MacX51-RUBY -tag RUBY_BRANCH_6_20
MacX51 clone MacX51-KIRIN -tag KIRIN_BRANCH_5_100
MacX51 clone MacX51-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX51 clone MacX51-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX51 clone MacX51-BASS -tag BASS_BRANCH_5_60   
MacX51 clone MacX51-BASS-REL -tag BASS_REL_5_60_???

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX51 clone MacX51-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3; wl ol_disable 1}
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

MacX51 clone MacX51-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.112 -ssid test4360MacX51AP


# 4360 MacBook Air
# Actual shipping 10.8.4 ML
# with B1 4360 X51
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX14power -button {web211 2}
UTF::MacOS md02tst5 -sta {MacX14 en0} \
	-power {MacX14power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 3640K  -custom 1 -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX14 clone MacX14nonly
MacX14nonly clone MacX14nonly-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX14nonly clone MacX14nonly-BISON -tag BISON_BRANCH_7_10
MacX14nonly clone MacX14nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
MacX14nonly clone MacX14nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 


# Clones
MacX14 clone MacX14 -custom 1
MacX14 clone MacX14-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX14 clone MacX14-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}
MacX14 clone MacX141-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX14 clone MacX141-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX14 clone MacX14-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX14 clone MacX14-AARDVARK-REL-TAG53 -tag AARDVARK_REL_6_30_223_53
MacX14 clone MacX14-AARDVARK-REL-TAG54 -tag AARDVARK_REL_6_30_223_54
MacX14 clone MacX14-AARDVARK-REL-TAG55 -tag AARDVARK_REL_6_30_223_5{56}
MacX14 clone MacX14-AARDVARK-REL-TAG56 -tag AARDVARK_REL_6_30_223_5{56}
MacX14 clone MacX14-AARDVARK-REL-TAG57 -tag AARDVARK_REL_6_30_223_57
MacX14 clone MacX14-AARDVARK-REL-TAG58 -tag AARDVARK_REL_6_30_223_58
MacX14 clone MacX14-AARDVARK-REL-TAG59 -tag AARDVARK_REL_6_30_223_59
MacX14 clone MacX14-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX14 clone MacX14-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX14 clone MacX14-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX14 clone MacX14-BISON -tag BISON_BRANCH_7_10
MacX14 clone MacX14-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
MacX14 clone MacX14-RUBY -tag RUBY_BRANCH_6_20
MacX14 clone MacX14-KIRIN -tag KIRIN_BRANCH_5_100
MacX14 clone MacX14-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX14 clone MacX14-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX14 clone MacX14-BASS -tag BASS_BRANCH_5_60   
MacX14 clone MacX14-BASS-REL -tag BASS_REL_5_60_???

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX14 clone MacX14-SOFTAP-TOT \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-TOT configure -ipaddr 192.168.1.110 -ssid test436MacX14AP

MacX14 clone MacX14-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-AARDVARK configure -ipaddr 192.168.1.110 -ssid test436MacX14AP
    
MacX14-SOFTAP-AARDVARK clone MacX14-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.110 -ssid test436MacX14AP

MacX14 clone MacX14-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-BISON configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

MacX14 clone MacX14-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX14-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX14AP

#### ADD UTF::Q for this rig
#####
UTF::Q md02a

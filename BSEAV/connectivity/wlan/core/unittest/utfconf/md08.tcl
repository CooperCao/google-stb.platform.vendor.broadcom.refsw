# -*-tcl-*-
#
# Testbed configuration file for Frank Fang md08testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext7/$::env(LOGNAME)/md08"

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
UTF::Power::Synaccess npc110 -lan_ip 192.168.1.110 -rev 1
UTF::Power::Synaccess npc111 -lan_ip 192.168.1.111 -rev 1
UTF::Power::WebRelay web112 -lan_ip 192.168.1.112

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #


    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP 4360STA MacX51} {
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


# UTF Endpoint1 FC15 - Traffic generators (no wireless cards)
# console is md08end1:40010
UTF::Linux md08end1 \
    -sta {lan p16p1} \
    -power {npc110 1}



# FC15 Linux PC
UTF::Linux md08tst1 -sta {4360SOFTAP eth0 4360SOFTAP-awdl wl0.1} \
    -power {npc110 2} \
    -console {md08end1:40011} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -type obj-debug-p2p-mchan \
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

# Clones for md08tst1
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
	


# FC15 Linux PC
UTF::Linux md08tst2 -sta {4360STA eth0 4360STA-awdl wl0.1} \
    -power {npc111 1} \
    -console {md08end1:40012} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -type obj-debug-p2p-mchan \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-tcpwindow 4M  -custom 1 \
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
UTF::Power::Laptop MacX51power -button {web112 1}
UTF::MacOS md08tst3 -sta {MacX51 en0} \
	-power {MacX51power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country US ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1} {%S wl country}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-embeddedimage 4352 \
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
UTF::Q md08


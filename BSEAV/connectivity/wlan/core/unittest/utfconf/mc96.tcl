# -*-tcl-*-
#
# Testbed configuration file for Frank Fang mc96testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc96"

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
UTF::Power::Synaccess npc241 -lan_ip 192.168.1.241 -rev 1
UTF::Power::Synaccess npc242 -lan_ip 192.168.1.242 -rev 1
UTF::Power::Synaccess npc235 -lan_ip 192.168.1.235 -rev 1
UTF::Power::Synaccess npc236 -lan_ip 192.168.1.236 -rev 1
UTF::Power::WebRelay  web237 -lan_ip 192.168.1.237
UTF::Power::Synaccess npc245 -lan_ip 192.168.1.245 -rev 1
UTF::Power::Synaccess npc246 -lan_ip 192.168.1.246 -rev 1
UTF::Power::WebRelay  web247 -lan_ip 192.168.1.247


# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 192.168.1.200 \
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

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP 4360SOFTAP1 4360STA MacX52c MacX51 MacX29c 4360WB} {
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
UTF::Sniffer mc96snf1 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc242 1} \
        -power_button {auto} \
        -console "mc96end1:40011"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
# console is mc96end1:40010
# power is npc241 1
UTF::Linux mc96end1 \
    -sta {lan p1p1} \
    -power {npc241 1}

# Define Sniffer (used in experimental BEAM form tests and some RvR)
# FC15 Linux PC
UTF::Sniffer mc96snf1 -sta {4360SNF1 eth0} \
    -power {npc241 2} \
    -console {mc96end1:40011} \
    -brand "linux-internal-wl" \
    -tag AARDVARK_BRANCH_6_30 \
    -preinstall_hook {{%S dmesg -n 7}} \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl country US/0 ; wl assert_type 1;:
    }


# FC15 Linux PC
UTF::Linux mc96tst1 -sta {4360SOFTAP eth0} \
    -power {npc242 1} \
    -console {mc96end1:40012} \
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

# Clones for mc96tst1
4360SOFTAP clone 4360SOFTAP-TOT -tag trunk
4360SOFTAP clone 4360SOFTAP-BISON -tag BISON_BRANCH_7_10


# softap settting for AARDVARK
4360SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
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
	
# FC15 Linux PC
UTF::Linux mc96tst2 -sta {4360SOFTAP1 eth0} \
    -power {npc242 2} \


# FC15 Linux PC
UTF::Linux mc96tst2 -sta {4360SOFTAP1 eth0} \
    -power {npc242 2} \
    -console {mc96end1:40013} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

4360SOFTAP1 configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

# Clones for mc96tst1
4360SOFTAP1 clone 4360SOFTAP1-TOT -tag trunk
4360SOFTAP1 clone 4360SOFTAP1-BISON -tag BISON_BRANCH_7_10

# softap settting for AARDVARK
4360SOFTAP1-TOT configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap1 -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down ; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

# softap settting for BISON
4360SOFTAP1-BISON configure -ipaddr 192.168.1.98 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

4360SOFTAP1 clone 4360SOFTAP1-ACT1 -tag trunk
4360SOFTAP1 clone 4360SOFTAP1-ACT2 -tag DINGO_BRANCH_9_10
4360SOFTAP1 clone 4360SOFTAP1-ACT3 -tag BIS120RC4PHY_BRANCH_7_16
4360SOFTAP1 clone 4360SOFTAP1-ACT4 -tag BIS120RC4_BRANCH_7_15
4360SOFTAP1 clone 4360SOFTAP1-ACT5 -tag EAGLE_BRANCH_10_10

       
# FC15 Linux PC
UTF::Linux mc96tst3 -sta {4360STA eth0} \
    -power {npc235 1} \
    -console {mc96end1:40014} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 4M  -custom 1 -perfonly 1 \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl mimo_bw_cap 1; \
        wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

# Clones
4360STA clone 4360STA-TOT -tag trunk
    
# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX52cpower -button {web237 1}
UTF::MacOS mc96tst4 -sta {MacX52c en0} \
	-power {MacX52cpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-tag AARDVARK_BRANCH_6_30 \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 3640K  -custom 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX52c clone MacX52c-TOT  -tag trunk

# Clones
MacX52c clone MacX52c-SOFTAP-TOT -tag trunk \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX52c-SOFTAP-TOT configure -ipaddr 192.168.1.110 -ssid test436MacX52cAP
    
# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX51power -button {web237 2}
UTF::MacOS mc96tst5 -sta {MacX51 en0 MacX51-awdl p2p1} \
	-power {MacX51power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-tag AARDVARK_BRANCH_6_30 \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 3640K  -custom 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX51 clone MacX51-TOT -tag trunk

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
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
MacX51-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.111 -ssid test4360MacX51AP

MacX51 clone MacX51-SOFTAP-ACT1 -tag trunk \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-ACT1 configure -ipaddr 192.168.1.111 -ssid MacX51SOFTAPACT1

#MacX51 clone MacX51-SOFTAP-ACT2 -tag DINGO_BRANCH_9_10
MacX51 clone MacX51-SOFTAP-ACT3 -tag BIS120RC4PHY_BRANCH_7_16 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-ACT3 configure -ipaddr 192.168.1.111 -ssid MacX51SOFTAPACT3

MacX51 clone MacX51-SOFTAP-ACT4 -tag BIS120RC4_BRANCH_7_15 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-ACT4 configure -ipaddr 192.168.1.111 -ssid MacX51SOFTAPACT4

MacX51 clone MacX51-SOFTAP-ACT5 -tag EAGLE_BRANCH_10_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX51-SOFTAP-ACT5 configure -ipaddr 192.168.1.111 -ssid MacX51SOFTAPACT5

# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX29cpower -button {web247 1}
UTF::MacOS mc96tst6 -sta {MacX29c en0 MacX29c-awdl p2p1} \
	-power {MacX29cpower} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1; wl oppr_roam_off 1  } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-tag AARDVARK_BRANCH_6_30 \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 3640K  -custom 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX29c clone MacX29c-TOT -tag trunk


# Clones
MacX29c clone MacX29c-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX29c clone MacX29c-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-post_assoc_hook {{%S wl msglevel +PS} {%S wl phy_watchdog 0} {%S wl pm2_sleep_ret 500}}
MacX29c clone MacX29c-AARDVARK-32244 -image /projects/hnd_software_ext6/work/richapur/AARDVARK_TWIG_6_30_223/src/wl/macos/build/Debug_10_9/AirPortBroadcom43XX.kext
MacX29c clone MacX29c1-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX29c clone MacX29c1-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX29c clone MacX29c-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX29c clone MacX29c-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX29c clone MacX29c-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX29c clone MacX29c-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX29c clone MacX29c-BISON -tag BISON_BRANCH_7_10
MacX29c clone MacX29c-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -custom 1
MacX29c clone MacX29c-RUBY -tag RUBY_BRANCH_6_20
MacX29c clone MacX29c-KIRIN -tag KIRIN_BRANCH_5_100
MacX29c clone MacX29c-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX29c clone MacX29c-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX29c clone MacX29c-BASS -tag BASS_BRANCH_5_60   
MacX29c clone MacX29c-BASS-REL -tag BASS_REL_5_60_???
MacX29c clone MacX29c-ACT1 -tag trunk
#MacX29c clone MacX29c-ACT2 -tag DINGO_BRANCH_9_10
MacX29c clone MacX29c-ACT3 -tag BIS120RC4PHY_BRANCH_7_16
MacX29c clone MacX29c-ACT4 -tag BIS120RC4_BRANCH_7_15
MacX29c clone MacX29c-ACT5 -tag EAGLE_BRANCH_10_10

# new clone for doing sofap on this mac  apmode 1 flips the bit when called with a regular softap and softap switches roles and becomes sta
MacX29c clone MacX29c-SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-AARDVARK configure -ipaddr 192.168.1.112 -ssid test4360MacX29cAP
    
MacX29c-SOFTAP-AARDVARK clone MacX29c-SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.112 -ssid test4360MacX29cAP

MacX29c clone MacX29c-SOFTAP-BISON -tag BISON_BRANCH_7_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-BISON configure -ipaddr 192.168.1.112 -ssid test4360MacX29cAP

MacX29c clone MacX29c-SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-BISON-REL-TAG configure -ipaddr 192.168.1.112 -ssid test4360MacX29cAP

MacX29c clone MacX29c-SOFTAP-ACT1 -tag trunk \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-ACT1 configure -ipaddr 192.168.1.112 -ssid MacX29cSOFTAPACT1

MacX29c clone MacX29c-SOFTAP-ACT3 -tag BIS120RC4PHY_BRANCH_7_16 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-ACT3 configure -ipaddr 192.168.1.112 -ssid MacX29cSOFTAPACT3

MacX29c clone MacX29c-SOFTAP-ACT4 -tag BIS120RC4_BRANCH_7_15 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-ACT4 configure -ipaddr 192.168.1.112 -ssid MacX29cSOFTAPACT4

MacX29c clone MacX29c-SOFTAP-ACT5 -tag EAGLE_BRANCH_10_10 \
	-apmode 1 -nochannels 1 -slowassoc 5 -noaes 1 -notkip 1 -nopm1 1 -nopm2 1 -datarate 0 -yart {} -custom {} \
    -wlinitcmds {wl down; wl apsta 0; wl PM 3; wl msglevel +assoc; wl assert_type 1; wl vht_features 3; wl country US; wl dtim 3}
MacX29c-SOFTAP-ACT5 configure -ipaddr 192.168.1.112 -ssid MacX29cSOFTAPACT5

# Windows Blue mc96tst7
# KVM is mc96kvm1
UTF::Cygwin mc96tst7 -sta {4360WB} \
    -power {npc245 2} \
    -osver 864 -kdpath kd.exe \
    -usemodifyos 1 \
   	-brand win8x_internal_wl \
   	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 4M  -custom 1 \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl mimo_bw_cap 1; \
        wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

# Clones
4360WB clone 4360WB-TOT
4360WB clone 4360WB-AARDVARK -tag AARDVARK_BRANCH_6_30 -custom 1
4360WB clone 4360WB-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -custom 1
4360WB clone 4360WB-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
4360WB clone 4360WB-AARDVARK01T -tag AARDVARK01T_BRANCH_6_37 -custom 1
4360WB clone 4360WB-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
4360WB clone 4360WB-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
4360WB clone 4360WB-BISON -tag BISON_BRANCH_7_10 -custom 1
4360WB clone 4360WB-BISON-REL-TAG -tag BIS120RC4_REL_7_15_* -brand win8_internal_wl  -custom 1
4360WB clone 4360WB-RUBY -tag RUBY_BRANCH_6_20
4360WB clone 4360WB-KIRIN -tag KIRIN_BRANCH_5_100
4360WB clone 4360WB-KIRIN-REL -tag KIRIN_REL_5_100_*
4360WB clone 4360WB-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4360WB clone 4360WB-BASS -tag BASS_BRANCH_5_60   
4360WB clone 4360WB-BASS-REL -tag BASS_REL_5_60_???   
4360WB clone 4360WB-ACT1 -tag trunk
#4360WB clone 4360WB-ACT2 -tag DINGO_BRANCH_9_10
#4360WB clone 4360WB-ACT3 -tag BIS120RC4PHY_BRANCH_7_16
4360WB clone 4360WB-ACT4 -tag BIS120RC4_BRANCH_7_15
4360WB clone 4360WB-ACT5 -tag EAGLE_BRANCH_10_10

# # 4360 MacBook Air
# # Needed to get around MAC Power Cycle to Debugger
# UTF::Power::Laptop MacX52c-spower -button {web247 2}
# UTF::MacOS mc96tst8 -sta {MacX52c-s en0} \
# 	-power {MacX52-scpower} \
# 	-power_button {auto} \
# 	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL  ; wl assert_type 1 } \
# 	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
# 	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
# 	-brand  "macos-internal-wl-ml" \
# 	-type Debug_10_8 \
# 	-coreserver AppleCore \
# 	-kextload true \
# 	-tag AARDVARK_BRANCH_6_30 \
# 	-slowassoc 5 \
# 	-channelsweep {-count 15} \
#     -datarate {-b 1.2G -i 0.5 -frameburst 1} \
# 	-nobighammer 1 -tcpwindow 3640K  -custom 1
# 	
# 
# # used when testing against an non-ac router, so we don't want to tet AC rates there
# MacX52c-s clone MacX52c-snonly  -tag AARDVARK_BRANCH_6_30
# MacX52c-snonly clone MacX52c-snonly-AARDVARK -tag AARDVARK_BRANCH_6_30 
# MacX52c-snonly clone MacX52c-snonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* 
# MacX52c-snonly clone MacX52c-snonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_* 
# 
# 
# # Clones
# MacX52c-s clone MacX52c-s-AARDVARK -tag AARDVARK_BRANCH_6_30
# MacX52c-s clone MacX52c-s-AARDVARK-PM2 -tag AARDVARK_BRANCH_6_30 \
# 	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
# 	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl PM 2} {%S wl PM} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}
# MacX52c-s clone MacX52c-s1-AARDVARK -tag AARDVARK_BRANCH_6_28
# MacX52c-s clone MacX52c-s1-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG-S -tag AARD223RC74_REL_6_30_225_19
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG50 -tag AARDVARK_REL_6_30_223_50 \
# 	-image /home/rodneyb/AARDVARK_TWIG_6_30_223/src/wl/macos/build/Debug_10_9/AirPortBroadcom43XX.kext
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG53 -tag AARDVARK_REL_6_30_223_53
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG54 -tag AARDVARK_REL_6_30_223_54
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG55 -tag AARDVARK_REL_6_30_223_5{56}
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG56 -tag AARDVARK_REL_6_30_223_5{56}
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG57 -tag AARDVARK_REL_6_30_223_57
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG58 -tag AARDVARK_REL_6_30_223_58
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG59 -tag AARDVARK_REL_6_30_223_59
# MacX52c-s clone MacX52c-s-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
# MacX52c-s clone MacX52c-s-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
# MacX52c-s clone MacX52c-s-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
# MacX52c-s clone MacX52c-s-RUBY -tag RUBY_BRANCH_6_20
# MacX52c-s clone MacX52c-s-KIRIN -tag KIRIN_BRANCH_5_100
# MacX52c-s clone MacX52c-s-KIRIN-REL -tag KIRIN_REL_5_100_*
# MacX52c-s clone MacX52c-s-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
# MacX52c-s clone MacX52c-s-BASS -tag BASS_BRANCH_5_60   
# MacX52c-s clone MacX52c-s-BASS-REL -tag BASS_REL_5_60_???

#### ADD UTF::Q for this rig
#####
UTF::Q mc96

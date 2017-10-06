# -*-tcl-*-
#
# Testbed configuration file for Frank Fang MC93testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux HSIC]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc93"

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
UTF::Power::Synaccess npc240 -lan_ip 192.168.1.240 -rev 1
UTF::Power::Synaccess npc241 -lan_ip 192.168.1.241 -rev 1
UTF::Power::Synaccess npc242 -lan_ip 192.168.1.242 -rev 1
UTF::Power::Synaccess npc243 -lan_ip 192.168.1.243 -rev 1
UTF::Power::Synaccess npc244 -lan_ip 192.168.1.244 -rev 1
UTF::Power::Synaccess npc245 -lan_ip 192.168.1.245 -rev 1
UTF::Power::Synaccess npc246 -lan_ip 192.168.1.246 -rev 1
UTF::Power::WebRelay  web247 -lan_ip 192.168.1.247
UTF::Power::WebRelay  web248 -lan_ip 192.168.1.248


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
    catch {4708/4360 restart wl0_radio=0}
    catch {4708/4331 restart wl1_radio=0}
    catch {47081/4360 restart wl0_radio=0}
    catch {47081/4331 restart wl1_radio=0}
 

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360STA 4360SOFTAP 4350SOFTAP X52cSOFTAP MacX4360 4360STA MacX14} {
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
UTF::Sniffer mc93snf1 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc242 1} \
        -power_button {auto} \
        -console "mc93end1:40014"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc93end1 \
    -sta {lan p1p1} 

# FC15 Linux PC
UTF::Linux mc93tst1 -sta {4360SOFTAP enp1s0} \
    -power {npc243 1} \
    -console {mc93end1:40016} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl assert_type 1;:
    }

4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl country US ; wl vht_features 3; wl assert_type 1;:
	}

# Clones for mc93tst1
4360SOFTAP clone 4360SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30
4360SOFTAP clone 4360SOFTAP-BISON -tag BISON_BRANCH_7_10

# softap settting for AARDVARK
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0;  wl country US ; wl vht_features 3; wl assert_type 1;:
	}

# softap settting for BISON
4360SOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl down; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0;  wl country US ; wl vht_features 3; wl assert_type 1;:
	}
	
4360SOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap
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

	    

# FC15 Linux PC
UTF::Linux mc93tst2 -sta {4350SOFTAP enp1s0} \
    -power {npc243 2} \
    -console {mc93end1:40017} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

4350SOFTAP configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}


# Clones for mc93tst2
4350SOFTAP clone 4350SOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 -brand "linux-internal-wl"
4350SOFTAP clone 4350SOFTAP-BISON -tag BISON_BRANCH_7_10 -brand "linux-internal-wl"
4350SOFTAP clone 4350SOFTAP-BISON-P2P -tag BISON_BRANCH_7_10 -brand "linux-internal-wl" -type obj-debug-p2p-mchan 
4350SOFTAP clone 4350SOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -brand "linux-internal-wl"
4350SOFTAP clone 4350SOFTAP-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_* -brand "linux-external-wl"
4350SOFTAP clone 4350SOFTAP-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
4350SOFTAP clone 4350SOFTAP-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"


# softap settting for AARDVARK
4350SOFTAP-AARDVARK configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap
4350SOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap
4350SOFTAP-AARDVARK-REL-TAG-EXT configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap
4350SOFTAP-AARDVARK-REL-10 configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap
4350SOFTAP-AARDVARK-REL-10-EXT configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap
4350SOFTAP-BISON  configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap
4350SOFTAP clone 4350SOFTAP-RUBY -tag RUBY_BRANCH_6_20
4350SOFTAP clone 4350SOFTAP-KIRIN -tag KIRIN_BRANCH_5_100
4350SOFTAP clone 4350SOFTAP-KIRIN-REL -tag KIRIN_REL_5_100_*
4350SOFTAP clone 4350SOFTAP-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
4350SOFTAP clone 4350SOFTAP-BASS -tag BASS_BRANCH_5_60   
4350SOFTAP clone 4350SOFTAP-BASS-REL -tag BASS_REL_5_60_???

4350SOFTAP-AARDVARK configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

4350SOFTAP-BISON configure -ipaddr 192.168.1.99 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4350FC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
	}

	
# FC15 Linux PC
UTF::Linux mc93tst3 -sta {X52cSOFTAP enp1s0} \
    -power {npc244 1} \
    -console {mc93end1:40022} \
    -slowassoc 5 -reloadoncrash 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-tcpwindow 4M \
	-datarate {-b 1.2G -i 0.5 -frameburst 1}  -custom 1 \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl country US ; wl obss_coex 0;  wl vht_features 3; wl assert_type 1;:
    }
    

X52cSOFTAP configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
	}

# Clones for mc93tst3
X52cSOFTAP clone X52cSOFTAP-AARDVARK -tag AARDVARK_BRANCH_6_30 -brand "linux-internal-wl"
X52cSOFTAP clone X52cSOFTAP-BISON -tag BISON_BRANCH_7_10 -brand "linux-internal-wl"
X52cSOFTAP clone X52cSOFTAP-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_* -brand "linux-internal-wl"
X52cSOFTAP clone X52cSOFTAP-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_* -brand "linux-external-wl"
X52cSOFTAP clone X52cSOFTAP-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_* -brand "linux-internal-wl"
X52cSOFTAP clone X52cSOFTAP-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_* -brand "linux-external-wl"


# softap settting for AARDVARK
X52cSOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap
X52cSOFTAP-AARDVARK-REL-TAG configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap
X52cSOFTAP-AARDVARK-REL-TAG-EXT configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap
X52cSOFTAP-AARDVARK-REL-10 configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap
X52cSOFTAP-AARDVARK-REL-10-EXT configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap
X52cSOFTAP clone X52cSOFTAP-RUBY -tag RUBY_BRANCH_6_20
X52cSOFTAP clone X52cSOFTAP-KIRIN -tag KIRIN_BRANCH_5_100
X52cSOFTAP clone X52cSOFTAP-KIRIN-REL -tag KIRIN_REL_5_100_*
X52cSOFTAP clone X52cSOFTAP-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
X52cSOFTAP clone X52cSOFTAP-BASS -tag BASS_BRANCH_5_60   
X52cSOFTAP clone X52cSOFTAP-BASS-REL -tag BASS_REL_5_60_???

X52cSOFTAP-AARDVARK configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
	}
X52cSOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid testX52cC15ap -tcpwindow 4M \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 3; wl obss_coex 0; wl country US ; wl assert_type 1;:
	}

         
    
# 4360 MacBook Air
# Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop MacX4360power -button {web247 1}
UTF::MacOS mc93tst4 -sta {MacX4360 en0} \
	-power {MacX4360power} \
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
	-nobighammer 1 -tcpwindow 3640K  -custom 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX4360 clone MacX4360nonly \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ALL ; wl assert_type 1; wl oppr_roam_off 1 }
MacX4360nonly clone MacX4360nonly-BISON -tag BISON_BRANCH_7_10
MacX4360nonly clone MacX4360nonly-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
MacX4360nonly clone MacX4360nonly-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX4360nonly clone MacX4360nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX4360nonly clone MacX4360nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_*


# Clones
MacX4360 clone MacX4360-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX4360 clone MacX4360-BISON -tag BISON_BRANCH_7_10
MacX4360 clone MacX43601-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX4360 clone MacX43601-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX4360 clone MacX4360-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX4360 clone MacX4360-AARDVARK-REL-TAG-EXT -tag AARDVARK_REL_6_30_223_*
MacX4360 clone MacX4360-AARDVARK-REL-10 -tag AARDVARK_REL_6_30_10_*
MacX4360 clone MacX4360-AARDVARK-REL-10-EXT -tag AARDVARK_REL_6_30_10_*
MacX4360 clone MacX4360-BISON -tag BISON_BRANCH_7_10
MacX4360 clone MacX4360-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
MacX4360 clone MacX4360-RUBY -tag RUBY_BRANCH_6_20
MacX4360 clone MacX4360-KIRIN -tag KIRIN_BRANCH_5_100
MacX4360 clone MacX4360-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX4360 clone MacX4360-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX4360 clone MacX4360-BASS -tag BASS_BRANCH_5_60   
MacX4360 clone MacX4360-BASS-REL -tag BASS_REL_5_60_???


# FC15 Linux PC
UTF::Linux mc93tst5 -sta {4360STA enp1s0} \
    -power {npc245 2} \
    -console {mc93end1:40021} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {$S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 4M  -custom 1 \
	-wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl mimo_bw_cap 1; \
        wl obss_coex 0; wl vht_features 3; wl country US ; wl assert_type 1;:
    }

# Clones
4360STA clone 4360STA-TOT -nochannels 1
4360STA clone 4360STA-AARDVARK -tag AARDVARK_BRANCH_6_30
4360STA clone 4360STA-BISON -tag BISON_BRANCH_7_10
4360STA clone 4360STA-BISON-REL-TAG -tag  BIS120RC4_REL_7_15_*
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
UTF::Power::Laptop MacX14power -button {web248 1}
UTF::MacOS mc93tst6 -sta {MacX14 en0} \
	-power {MacX14power} \
	-power_button {auto} \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl assert_type 1 ; wl oppr_roam_off 1 } \
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
	-brand  "macos-internal-wl-cab" \
	-type Debug_10_9 \
	-coreserver AppleCore \
	-kextload true \
	-slowassoc 5 \
	-channelsweep {-count 15} \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
	-nobighammer 1 -tcpwindow 3640K  -custom 1  -coldboot 1
	

# used when testing against an non-ac router, so we don't want to tet AC rates there
MacX14 clone MacX14nonly \
	-wlinitcmds { wl msglevel 0x101  ; wl btc_mode 0 ; wl down ; wl vht_features 3; wl country US ALL ; wl assert_type 1; wl oppr_roam_off 1 }
MacX14nonly clone MacX14nonly-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX14nonly clone MacX14nonly-BISON -tag BIS120RC4_REL_7_15_*
MacX14nonly clone MacX14nonly-BISON-REL-TAG -tag BISON_BRANCH_7_10
MacX14nonly clone MacX14nonly-AARDVARK-REL-TAG -tag AARDVARK_REL_6_30_223_*
MacX14nonly clone MacX14nonly-AARDVARK-REL-TWIG -tag AARDVARK_TWIG_6_30_*


# Clones
MacX14 clone MacX14-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX14 clone MacX14-BISON -tag BISON_BRANCH_7_10
MacX14 clone MacX14-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
MacX14 clone MacX141-AARDVARK -tag AARDVARK_BRANCH_6_28
MacX14 clone MacX141-AARDVARK-REL-TAG -tag AARDVARK_REL_6_28_*
MacX14 clone MacX14-AARDVARK-TWIG -tag AARDVARK_TWIG_6_30_223
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
MacX14 clone MacX14-RUBY -tag RUBY_BRANCH_6_20
MacX14 clone MacX14-KIRIN -tag KIRIN_BRANCH_5_100
MacX14 clone MacX14-KIRIN-REL -tag KIRIN_REL_5_100_*
MacX14 clone MacX14-KIRIN-REL-TAG -tag KIRIN_REL_5_106_98_*
MacX14 clone MacX14-BASS -tag BASS_BRANCH_5_60   
MacX14 clone MacX14-BASS-REL -tag BASS_REL_5_60_???
MacX14 clone MacX14-CURRY -tag BIS120RC4_REL_7_15_162_29



#
#
# AP Section
UTF::Router 4708 -sta {4708/4360 eth2} \
    -relay lan \
    -lan_ip 192.168.1.1 \
    -power {npc241 1} \
    -lanpeer lan \
    -console "mc93end1:40012" \
    -tag "BISON_BRANCH_7_10" \
    -brand "linux-2.6.36-arm-internal-router" \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -nvram {
	lan_ipaddr=192.168.1.1
	lan1_ipaddr=192.168.2.1
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=4708/4331
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=4708/4360
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
4708/4360 configure -attngrp G1
4708/4331 configure -attngrp G1

# AP Section
UTF::Router 47081 -sta {47081/4360 eth2} \
    -relay lan \
    -lan_ip 192.168.1.2 \
    -power {npc241 2} \
    -lanpeer lan \
    -console "mc93end1:40013" \
    -tag " BISON_BRANCH_7_10" \
    -brand "linux-2.6.36-arm-internal-router" \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -nvram {
	lan_ipaddr=192.168.1.2
	lan1_ipaddr=192.168.2.2
	watchdog=3000
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
	router_disable=1
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1 \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

# Clone for 4331 on 2G
47081/4360 clone 47081/4331 -sta {47081/4331 eth1} \
    -channelsweep {-band b} \
    -datarate {-b 400M -i 0.5 -frameburst 1} \
    -noradio_pwrsave 0

# Used for RvRFastSweep.test
47081/4360 configure -attngrp G2
47081/4331 configure -attngrp G2

        
#### ADD UTF::Q for this rig
#####
UTF::Q mc93    
#

# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MC95 
#
####### Controller section:
# mc95end1: FC11 (10.19.60.185)
# mc95end2: FC11 (10.19.60.186)
# subnet  : 10.19.60.xx/22
#
####### Router AP section:
# AP: 4706/4360 eth2      
#     4706/4331 eth1
# SOFTAP: 4360SOFTAP 
#         
#
####### STA section:
# mc95snf1: 4360FC15-SNF   (10.19.60.193)
# mc95tst1: MacX52cP108        (10.19.60.187)
# mc95tst2: MacX51P281     (10.19.60.188)
# mc95tst3: MacX29cP418        (10.19.60.189)
# mc95tst4: MacX51aP403     (10.19.60.190)
# mc95tst5: MacX14P125         (10.19.60.191)
# mc95tst6: MacX87P202        (10.19.60.192)
# mc95tst7: XXXXXXX        (10.19.61.12)
# mc95tst8: XXXXXXX        (10.19.61.13)
# mc95kvm1: MacX52cP108        (10.19.60.194)
#
####### Sniffer section:
# mc95snf1: 4360FC15-SNF  (10.19.60.193)
#
####### Attenuator section:
# AeroFlex attn: 172.5.5.95
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Panda


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc95"

# Add DataRatePlot to all embedded runs
set UTF::DataRatePlot 1

# StaNightlyCustom scripts ontop of StaNightly
set ::UTF::StaNightlyCustom {
        # Custom code here
        package require UTF::Test::throttle
        if {[$STA cget -custom] != ""} {
                throttle $Router $STA -chanspec 3 -nonthrottlecompare 1
        }
        UTF::Message INFO $STA "throttle test skipped"
}

# Define power controllers on cart
UTF::Power::Synaccess npc11 -lan_ip 172.5.5.11 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.5.5.41 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.5.5.51 -rev 1
UTF::Power::Synaccess npc61 -lan_ip 172.5.5.61 -rev 1
UTF::Power::Synaccess npc62 -lan_ip 172.5.5.62 -rev 1
UTF::Power::Synaccess npc81 -lan_ip 172.5.5.81 -rev 1
UTF::Power::Synaccess npc82 -lan_ip 172.5.5.82 -rev 1
UTF::Power::WebRelay  web55 -lan_ip 172.5.5.55 -invert 1
UTF::Power::WebRelay  web65 -lan_ip 172.5.5.65 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.5.5.91 \
	-relay "mc95end1" \
	-group {
		G1 {1 2 3} 
		G2 {4 5 6} 
		G3 {7 8 9} 
		G4 {10 11 12} 
		ALL {1 2 3 4 5 6 7 8 9 10 11 12}
	       }

#G1 configure default 0
#G2 configure default 0
#G3 configure default 0
#ALL configure default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    catch {ALL attn 0;}
    #catch {G3 attn 0;}
    #catch {G4 attn 0;}
    #
    # to ensure that Embedded Nightly only sees one AP
    #
    #catch {43311 restart wl0_radio=0}
    #catch {43311 restart wl1_radio=0}
    #catch {4706/4360 restart wl0_radio=0}
    #catch {4706/4331 restart wl1_radio=0}
    #catch {47061/4360 restart wl0_radio=0}
    #catch {47061/4331 restart wl1_radio=0}
    # ensure sniffer is unloaded
    #catch {4331SNF1 unload}
    #catch {4360SNF1 unload}    

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    #foreach S {MacX52cP108 MacX51P281 MacX29cP418 MacX51aP403 MacX4360X51P182-2 MacX87P202 4360FC15SOFTAP} {
	    #UTF::Try "$S Down" {
		    #catch {$S wl down}
		    #catch {$S deinit}
	    #}
    #}
    # unset S so it doesn't interfere
    #unset S
          
    # delete myself to make sure it doesn't rerun by some script issue
    #unset ::UTF::SetupTestBed

    return
}


#####################################################################
# Hostname: mc95end1
# Platform: 
# OS:       FC11
# Device:   XXXXXXXX
#####################################################################
# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc95end1 -lan_ip 10.19.60.185  \
    -sta {lan eth1} 

#####################################################################
# Hostname: mc95snf1
# Platform: DQ67EP
# OS:       FC15
# Device:   4360mc_P195
#####################################################################
#UTF::Sniffer mc95snf1 -sta {4360FC15-SNF eth0} 
#        -power {npc11 2} 
#        -power_button {auto} 
#        -console "mc95end1:40003" 
#        -tag AARDVARK_BRANCH_6_30 
UTF::Linux mc95snf1 -sta {4360FC15-SNF eth0} \
        -brand linux-internal-wl \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl msglevel +scan ; wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -power {npc11 2} \
        -power_button {auto} \
        -perfchans {36/80 36l 3} \
        -console "mc95end1:40003" \
        -tag AARDVARK_BRANCH_6_30 

#####################################################################
# Hostname: mc95tst1
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X52c_P108
# Options:  -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  
#        -brand  "macos-coverity-76-yosemite" 
#####################################################################
UTF::Power::Laptop TST1Power -button {web55 1}
UTF::MacOS mc95tst1 -sta {MacX52cP108 en0} \
        -power {TST1Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl curpower} {%S wl curppr}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl curpower} {%S wl curppr}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for mc95tst1
MacX52cP108 clone MacX52cP108-TOT -tag "trunk"
MacX52cP108 clone MacX52cP108-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

#####################################################################
# Hostname: mc95tst2
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X51_P281
#####################################################################
UTF::Power::Laptop TST2Power -button {web55 2}
UTF::MacOS mc95tst2 -sta {MacX51P281 en0} \
        -power {TST2Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for mc95tst2
MacX51P281 clone MacX51P281-TOT -tag "trunk"
MacX51P281 clone MacX51P281-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

#####################################################################
# Hostname: mc95tst3
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 2)
# Device:   4360X29c_P409
#####################################################################
# MC95.tst3 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop TST3Power -button {web55 3}
UTF::MacOS mc95tst3 -sta {MacX29cP418 en0} \
        -power {TST3Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for mc95tst3
MacX29cP418 clone MacX29cP418-TOT -tag "trunk"
MacX29cP418 clone MacX29cP418-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

#####################################################################
# Hostname: mc95tst4
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X51_P182
#####################################################################
# MC95.tst4 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop TST4Power -button {web55 4}
UTF::MacOS mc95tst4 -sta {MacX51aP403 en0} \
        -power {TST4Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +assoc ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for mc95tst4
MacX51aP403 clone MacX51aP403-TOT -tag "trunk"
MacX51aP403 clone MacX51aP403-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

#####################################################################
# Hostname: mc95tst5
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4331X33_P700
#####################################################################
# MC95.tst5 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop TST5Power -button {web65 1}
UTF::MacOS mc95tst5 -sta {MacX14P125 en1} \
        -power {TST5Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for mc95tst5
MacX14P125 clone MacX14P125-TOT -tag "trunk"
MacX14P125 clone MacX14P125-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

#####################################################################
# Hostname: mc95tst6
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   43224X21b_A722
#####################################################################
UTF::Power::Laptop TST6Power -button {web65 2}
UTF::MacOS mc95tst6 -sta {MacX87P202 en2} \
        -power {TST6Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1 -coldboot 1

# Clones for mc95tst6
MacX87P202 clone MacX87P202-TOT -tag "trunk"
MacX87P202 clone MacX87P202-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

#####################################################################
# Hostname: mc95tst8
# Platform: DQ67EP
# OS:       FC15
# Device:   4360mc_P195
#####################################################################
# KVM: mc95tst8 (4360FC15TST8)
# A Desktop DUT with FC15
# BCMs: 4360X51
UTF::Linux mc95tst8 -sta {4360FC15TST8 enp1s0} \
    -power {npc81 2} \
    -console {mc95end1:40002} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 3; wl assert_type 1; wl country US;:
    }

# softap settting for AARDVARK
4360FC15TST8 clone 4360FC15TST8-TOT -tag "trunk"

#####################################################################
# Hostname: mc95kvm1
# Platform: DQ67EP
# OS:       FC15
# Device:   4360mc_P195
#####################################################################
# KVM: mc95kvm1 (4360FC15SOFTAP)
# A Desktop DUT with FC15
# BCMs: 4360X51
UTF::Linux mc95kvm1 -sta {4360FC15SOFTAP eth0} \
    -power {npc81 2} \
    -console {mc95end1:40002} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 3; wl assert_type 1; wl country US;:
    }

4360FC15SOFTAP configure -ipaddr 192.168.1.97 -attngrp G2 -hasdhcpd 1 -ap 1 \
-ssid testX51F15SAP

# softap settting for AARDVARK
4360FC15SOFTAP clone 4360FC15SOFTAP-TOT -tag "trunk"
4360FC15SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state}} \
    -post_perf_hook {{%S wl pwrthrottle_state}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 3; wl assert_type 1; wl country US;:
        }

4360FC15SOFTAP clone 4360FC15SOFTAP-JAGUAR_BRANCH_14_10 -tag "JAGUAR_BRANCH_14_10"
4360FC15SOFTAP-JAGUAR_BRANCH_14_10 configure -ipaddr 192.168.1.97 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state}} \
    -post_perf_hook {{%S wl pwrthrottle_state}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 3; wl assert_type 1; wl country US;:
        }

4360FC15SOFTAP clone 4360FC15SOFTAP-IGUANA_BRANCH_13_10 -tag "IGUANA_BRANCH_13_10"
4360FC15SOFTAP-IGUANA_BRANCH_13_10 configure -ipaddr 192.168.1.97 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state}} \
    -post_perf_hook {{%S wl pwrthrottle_state}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 3; wl assert_type 1; wl country US;:
        }

4360FC15SOFTAP clone 4360FC15SOFTAP-EAGLE_BRANCH_10_10 -tag "EAGLE_BRANCH_10_10"
4360FC15SOFTAP-EAGLE_BRANCH_10_10 configure -ipaddr 192.168.1.97 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state}} \
    -post_perf_hook {{%S wl pwrthrottle_state}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 3; wl assert_type 1; wl country US;:
        }

# AP: Netgear R6300(v1) router
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc95end1:40001" \
    -power {npc41 1} \
    -tag "AARDVARK_BRANCH_6_30" \
    -nvram {
        # watchdog=3000 (default)
        lan_ipaddr=192.168.1.2
        lan1_ipaddr=192.168.2.2
        wl_msglevel=0x101
        wl0_ssid=4706/4360
        wl0_channel=1
        wl0_bw_cap=-1
        wl0_radio=0
        wl0_obss_coex=0
        wl1_ssid=4706/4331
        wl1_channel=36
        wl1_bw_cap=-1
        wl1_radio=0
        wl1_obss_coex=0
        #Only 1 AP can serve DHCP Addresses
        #router_disable=1
        et_msglevel=0; # WAR for PR#107305
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1

# Clone for external 4360
4706/4360 clone 4706x/4360 \
    -sta {4706x/4360 eth1} \
    -brand linux26-external-vista-router-full-src \
    -perfonly 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1}

4706/4360 clone 4706/4331-TOT -tag "trunk" -brand linux26-internal-router
4706/4360 clone 4706/4360-TOT -tag "trunk" -brand linux26-internal-router
4706/4360 clone 4706/4331-BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
4706/4360 clone 4706/4360-BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
4706/4331 clone 4706/4331-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" -brand linux26-internal-router
4706/4360 clone 4706/4360-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" -brand linux26-internal-router
4706/4331 clone 4706/4331-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" -brand linux26-internal-router
4706/4360 clone 4706/4360-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" -brand linux26-internal-router

# Used for RvRFastSweep.test
4706/4360 configure -attngrp G1
4706/4331 configure -attngrp G1
4706/4331-TOT configure -attngrp G1
4706/4360-TOT configure -attngrp G1
4706/4331-BISON configure -attngrp G1
4706/4360-BISON configure -attngrp G1
4706/4331-BIS120RC4PHY configure -attngrp G1
4706/4360-BIS120RC4PHY configure -attngrp G1
4706/4331-BIS120RC4 configure -attngrp G1
4706/4360-BIS120RC4 configure -attngrp G1

# AP2: Netgear R6300(v2) router
#    4708/4360 eth2 4708/4360.%15 wl0.%
#    4708/4331 eth1
UTF::Router 4708 -sta {
    4708/4360 eth2 4708/4360.%15 wl1.%
    4708/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux-2.6.36-arm-internal-router \
    -console "mc95end1:40004" \
    -power {npc81 1} \
    -tag "AARDVARK_BRANCH_6_30" \
    -nvram {
        # watchdog=3000 (default)
        lan_ipaddr=192.168.1.2
        lan1_ipaddr=192.168.2.2
        wl_msglevel=0x101
        wl0_ssid=MC95-4708/4360
        wl0_channel=1
        wl0_bw_cap=-1
        wl0_radio=0
        wl0_obss_coex=0
        wl1_ssid=MC95-4708/4331
        wl1_channel=36
        wl1_bw_cap=-1
        wl1_radio=0
        wl1_obss_coex=0
        #Only 1 AP can serve DHCP Addresses
        #router_disable=1
        et_msglevel=0; # WAR for PR#107305
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1

# Clone for external 4360
4708/4360 clone 4708x/4360 \
    -sta {4708x/4360 eth1} \
    -brand linux26-external-vista-router-full-src \
    -perfonly 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1}

4708/4331 clone 4708/4331-TOT -tag "trunk" -brand linux-2.6.36-arm-internal-router
4708/4360 clone 4708/4360-TOT -tag "trunk" -brand linux-2.6.36-arm-internal-router
4708/4331 clone 4708/4331-BISON -tag "BISON_BRANCH_7_10" -brand linux-2.6.36-arm-internal-router
4708/4360 clone 4708/4360-BISON -tag "BISON_BRANCH_7_10" -brand linux-2.6.36-arm-internal-router
4708/4331 clone 4708/4331-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" -brand linux-2.6.36-arm-internal-router
4708/4360 clone 4708/4360-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" -brand linux-2.6.36-arm-internal-router
4708/4331 clone 4708/4331-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" -brand linux-2.6.36-arm-internal-router
4708/4360 clone 4708/4360-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" -brand linux-2.6.36-arm-internal-router

# Used for RvRFastSweep.test
4708/4331 configure -attngrp G4
4708/4360 configure -attngrp G4
4708/4331-TOT configure -attngrp G4
4708/4360-TOT configure -attngrp G4
4708/4331-BISON configure -attngrp G4
4708/4360-BISON configure -attngrp G4
4708/4331-BIS120RC4PHY configure -attngrp G4
4708/4360-BIS120RC4PHY configure -attngrp G4
4708/4331-BIS120RC4 configure -attngrp G4
4708/4360-BIS120RC4 configure -attngrp G4

#### ADD UTF::Q for this rig
#####
UTF::Q mc95

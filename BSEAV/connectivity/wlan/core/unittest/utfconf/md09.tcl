# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MD09 
#
####### Controller section:
# md09end1: FC19 (10.19.61.110)
# md09kvm1: FC19 (10.19.61.120)
# subnet  : 10.19.61.xx/22
#
####### Router AP section:
# AP: 4706/4360 eth2      
#     4706/4331 eth1
#         
# SOFTAP: 
# md09tst1: FC19SOFTAP   (10.19.61.111)
# md08kvm2: APPLSOFTAP   (10.19.61.109)
#         
#
####### STA section:
# md09tst2: MD09STA1     (10.19.61.112)
# md09tst3: MD09STA2     (10.19.61.113)
# md09tst4: MD09STA3     (10.19.61.114)
# md09tst5: MD09STA4     (10.19.61.115)
# md09tst6: MD09STA5     (10.19.61.116)
# md09tst7: MD09STA6     (10.19.61.117)
# md09tst8: MD09STA7     (10.19.61.118)
# md09kvm2: MD09STA8     (10.19.61.121)
#
####### Sniffer section:
# md09snf1: FC19SNF1     (10.19.61.119)
#
####### Attenuator section:
# AeroFlex attn: 172.9.9.95
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Panda


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/md09"

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
UTF::Power::Synaccess npc11 -lan_ip 172.9.9.11 -rev 1
UTF::Power::Synaccess npc12 -lan_ip 172.9.9.12 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.9.9.41 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.9.9.51 -rev 1
UTF::Power::Synaccess npc61 -lan_ip 172.9.9.61 -rev 1
UTF::Power::Synaccess npc62 -lan_ip 172.9.9.62 -rev 1
UTF::Power::Synaccess npc81 -lan_ip 172.9.9.81 -rev 1
UTF::Power::Synaccess npc82 -lan_ip 172.9.9.82 -rev 1
UTF::Power::WebRelay  web15 -lan_ip 172.9.9.15 -invert 1
UTF::Power::WebRelay  web25 -lan_ip 172.9.9.25 -invert 1
UTF::Power::WebRelay  web65 -lan_ip 172.9.9.65 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.9.9.91 \
	-relay "md09end1" \
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
    #foreach S {MacX52cP108 MD09STA1 MD09STA2 MD09STA3 MacX4360X51P182-2 MD09STA5 4360FC15SOFTAP} {
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
# Hostname: md09end1
# Platform: 
# OS:       FC11
# Device:   XXXXXXXX
#####################################################################
# UTF Endpoint1 FC19 - Traffic generators (no wireless cards)
UTF::Linux md09end1 -lan_ip 10.19.61.110  \
    -sta {lan p1p1} 


#####################################################################
# Hostname: md09snf1
# Platform: DQ67EP
# OS:       FC15
# Device:   4360mc_P195
#####################################################################
#UTF::Sniffer md09snf1 -sta {APPLSOFTAP eth0} 
#        -power {npc11 2} 
#        -power_button {auto} 
#        -console "md09end1:40003" 
#        -tag AARDVARK_BRANCH_6_30 
UTF::Linux md09snf1 -sta {APPLSOFTAP eth0} \
        -brand linux-internal-wl \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl msglevel +scan ; wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -power {npc11 2} \
        -power_button {auto} \
        -perfchans {36/80 36l 3} \
        -console "md09end1:40003" \
        -tag AARDVARK_BRANCH_6_30 

#####################################################################
# Hostname: md08kvm2
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X52c_P108
# Options:  -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  
#####################################################################
UTF::Power::Laptop APPLSAP2Power -button {web25 1}
UTF::MacOS md08kvm2 -sta {APPLSOFTAP en1} \
        -power {APPLSAP2Power} \
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

APPLSOFTAP configure -ipaddr 192.168.1.99 -attngrp G2 \
	-hasdhcpd 1 -ap 1 -ssid AppleSoftAP -tcpwindow 3640K \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
        }

# Clones for md08kvm2
APPLSOFTAP clone APPLSOFTAP-TOT -tag trunk -brand "linux-internal-wl"
APPLSOFTAP-TOT configure -ipaddr 192.168.1.99 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid AppleSoftAP -tcpwindow 3640K \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

APPLSOFTAP clone APPLSOFTAP-BISON -tag BISON_BRANCH_7_10
APPLSOFTAP-BISON configure -ipaddr 192.168.1.99 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid AppleSoftAP -tcpwindow 3640K \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

APPLSOFTAP clone APPLSOFTAP-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" 
APPLSOFTAP-BIS120RC4PHY configure -ipaddr 192.168.1.99 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid AppleSoftAP -tcpwindow 3640K \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

APPLSOFTAP clone APPLSOFTAP-BIS10RC4 -tag "BIS120RC4_BRANCH_7_15" 
APPLSOFTAP-BIS10RC4 configure -ipaddr 192.168.1.99 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid AppleSoftAP -tcpwindow 3640K \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

APPLSOFTAP clone APPLSOFTAP-REL-BIS10RC4 -tag "BIS120RC4_REL_7_15_*" 
APPLSOFTAP-REL-BIS10RC4 configure -ipaddr 192.168.1.99 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid AppleSoftAP -tcpwindow 3640K \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

#####################################################################
# Hostname: md09tst1
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X52c_P108
# Options:  -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  
#####################################################################
UTF::Linux md09tst1 -sta {FC19SOFTAP enp1s0} \
    -power {npc12 1} \
    -console {md09end1:40001} \
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

FC19SOFTAP configure -ipaddr 192.168.1.97 -attngrp G1 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl country US ;:
        }

FC19SOFTAP clone FC19SOFTAP-TOT -tag trunk -brand "linux-internal-wl"
FC19SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G1 \
        -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

FC19SOFTAP clone FC19SOFTAP-BISON -tag BISON_BRANCH_7_10
FC19SOFTAP-BISON configure -ipaddr 192.168.1.97 -attngrp G1 \
        -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

# Clones for md09tst1
FC19SOFTAP clone FC19SOFTAP-BISON-REL-TAG -tag BIS120RC4_REL_7_15_*
FC19SOFTAP clone FC19SOFTAP-APPL-NIC -tag BIS120RC4_REL_7_15_166_5

#####################################################################
# Hostname: md09tst2
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   43602X87 P303
#####################################################################
UTF::Power::Laptop TST2Power -button {web15 1}
UTF::MacOS md09tst2 -sta {MD09STA1 en0} \
        -power {TST2Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for md09tst2
MD09STA1 clone MD09STA1-TOT -tag "trunk"
MD09STA1 clone MD09STA1-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA1 clone MD09STA1-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09tst3
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 2)
# Device:   43602X238 P102
#####################################################################
# MC95.tst3 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop TST3Power -button {web15 2}
UTF::MacOS md09tst3 -sta {MD09STA2 en0} \
        -power {TST3Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for md09tst3
MD09STA2 clone MD09STA2-TOT -tag "trunk"
MD09STA2 clone MD09STA2-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA2 clone MD09STA2-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09tst4
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X52c A205
#####################################################################
# MC95.tst4 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop TST4Power -button {web15 3}
UTF::MacOS md09tst4 -sta {MD09STA3 en0} \
        -power {TST4Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for md09tst4
MD09STA3 clone MD09STA3-TOT -tag "trunk"
MD09STA3 clone MD09STA3-DOME -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA3 clone MD09STA3-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA3 clone MD09STA3-BIS715CASCADE_TWIG_7_21 -tag "BIS715CASCADE_TWIG_7_21_*" -custom 1

#####################################################################
# Hostname: md09tst5
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4331X29b A505
#####################################################################
# MC95.tst5 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop TST5Power -button {web15 4}
UTF::MacOS md09tst5 -sta {MD09STA4 en2} \
        -power {TST5Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for md09tst5
MD09STA4 clone MD09STA4-TOT -tag "trunk"
MD09STA4 clone MD09STA4-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA4 clone MD09STA4-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09tst6
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X29c P302
#####################################################################
UTF::Power::Laptop TST6Power -button {web65 1}
UTF::MacOS md09tst6 -sta {MD09STA5 en0} \
        -power {TST6Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1 -coldboot 1

# Clones for md09tst6
MD09STA5 clone MD09STA5-TOT -tag "trunk"
MD09STA5 clone MD09STA5-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA5 clone MD09STA5-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09tst7
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X51 P204
#####################################################################
UTF::Power::Laptop TST7Power -button {web65 2}
UTF::MacOS md09tst7 -sta {MD09STA6 en0} \
        -power {TST7Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1 -coldboot 1

# Clones for md09tst7
MD09STA6 clone MD09STA6-TOT -tag "trunk"
MD09STA6 clone MD09STA6-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA6 clone MD09STA6-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09tst8
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4350X14 P172
#####################################################################
UTF::Power::Laptop TST8Power -button {web65 3}
UTF::MacOS md09tst8 -sta {MD09STA7 en0} \
        -power {TST8Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl country} {%S wl curpower}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl country} {%S wl curpower}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1 -coldboot 1

# Clones for md09tst7
MD09STA7 clone MD09STA7-TOT -tag "trunk"
MD09STA7 clone MD09STA7-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA7 clone MD09STA7-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09kvm2
# Platform: MacBook Air
# OS:       10.9 13A446 (HD 2 1)
# Device:   4360X51a P404
#####################################################################
UTF::Power::Laptop KVM2Power -button {web65 4}
UTF::MacOS md09kvm2 -sta {MD09STA8 en0} \
        -power {KVM2Power} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1 -coldboot 1

# Clones for md09kvm2
MD09STA8 clone MD09STA8-TOT -tag "trunk"
MD09STA8 clone MD09STA8-APPLREL -tag "BIS715CASCADE_REL_7_21_*" -custom 1
MD09STA8 clone MD09STA8-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*" -custom 1

#####################################################################
# Hostname: md09kvm1
# Platform: DQ67EP
# OS:       FC15
# Device:   4360mc_P195
#####################################################################
# KVM: md09kvm1 (4360FC15SOFTAP)
# A Desktop DUT with FC15
# BCMs: 4360X51

# AP: Netgear R6300(v1) router
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "md09end1:40001" \
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

# AP2: Netgear R6300(v1) router - 2
UTF::Router 47061 -sta {
    47061/4360 eth2 4706/4360.%15 wl0.%
    47061/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "md09end1:40005" \
    -power {npc81 2} \
    -tag "AARDVARK_BRANCH_6_30" \
    -nvram {
        # watchdog=3000 (default)
        lan_ipaddr=192.168.1.2
        lan1_ipaddr=192.168.2.2
        wl_msglevel=0x101
        wl0_ssid=47061/4360
        wl0_channel=1
        wl0_bw_cap=-1
        wl0_radio=0
        wl0_obss_coex=0
        wl1_ssid=47061/4331
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
47061/4360 clone 47061x/4360 \
    -sta {47061x/4360 eth1} \
    -brand linux26-external-vista-router-full-src \
    -perfonly 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1}

47061/4360 clone 47061/4331-TOT -tag "trunk" -brand linux26-internal-router
47061/4360 clone 47061/4360-TOT -tag "trunk" -brand linux26-internal-router
47061/4331 clone 47061/4331-BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
47061/4360 clone 47061/4360-BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
47061/4331 clone 47061/4331-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" -brand linux26-internal-router
47061/4360 clone 47061/4360-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" -brand linux26-internal-router
47061/4331 clone 47061/4331-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" -brand linux26-internal-router
47061/4360 clone 47061/4360-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" -brand linux26-internal-router

# Used for RvRFastSweep.test
47061/4360 configure -attngrp G1
47061/4331 configure -attngrp G1
47061/4331-TOT configure -attngrp G1
47061/4360-TOT configure -attngrp G1
47061/4331-BISON configure -attngrp G1
47061/4360-BISON configure -attngrp G1
47061/4331-BIS120RC4PHY configure -attngrp G1
47061/4360-BIS120RC4PHY configure -attngrp G1
47061/4331-BIS120RC4 configure -attngrp G1
47061/4360-BIS120RC4 configure -attngrp G1

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
    -console "md09end1:40004" \
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
UTF::Q md09

# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MC91 
#
####### Controller section:
# mc91end1: FC11 (10.19.60.139)
# subnet  : 10.19.60.xx/22
#
####### Router AP section:
# AP: 4706/4360 eth2      (mc91end1:40001)
#     4706/4331 eth1
# SOFTAP: 4360SOFTAP eth0 (mc91end1:40002)
#         
#
####### STA section:
# mc91snf1: 4360FC15-SNF  (10.19.60.146, mc91end1:40003)
# mc91tst1: 4360SOFTAP     (10.19.60.140, mc91end1:40002)
# mc91tst2: 4335b0PANDA    (10.19.60.141, mc91end1:40005,40006)
# mc91tst3: 4350FC15 eth0  (10.19.60.142)
# mc91tst4: MacX51a en0    (10.19.60.143, MacBookAir ZIN 10.8 FamilyDrop 28)
# mc91tst5: MacX28 en0     (10.19.60.144, MacBookPro ZIN 10.8 FamilyDrop 28)
# mc91tst6: MacX52c en0   (10.19.60.145, MacBookAir ZIN 10.8 FamilyDrop 28)
# mc91tst7: 
#
####### Sniffer section:
# mc91snf1: 4360FC15-SNF  (10.19.60.146, mc91end1:40003)
#
####### Attenuator section:
# attn: 172.4.4.91
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Panda


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc91"

# Power throttle
set ::UTF::StaNightlyCustom {
# Custom code here
package require UTF::Test::throttle
        if {[$STA cget -custom] eq ""} {
                UTF::Message INFO $STA "throttle test skipped"
                return
        }
        throttle $Router $STA -chanspec 3 -nonthrottlecompare 1
}

# Define power controllers on cart
UTF::Power::Synaccess npc11 -lan_ip 172.4.4.11 -rev 1
UTF::Power::Synaccess npc12 -lan_ip 172.4.4.12 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.4.4.41 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.4.4.51 -rev 1
UTF::Power::Synaccess npc61 -lan_ip 172.4.4.61 -rev 1
UTF::Power::Synaccess npc62 -lan_ip 172.4.4.62 -rev 1
UTF::Power::Synaccess npc72 -lan_ip 172.4.4.72 -rev 1
UTF::Power::Synaccess npc82 -lan_ip 172.4.4.82 -rev 1
UTF::Power::WebRelay  web63 -lan_ip 172.4.4.63 -invert 1
UTF::Power::WebRelay  web71 -lan_ip 172.4.4.71 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.4.4.91 \
	-relay "mc91end1" \
	-group {
		G1 {1 2 3} 
		G2 {4 5 6} 
		G3 {7 8 9} 
		ALL {1 2 3 4 5 6 7 8 9}
	       }

#G1 configure default 0
#G2 configure default 0
#G3 configure default 0
#ALL configure default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
#    #
#    # Make Sure Attenuators are set to 0 value
#    #
    catch {ALL attn 0;}
#    #catch {G3 attn 0;}
#    #catch {G4 attn 0;}
#    #
#    # to ensure that Embedded Nightly only sees one AP
#    #
#    catch {43311 restart wl0_radio=0}
#    catch {43311 restart wl1_radio=0}
#    catch {4706/4360 restart wl0_radio=0}
#    catch {4706/4331 restart wl1_radio=0}
#    #catch {47061/4360 restart wl0_radio=0}
#    #catch {47061/4331 restart wl1_radio=0}
#    # ensure sniffer is unloaded
#    #catch {4331SNF1 unload}
#    #catch {4360SNF1 unload}    
#
#    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4360SOFTAP 4360X87F19} {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
		    catch {$S deinit}
	    }
    }
    # unset S so it doesn't interfere
    unset S
          
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed
#
    return
}


# Define Sniffer
# mc91snf1: 4360FC15-SNF  (10.19.60.146, mc91end1:40003)
#    -tag AARD118RC34_REL_6_30_160_43 
UTF::Sniffer mc91snf1 -sta {4360FC15-SNF eth0} \
        -power {npc11 2} \
        -power_button {auto} \
        -console "mc91end1:40003" \
        -tag AARDVARK_BRANCH_6_30 \

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc91end1 -lan_ip 10.19.60.139  \
    -sta {lan eth1} 

# FC15 Linux PC
UTF::Linux mc91tst1 -sta {4360SOFTAP enp1s0} \
    -power {npc11 1} \
    -console {mc91end1:40002} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
    -tcpwindow 4M \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US/0:
    }

4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G2 -hasdhcpd 1 -ap 1 -ssid test4360FC15ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

# Clones for mc91tst1
4360SOFTAP clone 4360SOFTAP-TOT -tag "trunk"
4360SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAPTOT -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US/0;:
        }

4360SOFTAP clone 4360SOFTAP-EAGLE -tag "EAGLE_BRANCH_10_10" 
4360SOFTAP-EAGLE configure -ipaddr 192.168.1.97 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAPEAGLE -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US/0;:
        }

4360SOFTAP clone 4360SOFTAP-IGUANA_BRANCH_13_10 -tag "IGUANA_BRANCH_13_10" 
4360SOFTAP-IGUANA_BRANCH_13_10 configure -ipaddr 192.168.1.97 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAPEAGLE -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US/0;:
        }

# MC91.tst2 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop X87Power -button {web63 1}
UTF::MacOS mc91tst2 -sta {MacX87 en0} \
        -power {X87Power} \
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

# Clones for mc91tst2
MacX87 clone MacX87-TOT -tag trunk
MacX87 clone MacX87-BIS715GALA_BRANCH_7_21 -tag BIS715GALA_BRANCH_7_21
MacX87 clone MacX87-BIS715GALA_TWIG_7_21_139 -tag BIS715GALA_TWIG_7_21_139
MacX87 clone MacX87-APPLREL -tag "BIS715CASCADE_REL_7_21_171_*" -custom 1

# MC91.tst3 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop X29CPower -button {web63 2}
UTF::MacOS mc91tst3 -sta {MacX51 en0} \
        -power {X29CPower} \
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

# Clones for mc91tst3
MacX51 clone MacX51-TOT -tag trunk
MacX51 clone MacX51-BIS715GALA_BRANCH_7_21 -tag BIS715GALA_BRANCH_7_21
MacX51 clone MacX51-BIS715GALA_TWIG_7_21_139 -tag BIS715GALA_TWIG_7_21_139
MacX51 clone MacX51-APPLREL -tag "BIS715CASCADE_REL_7_21_171_*" -custom 1

# MC91.tst4 - MacBook Air with Cab 13A446 OS, BRCM 4360X51
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop X29CPower -button {web71 1}
UTF::MacOS mc91tst4 -sta {MacX51a en0} \
        -power {X29CPower} \
        -power_button {auto} \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl txcore} {%S wl spatial_policy} {%S wl curpower} {%S wl nrate }} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl txcore} {%S wl spatial_policy} {%S wl curpower} {%S wl nrate }} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
    	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 -tcpwindow 3640K -custom 1

# Clones for mc91tst4
MacX51a clone MacX51a-TOT -tag trunk
MacX51a clone MacX51a-BIS715GALA_BRANCH_7_21 -tag BIS715GALA_BRANCH_7_21
MacX51a clone MacX51a-APPLREL -tag "BIS715CASCADE_REL_7_21_171_*" -custom 1

# MC91.tst5 - MacBook Pro with Lion 10.7 with X28 4331 nic
## Needed to get around MAC Power Cycle to Debugger
#UTF::MacOS mc91tst5 -sta {MacX29c en1} 
UTF::Power::Laptop X28Power -button {web71 2}
UTF::MacOS mc91tst5 -sta {MacX29c en0} \
        -power {X28Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 3; wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true 

# Clones for mc91tst5
MacX29c clone MacX29c-TOT -tag trunk
MacX29c clone MacX29c-BIS715GALA_BRANCH_7_21 -tag BIS715GALA_BRANCH_7_21
MacX29c clone MacX29c-APPLREL -tag "BIS715CASCADE_REL_7_21_171_*" -custom 1

# MC91.tst6 - MacBook Air with Lion 10.7 with X29 4331 nic
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop X52cPower -button {web71 4}
UTF::MacOS mc91tst6 -sta {MacX52c en0} \
        -power {X52cPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; \
	   wl btc_mode 0; wl down; wl vht_features 3; \
	   wl country ALL ; wl up ; wl assert_type 1 } \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true

# Clones for mc91tst6
MacX52c clone MacX52c-TOT -tag trunk
MacX52c clone MacX52c-BIS715GALA_BRANCH_7_21 -tag BIS715GALA_BRANCH_7_21
MacX52c clone MacX52c-APPLREL -tag "BIS715CASCADE_REL_7_21_171_*" -custom 1


# MC91.tst6 
## Needed to get around MAC Power Cycle to Debugger
#        -dhd_tag trunk 
#        -brand linux-external-dongle-sdio 
#        -dhd_brand linux-internal-dongle 
#        -nvram bcm94339wlipa.txt 
#        -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} 
#        -type 4339a0-roml/sdio-ag-pool-p2p-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus-lpc-wl11u-tdls-ampduhostreorder-ve-proxd-mfp-okc-txbf-ipa 
UTF::MacOS mc91tst6 -sta {MacX52c en0} \

UTF::Power::Laptop TST7Power -button {web71 3}
UTF::MacOS mc91tst7 -sta {MacX52c205 en0} \
        -power {TST7Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl msglevel +scan ; \
	   wl btc_mode 0; wl down; wl vht_features 3; \
	   wl country ALL ; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump stats} {%S wl wme_counters} {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true

# Clones for mc91tst7
MacX52c205 clone MacX52c205-TOT -tag trunk
MacX52c205 clone MacX52c205-BIS715GALA_BRANCH_7_21 -tag BIS715GALA_BRANCH_7_21
MacX52c205 clone MacX52c205-APPLREL -tag "BIS715CASCADE_REL_7_21_171_*" -custom 1

# AP Section 
# AP1: Netgear R6300v1 (4706/4360-4331)
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.2 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc91end1:40001" \
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

4706/4331 clone 4706/4331-TOT -tag "trunk" -brand linux26-internal-router
4706/4360 clone 4706/4360-TOT -tag "trunk" -brand linux26-internal-router
4706/4331 clone 4706/4331-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router
4706/4360 clone 4706/4360-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router

# Used for RvRFastSweep.test
4706/4360 configure -attngrp G1
4706/4331 configure -attngrp G1
4706/4331-TOT configure -attngrp G1
4706/4360-TOT configure -attngrp G1
4706/4331-BISON04T configure -attngrp G1
4706/4360-BISON04T configure -attngrp G1

# AP2: Netgear R6300v1 (4706/4360-4331)
UTF::Router 47061 -sta {
    47061/4360 eth2 47061/4360.%15 wl0.%
    47061/4331 eth1
    } \
    -lan_ip 192.168.1.2 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc91end1:40007" \
    -power {npc12 1} \
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

47061/4331 clone 47061/4331-TOT -tag "trunk" -brand linux26-internal-router
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
    -console "mc91end1:40008" \
    -power {npc41 2} \
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
4708/4331 configure -attngrp G1
4708/4360 configure -attngrp G1
4708/4331-TOT configure -attngrp G1
4708/4360-TOT configure -attngrp G1
4708/4331-BISON configure -attngrp G1
4708/4360-BISON configure -attngrp G1
4708/4331-BIS120RC4PHY configure -attngrp G1
4708/4360-BIS120RC4PHY configure -attngrp G1
4708/4331-BIS120RC4 configure -attngrp G1
4708/4360-BIS120RC4 configure -attngrp G1

# AP Router: ASUS RT-AC68U
#    4708/4360 eth2 4708/4360.%15 wl0.%
#    4708/4331 eth1
UTF::Router 4709 -sta {4709/4360 eth2 4709/4360.%15 wl1.%} \
    -relay lan \
    -lan_ip 192.168.1.1 \
    -power {npc41 2} \
    -lanpeer lan \
    -console "mc91:40008" \
    -tag "BISON_BRANCH_7_10" \
    -brand "linux-2.6.36-arm-internal-router" \
    -nvram {
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=Broadcom
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl1_ssid=Broadcom
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1 \
    -perfchans {36/80 36l 36} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}

4709/4360 clone 4709/4360-BISON -tag "BISON_BRANCH_7_10" -brand linux-2.6.36-arm-internal-router
4709/4360-BISON configure -attngrp G1

#### ADD UTF::Q for this rig
#####
UTF::Q mc91

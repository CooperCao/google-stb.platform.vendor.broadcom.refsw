# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MC90 
#
####### Controller section:
# mc90end1: FC11 (10.19.60.82)
# subnet  : 10.19.60.xx/22
#
####### Router AP section:
# AP1: 4709/4360 eth2         (mc90end1:40002) 
# AP1: 4708/4360 eth2         (mc90end1:40002) 
# AP2: 4706/4360 eth2     (mc90end1:40001)
#      4706/4331 eth1
# SOFTAP: 4360SOFTAP eth0 (mc90end1:40003)
#      4706/4331 eth1
#
####### STA section:
# mc90snf1: 4360W8X64SNF   (10.19.60.139)
# mc90tst1: 4360SOFTAP     (10.19.60.140, mc90end1:40003)
# mc90tst2: 4360W8X64 eth0 (10.19.60.141, mc90end1:40001)
# mc90tst3: 4360FC19 eth0  (10.19.60.142, mc90end1:40004)
# mc90tst4: MacX51P202 en0   (10.19.60.143, MacBookAir ZIN 10.8 FamilyDrop 28)
# mc90tst5: MacX51 en0     (10.19.60.144, MacBookPro ZIN 10.8 FamilyDrop 28)
# mc90tst6: MacX29 en0     (10.19.60.145, MacBookAir ZIN 10.8 FamilyDrop 28)
#
####### Sniffer section:
# mc90snf1: 4331SNF1 eth1 (10.19.60.90, mc90end1:40009)
#
####### Attenuator section:
# attn: 172.3.3.91
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc90"

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
UTF::Power::Synaccess npc11 -lan_ip 172.3.3.11 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.3.3.41 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.3.3.51 -rev 1
UTF::Power::Synaccess npc81 -lan_ip 172.3.3.81 -rev 1
UTF::Power::Synaccess npc82 -lan_ip 172.3.3.82 -rev 1
#UTF::Power::Synaccess npc12 -lan_ip 172.1.1.12 -rev 1
#UTF::Power::Synaccess npc13 -lan_ip 172.1.1.13 -rev 1
#UTF::Power::Synaccess npc14 -lan_ip 172.1.1.14 -rev 1
UTF::Power::WebRelay  web71 -lan_ip 172.3.3.71 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.3.3.91 \
	-relay "mc90end1" \
	-group {
		G1 {1 2 3} 
		G2 {4 5 6} 
		G3 {7 8 9} 
		ALL {1 2 3 4 5 6 7 8 9}
	       }

#G1 configure default 0
#G2 configure default 0
#G3 configure default 0
#G4 configure default 0
#ALL configure default 0

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    catch {ALL attn 0;}
    catch {G3 attn 0;}
    catch {G4 attn 0;}

    # Make sure radios are off on APs before testing
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
    foreach S {4360W8X64SNF 4360SOFTAP 4360W8X64 4360FC19 MacX51P202 MacX51 MacX29} {
#	    UTF::Try "$S Down" {
#		    catch {$S wl down}
#		    catch {$S deinit}
#	    }
    }
    # unset S so it doesn't interfere
    unset S
          
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}


# Define Sniffer
UTF::Sniffer mc90snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc13 1} \
        -power_button {auto} \
        -console "mc90end1:40009"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc90end1 -lan_ip 10.19.60.131  \
    -sta {lan eth1} 

# FC15 Linux PC
UTF::Linux mc90tst1 -sta {4360SOFTAP eth0} \
    -power {npc11 1} \
    -console {mc90end1:40003} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US;:
    }

4360SOFTAP configure -ipaddr 192.168.1.97 -attngrp G3 -hasdhcpd 1 -ap 1 -ssid test4360FC19ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

# Clones for mc90tst1
4360SOFTAP clone 4360SOFTAP-TOT -tag trunk -brand "linux-internal-wl"
4360SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G3 \
        -hasdhcpd 1 -ap 1 -ssid test4360FC19ap -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

4360SOFTAP clone 4360SOFTAP-JAGUAR_BRANCH_14_10 -tag JAGUAR_BRANCH_14_10 \
	-brand "linux-internal-wl"
4360SOFTAP-JAGUAR_BRANCH_14_10 configure -ipaddr 192.168.1.97 -attngrp G3 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAP_JAGUAR_BRANCH_14_10 -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

4360SOFTAP clone 4360SOFTAP-BIS715T254 -tag BIS715T254_BRANCH_7_52 \
	-brand "linux-internal-wl"
4360SOFTAP-BIS715T254 configure -ipaddr 192.168.1.97 -attngrp G3 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAP_BIS715T254 -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

4360SOFTAP clone 4360SOFTAP-EAGLE_10_10 -tag EAGLE_BRANCH_10_10 \
	-brand "linux-internal-wl"
4360SOFTAP-EAGLE_10_10 configure -ipaddr 192.168.1.97 -attngrp G3 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAP_EAGLE_10_10 -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

4360SOFTAP clone 4360SOFTAP-IGUANA_BRANCH_13_10 -tag "IGUANA_BRANCH_13_10"
4360SOFTAP-IGUANA_BRANCH_13_10 configure -ipaddr 192.168.1.97 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid 4360SOFTAPEAGLE -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US/0;:
        }

# STA2: WIN8x64 - Dell Laptop E6420
#   4360 - 11ac 3x3
# -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  
UTF::Cygwin mc90tst2 \
        -osver 864 \
        -user user \
        -installer inf \
        -lan_ip mc90tst2 \
        -sta {4360W8X64} \
        -tcpwindow 4M -custom 1 \
        -power {npc81 1} \
        -power_button {auto} \
        -brand win8x_internal_wl \
        -wlinitcmds {wl msglevel 0x101; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {$S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1}

# clones for mc90tst2
# --------------------------------------------------------
4360W8X64 clone 4360W8X64-TOT -tag "trunk"

# mc90tst3: Desktop DUT Dell E4310
# OS: Fedora Core 15
# BCMs: "Linux 4331hm P152"
# -type obj-debug-p2p-mchan
UTF::Linux mc90tst3 \
        -sta "4360FC19 enp1s0" \
        -brand linux-internal-wl \
        -console "mc90end1:40004" \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} \
         {%S wl phy_cal_disable 1}} \
        -power "npc81 2"

# Clones of STA 4360FC19 with Different Options for Test
#4360FC19 clone 4360FC19-DINGO2_9_15 -tag "DINGO2_BRANCH_9_15"
#4360FC19 clone 4360FC19-DINGO_9_10 -tag "DINGO_BRANCH_9_10"
4360FC19 clone 4360FC19-TOT -tag "trunk"
#4360FC19 clone 4360FC19-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
4360FC19 clone 4360FC19-EAGLE_10_10 -tag "EAGLE_BRANCH_10_10"
4360FC19 clone 4360FC19-JAGUAR_BRANCH_14_10 -tag "JAGUAR_BRANCH_14_10"

# MC90.tst4 - MacBook Air with Lion 10.7 with X33 4331 nic
## Needed to get around MAC Power Cycle to Debugger
#        -wlinitcmds { wl msglevel 0x101 ; wl btc_mode 0; wl down; wl vht_features 1; wl amsdu 0; wl up ; wl assert_type 1 ; wl tempsense_disable 1} \
#        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0}} \
#        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl tempsense_disable 1}} \
UTF::Power::Laptop X33Power -button {web71 1}
UTF::MacOS mc90tst4 -sta {MacX51P202 en0} \
        -user root \
        -power {X33Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds { wl msglevel 0x101 ; wl btc_mode 0; wl down; wl vht_features 1; wl amsdu 0; wl up ; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
    	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -perfchans {36/80 36l 3} \
        -nobighammer 1 

#        -perfchans {36/80 36l 3} 
# Clones for mc90tst4
MacX51P202 clone MacX51P202-DINGO2_9_15 -tag "DINGO2_BRANCH_9_15"
MacX51P202 clone MacX51P202-TOT -tag "trunk"
#MacX51P202 clone MacX51P202-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
MacX51P202 clone MacX51P202-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21"

# MC90.tst5 - MacBook Pro with Lion 10.7 with X28 4331 nic
## Needed to get around MAC Power Cycle to Debugger
#UTF::MacOS mc90tst5 -sta {MacX51 en1} 
UTF::Power::Laptop X28Power -button {web71 2}
UTF::MacOS mc90tst5 -sta {MacX51 en0} \
        -power {X28Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl up; wl assert_type 0;} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-10_12" \
        -type Debug_10_12 \
        -coreserver AppleCore \
    	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -kextload true \
        -nobighammer 1 

#       -perfchans {36/80 36l 3}

# Clones for mc90tst5
MacX51 clone MacX51-TOT -tag "trunk"
MacX51 clone MacX51-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
MacX51 clone MacX51-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21"

# MC90.tst6 - MacBook Air with Lion 10.7 with X29 4331 nic
## Needed to get around MAC Power Cycle to Debugger
UTF::Power::Laptop X29Power -button {web71 3}
UTF::MacOS mc90tst6 -sta {MacX29 en0} \
        -power {X29Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-cab" \
        -type Debug_10_9 \
        -coreserver AppleCore \
        -kextload true \
        -slowassoc 5 \
        -channelsweep {-count 15} \
    	-datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nobighammer 1 

# Clones for mc90tst6
MacX29 clone MacX29-TOT -tag "trunk"
MacX29 clone MacX29-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
MacX29 clone MacX29-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21"

# STAx: WIN8x64 - Dell Laptop E6420
#   4360 - 11ac 3x3
UTF::Cygwin mc90snf1 \
        -osver 864 \
        -user user \
        -installer inf \
        -lan_ip mc90snf1 \
        -sta {4360W8X64SNF} \
        -tcpwindow 4M \
        -power {npc82 1} \
        -power_button {auto} \
        -brand win8x_internal_wl \
        -wlinitcmds {wl msglevel 0x101; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {$S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1}

# clones for mc90snf1
# --------------------------------------------------------
4360W8X64SNF clone 4360W8X64SNF-TOT -tag "trunk"
4360W8X64SNF clone 4360W8X64SNF-BISON -tag "BISON_BRANCH_7_10"
4360W8X64SNF clone 4360W8X64SNF-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16"
4360W8X64SNF clone 4360W8X64SNF-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15"

#Linksys E4200 4718/4331 Router AP1
#UTF::Router 4709 -sta {4709/4360 eth2 4709/4360.%15 wl1.%} \
#    -relay lan \
#    -lan_ip 192.168.1.1 \
#    -power {npc41 2} \
#    -lanpeer lan \
#    -console "mc90end1:40002" \
#    -tag "BISON_REL_7_10_117_1" \
#    -brand "linux-2.6.36-arm-internal-router-dhdap" \
#    -nvram {
#        watchdog=3000
#        wl_msglevel=0x101
#        console_loglevel=7
#        wl0_ssid=Broadcom
#        wl0_chanspec=3
#        wl0_obss_coex=0
#        wl0_bw_cap=-1
#        wl0_radio=0
#        wl1_ssid=Broadcom
#        wl1_chanspec=36
#        wl1_obss_coex=0
#        wl1_bw_cap=-1
#        wl1_radio=0
#    } \
#    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
#    -noradio_pwrsave 1 \
#    -perfchans {36/80 36l 36} \
#    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1}
#
## Used for RvRFastSweep.test
#4709/4360 configure -attngrp G2
UTF::Router 4708 -sta {
    4708/4360 eth2 4708/4360.%15 wl1.%
    4708/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux-2.6.36-arm-internal-router \
    -console "mc90end1:40002" \
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
4708/4331 clone 4708/4331-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux-2.6.36-arm-internal-router
4708/4360 clone 4708/4360-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux-2.6.36-arm-internal-router


#Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "43312 eth2" \
        -lan_ip 192.168.1.1 \
    -relay "mc90end1" \
    -lanpeer lan \
    -console "mc90end1:40005" \
    -power "npc41 2" \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65" \
        -nvram {
                boot_hw_model=E4200
                wandevs=et0
                {lan_ifnames=vlan1 eth1 eth2}
                et0macaddr=00:90:4c:07:00:8c
                macaddr=00:90:4c:07:00:9d
                sb/1/macaddr=00:90:4c:07:10:00
                pci/1/1/macaddr=00:90:4c:07:11:00
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
                wl0_ssid=test43312-ant0
                wl1_ssid=test43312-ant1
                wl0_channel=1
                wl1_channel=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                wl1_obss_coex=0
        }

# Used for RvRFastSweep.test
43312 configure -attngrp G2

# AP Section 
# AP2: Linksys E2000/4717 11n 2x2 wireless router
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.2 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc90end1:40001" \
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
        #wl1_channel=36
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
4706/4360 clone 4706/4360-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router
4706/4331 clone 4706/4331-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router

# Used for RvRFastSweep.test
4706/4331 configure -attngrp G1
4706/4360 configure -attngrp G1
4706/4331-TOT configure -attngrp G1
4706/4360-TOT configure -attngrp G1
4706/4360-BISON04T configure -attngrp G1
4706/4331-BISON04T configure -attngrp G1

#### ADD UTF::Q for this rig
#####
UTF::Q mc90

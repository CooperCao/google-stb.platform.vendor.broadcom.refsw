# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MC85 
#
####### Controller section:
# mc85end1: FC11 (10.19.60.93)
# subnet  : 10.19.60.xx/22
#
####### Router AP section:
# AP1:    43311 eth2 4331-2G eth1
# AP2:    4706/4360 eth2 4706/4331 eth1
# SOFTAP: FC19SOFTAP eth0 (mc85kvm2)
#
####### STA section:
# mc85tst1: 4331F15 eth0 (10.19.60.95, mc85end1:40001)
# mc85tst2: 4360X51F15 eth0 (10.19.60.96, mc85end1:40002)
# mc85tst3: 4352WIN8     (10.19.60.97)
# mc85tst4: MacX29C en0  (10.19.60.98)
# mc85tst5: MacX51  en0  (10.19.60.99)
# mc85tst6: MacX28B en1  (10.19.60.100)
# mc85kvm2: FC19SOFTAP em0  (10.19.60.103, mc85end1:40007)
#
####### Sniffer section:
# mc85snf1: 4360SNF1 eth0 (10.19.60.101, mc85end1:40005)
#
####### Attenuator section:
# af: 172.2.2.91 
#     -group {G1 {1 2 3} G2 {4 5 6} ALL {1 2 3 4 5 6}}
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc85"

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
UTF::Power::Synaccess npc08 -lan_ip 172.2.2.8 -rev 1
UTF::Power::Synaccess npc91 -lan_ip 172.2.2.9 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.2.2.41 -rev 1
UTF::Power::Synaccess npc51 -lan_ip 172.2.2.51 -rev 1
UTF::Power::Synaccess npc61 -lan_ip 172.2.2.61 -rev 1
UTF::Power::WebRelay  web71 -lan_ip 172.2.2.71 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.2.2.91 \
	-relay "mc85end1" \
	-group {
		G1 {1 2 3} 
		G2 {4 5 6} 
		ALL {1 2 3 4 5 6}
	       }

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    catch {ALL attn 0;}
    #catch {G3 attn 0;}
    #catch {G4 attn 0;}

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
    catch {FC19SOFTAP unload}
    #catch {4360SNF1 unload}

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4331F15 4360X51F15 4352WIN8 MacX51 MacX28B FC19SOFTAP} {
            UTF::Try "$S Down" {
                    catch {$S wl down}
                    catch {$S deinit}
            }
    }
    # unset S so it doesn't interfere
    unset S

    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}

# Define Sniffer
# mc85snf1: 4360SNF1  (10.19.60.146, mc85end1:40005)
# 
UTF::Sniffer mc85snf1 -user root \
        -sta {4360SNF1 eth0} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc61 1} \
        -power_button {auto} \
        -console "mc85end1:40005"


# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc85end1 -lan_ip 10.19.60.93  \
    -sta {lan eth1} 

# KVM: mc85kvm2 (FC19SOFTAP)
# A Desktop DUT with FC19
# BCMs: 4360X51
UTF::Linux mc85kvm2 -sta {FC19SOFTAP enp1s0} \
    -power {npc61 2} \
    -console {mc85end1:40007} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07} {%S wl country} {%S wl curpower}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07} {%S wl country} {%S wl curpower}} \
    -tcpwindow 4M \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 1; wl assert_type 1; wl country US;:
    }
FC19SOFTAP configure -ipaddr 192.168.1.97 -attngrp G2 -hasdhcpd 1 -ap 1 \
-ssid testX51F15SAP

# Clones for mc85kvm2
FC19SOFTAP clone FC19SOFTAP-TOT -tag "trunk"
FC19SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -post_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 1; wl assert_type 1; wl country US;:
        }

# STA: mc85tst1
# A Desktop DUT with FC15
# BCMs: "Linux 4331hm P152"
# -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  
UTF::Linux mc85tst1 \
        -sta "4331F15 eth0" \
        -brand linux-internal-wl \
        -console "mc85end1:40001" \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl msglevel +scan ; wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} \
         {%S wl phy_cal_disable 1}} \
        -power "npc08 1"

# Clones of STA 4331F15 with Different Options for Test
#4331F15 clone 4331F15-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16"
#4331F15 clone 4331F15-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15"
4331F15 clone 4331F15-TOT -tag "trunk"
4331F15 clone 4331F15-DINGO2_9_15 -tag "DINGO2_BRANCH_9_15" 
4331F15 clone 4331F15-DINGO_9_10 -tag "DINGO_BRANCH_9_10" 
4331F15 clone 4331F15-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
4331F15 clone 4331F15-BIS120RC4REL -tag "BIS120RC4_REL_7_15_*" -custom 1
4331F15 clone 4331F15-BIS715REL -tag "BIS715GALA_REL_7_21_*" -custom 1
4331F15 clone 4331F15-EAGLE_10_10 -tag "EAGLE_BRANCH_10_10"
4331F15 clone 4331F15-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

# STA: mc85tst2
# A Desktop DUT with FC15
# BCMs: "Linux 4360mc P192"
#        -console "mc85end1:40002" 
UTF::Linux mc85tst2 \
        -sta "4360X51F15 eth0" \
        -brand linux-internal-wl \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl msglevel +scan ; wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} \
         {%S wl phy_cal_disable 1}} \
        -power "npc08 2"

# Clones of STA 4360X51F15 with Different Options for Test
#4360X51F15 clone 4360X51F15-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16"
#4360X51F15 clone 4360X51F15-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15"
4360X51F15 clone 4360X51F15-TOT -tag "trunk"
4360X51F15 clone 4360X51F15-DINGO2_9_15 -tag "DINGO2_BRANCH_9_15" 
4360X51F15 clone 4360X51F15-DINGO_9_10 -tag "DINGO_BRANCH_9_10" 
4360X51F15 clone 4360X51F15-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
4360X51F15 clone 4360X51F15-EAGLE_10_10 -tag "EAGLE_BRANCH_10_10"
4360X51F15 clone 4360X51F15-BIS715REL -tag "BIS715GALA_REL_7_21_*" -custom 1
4360X51F15 clone 4360X51F15-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

# STA: mc85tst3
# STA Laptop DUT Dell E4620 with Windows 8 OS
# BCMs: "4352hmb P405"
#        
UTF::Linux mc85tst3 -sta {4360F19 enp5s0} \
    -power {npc91 1} \
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

# Clones of STA 4360F19 with Different Options for Test
#4360F19 clone 4360F19-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16"
#4360F19 clone 4360F19-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15"
4360F19 clone 4360F19-TOT -tag "trunk"
4360F19 clone 4360F19-DINGO2_9_15 -tag "DINGO2_BRANCH_9_15" 
4360F19 clone 4360F19-DINGO_9_10 -tag "DINGO_BRANCH_9_10" 
4360F19 clone 4360F19-BIS715T254 -tag "BIS715T254_BRANCH_7_52"
4360F19 clone 4360F19-EAGLE_10_10 -tag "EAGLE_BRANCH_10_10"
4360F19 clone 4360F19-BIS715REL -tag "BIS715GALA_REL_7_21_*" -custom 1
4360F19 clone 4360F19-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1


# STA: mc85tst4 - MacBook Air 
# OS version: 10.8.3 12D22 (HD 2 1)
# Family-Drop version: 
# BCMs: "4360X29c P302"
#        
UTF::Power::Laptop X29dPower -button {web71 1}
UTF::MacOS mc85tst4 -user root -sta {MacX29C en0} \
        -power {X29dPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl txcore} {%S wl spatial_policy} {%S wl curpower} {%S wl nrate }} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl txcore} {%S wl spatial_policy} {%S wl curpower} {%S wl nrate }} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -kextload true 

# Clones for mc85tst4
MacX29C clone MacX29C-TOT -tag "trunk"
MacX29C clone MacX29C-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

# STA: mc85tst5 - MacBook Air 
# OS version: 13A411 (CAB 10.9 HD2/2)
# BCMs: "MacOS 4331X51 P700(X0)"
#        
UTF::Power::Laptop X51Power -button {web71 2}
UTF::MacOS mc85tst5 -user root -sta {MacX51 en0} \
        -power {X51Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0; \
         wl olpc_msg_lvl 7; wl assert_type 1 } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel} {%S wl PM}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl PM}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -perfchans {36/80 36l 3} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -kextload true 

# Clones for mc85tst5
MacX51 clone MacX51-TOT -tag "trunk"
MacX51 clone MacX51-BIS715GALA_BRANCH_7_21 -tag "BIS715GALA_BRANCH_7_21" -custom 1

# STA: mc85tst6 - MacBook Pro 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331X28b P103(X0)"
#        
UTF::Power::Laptop X28bPower -button {web71 3}
UTF::MacOS mc85tst6 -sta {MacX28B en1} \
        -power {X28bPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan ; wl btc_mode 0} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-ml" \
        -type Debug_10_8 \
        -coreserver AppleCore \
        -kextload true -datarate 0

# Clones for mc85tst6
MacX28B clone MacX28B-TOT -tag "trunk"

# Router (AP) section
# AP1
#Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "43311 eth2 4331-2g eth1" \
        -lan_ip 192.168.1.1 \
    -relay "mc85end1" \
    -lanpeer lan \
    -console "mc85end1:40003" \
    -power "npc41 1" \
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
                wl0_radio=1
                wl1_radio=1
                wl1_nbw_cap=0
                wl_msglevel=0x101
                wl0_ssid=mc85test43311-ant0
                wl1_ssid=mc85test43311-ant1
                wl0_channel=1
                wl1_channel=36
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                wl1_obss_coex=0
        }

# AP2
# 4706 AP with 4360 and 4331 cards
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.2 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc85end1:40004" \
    -power {npc51 1} \
    -tag "AARDVARK_BRANCH_6_30" \
    -nvram {
        # watchdog=3000 (default)
        lan_ipaddr=192.168.1.2
        lan1_ipaddr=192.168.2.2
        wl_msglevel=0x101
        wl0_ssid=4706/4331
        wl0_channel=1
        wl0_bw_cap=-1
        wl0_radio=0
        wl0_obss_coex=0
        wl1_ssid=4706/4360
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
    -sta {4706x/4360 eth2} \
    -brand linux26-external-vista-router-full-src \
    -perfonly 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1}

4706/4360 clone 4706/4360-TOT -tag "trunk" -brand linux26-internal-router
4706/4331 clone 4706/4331-TOT -tag "trunk" -brand linux26-internal-router
4706/4360 clone 4706/4360-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router
4706/4331 clone 4706/4331-BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router


# Used for RvRFastSweep.test
43311     configure -attngrp G1
4706/4360 configure -attngrp G1
4706/4331 configure -attngrp G1
4706/4360-TOT configure -attngrp G1
4706/4331-TOT configure -attngrp G1
4706/4360-BISON04T configure -attngrp G1
4706/4331-BISON04T configure -attngrp G1

#### ADD UTF::Q for this rig
#####
UTF::Q mc85


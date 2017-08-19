# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MC83 
#
####### Controller section:
# mc83end2: FC11 (10.19.60.83)
# subnet  : 10.19.60.xx/22
#
####### Router AP section:
# AP1: 43311 eth2         (mc83end2:40002) 
# AP2: 4706/4360 eth2     (mc83end2:40005)
#      4706/4331 eth1
# SOFTAP: FC19SOFTAP enp1s0 (mc83kvm1)
#
####### STA section:
# mc83tst1: MacX52c      (10.19.60.84)
# mc83tst2: 4360FC19STA eth0 (10.19.60.85, mc83end2:40001)
# mc83tst3: X238FC19 eth0 (10.19.60.86, mc83end2:40004)
# mc83tst4: MacX28B  en1  (10.19.60.87)
# mc83tst5: MacX29D  en0  (10.19.60.88)
# mc83tst6: MacX33   en0  (10.19.60.89)
# mc83kvm1: FC19SOFTAP   enp1s0  (10.19.60.91)
#
####### Sniffer section:
# mc83snf1: 4331SNF1 eth1 (10.19.60.90, mc83end2:40003)
#
####### Attenuator section:
# attn: 172.1.1.91
#

# Load Packages
#set ::env(UTFDPORT) 9977
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package require UTFD


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/mc83"

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
UTF::Power::Synaccess npc10 -lan_ip 172.1.1.10 -rev 1
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -rev 1
UTF::Power::Synaccess npc12 -lan_ip 172.1.1.12 -rev 1
UTF::Power::Synaccess npc13 -lan_ip 172.1.1.13 -rev 1
UTF::Power::Synaccess npc14 -lan_ip 172.1.1.14 -rev 1
UTF::Power::WebRelay  web15 -lan_ip 172.1.1.15 -invert 1
UTF::Power::WebRelay  web19 -lan_ip 172.1.1.19 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.1.1.91 \
	-relay "mc83end2" \
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
    foreach S {4360FC19STA X238FC19 MacX28B MacX29D MacX33} {
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
UTF::Sniffer mc83snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc13 1} \
        -power_button {auto} \
        -console "mc83end2:40003"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc83end2 -lan_ip 10.19.60.83  \
    -sta {lan eth1} 

# KVM: mc83kvm1 (FC19SOFTAP)
# A Desktop DUT with FC19
# BCMs: 4360X51
UTF::Linux mc83kvm1 -sta {FC19SOFTAP enp1s0} \
    -power {npc12 2} \
    -console {mc83end2:40002} \
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
FC19SOFTAP configure -ipaddr 192.168.1.197 -attngrp G2 -hasdhcpd 1 -ap 1 \
-ssid testX51F15SAP

# Clones for mc83kvm1
FC19SOFTAP clone FC19SOFTAP-TOT -tag "trunk"
FC19SOFTAP-TOT configure -ipaddr 192.168.1.197 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -post_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 1; wl assert_type 1; wl country US;:
        }

FC19SOFTAP clone FC19SOFTAP-JAGUAR_BRANCH_14_10 -tag "JAGUAR_BRANCH_14_10"
FC19SOFTAP-TOT configure -ipaddr 192.168.1.197 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAP -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -post_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 1; wl assert_type 1; wl country US;:
        }

FC19SOFTAP clone FC19SOFTAP-EAGLE_BRANCH_10_10 -tag "EAGLE_BRANCH_10_10"
FC19SOFTAP-TOT configure -ipaddr 192.168.1.197 -attngrp G2 \
    -hasdhcpd 1 -ap 1 -ssid testX51F15SAPDINGO2 -tcpwindow 4M \
    -pre_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -post_perf_hook {{%S wl pwrthrottle_state} {%S wl assoclist} {%S wl pktq_stats p:98:b8:e3:92:47:07}} \
    -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; \
        wl btc_mode 0; wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
        wl vht_features 1; wl assert_type 1; wl country US;:
        }

FC19SOFTAP clone FC19SOFTAP-IGUANA_BRANCH_13_10 -tag "IGUANA_BRANCH_13_10"
FC19SOFTAP-IGUANA_BRANCH_13_10 configure -ipaddr 192.168.1.97 -attngrp G2 \
        -hasdhcpd 1 -ap 1 -ssid FC19SOFTAPEAGLE -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl msglevel +scan; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1; wl country US/0;:
        }

# mc83kvm2: Laptop DUT Dell E4310 
# OS: Fedora Core 15
# BCMs: "Linux 4352hmb P335"
#
#    	-nvram_add {macaddr=00:90:4C:12:D0:03} 
#       -perfchans {36/80 36l 36 11u 3} 
#       -dhd_image "/projects/hnd/swbuild/build_linux/DHD_BRANCH_1_359/linux-internal-dongle-pcie/2016.4.14.0/release/bcm/host/dhd-msgbuf-pciefd-debug-3.11.1-200.fc19.x86_64/dhd.ko" 
#       -nvram "/projects/hnd/swbuild/build_linux/DIN2930R18_BRANCH_9_44/linux-external-dongle-pcie/2016.4.14.0/release/bcm/firmware/../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" 

UTF::DHD mc83kvm2 \
        -sta "4355C0FC19A eth0" \
        -hostconsole "mc83end2:40011" \
        -power "npc60 1" \
        -power_button "auto" \
     	-dhd_brand linux-internal-dongle-pcie \
     	-brand linux-external-dongle-pcie \
     	-driver dhd-msgbuf-pciefd-debug \
     	-dhd_tag DHD_BRANCH_1_359 \
     	-app_tag DHD_BRANCH_1_359 \
     	-tag DIN2930R18_BRANCH_9_44 \
     	-nvram "../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" \
        -type "4355c0-roml/config_pcie_release/rtecdc.bin" \
        -clm_blob "4355_kristoff.clm_blob" \
    	-nocal 1 -slowassoc 5 -iperfdaemon 0 \
    	-udp 800m -noaes 1 -notkip 1 \
    	-tcpwindow 2m \
    	-yart {-attn5g 16-95 -attn2g 48-95 -pad 26}

# Clones of STA 4355C0FC19A with Different Options for Test
4355C0FC19A clone 4355C0FC19A-TOT -tag "trunk"
4355C0FC19A clone 4355C0FC19A-EAGLE_BRANCH_10_10 -tag "EAGLE_BRANCH_10_10"

# mc83tst1: Laptop DUT Dell E4310 
# OS: Windows 8 32 bit
# BCMs: "Win8 4352hmb P338"
#
# -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  
UTF::Power::Laptop X87Power -button {web19 4}
UTF::MacOS mc83tst1 -sta {MacX52c en0} \
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

# Clones of STA MacX52c with Different Options for Test
MacX52c clone MacX52c-TOT -tag "trunk"
MacX52c clone MacX52c-BIS715GALA_REL_7_21_122 -tag "BIS715GALA_REL_7_21_122"

# mc83tst2: Laptop DUT Dell E4310 
# OS: Fedora Core 15
# BCMs: "Linux 4352hmb P335"
#
UTF::Linux mc83tst2 \
        -sta "4360FC19STA enp1s0" \
        -console "mc83end2:40001" \
        -tcpwindow 4M -custom 1 \
        -brand linux-internal-wl \
        -wlinitcmds {
		ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; \
		wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
		wl country US ; wl vht_features 3; wl assert_type 1; wl phymsglevel+cal;: \
		} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} \
         {%S wl phy_cal_disable 1}} \
        -power "npc11 1"

# Clones of STA 4360FC19STA with Different Options for Test
4360FC19STA clone 4360FC19STA-TOT -tag "trunk"
4360FC19STA clone 4360FC19STA-EAGLE_BRANCH_10_10 -tag "EAGLE_BRANCH_10_10"

# mc83tst3: Laptop DUT Dell E4310 
# OS: Fedora Core 15
# BCMs: "Linux 4331hm P152"
#
#UTF::DHD mc83tst3 
#        -sta "4355C0FC19 eth0" 
#        -console "mc83end2:40004" 
#        -tcpwindow 4M -custom 1 
#    	-dhd_brand linux-internal-dongle-pcie 
#    	-brand linux-external-dongle-pcie 
#    	-driver dhd-msgbuf-pciefd-debug 
#    	-dhd_tag DHD_BRANCH_1_359 
#    	-app_tag DHD_BRANCH_1_359 
#    	-tag DIN2930R18_BRANCH_9_44 
#    	-nvram "../../olympic/C-4355__s-C0/P-kristoff_M-PRNL_V-m__m-5.7.txt" 
#        -type "4355c0-roml/config_pcie_release/rtecdc.bin" 
#        -clm_blob "4355_kristoff.clm_blob" 
#        -power "npc11 2"

# Clones of STA 4355C0FC19 with Different Options for Test
#-type obj-debug-p2p-mchan 
UTF::Linux mc83tst3 \
        -sta "X238FC19 enp1s0" \
        -console "mc83end2:40004" \
        -brand linux-internal-wl \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {
		ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; \
		wl dtim 3; service dhcpd stop; wl mimo_bw_cap 1; \
		wl country US ; wl vht_features 3; wl assert_type 1; wl phymsglevel+cal;: \
		} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} \
         {%S wl phy_cal_disable 1}} \
        -power "npc11 2"

# Clones of STA X238FC19 with Different Options for Test
X238FC19 clone X238FC19-TOT -tag "trunk"
X238FC19 clone X238FC19-EAGLE_BRANCH_10_10 -tag "EAGLE_BRANCH_10_10"

# MC83tst4 - MacBook Pro 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331X28b P102(X0)"
#
UTF::Power::Laptop X28bPower -button {web19 1}
UTF::MacOS mc83tst4 -sta {MacX28B en1} \
        -power {X28bPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true 

# Clones for mc83tst4
MacX28B clone MacX28B-TOT -tag "trunk"
MacX28B clone MacX28B-BIS715GALA_REL_7_21_9x -tag "BIS715GALA_REL_7_21_9?"
MacX28B clone MacX28B-BIS715GALA_REL_7_21_94 -tag "BIS715GALA_REL_7_21_94_*"
MacX28B clone MacX28B-APPLREL -tag "BIS715CASCADE_BRANCH_7_21_*"
MacX28B clone MacX28B-APPLATEST -tag "BIS715CASCADE_BRANCH_7_21_*"


# MC83tst5 - MacBook Air 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331csdax P201(X0)"
#
UTF::Power::Laptop X29dPower -button {web19 2}
UTF::MacOS mc83tst5 -sta {MacX29D en0} \
        -power {X29dPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan +error +assoc +inform +wsec} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true 

# Clones for mc83tst5
#MacX29D clone MacX29D-BIS715T254_7_52 -tag "BIS715T254_BRANCH_7_52"
#MacX29D clone MacX29D-BIS120RC4REL -tag "BIS120RC4_REL_7_15_*" -custom 1
#MacX29D clone MacX29D-MOSTACT1 -tag "trunk"
#MacX29D clone MacX29D-MOSTACT2 -tag "DINGO_BRANCH_9_10"
#MacX29D clone MacX29D-MOSTACT3 -tag "BISON_BRANCH_7_10"
MacX29D clone MacX29D-TOT -tag "trunk"
MacX29D clone MacX29D-BIS715GALA_REL_7_21_9x -tag "BIS715GALA_REL_7_21_9?"
MacX29D clone MacX29D-BIS715GALA_REL_7_21_94 -tag "BIS715GALA_REL_7_21_94_*"

# MC83tst6 - MacBook Air 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331X33 P608(X0)"
#
UTF::Power::Laptop X33Power -button {web19 3}
UTF::MacOS mc83tst6 -sta {MacX33 en0} \
        -power {X33Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-syr" \
        -type Debug_10_10 \
        -coreserver AppleCore \
        -kextload true

# Clones for mc83tst6
#MacX33 clone MacX33-BIS715T254_7_52 -tag "BIS715T254_BRANCH_7_52"
#MacX33 clone MacX33-BIS120RC4REL -tag "BIS120RC4_REL_7_15_*" -custom 1
#MacX33 clone MacX33-MOSTACT1 -tag "trunk"
#MacX33 clone MacX33-MOSTACT2 -tag "DINGO_BRANCH_9_10"
#MacX33 clone MacX33-MOSTACT3 -tag "BISON_BRANCH_7_10"
MacX33 clone MacX33-TOT -tag "trunk"
MacX33 clone MacX33-BIS715GALA_REL_7_21_9x -tag "BIS715GALA_REL_7_21_9?"
MacX33 clone MacX33-BIS715GALA_REL_7_21_94 -tag "BIS715GALA_REL_7_21_94_*"

# Router (AP) section
# AP1
# Linksys E4200 4718/4331 Router AP1
#UTF::Router AP1 \
#    -sta "43311 eth2" \
#        -lan_ip 192.168.1.1 \
#    -relay "mc83end2" \
#    -lanpeer lan \
#    -console "mc83end2:40002" \
#    -power "npc10 2" \
#    -brand linux-external-router \
#    -tag "AKASHI_REL_5_110_43" \
#        -nvram {
#                boot_hw_model=E4200
#                wandevs=et0
#                {lan_ifnames=vlan1 eth1 eth2}
#                et0macaddr=00:90:4c:07:00:8c
#                macaddr=00:90:4c:07:00:9d
#                sb/1/macaddr=00:90:4c:07:10:00
#                pci/1/1/macaddr=00:90:4c:07:11:00
#                lan_ipaddr=192.168.1.1
#                lan_gateway=192.168.1.1
#                dhcp_start=192.168.1.100
#                dhcp_end=192.168.1.149
#                lan1_ipaddr=192.168.2.1
#                lan1_gateway=192.169.2.1
#                dhcp1_start=192.168.2.100
#                dhcp1_end=192.168.2.149
#                fw_disable=1
#                #Only 1 AP can serve DHCP Addresses
#                #router_disable=1
#                #simultaneous dual-band router with 2 radios
#                wl0_radio=0
#                wl1_radio=0
#                wl1_nbw_cap=0
#                wl_msglevel=0x101
#                wl0_ssid=test43311-ant0
#                wl1_ssid=test43311-ant1
#                wl0_channel=1
#                wl1_channel=1
#                # Used for RSSI -35 to -45 TP Variance
#                antswitch=0
#                # Used to WAR PR#86385
#                wl0_obss_coex=0
#                wl1_obss_coex=0
#        }
#
## Used for RvRFastSweep.test
#43311 configure -attngrp G1


UTF::Router 4709 -sta {4709/4360 eth2 4709/4360.%15 wl1.%} \
    -relay lan \
    -lan_ip 192.168.1.1 \
    -power {npc41 2} \
    -lanpeer lan \
    -console "mc83:40005" \
    -tag "BISON04T_REL_7_14_43_22" \
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

4709/4360 clone 4709/4360-TOT -tag "trunk" -brand linux-2.6.36-arm-internal-router
4709/4360-TOT configure -attngrp G1

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
    -console "mc83end2:40005" \
    -power {npc12 1} \
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

4706/4360 clone 4706/4360-TOT -tag "trunk" -brand linux26-internal-router
4706/4331 clone 4706/4331-TOT -tag "trunk" -brand linux26-internal-router
4706/4360 clone 4706/4360-BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
4706/4331 clone 4706/4331-BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
4706/4360 clone 4706/4360-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" \
    -brand linux26-internal-router
4706/4331 clone 4706/4331-BIS120RC4PHY -tag "BIS120RC4PHY_BRANCH_7_16" \
    -brand linux26-internal-router
4706/4360 clone 4706/4360-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" \
    -brand linux26-internal-router
4706/4331 clone 4706/4331-BIS120RC4 -tag "BIS120RC4_BRANCH_7_15" \
    -brand linux26-internal-router
4706/4360 clone 4706/4360-MOSTACT1 -tag "trunk" \
	-brand linux26-internal-router
4706/4360 clone 4706/4360-MOSTACT2 -tag "DINGO_BRANCH_9_10" \
	-brand linux26-internal-router
4706/4360 clone 4706/4360-MOSTACT3 -tag "BISON_BRANCH_7_10" \
	-brand linux26-internal-router
4706/4331 clone 4706/4331-MOSTACT1 -tag "trunk" \
	-brand linux26-internal-router
4706/4331 clone 4706/4331-MOSTACT2 -tag "DINGO_BRANCH_9_10" \
	-brand linux26-internal-router
4706/4331 clone 4706/4331-MOSTACT3 -tag "BISON_BRANCH_7_10" \
	-brand linux26-internal-router

# Used for RvRFastSweep.test
4706/4360 configure -attngrp G2
4706/4331 configure -attngrp G2
4706/4331-TOT configure -attngrp G2
4706/4360-TOT configure -attngrp G2
4706/4331-BISON configure -attngrp G2
4706/4360-BISON configure -attngrp G2
4706/4331-MOSTACT1 configure -attngrp G2
4706/4360-MOSTACT1 configure -attngrp G2
4706/4331-MOSTACT2 configure -attngrp G2
4706/4360-MOSTACT2 configure -attngrp G2
4706/4331-MOSTACT3 configure -attngrp G2
4706/4360-MOSTACT3 configure -attngrp G2

#### ADD UTF::Q for this rig
#####
UTF::Q mc83


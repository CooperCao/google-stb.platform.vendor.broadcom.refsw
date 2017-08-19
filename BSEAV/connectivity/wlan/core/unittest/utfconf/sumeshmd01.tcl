# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MC83 
#
####### Controller section:
# md01end1: FC15 (10.19.61.14)
# subnet  : 10.19.60.xx/22
#
####### Router AP section:
# AP1: 43311 eth2         (md01end1:40002) 
# AP2: 4706/4360 eth2     (md01end1:40005)
#      4706/4331 eth1
#
####### STA section:
# md01tst1: 4331WIN7      (10.19.61.16)
# md01tst2: 4352FC15 eth0 (10.19.61.17, md01end1:40001)
# md01tst3: 4331FC15 eth0 (10.19.60.86, md01end1:40004)
# md01tst4: MacX28B  en1  (10.19.60.87)
# md01tst5: MacX29D  en0  (10.19.60.88)
# md01tst6: MacX33   en0  (10.19.60.89)
#
####### Sniffer section:
# md01snf1: 4331SNF1 eth1 (10.19.60.90, md01end1:40003)
#
####### Attenuator section:
# attn: 172.1.1.91
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/$::env(LOGNAME)/md01"

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
UTF::Power::Synaccess npc11 -lan_ip 172.6.6.11 -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.6.6.21 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.6.6.31 -rev 1
#UTF::Power::Synaccess npc14 -lan_ip 172.1.1.14 -rev 1
#UTF::Power::WebRelay  web19 -lan_ip 172.1.1.19 -invert 1

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.6.6.91 \
	-relay "md01end1" \
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
    catch {43311 restart wl0_radio=0}
    catch {43311 restart wl1_radio=0}
    catch {4706/4360 restart wl0_radio=0}
    catch {4706/4331 restart wl1_radio=0}
    #catch {47061/4360 restart wl0_radio=0}
    #catch {47061/4331 restart wl1_radio=0}
    # ensure sniffer is unloaded
    #catch {4331SNF1 unload}
    #catch {4360SNF1 unload}    

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4331WIN7 4352FC15 4331FC15 MacX28B MacX29D MacX33} {
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
UTF::Sniffer md01snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc13 1} \
        -power_button {auto} \
        -console "md01end1:40003"

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux md01end1 -lan_ip 10.19.61.14  \
    -sta {lan eth1} 

# md01tst1: Laptop DUT Dell E4310 
# OS: Windows 8 32 bit
# BCMs: "Win8 4352hmb P338"
#
# -yart {-attn "10-75" -mirror -frameburst 1 -b 1.2G}  

UTF::Cygwin md01tst1 -user user \
	-sta {4331WIN7} -lan_ip 10.19.61.16  \
	-osver 7 \
	-installer inf \
	-allowdevconreboot 1 \
	-tcpwindow 4M -custom 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -power "npc31 2" \
	-brand win_internal_wl \
        -post_perf_hook {{%S wl dump ampdu}} 

# Clones of STA 4331WIN7 with Different Options for Test
#                4331WIN7-AARDVARK -tag "AARDVARK_BRANCH_6_30"

# md01tst2: Laptop DUT Dell E4310 
# OS: Fedora Core 15
# BCMs: "Linux 4352hmb P335"
#
UTF::Linux md01tst2 \
        -sta "4360FC15 eth0" \
        -brand linux-internal-wl \
        -console "md01end1:40001" \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -power "npc11 2"

# Clones of STA 4360FC15 with Different Options for Test
#4360FC15 clone 4360FC15-AARDVARK -tag AARDVARK_BRANCH_6_30
4360FC15 clone 4360FC15-AARDVARK -tag AARDVARK_BRANCH_6_30 -perfchans {36/80 36l 3}

# md01tst3: Laptop DUT Dell E4310 
# OS: Fedora Core 15
# BCMs: "Linux 4331hm P152"
#
UTF::Linux md01tst3 \
        -sta "4331FC15 eth0" \
        -brand linux-internal-wl \
        -console "md01end1:40004" \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl msglevel +scan ; wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} \
         {%S wl phy_cal_disable 1}} \
        -power "npc14 1"

# Clones of STA 4331FC15 with Different Options for Test
#-type obj-debug-p2p-mchan 
4331FC15 clone 4331FC15-AARDVARK -tag AARDVARK_BRANCH_6_30

# MC83tst4 - MacBook Pro 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331X28b P102(X0)"
#
UTF::Power::Laptop X28bPower -button {web19 1}
UTF::MacOS md01tst4 -sta {MacX28B en1} \
        -power {X28bPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-ml" \
        -type Debug_10_8 \
        -coreserver AppleCore \
        -kextload true -datarate 0

# Clones for md01tst4
MacX28B clone MacX28B-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX28B clone MacX28B-KIRIN -tag KIRIN_REL_5_106_98_105
MacX28B clone MacX28B-RUBY -tag RUBY_BRANCH_6_20
MacX28B clone MacX28B-AARDVARK-REL223LATEST -tag AARDVARK_REL_6_30_223_* -custom 1


# MC83tst5 - MacBook Air 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331csdax P201(X0)"
#
UTF::Power::Laptop X29dPower -button {web19 2}
UTF::MacOS md01tst5 -sta {MacX29D en0} \
        -power {X29dPower} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan +error +assoc +inform +wsec} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-ml" \
        -type Debug_10_8 \
        -coreserver AppleCore \
        -kextload true -datarate 0

# Clones for md01tst5
MacX29D clone MacX29D-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX29D clone MacX29D-KIRIN -tag KIRIN_REL_5_106_98_105
MacX29D clone MacX29D-RUBY -tag RUBY_BRANCH_6_20
MacX29D clone MacX29D-AARDVARK-REL223LATEST -tag AARDVARK_REL_6_30_223_* -custom 1

# MC83tst6 - MacBook Air 
# OS version: 12C54 (ZIN 10.8.2)
# Family-Drop version: Zin/Caravel_511.6
# BCMs: "MacOS 4331X33 P608(X0)"
#
UTF::Power::Laptop X33Power -button {web19 3}
UTF::MacOS md01tst6 -sta {MacX33 en0} \
        -power {X33Power} \
        -power_button {auto} \
        -tcpwindow 3640K -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl msglevel}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
        -brand  "macos-internal-wl-ml" \
        -type Debug_10_8 \
        -coreserver AppleCore \
        -kextload true -datarate 0

# Clones for md01tst6
MacX33 clone MacX33-AARDVARK -tag AARDVARK_BRANCH_6_30
MacX33 clone MacX33-KIRIN -tag KIRIN_REL_5_106_98_105
MacX33 clone MacX33-RUBY -tag RUBY_BRANCH_6_20
MacX33 clone MacX33-AARDVARK-REL223LATEST -tag AARDVARK_REL_6_30_223_* -custom 1

# Router (AP) section
# AP1
# Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "43311 eth2" \
        -lan_ip 192.168.1.1 \
    -relay "md01end1" \
    -lanpeer lan \
    -console "md01end1:40002" \
    -power "npc11 1" \
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
                wl0_ssid=test43311-ant0
                wl1_ssid=test43311-ant1
                wl0_channel=1
                wl1_channel=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
                # Used to WAR PR#86385
                wl0_obss_coex=0
                wl1_obss_coex=0
        }

# Used for RvRFastSweep.test
43311 configure -attngrp G1
43311 clone 43311ARV -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router

# AP2 Netgear R6300
# 4706 AP with 4360 and 4331 cards
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.% 
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "md01end1:40005" \
    -power {npc21 2} \
    -tag "AARDVARK_BRANCH_6_30" \
    -nvram {
	# watchdog=3000 (default)
	lan_ipaddr=192.168.1.1
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

4706/4360 clone 4706/4360ARV -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router
4706/4331 clone 4706/4331ARV -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router
4706/4360 clone 4706/4360TOT -tag "NIGHTLY" -brand linux26-internal-router

# Used for RvRFastSweep.test
4706/4360 configure -attngrp G2
4706/4331 configure -attngrp G2

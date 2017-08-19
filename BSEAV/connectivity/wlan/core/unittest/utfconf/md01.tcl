# -*-tcl-*-
#
# Testbed configuration file for Frank Fang's MD01 
#
####### Controller section:
# md01end1: FC19 (10.19.61.14)
# subnet  : 10.19.61.xx/22
#
####### Router AP section:
# AP1: 43311 eth2         (md01end1:40002) 
# AP2: 4706/4360 eth2     (md01end1:40005)
#      4706/4331 eth1
#
####### STA section:
# md01tst1: WgDUT1  FC19  (10.19.61.16)
# md01tst2: WgDUT2  FC19  (10.19.61.17)
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
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/md01"

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
UTF::Power::Synaccess npc41 -lan_ip 172.6.6.41 -rev 1

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
    #catch {ALL attn 0;}
    #catch {G3 attn 0;}
    #catch {G4 attn 0;}

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
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

UTF::DHD md01tst1 \
	-sta "WgDUT1 eth1" \
        -driver dhd-msgbuf-pciefd-dmg-debug \
        -dhd_image "/projects/hnd_sig_img1/ffang/build/BCM20130A0/BCM20130A0_MI_001.001.079/DHD/dhd.ko" \
        -type "/projects/hnd_sig_img1/ffang/build/BCM20130A0/BCM20130A0_MI_001.001.079/A_20130A0/20130.bin" \
	-tcpwindow 4M -custom 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -power "npc41 1" \
	-brand linux-internal-dongle-pcie \
        -post_perf_hook {{%S wl dump ampdu}} 


# md01tst2: Laptop DUT Dell E4310 
# OS: Fedora Core 15
# BCMs: "Linux 4352hmb P335"
#
UTF::DHD md01tst2 \
        -sta "WgDUT2 eth1" \
        -driver dhd-msgbuf-pciefd-dmg-debug \
        -dhd_image "/projects/hnd_sig_img1/ffang/build/BCM20130A0/BCM20130A0_MI_001.001.079/DHD/dhd.ko" \
        -type "/projects/hnd_sig_img1/ffang/build/BCM20130A0/BCM20130A0_MI_001.001.079/A_20130A0/20130.bin" \
        -tcpwindow 4M -custom 1 \
        -wlinitcmds {wl msglevel 0x101 ; \
         wl mpc 0; service dhcpd stop;:} \
        -post_perf_hook {{%S wl dump ampdu} \
         {%S wl phy_cal_disable 0}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -power "npc41 2"

# FC19 Linux PC
UTF::Linux md01tst3 -sta {4360STA enp1s0} \
    -power {npc42 1} \
    -console {md01end1:40004} \
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
    	{%S wl phy_cal_disable 1}} 

# Clones of STA X238FC19 with Different Options for Test
4360STA clone 4360STA-TOT -tag "NIGHTLY"

# FC19 Linux PC
UTF::Linux md01tst4 -sta {4360STA2 enp1s0} \
    -power {npc42 2} \
    -console {md01end1:40004} \
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
    	{%S wl phy_cal_disable 1}} 

# Clones of STA X238FC19 with Different Options for Test
4360STA2 clone 4360STA2-TOT -tag "NIGHTLY"

# Router (AP) section
# FC19 Linux PC
UTF::Linux md01kvm1 -sta {4360SOFTAP enp1s0} \
    -power {npc42 2} \
    -console {md01end1:40005} \
    -slowassoc 5 -reloadoncrash 1 \
    -nobighammer 1 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M 

4360SOFTAP clone 4360SOFTAP-TOT -tag NIGHTLY -brand "linux-internal-wl"
4360SOFTAP-TOT configure -ipaddr 192.168.1.97 -attngrp G1 \
        -hasdhcpd 1 -ap 1 -ssid performance -tcpwindow 4M \
        -wlinitcmds {
        ifdown eth0; wl msglevel 0x101; wl mpc 0; wl btc_mode 0; wl dtim 3; \
        service dhcpd stop; wl mimo_bw_cap 1; wl vht_features 1; wl assert_type 1;:
        }

#### ADD UTF::Q for this rig
#####
UTF::Q md01

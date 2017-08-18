# -*-tcl-*-
#
# Testbed configuration file for Milosz Zielinski's MC98 
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer


# UTFD support
set ::env(UTFDPORT) 9978
package require UTFD


UTF::Linux xlinux

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc98"

# Enable Windows TestSigning
set UTF::TestSigning 1

# Define power controllers on cart
UTF::Power::Synaccess npc21 -lan_ip 172.5.5.21 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.5.5.41 -rev 1
UTF::Power::Synaccess npc71 -lan_ip 172.5.5.71 -rev 1

#UTF::Power::WebRelay  web190 -lan_ip 192.168.1.90 -invert 1

# Attenuator - Aeroflex
#                ALL {1 2 3 4 5 6 7 8 9 10 11 12}
UTF::Aeroflex af -lan_ip 172.5.5.91 \
	-relay "mc98end1" \
	-group {
		G1 {1 2 3} 
		G2 {4 5 6} 
		G3 {7 8 9} 
		G4 {10 11 12} 
                ALL {1 2 3 7 8 9}
	       }

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    catch {ALL attn 0;}
    catch {G1 attn 0;}
    catch {G2 attn 0;}
    catch {G3 attn 13;}
    catch {G4 attn 0;}

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    catch {4331-a restart wl0_radio=0}
    catch {4331-a restart wl1_radio=0}
    catch {4706/4360 restart wl0_radio=0}
    catch {4706/4331 restart wl1_radio=0}
    # ensure sniffer is unloaded
    #catch {4331SNF1 unload}
    #catch {4360SNF1 unload}    

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S {4331fc15 W8X64 } {
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


if {0} {
# Define Sniffer
UTF::Sniffer mc98snf1 -user root \
        -sta {4331SNF1 eth1} \
        -tag AARDVARK_BRANCH_6_30 \
        -power {npc13 1} \
        -power_button {auto} \
        -console "mc98end1:40001"
}

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc98end1 -lan_ip 10.22.20.123  \
    -sta {lan eth1} 

# UTF Endpoint2 FC11 - Traffic generators (no wireless cards)
#UTF::Linux mc98end2 -lan_ip 10.22.20.124  \
#    -sta {lan eth1} 

# mc98tst1: Desktop DUT
# OS: Windows FC15
# BCMs: "Linux 4331hm P152"
#
UTF::Linux mc98tst1 \
        -sta "4331fc15 eth0" \
        -console "mc98end1:40004" \
        -wlinitcmds {wl mpc 0; service dhcpd stop;:} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} } \
        -power "npc41 1"



# Clones of STA 4331FC15 with Different Options for Test
#4331fc15 clone 4331fc15-AARDVARK -tag AARDVARK_BRANCH_6_30
#4331fc15 clone 4331fc15-637      -tag AARDVARK01T_BRANCH_6_37
4331fc15 clone 4331fc15-TOT
4331fc15 clone 4331fc15-TOTp2p   -type debug-p2p-mchan
4331fc15 clone 4331fc15-BISON    -tag BISON_BRANCH_7_10
4331fc15 clone 4331fc15-EAGLE    -tag EAGLE_BRANCH_10_10
4331fc15 clone 4331fc15-BISONp2p    -tag BISON_BRANCH_7_10 -type debug-p2p-mchan

# example clones
#4331fc15 clone 4331fc15-greg1 -tag BISON_BRANCH_7_10 -wlinitcmds {wl msglevel 0x101 ; wl msglevel +assoc ; wl mpc 0; service dhcpd stop;:}
#4331fc15 clone 4331fc15-greg2 -tag BISON_BRANCH_7_10 -wlinitcmds {wl msglevel 0x101 ; wl msglevel +scan ; wl mpc 0; wl rssi_win 0x208; service dhcpd stop;:}
#4331fc15 clone 4331fc15-greg3 -tag BISON_BRANCH_7_10 -wlinitcmds {wl msglevel 0x101 ; wl msglevel +assoc ; wl mpc 0; wl rssi_win 0x208; service dhcpd stop;:}

         
4331fc15 clone 4331fc15-EAGLE-JQ-chansweep    -tag EAGLE_BRANCH_10_10 \
         -channelsweep {
                -chanspec { 36 40 44 48 52 56 60 64 100 104 108 112 116 132 136 140 149 153 157 161 165
                            40u 48u 56u 64u 104u 112u 136u 153u 161u 36l 44l 52l 60l 100l 108l 132l 149l 157l
                }
        }


4331fc15 clone 4331fc15-iperfhooks-BISON -tag BISON_BRANCH_7_10 -post_perf_hook {   \
                {%S wl status} \
                {%S wl ver} \
                {%S wl revinfo} \
                {%S wl ampdu} \
                {%S wl interference} \
                {%S wl btc_mode} \
                {%S wl phy_watchdog} \
                {%S wl dump ampdu} \
                {%S wl counters} \
                {%S wl dump rssi} \
                {%S wl rxchain} \
                {%S wl txchain} \
                {%S wl txcore} \
                {%S wl phy_tempsense} \
                {%S wl scansuppress} \
                {%S wl PM} \
                {%S wl msglevel} \
                {%S wl phy_cal_disable 0}} \
    -pre_perf_hook {    \
                    {%S wl status} \
                    {%S wl ver} \
                    {%S wl revinfo} \
                    {%S wl ampdu} \
                    {%S wl interference} \
                    {%S wl btc_mode} \
                    {%S wl phy_watchdog} \
                    {%S wl ampdu_clear_dump} \
                    {%S wl dump ampdu} \
                    {%S wl counters} \
                    {%S wl dump rssi} \
                    {%S wl rxchain} \
                    {%S wl txchain} \
                    {%S wl txcore} \
                    {%S wl phy_tempsense} \
                    {%S wl scansuppress} \
                    {%S wl PM} \
                    {%S wl phy_cal_disable 1}}



# mc98tst3: Laptop DUT Dell E4310 
# OS: Windows 8 x64 build 9200
# BCMs: "Linux 4331hm P152"
# power cycled on 2015.5.27 due to no ping
# 2015.7.02 update installed on 2015.7.08
UTF::Cygwin mc98tst3 \
        -osver 864 \
        -user user \
        -installer inf \
        -lan_ip mc98tst3 \
        -sta {W8X64} \
        -sign true   \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -power "npc41 2"

W8X64 clone W8X64-TOT
#W8X64 clone W8X64-AARD-637  -tag AARDVARK01T_BRANCH_6_37
#W8X64 clone W8X64-AARD-TOB  -tag AARDVARK_BRANCH_6_30
W8X64 clone W8X64-BISON     -tag BISON_BRANCH_7_10
W8X64 clone W8X64-BISON735  -tag BISON05T_BRANCH_7_35
W8X64 clone W8X64-BISON735105  -tag BISON05T_TWIG_7_35_105
W8X64 clone W8X64-x86-TOT   -osver 8


# Router (AP) section
# AP1
# Linksys E4200 4718/4331 Router AP1
UTF::Router AP1 \
    -sta "4331-a eth2" \
    -lan_ip 192.168.1.3 \
    -relay "mc98end1" \
    -lanpeer lan \
    -console "mc98end1:40002" \
    -power "npc21 1" \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_65"  \
        -nvram {
                boot_hw_model=E4200
                wandevs=et0
                {lan_ifnames=vlan1 eth1 eth2}
                et0macaddr=00:90:4c:07:00:8c
                macaddr=00:90:4c:07:00:9d
                sb/1/macaddr=00:90:4c:07:10:00
                pci/1/1/macaddr=00:90:4c:07:11:00
                lan_ipaddr=192.168.1.3
                lan_gateway=192.168.1.3
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
4331-a clone 43311ARV -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router

# AP Section 
# AP2: Netgear R6300/4706/4360 11ac 3x3 wireless router
UTF::Router 4706 -sta {
    4706/4360 eth2 4706/4360.%15 wl0.%
    4706/4331 eth1
    } \
    -lan_ip 192.168.1.1 \
    -relay lan \
    -lanpeer lan \
    -brand linux26-internal-router \
    -console "mc98end1:40003" \
    -power {npc71 1} \
    -tag "BISON04T_BRANCH_7_14" \
    -nvram {
        # watchdog=3000 (default)
        lan_ipaddr=192.168.1.1
        lan1_ipaddr=192.168.2.1
        wl_msglevel=0x101
        wl0_ssid=4706/4360
        wl0_channel=1
        wl0_bw_cap=-1
        wl0_radio=0
        wl0_obss_coex=0
        wl1_ssid=4706/4331-mc98
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


4706/4360 clone 4360ARV -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router
4706/4360 clone 4360_01T -tag "AARDVARK01T_TWIG_6_37_14" -brand linux26-internal-router
4706/4360 clone 4360BISON -tag "BISON_BRANCH_7_10" -brand linux26-internal-router
4706/4360 clone 4360BISON04T -tag "BISON04T_BRANCH_7_14" -brand linux26-internal-router


#set ::UTFD::norestore 1


set ::UTFD::intermediate_sta_list(0)  4331fc15-TOT
set ::UTFD::intermediate_sta_OS(0)    "FC15"
set ::UTFD::intermediate_last_svn(0)  0
set ::UTFD::intermediate_sta_list(1)  W8X64-TOT
set ::UTFD::intermediate_sta_OS(1)    Win8x
set ::UTFD::intermediate_last_svn(1)  0
set ::UTFD::intermediate_sta_list(2)  4331fc15-EAGLE
set ::UTFD::intermediate_sta_OS(2)    "FC15"
set ::UTFD::intermediate_last_svn(2)  0
set ::UTFD::intermediate_sta_list(3)  W8X64-BISON735
set ::UTFD::intermediate_sta_OS(3)    Win8x
set ::UTFD::intermediate_last_svn(3)  0
set ::UTFD::intermediate_ap           "4706/4360"
set ::UTFD::intermediate_ap_name      "4360 AP"
set ::UTFD::max_STAindex_count        4
set ::UTFD::rigname                   "mc98"
set ::UTFD::driver_build_path         "/projects/hnd_sig_ext13/milosz/testbuildlist"
set ::UTFD::max_testbuild_age         48
set ::UTFD::svn_path_base             http://svn.sj.broadcom.com/svn/wlansvn/proj



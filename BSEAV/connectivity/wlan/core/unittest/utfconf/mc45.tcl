# -*-tcl-*-
#
# Testbed configuration file for Milosz Zielinski's MC45 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package require UTF::TclReadLines


set ::env(UTFDPORT) 9978
package require UTFD

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results

set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc45"

# Enable Windows TestSigning
set UTF::TestSigning 1

# Define power controllers on cart
UTF::Power::Synaccess npc45 -lan_ip 172.3.1.45
UTF::Power::Synaccess npc55 -lan_ip 172.3.1.55
UTF::Power::Synaccess npc65 -lan_ip 172.3.1.65
UTF::Power::Synaccess npc75 -lan_ip 172.3.1.75
UTF::Power::Synaccess npc85 -lan_ip 172.3.1.85

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.3.1.200 \
    -relay "mc45end1" \
    -group {G1 {1 2 3} G2 {4 5 6} ALL {1 2 3 4 5 6}}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    ALL attn 0;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    #AP1 restart wl0_radio=0
    #AP2 restart wl0_radio=0
    #AP5 restart wl0_radio=0

    
    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# Define Sniffer
UTF::Sniffer mc45snf1 -user root \
        -sta {43224SNF1 eth1} \
        -tag BASS_BRANCH_5_60 \
        -power {npc85 1} \
        -power_button {auto}

# UTF Endpoint1 FC11 - Traffic generators (no wireless cards)
UTF::Linux mc45end1 \
    -sta {lan eth1} 

# UTF Endpoint2 FC11 - Traffic generators (no wireless cards)
#UTF::Linux mc45end2 \
#    -sta {lan1 eth1}



# STA Laptop DUT Dell E6400 FC15 (in top large Ramsey)
#    -wlinitcmds {sysctl -w net.ipv4.tcp_congestion_control=bic}
#    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}
UTF::Linux mc45tst1 -sta {4352FC15 eth0} \
    -tcpwindow 512k \
    -console "mc45end1:40007" \
    -power {npc45 1} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

# Clones for mc45tst1
4352FC15 clone 4352FC15-BISON -tag BISON_BRANCH_7_10
4352FC15 clone 4352FC15-EAGLE -tag EAGLE_BRANCH_10_10
  


if {0} {
# STA Laptop DUT Dell E6400 FC9
UTF::Linux mc45tst2 -sta {4313FC9 eth1} \
    -tcpwindow 512k \
    -console "mc45end1:40009" \
    -power {npc55 1} \
    -power_button {auto} \
    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}}     

# Clones for mc45tst2

4313FC9 clone 4313FC9-AARDVARK -tag AARDVARK_BRANCH_6_30
4313FC9 clone 4313FC9-BISON -tag BISON_BRANCH_7_10
}

# STA Laptop DUT Dell E6400 with 43228 (in top large Ramsey)
# -2014.12.3: Added BISON clone.
# Windows Update completed - 2015.7.1
UTF::Cygwin mc45tst3 -user user -sta {43228WIN7} \
    -osver 7 \
    -installer inf \
    -allowdevconreboot 1 \
    -tcpwindow 512k \
    -console "mc45end1:40004" \
    -power {npc45 2} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} 

# Clones for mc45tst3
#43228WIN7 clone 43228WIN7-AARDVARK -tag AARDVARK_BRANCH_6_30
43228WIN7 clone 43228WIN7-BISON    -tag BISON_BRANCH_7_10
43228WIN7 clone 43228WIN7-BISON735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400 (in bottom large Ramsey)
# scr2mc1kvm150 2
# 2015.4.14 update installed on 2015.4.16
# rebooted on 2015.5.4 to resolve adapter not found problems
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.21
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.7.01
# 2015.7.06 update installed on 2015.7.08
UTF::Cygwin mc45tst4 -user user -sta {4352WIN10} \
    -osver 10 \
    -sign 1 \
    -power {npc55 2} \
    -power_button {auto} \
    -kdpath kd.exe \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -brand win8x_internal_wl \
    -usemodifyos 1 \
    -node DEV_43B1 \
    -udp 300m

4352WIN10 clone 4352WIN10-BISON    -tag BISON_BRANCH_7_10
4352WIN10 clone 4352WIN10-BISON735 -tag BISON05T_BRANCH_7_35
4352WIN10 clone 4352WIN10-WDI-BISON735 -brand winthresh_internal_wl -tag BISON05T_BRANCH_7_35
4352WIN10 clone 4352WIN10-WDI-TOT      -brand winthresh_internal_wl

4352WIN10 clone 4352WIN10-BISON735-3l -tag BISON05T_BRANCH_7_35 \
        -perfchans {3l}


if {0} {
# Linksys 320N 4717/4322 wireless router AP1.
UTF::Router AP1 \
	-sta "47171 eth1" \
	-lan_ip 192.168.1.1 \
	-relay "mc45end1" \
	-lanpeer lan \
	-console "mc45end1:40001" \
	-power {npc65 1} \
	-brand linux-external-router \
	-tag "MILLAU_REL_5_70_48_*" \
	-nvram {
        et0macaddr=00:90:4c:03:00:8b
		macaddr=00:90:4c:03:00:9b
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
		fw_disable=1
		router_disable=1
		wl_msglevel=0x101
		wl0_ssid=test4717-1
		wl0_channel=1
		wl0_radio=0
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	}
	
# Clones for 47171
47171 clone 47171-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router
47171 clone 47171-AKASHI -tag "AKASHI_REL_5_110_*" -brand linux-internal-router
47171 clone 47171-AA -tag "AARDVARK_BRANCH_6_30" -brand linux-internal-router
47171 clone 47171-AA37-14 -tag "AARDVARK01T_BRANCH_6_37_14" -brand linux-internal-router
47171 clone 47171-BISON -tag "BISON_BRANCH_7_10" -brand linux-internal-router
}


if {0} {
#    -tag "AKASHI_BRANCH_5_110_*" 
#    -brand linux26-external-vista-router-combo 
# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
    -sta "47172 eth1" \
    -lan_ip 192.168.1.2 \
    -relay "mc45end1" \
    -lanpeer lan \
    -power {npc75 1} \
    -console "mc45end1:40003" \
    -brand "linux-external-router-combo" \
    -tag "MILLAU_REL_5_70_48_*" \
	-nvram {
        et0macaddr=00:90:4c:04:00:8b
		macaddr=00:90:4c:04:00:9b
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
		fw_disable=1
		#router_disable=1
		wl_msglevel=0x101
		wl0_ssid=test4717-2
		wl0_channel=1
		wl0_radio=0
                wl0_plc=1
                wl_plc=1
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	}
# Clones for 47172
47172 clone 47172-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router

# Clones for 5358P300
#47172 clone 5358P300 -tag "AKASHI_BRANCH_5_110_*" -brand linux26-external-vista-router-combo -nvram {wl0_plc=1 wl_plc=1}
}


if {0} {
# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
    -sta "47172 eth1" \
    -lan_ip 192.168.1.2 \
    -relay "mc45end1" \
    -lanpeer lan \
    -console "mc45end1:40003" \
    -power {npc75 1} \
    -brand linux-external-router \
    -tag "MILLAU_REL_5_70_48_*" \
	-nvram {
        et0macaddr=00:90:4c:02:00:8b
		macaddr=00:90:4c:02:00:9b
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	        dhcp1_end=192.168.2.199
		fw_disable=1
#		router_disable=1
		wl_msglevel=0x101
		wl0_ssid=test4717-2
		wl0_channel=1
		wl0_radio=0
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	}
# Clones for 47172
47172 clone 47172-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router
}



# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
    -sta "47172 eth1" \
    -lan_ip 192.168.1.2 \
    -relay "mc45end1" \
    -lanpeer lan \
    -console "mc45end1:40003" \
    -power {npc75 1} \
    -brand linux-external-router \
    -tag "MILLAU_REL_5_70_48_*" \
	-nvram {
        et0macaddr=00:90:4c:02:00:8b
		macaddr=00:90:4c:02:00:9b
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
  		dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.150
	        dhcp1_end=192.168.2.199
		fw_disable=1
#		router_disable=1
		wl_msglevel=0x101
		wl0_ssid=test4717-2
		wl0_channel=1
		wl0_radio=0
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
	    # Used to WAR PR#86385
		wl0_obss_coex=0
	}
# Clones for 47172
47172 clone 47172-MILLAU -tag "MILLAU_REL_5_70_*" -brand linux-internal-router



set ::UTFD::intermediate_sta_OS(0)    FC15
set ::UTFD::intermediate_sta_OS(1)    FC15
set ::UTFD::intermediate_sta_OS(2)    Win8x
set ::UTFD::intermediate_sta_OS(3)    Win8x
set ::UTFD::intermediate_last_svn(0)  0
set ::UTFD::intermediate_last_svn(1)  0
set ::UTFD::intermediate_last_svn(2)  0
set ::UTFD::intermediate_last_svn(3)  0
set ::UTFD::intermediate_sta_list(0)  4352FC15
set ::UTFD::intermediate_sta_list(1)  4352FC15-EAGLE
set ::UTFD::intermediate_sta_list(2)  4352WIN10
set ::UTFD::intermediate_sta_list(3)  4352WIN10-BISON735
set ::UTFD::intermediate_ap           "47172"
set ::UTFD::intermediate_ap_name      "4717 AP"
set ::UTFD::max_STAindex_count        4
set ::UTFD::rigname                   "mc45"
set ::UTFD::driver_build_path         "/projects/hnd_sig_ext13/milosz/testbuildlist"
set ::UTFD::max_testbuild_age         48
set ::UTFD::svn_path_base             http://svn.sj.broadcom.com/svn/wlansvn/proj



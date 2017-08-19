# -*-tcl-*-
#
# Testbed configuration file for Milosz Zielinski's MC30 
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::DHD


set ::env(UTFDPORT) 9978
package require UTFD


UTF::Linux xlinux

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc30"

# Define power controllers on cart
UTF::Power::Synaccess npc45 -lan_ip 172.2.1.45
UTF::Power::WebRelay  web46 -lan_ip 172.2.1.46
UTF::Power::Synaccess npc55 -lan_ip 172.2.1.55
UTF::Power::Synaccess npc65 -lan_ip 172.2.1.65
#UTF::Power::Synaccess npc75 -lan_ip 172.2.1.75
#UTF::Power::Synaccess npc85 -lan_ip 172.2.1.85

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.2.1.200 \
	-relay "mc30end1" \
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
    AP1 restart wl0_radio=0
#    AP2 restart wl0_radio=0

    
    # delete myself
    unset ::UTF::SetupTestBed

    return
}

# Define Sniffer
UTF::Sniffer mc30snf1 -user root \
        -sta {4322SNF1 eth1} \
        -tag BASS_BRANCH_5_60 \
        -power {npc85 1} \
        -power_button {auto}

# UTF Endpoint1 FC11 - Traffic generator (no wireless cards)
UTF::Linux mc30end1 -lan_ip 10.22.23.167  \
    -sta {lan eth1} 



# STA Laptop DUT Dell E6400 with 43236 USB dongle
#  -per Tim on 2015.4.8, 43236 is not officially supported on trunk due to lack of memory
#  failed load on 2015.5.7 on trunk.  2015.5.6 works, so likely due to lack of memory, per Tim
# 2015.6.24 - Windows update completed except for Internet Explorer
UTF::WinDHD mc30tst1 -user user -sta {43236WIN7} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -allowdevconreboot 1 \
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -power_sta {web46 1} \
    -power {npc45 1} \
    -power_button {auto} \
    -console "mc30end1:40007" \
    -alwayspowercycledongle 0 \
    -noafterburner 1 \
    -hack 0 \
    -debuginf 1 \
    -embeddedimage 43236 \
    -post_perf_hook {{%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} 


# Clones for 43236WIN7
43236WIN7 clone 43236WIN7-TOT
43236WIN7 clone 43236WIN7-BISON735 -tag BISON05T_BRANCH_7_35
#43236WIN7 clone 43236WIN7-AARDVARK -tag AARDVARK_BRANCH_6_30


UTF::Linux mc30tst2 -sta "43142FC15 eth0" \
        -console "mc30end1:40009" \
        -wlinitcmds {wl mpc 0; service dhcpd stop;:} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -pre_perf_hook {{%S wl ampdu_clear_dump} } \
        -post_perf_hook {{%S wl dump ampdu} {%S wl msglevel} } \
        -power "npc55 1"

43142FC15 clone 43142FC15-TOT
43142FC15 clone 43142FC15-EAGLE -tag EAGLE_BRANCH_10_10


# STA Laptop DUT Dell E640 with 43224NIC now replaced with 43237 P.305
UTF::Cygwin mc30tst3 -user user -sta {43227WIN7} \
    -osver 7 \
    -installer inf \
    -allowdevconreboot 1 \
    -tcpwindow 512k \
    -power {npc45 2} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}   
    
# Clones for mc30tst3
43227WIN7 clone 43227WIN7-AARDVARK -tag AARDVARK_BRANCH_6_30
43227WIN7 clone 43227WIN7-BISON -tag BISON_BRANCH_7_10
43227WIN7 clone 43227WIN7-BISON735 -tag BISON05T_BRANCH_7_35
43227WIN7 clone 43227WIN7-RUBY -tag RUBY_BRANCH_6_20
43227WIN7 clone 43227WIN7-KIRIN -tag KIRIN_BRANCH_5_100
43227WIN7 clone 43227WIN7-KIRIN-REL -tag KIRIN_REL_5_100_*
43227WIN7 clone 43227WIN7-KIRIN-REL-TAG -tag KIRIN_REL_5_100_82_*
43227WIN7 clone 43227WIN7-BASS -tag BASS_BRANCH_5_60 -nocal 1
43227WIN7 clone 43227WIN7-BASS-REL -tag BASS_REL_5_60_48_35 -nocal 1   



# STA Laptop DUT Dell E6400 with 43228
# bottom large Ramsey
# cleaned out driver and re-loaded by hand to get going - 2015.5.26
# 2015.7.1 - Windows update: all required are installed
UTF::Cygwin mc30tst4 -user user -sta {43228WIN7} \
    -osver 7 \
    -installer inf \
    -allowdevconreboot 1 \
    -tcpwindow 512k \
    -power {npc55 2} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} 

# Clones for mc30tst4
43228WIN7 clone 43228WIN7-BISON735 -tag BISON05T_BRANCH_7_35
43228WIN7 clone 43228WIN7-TOT
    
    
    
# Linksys 320N 4717/4322 wireless router AP1.
UTF::Router AP1 \
	-sta "47171 eth1" \
	-lan_ip 192.168.1.1 \
	-relay "mc30end1" \
	-lanpeer lan \
	-console "mc30end1:40001" \
	-power {npc65 1} \
	-brand linux-external-router \
	-tag "MILLAU_REL_5_70_48_*" \
	-nvram {
        et0macaddr=00:90:4c:01:00:8b
		macaddr=00:90:4c:01:00:9b
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	        dhcp1_end=192.168.2.149
		fw_disable=1
		#router_disable=1
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
	
if {0} {
#  removed device - JGP - 2015.1.26
# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
    -sta "47172 eth1" \
	-lan_ip 192.168.1.2 \
    -relay "mc30end1" \
    -lanpeer lan \
    -console "mc30end1:40003" \
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
		router_disable=1
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


set ::UTFD::intermediate_sta_OS(0)    Win8x
set ::UTFD::intermediate_sta_OS(1)    Win8x
set ::UTFD::intermediate_sta_OS(2)    Win8x
set ::UTFD::intermediate_sta_OS(3)    Win8x
set ::UTFD::intermediate_last_svn(0)  0
set ::UTFD::intermediate_last_svn(1)  0
set ::UTFD::intermediate_last_svn(2)  0
set ::UTFD::intermediate_last_svn(3)  0
set ::UTFD::intermediate_sta_list(0)  43227WIN7
set ::UTFD::intermediate_sta_list(1)  43227WIN7-BISON735
set ::UTFD::intermediate_sta_list(2)  43228WIN7
set ::UTFD::intermediate_sta_list(3)  43228WIN7-BISON735
set ::UTFD::intermediate_ap           "47171"
set ::UTFD::intermediate_ap_name      "4717 AP"
set ::UTFD::max_STAindex_count        0
set ::UTFD::rigname                   "mc30"
set ::UTFD::driver_build_path         "/projects/hnd_sig_ext13/$::env(LOGNAME)/testbuildlist"
set ::UTFD::max_testbuild_age         48



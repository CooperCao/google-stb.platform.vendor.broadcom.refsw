# -*-tcl-*-
#
# Testbed configuration file for Milosz Zielinski's MC29 testbed

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
# set the following 2 lines for doing UDP Traffic over iperf
#package require UTF::Multiperf
#set UTF::controlchart_iperf "--udp -b 200M -l 1024"
#package require UTF::TclReadLines


set ::env(UTFDPORT) 9978
package require UTFD
# Needed for Multiple STA and AP tests (in progress
#set ::sta_list "4312Vista 4322Vista 4312XP 4313WIN7 4312XP"
#set ::ap_list "47171 47172"


UTF::Linux xlinux

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc27"

# Enable Windows TestSigning
set UTF::TestSigning 1

# Define power controllers on cart
UTF::Power::Synaccess npc35 -lan_ip 172.1.1.35
#UTF::Power::Synaccess npc36 -lan_ip 172.1.1.36
UTF::Power::Synaccess npc45 -lan_ip 172.1.1.45
#UTF::Power::Synaccess npc46 -lan_ip 172.1.1.46
UTF::Power::Synaccess npc55 -lan_ip 172.1.1.55
UTF::Power::Synaccess npc65 -lan_ip 172.1.1.65
UTF::Power::Synaccess npc75 -lan_ip 172.1.1.75

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.1.1.200 \
	-relay "mc27end1" \
	-group {G1 {1 2 3} G2 {4 5 6}}

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    G2 attn 0
    
    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    AP1 restart wl0_radio=0
#    AP2 restart wl0_radio=0

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S { 4352WIN81  43142WIN8x64 43142WIN81  43228WIN7} {
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
UTF::Sniffer mc27snf1 -user root \
        -sta {43224SNF1 eth1} \
        -tag BASS_BRANCH_5_60 \
        -power {npc75 1} \
        -power_button {auto}

# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
# moved hard drive to a different system - 2015.4.23
UTF::Linux mc27end1 \
    -sta {lan eth1} 


# STA Laptop DUT DELL E6400 with 4352 - Windows 8.1 (build 9600)
#        -wlinitcmds {wl msglevel 0x101; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 0}
#        -datarate {-b 1.2G -i 0.5 -frameburst 1} 
#        -power_button {auto}
#        -perfchans {1 2 3 4 5 6 7 }
# 2015.3.31 update installed on 2015.4.15
# 2015.7.02 update installed on 2015.7.08
UTF::Cygwin mc27tst1 -user user -sta {4352WIN81} \
        -osver 81 \
        -sign true \
        -installer  InstallDriver \
        -lan_ip mc27tst1 \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -power "npc35 1"

4352WIN81 clone 4352WIN81-TOT
4352WIN81 clone 4352WIN81-BISON     -tag BISON_BRANCH_7_10
4352WIN81 clone 4352WIN81-BISON735  -tag BISON05T_BRANCH_7_35


# STA Laptop DUT DELL E6400 with 43142 - Win8 x64
# 2015.3.31 update installed on 2015.4.17
# 2015.7.02 update installed on 2015.7.08
UTF::Cygwin mc27tst2 -user user -sta {43142WIN8x64} \
        -osver 864 \
        -installer  InstallDriver \
        -sign true   \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -power "npc45 2" \
        -perfchans {1 2 3 4 5 6 7}

43142WIN8x64 clone 43142WIN8x64-TOT
43142WIN8x64 clone 43142WIN8x64-BISON735  -tag BISON05T_BRANCH_7_35


# STA Laptop DUT DELL E6400 with 43142 - Windows 8.1
# 2015.3.31 update installed on 2015.4.15
# 2015.7.02 update installed on 2015.7.10
UTF::Cygwin mc27tst3 -user user -sta {43142WIN81} \
        -osver 81 \
        -sign true \
        -installer  InstallDriver \
        -lan_ip mc27tst3 \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -power "npc35 2" \
        -power_button {auto} \
        -perfchans {1 2 3 4 5 6 7 }

43142WIN81 clone 43142WIN81-TOT
43142WIN81 clone 43142WIN81-BISON735  -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6410 WIN7 - 43228 P451
# power cycled on 2015.5.11 - hung - blue screen
# Windows Update: patches complete 2015.6.22
UTF::Cygwin mc27tst4 -sta {43228WIN7} -user user \
    -osver 7 \
    -power {npc45 1} \
    -power_button {auto} \
    -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0}\
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -perfchans { 3l 3 } \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -datarate {-i 0.5 -frameburst 1} \
    -perfchans {1 2 3 4 5 6 7 }

43228WIN7 clone 43228WIN7-BISON    -tag BISON_BRANCH_7_10
43228WIN7 clone 43228WIN7-BISON735 -tag BISON05T_BRANCH_7_35



# Linksys 320N 4717/4322 wireless router AP1.
UTF::Router AP1 \
	-sta "47171 eth1" \
	-lan_ip 192.168.1.1 \
	-relay "mc27end1" \
	-lanpeer lan \
	-console "mc27end1:40001" \
	-power {npc55 1} \
	-brand linux-external-router \
	-tag "AKASHI_REL_5_110_*" \
	-nvram {
		et0macaddr=00:90:4c:07:00:8b
		macaddr=00:90:4c:07:01:9b
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
#47171 clone 47171-BISON -tag BISON_BRANCH_7_10 -brand linux26-external-vista-router-combo -trx "linux-gzip"
47171 clone 47171-trunk -tag trunk -brand linux-internal-router
	

#
# unplugged AP2 - router was blinking a blue light like it was continuously resetting - 2014.11.26
#

if {0} {
# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
        -sta "47172 eth1" \
	-lan_ip 192.168.1.2 \
        -relay "mc27end1" \
        -lanpeer lan \
        -console "mc27end1:40003" \
        -power {npc65 1} \
        -brand linux-external-router \
        -tag "AKASHI_REL_5_110_*" \
        -nvram {
		et0macaddr=00:90:4c:08:00:8b
		macaddr=00:90:4c:08:01:9b
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.150
	        dhcp_end=192.168.22.199
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.168.2.2
		dhcp1_start=192.168.2.150
	        dhcp1_end=192.168.2.199
		fw_disable=1
		#router_disable=1
		lan_proto=static
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


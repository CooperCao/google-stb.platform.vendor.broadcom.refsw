# -*-tcl-*-
#
# Testbed configuration file for SCR2MC1 
#

# KVMS use admin as username and wlan root passwords


# load packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Multiperf


set ::env(UTFDPORT) 9978
package require UTFD


# Enable Windows TestSigning
set UTF::TestSigning 1


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/scr2mc1"

# this is needed for multiple STA to multiple AP tests
set ::sta_list "4360WIN81x64 4352WIN7 4360WIN7 43224WIN8 4322WIN71 4321WIN7"

set ::ap_list "47171 AP2-4331 AP2-4360 4712"


# Power controllers
UTF::Power::Synaccess npc102 -lan_ip 10.22.23.102
UTF::Power::Synaccess npc103 -lan_ip 10.22.23.103
UTF::Power::Synaccess npc104 -lan_ip 10.22.23.104
UTF::Power::Synaccess npc105 -lan_ip 10.22.23.105
UTF::Power::Synaccess npc106 -lan_ip 10.22.23.106
UTF::Power::Synaccess npc107 -lan_ip 10.22.23.107
UTF::Power::Synaccess npc108 -lan_ip 10.22.23.108
UTF::Power::Synaccess npc109 -lan_ip 10.22.23.109
#UTF::Power::Synaccess npc160 -lan_ip 10.22.23.160
#UTF::Power::Synaccess npc161 -lan_ip 10.22.23.161
#UTF::Power::Synaccess npc162 -lan_ip 10.22.23.162
#UTF::Power::Synaccess npc163 -lan_ip 10.22.23.163
#UTF::Power::Synaccess npc164 -lan_ip 10.22.23.164

# Attenuator - Aeroflex
#UTF::Aeroflex af -lan_ip 10.22.23.157 \
#        -relay "scr2mc1end1" \
#        -group {G1 {1 2 3} ALL {1 2 3 4 5 6}}

# UTF Setup Defaults for TestBed during tests
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
#    ALL attn 0

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    47171    restart wl0_radio=0
    AP2-4331 restart wl0_radio=0
    AP2-4360 restart wl0_radio=0
    AP2-4331 restart wl1_radio=0
    AP2-4360 restart wl1_radio=0
    4712     restart wl0_radio=0
#    47173 restart wl0_radio=0
#    43211 restart wl0_radio=0
#    43181 restart wl0_radio=0
#    43212 restart wl1_radio=0

    
   # This is needed to ensure we start without any interference.
    foreach S {47171 AP2-4331 AP2-4360 4712 4360WIN81x64 4360XP1 4360WIN7 4322WIN71 4352WIN7 4321WIN7} {
        catch {$S wl down}
        $S deinit    
    }

    
   return
}

# Turn off most RvR intialization
# set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
# set ::rvr_ap_init ""

set UTF::CountedErrors 1
set UTF::DataRatePlot 1
set UTF::Interrupts 1
set UTF::AggressiveCleaninfs 0

# Define Sniffer FC9 E4200
UTF::Sniffer scr2mc1snf1 -user root \
        -sta {43224SNF1 eth1} \
        -tag RUBY_BRANCH_6_20 \
        -power {npc106 8} \
        -power_button {auto}


# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux scr2mc1end1 \
    -sta "lan eth1" \
    -power {npc105 1} \
    -power_button {auto} \
	-tcpwindow 1024k
	
#STA Section
    
# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 1
# Win 8.1 x64
# Updated to 2015.3.31 image on 2015.4.22
UTF::Cygwin scr2mc1tst1 -user user -sta {4360WIN81x64} \
    -osver 8164 \
    -sign 1 \
    -power {npc109 1} \
    -kdpath kd.exe \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -brand win8x_internal_wl \
    -usemodifyos 1 \
    -node DEV_43A0

4360WIN81x64 clone 4360WIN81x64-BISON -tag BISON_BRANCH_7_10
4360WIN81x64 clone 4360WIN81x64-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm150 2
# system was stuck in boot, due to bad date/time.  Bad battery?  - 2015.3.27
# system was stuck in boot, due to unable to find a hard-drive.  2015.6.30   - could be hard drive going bad
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.18
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.6.30
# 2015.7.06 update installed on 2015.7.08
UTF::Cygwin scr2mc1tst2 -user user -sta {4352WIN10} \
    -osver 10 \
    -sign 1 \
    -power {npc109 2} \
    -kdpath kd.exe \
    -brand win8x_internal_wl \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -usemodifyos 1 \
    -node DEV_43B1 \
    -udp 300m

4352WIN10 clone 4352WIN10-BISON   -tag BISON_BRANCH_7_10
4352WIN10 clone 4352WIN10-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 3
# power cycled on 2015.4.3 (had blue screen failure)
# all Win7 patches up to date: 2015.7.17
UTF::Cygwin scr2mc1tst3 -user user -sta {43224WIN72} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc109 3} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

43224WIN72 clone 43224WIN72-AA -tag AARDVARK_BRANCH_6_30
43224WIN72 clone 43224WIN72-BISON -tag BISON_BRANCH_7_10
43224WIN72 clone 43224WIN72-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 4 
# powered off and removed from test - 2015.4.20
UTF::Cygwin scr2mc1tst4 -user user -sta {4360XP1} \
    -osver 5 \
    -installer inf \
    -node {DEV_43A0} \
    -tcpwindow 512k \
    -power {npc109 4} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -wlinitcmds {wl assert_type 2}
    
4360XP1 clone 4360XP1-AA -tag AARDVARK_BRANCH_6_30
4360XP1 clone 4360XP1-BISON -tag BISON_BRANCH_7_10
4360XP1 clone 4360XP1-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 5 
# Updated to 2015.3.31 image on 2015.4.22
# 2015.7.02 update installed on 2015.7.08
UTF::Cygwin scr2mc1tst5 -user user -sta {43142WIN81} \
    -osver 81 \
    -sign 1 \
    -power {npc109 5} \
    -kdpath kd.exe \
    -brand win8x_internal_wl \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -usemodifyos 1 \
    -node DEV_4365

43142WIN81 clone 43142WIN81-BISON -tag BISON_BRANCH_7_10
43142WIN81 clone 43142WIN81-BISON_735 -tag BISON05T_BRANCH_7_35



# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 6
UTF::Cygwin scr2mc1tst6 -user user -sta {4360WIN7} \
    -osver 7 \
    -installer inf \
    -node {DEV_43A0} \
    -tcpwindow 512k \
    -power {npc109 6} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -wlinitcmds {wl assert_type 2}

#4360WIN7 clone 4360WIN7-AA -tag AARDVARK_BRANCH_6_30
4360WIN7 clone 4360WIN7-BISON -tag BISON_BRANCH_7_10
4360WIN7 clone 4360WIN7-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 7
# power cycled on 2015.2.4
# -seems system is unreliable
# powered off via NPC on 2015.2.9
# re-image system and replace WiFi chip before placing back in service
UTF::Cygwin scr2mc1tst7 -user user -sta {4311XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -power {npc109 7} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}    

4311XP clone 4311XP-AA -tag AARDVARK_BRANCH_6_30
4311XP clone 4311XP-BISON -tag BISON_BRANCH_7_10
4311XP clone 4311XP-BISON_735 -tag BISON05T_BRANCH_7_35
4311XP clone 4311XP-AA8 -tag AARDVARK_TWIG_6_30_14
    
# STA Laptop DUT Dell E6400
# scr2mc1kvm150 Port 8    
# unplugged as of 2015.1.28
# powered off via NPC on 2015.2.9
UTF::Cygwin scr2mc1tst8 -user user -sta {4311Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -power {npc109 8} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}  

4311Vista clone 4311Vista-AA -tag AARDVARK_BRANCH_6_30
4311Vista clone 4311Vista-BISON -tag BISON_BRANCH_7_10
4311Vista clone 4311Vista-BISON_735 -tag BISON05T_BRANCH_7_35
    
# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 1 
# powered off and removed system 2015.3.6
UTF::Cygwin scr2mc1tst9 -user user -sta {4322XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc107 1} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

4322XP clone 4322XP-AA -tag AARDVARK_BRANCH_6_30
4322XP clone 4322XP-BISON -tag BISON_BRANCH_7_10
4322XP clone 4322XP-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 2
# BCM: 43228
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.18
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.6.30
# 2015.7.06 update installed on 2015.7.08
UTF::Cygwin scr2mc1tst10 -user user -sta {43228WIN10} \
        -osver 10 \
        -sign true \
        -node DEV_4359 \
        -usemodifyos 1 \
        -kdpath kd.exe \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -power "npc107 2" \
        -power_button {auto}

43228WIN10 clone 43228WIN10-TOT
43228WIN10 clone 43228WIN10-BISON     -tag BISON_BRANCH_7_10
43228WIN10 clone 43228WIN10-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 3
# refuses ssh and fails power cycle
#  **** check this chip.  Label on machine says 4322
# continues to refuse ssh after power cycling by hand - 2015.2.9
# unplugged - 2015.2.9
UTF::Cygwin scr2mc1tst11 -user user -sta {43224WIN8} \
    -osver 8 \
    -sign true \
    -installer inf \
    -tcpwindow 1024k \
    -ssh ssh \
    -power {npc107 3} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}


43224WIN8 clone 43224WIN8-EXT-53 -tag AARDVARK_REL_6_30_53 -brand win8_external_wl -type Bcm
43224WIN8 clone 43224WIN8-INT-53 -tag AARDVARK_REL_6_30_53 -brand win8_internal_wl
43224WIN8 clone 43224WIN8-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 4
# powered off and removed from test - 2015.4.20
UTF::Cygwin scr2mc1tst12 -user user -sta {43224XP} \
    -osver 5 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc107 4} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

43224XP clone 43224XP-AA -tag AARDVARK_BRANCH_6_30
43224XP clone 43224XP-BISON -tag BISON_BRANCH_7_10
43224XP clone 43224XP-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 5
# all Win7 patches up to date: 2015.7.1
UTF::Cygwin scr2mc1tst13 -user user -sta {4322WIN71} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc107 5} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

4322WIN71 clone 4322WIN71-AA -tag AARDVARK_BRANCH_6_30
4322WIN71 clone 4322WIN71-BISON -tag BISON_BRANCH_7_10
4322WIN71 clone 4322WIN71-BISON_735 -tag BISON05T_BRANCH_7_35
# temporary alias... until 04/14/2015, then no longer used
4322WIN71 clone 43224WIN71-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 6
# all Win7 patches up to date: 2015.7.14
UTF::Cygwin scr2mc1tst14 -user user -sta {4352WIN7} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc107 6} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

    # -wlinitcmds {wl down; wl amsdu 0; wl up}
    
4352WIN7 clone 4352WIN7-BISON -tag BISON_BRANCH_7_10
4352WIN7 clone 4352WIN7-BISON_735 -tag BISON05T_BRANCH_7_35
4352WIN7 clone 4352WIN7-AA -tag AARDVARK_BRANCH_6_30



# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 7
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.18
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.6.30
# 2015.7.06 update installed on 2015.7.08
UTF::Cygwin scr2mc1tst15 -user user -sta {43142WIN10} \
    -osver 10 \
    -sign 1 \
    -power {npc107 7} \
    -kdpath kd.exe \
    -brand win8x_internal_wl \
    -usemodifyos 1 \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -node DEV_4365 \
    -udp 300m

43142WIN10 clone 43142WIN10-BISON   -tag BISON_BRANCH_7_10
43142WIN10 clone 43142WIN10-BISON_735 -tag BISON05T_BRANCH_7_35



# STA Laptop DUT Dell E6400
# scr2mc1kvm151 Port 8
# aspm 0x100 is disabling L0s and L1 due to PR#65045 and PR#64659
# No video at the kvm
UTF::Cygwin scr2mc1tst16 -user user -sta {4321Vista} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -tcpslowstart 4 \
    -power {npc107 8} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -brand win_external_wl -type Bcm

4321Vista clone 4321Vista-AA59 -tag AARDVARK_{TWIG,REL}_6_30_59{,_*}    
4321Vista clone 4321Vista-AA -tag AARDVARK_BRANCH_6_30
4321Vista clone 4321Vista-AA8 -tag AARDVARK_TWIG_6_30_14
4321Vista clone 4321Vista-BISON_735 -tag BISON05T_BRANCH_7_35

    
# STA Laptop DUT Dell E6400    
# scr2mc1kvm154 Port 1
# aspm 0x100 is disabling L0s and L1 due to PR#65045 and PR#64659
# Has the infinite "Starting Windows" screen.  Need to replace the HDD
# 2015.2.11 -powered off via NPC
UTF::Cygwin scr2mc1tst17 -user user -sta {4311WIN7} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -power {npc108 5} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} 

4311WIN7 clone 4311WIN7-AA -tag AARDVARK_BRANCH_6_30
4311WIN7 clone 4311WIN7-BISON -tag BISON_BRANCH_7_10
4311WIN7 clone 4311WIN7-BISON_735 -tag BISON05T_BRANCH_7_35
    

# STA Laptop DUT Dell E6400    
# scr2mc1kvm154 Port 2
# aspm 0x100 is disabling L0s and L1 due to PR#65045 and PR#64659
UTF::Cygwin scr2mc1tst18 -user user -sta {4321WIN7} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -wlinitcmds {wl aspm 0x100} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -power {npc108 6} \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}} \
    -brand win_external_wl -type Bcm

4321WIN7 clone 4321WIN7-AA -tag AARDVARK_BRANCH_6_30
4321WIN7 clone 4321WIN7-AA8 -tag AARDVARK_TWIG_6_30_14
4321WIN7 clone 4321WIN7-BISON_735 -tag BISON05T_BRANCH_7_35


# 10.22.23.159
# DUT2 Frank system on AP rack with 43143USB full dongle driver
# UTF::WinDHD scr2mc1tst31 -user user -sta {43143w8 00:90:4C:0E:81:23}
# Updated to 2015.3.31 image
# Power cycled on 2015.5.11 - device wasn't being recognized
UTF::WinDHD scr2mc1tst31 -user user -sta {43143w81} \
    -osver 81 \
    -brand win8x_internal_wl \
    -embeddedimage 43143b0 \
    -usemodifyos 1 \
    -kdpath kd.exe \
    -console "scr2mc1end1:40004" \
    -power {npc102 8} \
    -power_button {auto} \
    -installer inf \
    -type "checked/DriverOnly_BMac" \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -wlinitcmds {wl down; wl mimo_bw_cap 1} \
    -tcpwindow auto \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

43143w81 clone 43143w81b \
    -tag BISON_BRANCH_7_10 \
    -nobighammer 0
    
43143w81 clone 43143w81b735 \
    -tag BISON05T_BRANCH_7_35 \
    -nobighammer 0
    
      



# ***********************
# AP Section
# AP Section
# AP Section
# ***********************
    
# Linksys 320N 4717/4322 wireless router AP1.
# pings as of 2015.2.11
UTF::Router AP1 \
    -sta "47171 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.2 \
    -console "scr2mc1end1:40001" \
    -power {npc102 5} \
    -power_button {auto} \
    -brand linux-external-router \
	-tag "MILLAU_REL_5_70_48_*" \
	-nvram {
        et0macaddr=00:90:4c:01:00:8b
        macaddr=00:90:4c:01:01:9b
        lan_ipaddr=192.168.1.2
        lan_gateway=192.168.1.2
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47171
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        # Used to WAR PR#86385
        wl0_obss_coex=0
    }
    
47171 clone 47171-AA -tag AARDVARK_BRANCH_6_30
47171 clone 47171-AA39 -tag AARDVARK_REL_6_30_39_*


# -image /home/jeffg/src/tools/unittest/jq/1.18.0/mips/linux.trx
 #   (wl -i <interface name> rx_amsdu_in_ampdu 0)
 # -wlinitcmds {wl -i eth2 down; wl -i eth2 hirssi_en 0; wl -i eth2 up}
 # -wlinitcmds {wl -i eth2 down; wl -i eth2 phy_watchdog 0; wl -i eth2 up}
 
UTF::Router AP2-4706 \
    -lan_ip 192.168.1.1 \
    -sta {AP2-4360 eth2 AP2-4360-4706.%15 wl1.%
       AP2-4331 eth1 AP2-4331-4706.%15 wl0.%} \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -console "scr2mc1end1:40002" \
    -brand linux26-internal-router \
    -power {npc102 6} \
    -nvram {
        fw_disable=1
        wl_msglevel=0x101
        macaddr=00:90:4c:0d:bf:d5
	wl0_ssid=4706/4331-scr2mc1
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
	# Fixing 2G Low Signal Stength
	1:boardflags2=0x4000000
    } \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -noradio_pwrsave 1

  
AP2-4360 clone AP2-4360-BISON -tag BISON_BRANCH_7_10    
AP2-4331 clone AP2-4331-BISON -tag BISON_BRANCH_7_10
    
AP2-4360 clone AP2-4360-AA -tag AARDVARK_BRANCH_6_30
AP2-4331 clone AP2-4331-AA -tag AARDVARK_BRANCH_6_30

AP2-4360 clone AP2-4360-AA37-14 -tag AARDVARK01T_TWIG_6_37_14
AP2-4331 clone AP2-4331-AA37-14 -tag AARDVARK01T_TWIG_6_37_14

# Clone for external 4360
# AP2-4360 clone AP2-4360x \
    -sta {AP2x-4360-4706 eth2} \
    -brand linux26-external-vista-router-full-src \
    -perfonly 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1}

# AP2-4706 clone AP2x -brand "linux26-external-vista-router-combo"    
    

# Linksys 320N 4717/4322 wireless router AP3.
# pings as of 2015.2.11
UTF::Router AP3 \
    -sta "47173 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.4 \
    -brand linux-external-router \
    -nvram {
        et0macaddr=00:90:4c:03:00:8b
        macaddr=00:90:4c:03:01:9b
        lan_ipaddr=192.168.1.4
        lan_gateway=192.168.1.4
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47173
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }       

    
if {0} {
# BCM9705 w/4321MP as AP4.
# 2015.2.11 - doesn't exist
UTF::Router AP4 \
    -sta "43211 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.5 \
    -console "scr2mc1end1:40004" \
    -power {npc102 8} \
    -power_button {auto} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_*" \
    -model bcm94705GMP \
    -serial_num 1271637 \
    -nvram {
        lan_ipaddr=192.168.1.5
        wl_msglevel=0x101
        wl0_ssid=43211
        wl0_channel=1
        wl0_radio=0
        fw_disable=1
        router_disable=1
    }
    
43211 clone 43211-BISON -tag BISON_BRANCH_7_10
43211 clone 43211-AA37-14 -tag AARDVARK01T_TWIG_6_37_14
}
           

# BCM9704 w/4321MP w/4318
# 2014.12.17 - doesn't ping - seems to be power cycling
# 2014.12.17 -power cycling it by hand didn't seem to help
# 2015.2.11 - system doesn't work - powered off via NPC
UTF::Router AP5 \
    -sta "43212 eth2 43181 eth3" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.6 \
    -console "scr2mc1end1:40005" \
    -power {npc102 4} \
    -power_button {auto} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_*" \
    -nvram {
        lan_ipaddr=192.168.1.6
        wl_msglevel=0x101
        wl0_ssid=43181
        wl0_channel=1
        wl0_radio=0
        wl1_ssid=43212
        wl1_channel=1
        wl1_radio=0
        fw_disable=1
        router_disable=1
    }

#43212 clone 43212-COMANCHE -tag COMANCHE_REL_4_200_19    
43212 clone 43212-COMANCHE2-22 -tag COMANCHE2_REL_5_22_90
#43212 clone 43212-COMANCHE2-20 -tag COMANCHE2_REL_5_20_72

# BCM4712AGR 
# pings as of 2015.2.11
UTF::Router AP6 \
    -sta "4712 eth1" \
    -relay "scr2mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.7 \
    -console "scr2mc1end1:40006" \
    -power {npc108 8} \
    -power_button {auto} \
    -brand linux-external-router \
    -tag "AKASHI_REL_5_110_*" \
    -nvram {
        lan_ipaddr=192.168.1.7
        wl_msglevel=0x101
        wl0_ssid=47121
        wl0_radio=0
        fw_disable=1
        router_disable=1
    }    
    
#4712 clone 4712-BISON -tag BISON_BRANCH_7_10
#4712 clone 4712-AA37-14 -tag AARDVARK01T_TWIG_6_37_14
#4712 clone 4712-COMANCHE -tag COMANCHE_REL_4_200_19
4712 clone 4712-COMANCHE2-22 -tag COMANCHE2_REL_5_22_90
4712 clone 4712-COMANCHE2-20 -tag COMANCHE2_REL_5_20_72
4712 clone 4712-BEARS -tag BEARS_REL_3_130_RC20


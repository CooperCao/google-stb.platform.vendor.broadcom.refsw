# -*-tcl-*-
#
# Configuration file for Jim Palte's scr1mc1 testbed
#

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

set ::env(UTFDPORT) 9978
package require UTFD


# DHCP Server 192.168.1.254

# Enable Windows TestSigning
set UTF::TestSigning 1

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/scr1mc1"

# Power controllers
#UTF::Power::Synaccess npc40 -lan_ip 10.22.23.40
UTF::Power::Synaccess npc41 -lan_ip 10.22.23.41
UTF::Power::Synaccess npc42 -lan_ip 10.22.23.42
UTF::Power::Synaccess npc43 -lan_ip 10.22.23.43
UTF::Power::Synaccess npc44 -lan_ip 10.22.23.44
#UTF::Power::Synaccess npc45 -lan_ip 10.22.23.45


# UTF Setup Defaults for TestBed during tests
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    #ALL attn 0;

    # Make sure radios are off on APs before testing
    # to ensure that Embedded Nightly only sees one AP
    #
    # AP11 restart wl0_radio=0
    # AP11 restart wl1_radio=1
    AP2 restart wl0_radio=0
    AP3 restart wl0_radio=0
#    AP4 restart wl0_radio=0
#    AP5 restart wl0_radio=0
#    AP6 restart wl0_radio=0


    # Thi is needed to ensure we start without any inteference.
    foreach S {AP2 AP3 43224WIN71 43224WIN72 43224WIN73 43224w8 4312WIN71 43142WIN8 43142WIN73 4352WIN74 4322WIN71 4322WIN72 4322WIN73 4322WIN74 4312Vista1 43228WIN81 43228WIN10 43228WIN8} {
        catch {$S wl down}
        $S deinit    
    }

    return
}

set UTF::CountedErrors 1
set UTF::DataRatePlot 1
set UTF::Interrupts 1



# Define Sniffer
UTF::Sniffer scr1mc1snf1 -user root \
        -sta {43224SNF1 eth1} \
        -tag BASS_BRANCH_5_60 \
        -power {npc44 7} \
        -power_button {auto}


# UTF Endpoint - Traffic generators (no wireless cards)
UTF::Linux scr1mc1end1 \
    -sta "lan eth1" \
    -power {npc41 1} \
    -power_button {auto} 


# STA Laptop DUT Dell E6400
# KVM46 1
UTF::Cygwin scr1mc1tst1 -user user -sta {43224WIN71} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc42 1} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} 

43224WIN71 clone 43224WIN71-AA -tag AARDVARK_REL_6_30
43224WIN71 clone 43224WIN71-BISON     -tag BISON_BRANCH_7_10
43224WIN71 clone 43224WIN71-BISON_735 -tag BISON05T_BRANCH_7_35
43224WIN71 clone 43224WIN71-AA37 -tag AARDVARK01T_BRANCH_6_37


# STA Laptop DUT Dell E6400
# KVM46 2
#    -STA not installed as of 2014.11.10
UTF::Cygwin scr1mc1tst2 -user user -sta {43224WIN72} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc42 2} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

43224WIN72 clone 43224WIN72-AA -tag AARDVARK_REL_6_30
43224WIN72 clone 43224WIN72-BISON -tag BISON_BRANCH_7_10
43224WIN72 clone 43224WIN72-BISON_735 -tag BISON05T_BRANCH_7_35
43224WIN72 clone 43224WIN72-AA37 -tag AARDVARK01T_BRANCH_6_37

# STA Laptop DUT Dell E6400
# KVM46 3
# all Win7 patches up to date: 2015.7.16
UTF::Cygwin scr1mc1tst3 -user user -sta {43224WIN73} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc42 3} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

43224WIN73 clone 43224WIN73-AA -tag AARDVARK_REL_6_30
43224WIN73 clone 43224WIN73-BISON -tag BISON_BRANCH_7_10
43224WIN73 clone 43224WIN73-BISON_735 -tag BISON05T_BRANCH_7_35
43224WIN73 clone 43224WIN73-AA37 -tag AARDVARK01T_BRANCH_6_37


# STA Laptop DUT Dell E6400
# KVM46 4
# Hung on boot: No bootable devices.  2015.2.15.  Power cycled and system came up.
# Updated to 2015.3.31 image on 2015.4.28
# Updated to 2015.7.02 image on 2015.7.08
UTF::Cygwin scr1mc1tst4 -user user -sta {43224w8} \
    -osver 8 \
    -installer inf \
    -tcpwindow 512k \
    -brand win8x_internal_wl \
    -ssh fsh \
    -power {npc42 4} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

43224w8 clone 43224w8-BISON -tag BISON_BRANCH_7_10
43224w8 clone 43224w8-BISON_735 -tag BISON05T_BRANCH_7_35
# -wlinitcmds {wl msglevel 0x4118} \
# -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl msglevel 0x4118}} \



# STA Laptop DUT Dell E6400
# KVM47 1
# WlanOpenHandle() failed...
# Powered off, awaiting new chip and probably OS re-install
UTF::Cygwin scr1mc1tst5 -user user -sta {4312WIN71} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc42 5} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

4312WIN71 clone 4312WIN71-AA -tag AARDVARK_REL_6_30
4312WIN71 clone 4312WIN71-BISON -tag BISON_BRANCH_7_10
4312WIN71 clone 4312WIN71-BISON_735 -tag BISON05T_BRANCH_7_35
4312WIN71 clone 4312WIN71-AA37 -tag AARDVARK01T_BRANCH_6_37


# STA Laptop DUT Dell E6400
# KVM47 2
# 2015.4.13: Imaged to 2015.3.31 
# 2015.7.08: Imaged to 2015.7.02
UTF::Cygwin scr1mc1tst6 -user user -sta {43142WIN8} \
    -osver 8 \
    -sign true \
    -installer inf \
    -tcpwindow 4M \
    -brand win8x_internal_wl \
    -power {npc42 6} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -wlinitcmds {wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 0} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}}

43142WIN8 clone 43142WIN8-TOT
43142WIN8 clone 43142WIN8-BISON -tag BISON_BRANCH_7_10
43142WIN8 clone 43142WIN8-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# KVM47 3
# Win7 with 43142 chip
# All patches up to date - 2015.7.8
UTF::Cygwin scr1mc1tst7 -user user -sta {43142WIN73} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc42 7} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

43142WIN73 clone 43142WIN73-AA -tag AARDVARK_REL_6_30
43142WIN73 clone 43142WIN73-BISON -tag BISON_BRANCH_7_10
43142WIN73 clone 43142WIN73-BISON_735 -tag BISON05T_BRANCH_7_35
43142WIN73 clone 43142WIN73-AA37 -tag AARDVARK01T_BRANCH_6_37

# STA Laptop DUT Dell E6400
# KVM47 4
# all Win7 patches up to date: 2015.7.9
UTF::Cygwin scr1mc1tst8 -user user -sta {4352WIN74} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc42 8} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}}

4352WIN74 clone 4352WIN74-BISON -tag BISON_BRANCH_7_10
4352WIN74 clone 4352WIN74-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# KVM48 1    
# all Win7 patches up to date: 2015.7.14
UTF::Cygwin scr1mc1tst9 -user user -sta {4322WIN71} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -node DEV_432B \
    -power {npc43 1} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -datarate 0

4322WIN71 clone 4322WIN71-AA -tag AARDVARK_REL_6_30
4322WIN71 clone 4322WIN71-BISON -tag BISON_BRANCH_7_10
4322WIN71 clone 4322WIN71-BISON_735 -tag BISON05T_BRANCH_7_35
4322WIN71 clone 4322WIN71-AA37 -tag AARDVARK01T_BRANCH_6_37


# STA Laptop DUT Dell E6400
# KVM48 2
# rebooted system on 2015.3.16 due to WLANOpen() failure
# all Win7 patches up to date: 2015.7.14
UTF::Cygwin scr1mc1tst10 -user user -sta {4322WIN72} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc43 2} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -datarate 0

4322WIN72 clone 4322WIN72-AA -tag AARDVARK_REL_6_30_*
4322WIN72 clone 4322WIN72-BISON -tag BISON_BRANCH_7_10
4322WIN72 clone 4322WIN72-BISON_735 -tag BISON05T_BRANCH_7_35
4322WIN72 clone 4322WIN72-AA37 -tag AARDVARK01T_BRANCH_6_37

# STA Laptop DUT Dell E6400
# KVM48 3
# SSHD not installed properly
# - unplugged pending re-install  2015.1.20
# - trunk support removed on 2015.3.25, so replace chip also
UTF::Cygwin scr1mc1tst11 -user user -sta {4322WIN73} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc43 3} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -datarate 0

4322WIN73 clone 4322WIN73-AA -tag AARDVARK_REL_6_30
4322WIN73 clone 4322WIN73-BISON -tag BISON_BRANCH_7_10
4322WIN73 clone 4322WIN73-BISON_735 -tag BISON05T_BRANCH_7_35
4322WIN73 clone 4322WIN73-AA37 -tag AARDVARK01T_BRANCH_6_37

# STA Laptop DUT Dell E6400
# KVM48 4
# shut down pending chip replacement - 4322 testing duplicates tst9 and tst10
UTF::Cygwin scr1mc1tst12 -user user -sta {4322WIN74} \
    -osver 7 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc43 4} \
    -power_button {auto} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl dump ampdu}} \
    -datarate 0

4322WIN74 clone 4322WIN74-AA -tag AARDVARK_REL_6_30_*
4322WIN74 clone 4322WIN74-BISON -tag BISON_BRANCH_7_10
4322WIN74 clone 4322WIN74-BISON_735 -tag BISON05T_BRANCH_7_35
4322WIN74 clone 4322WIN74-AA37 -tag AARDVARK01T_BRANCH_6_37

# STA Laptop DUT Dell E6400
# KVM49 1

#    VISTA IS NO LONGER SUPPORTED - UPDATE THESE SYSTEMS TO A NEW OS
#    System unplugged as of 2015.2.5
UTF::Cygwin scr1mc1tst13 -user user -sta {4312Vista1} \
    -osver 6 \
    -installer inf \
    -tcpwindow 512k \
    -power {npc43 5} \
    -power_button {auto} \
    -tcpslowstart 4 \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}

4312Vista1 clone 4312Vista1-AA -tag AARDVARK_REL_6_30
4312Vista1 clone 4312Vista1-BISON -tag BISON_BRANCH_7_10
4312Vista1 clone 4312Vista1-BISON_735 -tag BISON05T_BRANCH_7_35
4312Vista1 clone 4312Vista1-AA37 -tag AARDVARK01T_BRANCH_6_37

# scr1mc1tst14: Laptop DUT Dell E6400 
# OS: Windows 8.1
# KVM49 2
# BCMs: "43228 dual band"
# Updated to 2015.3.31 image on 2015.4.22
# 2015.7.02 update installed on 2015.7.09
UTF::Cygwin scr1mc1tst14 -user user -sta {43228WIN81} \
        -osver 81 \
        -sign true \
        -installer inf \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -wlinitcmds {wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 0} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -power "npc43 6" \
        -power_button {auto}

43228WIN81 clone 43228WIN81-TOT
43228WIN81 clone 43228WIN81-BISON -tag BISON_BRANCH_7_10
43228WIN81 clone 43228WIN81-BISON_735 -tag BISON05T_BRANCH_7_35


# STA Laptop DUT Dell E6400
# KVM49 3
# BCMs: "43228 dual band"
# 2015.4.21: Imaged to 2015.4.14 
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.18
# 2015.6.08 update installed on 2015.6.08
# 2015.6.29 update installed on 2015.7.01
# 2015.7.06 update installed on 2015.7.08
UTF::Cygwin scr1mc1tst15 -user user -sta {43228WIN10} \
        -osver 10 \
        -sign true \
        -node DEV_4359 \
        -usemodifyos 1 \
        -kdpath kd.exe \
        -tcpwindow 4M \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -brand win8x_internal_wl \
        -power "npc43 7" \
        -power_button {auto}

43228WIN10 clone 43228WIN10-TOT
43228WIN10 clone 43228WIN10-BISON -tag BISON_BRANCH_7_10
43228WIN10 clone 43228WIN10-BISON_735 -tag BISON05T_BRANCH_7_35


# scr1mc1tst16: Laptop DUT Dell E6400 
# OS: Windows 8
# KVM49 4
# BCMs: "43228 dual band"
# Device crashed and was re-started on 2015.2.12 (message about being unable to shut down)
# Updated to 2015.3.31 image on 2015.4.22
# Updated to 2015.7.02 image on 2015.7.08
UTF::Cygwin scr1mc1tst16 -user user -sta {43228WIN8} \
        -osver 8 \
        -sign true \
        -installer inf \
        -tcpwindow 4M \
        -brand win8x_internal_wl \
        -wlinitcmds {wl down; wl bw_cap 2g -1} \
        -wlinitcmds {wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 0} \
        -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -power "npc43 8" \
        -power_button {auto}

43228WIN8 clone 43228WIN8-TOT
43228WIN8 clone 43228WIN8-BISON -tag BISON_BRANCH_7_10
43228WIN8 clone 43228WIN8-BISON_735 -tag BISON05T_BRANCH_7_35



# Linksys E4200 4318/4331 wireless router AP11
UTF::Router AP11 \
    -sta "43311 eth2" \
    -relay "scr1mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.11 \
    -console "scr1mc1end1:40001" \
    -power {npc44 1} \
    -power_button {auto} \
    -brand linux-external-router \
	-tag "AKASHI_REL_5_110_*" \
	-nvram {
		boot_hw_model=E4200
        et0macaddr=00:90:4c:06:00:8b
        macaddr=00:90:4c:06:01:9b
        sb/1/macaddr=00:90:4c:06:01:00
        pci/1/1/macaddr=00:90:4c:06:01:01
        lan_ipaddr=192.168.1.11
        lan_gateway=192.168.1.11
        dhcp_start=192.168.1.101
        dhcp_end=192.168.1.119
        lan1_ipaddr=192.168.2.11
        lan1_gateway=192.169.2.11
        dhcp1_start=192.168.2.101
        dhcp1_end=192.168.2.119
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47181
        wl1_ssid=test47182
        wl0_channel=1
        wl1_channel=1
        wl0_radio=0
        wl1_radio=0
        wl1_nbw_cap=0
        antswitch=0
         # Used to WAR PR#86385
        wl0_obss_coex=0
        wl1_obss_coex=0
    }
 
# Linksys 320N 4717/4322 wireless router AP2.
UTF::Router AP2 \
    -sta "47172 eth1" \
    -relay "scr1mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.2 \
    -console "scr1mc1end1:40002" \
    -power {npc44 2} \
    -power_button {auto} \
    -brand linux-internal-router \
	-tag trunk \
    -nvram {
        et0macaddr=00:90:4c:07:00:8b
        macaddr=00:90:4c:07:01:9b
        lan_ipaddr=192.168.1.2
        lan_gateway=192.168.1.2
        dhcp_start=192.168.1.120
        dhcp_end=192.168.1.139
        lan1_ipaddr=192.168.2.2
        lan1_gateway=192.169.2.2
        dhcp1_start=192.168.2.120
        dhcp1_end=192.168.2.139
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47172
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        # Used to WAR PR#86385
        wl0_obss_coex=0
        wl1_obss_coex=0
    }

47172 clone 47172-AKASHI -tag AKASHI_REL_5_110_*    
47172 clone 47172-COMANCHE -tag COMANCHE_REL_4_200_19
47172 clone 47172-COMANCHE2-22 -tag COMANCHE2_REL_5_22_90
47172 clone 47172-COMANCHE2-20 -tag COMANCHE2_REL_5_20_72
47172 clone 47172-MILLAU -tag MILLAU_REL_5_70_48_*
47172 clone 47172-AA37 -tag AARDVARK01T_TWIG_6_37_14 -brand linux26-external-vista-router-combo -trx "linux-gzip"
    
# Linksys 320N 4717/4322 wireless router AP3.
UTF::Router AP3 \
    -sta "47173 eth1" \
    -relay "scr1mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.3 \
    -console "scr1mc1end1:40003" \
    -power {npc44 3} \
    -power_button {auto} \
    -brand linux-external-router \
	-tag "MILLAU_REL_5_70_48_*" \
    -nvram {
        et0macaddr=00:90:4c:08:00:8b
        macaddr=00:90:4c:08:01:9b
        lan_ipaddr=192.168.1.3
        lan_gateway=192.168.1.3
        dhcp_start=192.168.1.140
        dhcp_end=192.168.1.159
        lan1_ipaddr=192.168.2.3
        lan1_gateway=192.169.2.3
        dhcp1_start=192.168.2.140
        dhcp1_end=192.168.2.159
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47173
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }

47173 clone 47173-Bison -tag BISON_BRANCH_7_10 -brand linux26-external-vista-router-combo
47173 clone 47173-AA37-14 -tag AARDVARK01T_TWIG_6_37_14 -brand linux26-external-vista-router-full-src
    

if {0} {
# commented out these APs since they don't ping, as of 2015.2.12

# Linksys 320N 4717/4322 wireless router AP4.
UTF::Router AP4 \
    -sta "47174 eth1" \
    -relay "scr1mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.4 \
    -console "scr1mc1end1:40004" \
    -power {npc44 4} \
    -power_button {auto} \
    -brand linux-internal-router \
	-tag "MILLAU_REL_5_70_*" \
    -nvram {
        et0macaddr=00:90:4c:09:00:8b
        macaddr=00:90:4c:09:01:9b
        lan_ipaddr=192.168.1.4
        lan_gateway=192.168.1.4
        dhcp_start=192.168.1.160
        dhcp_end=192.168.1.179
        lan1_ipaddr=192.168.2.4
        lan1_gateway=192.169.2.4
        dhcp1_start=192.168.2.160
        dhcp1_end=192.168.2.179
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47174
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }

# Linksys 320N 4717/4322 wireless router AP5.
UTF::Router AP5 \
    -sta "47175 eth1" \
    -relay "scr1mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.5 \
    -console "scr1mc1end1:40005" \
    -power {npc44 5} \
    -power_button {auto} \
    -brand linux-internal-router \
	-tag "COMANCHE2_REL_5_22_90" \
    -nvram {
        et0macaddr=00:90:4c:10:00:8b
        macaddr=00:90:4c:10:01:9b
        lan_ipaddr=192.168.1.5
        lan_gateway=192.168.1.5
        dhcp_start=192.168.1.180
        dhcp_end=192.168.1.199
        lan1_ipaddr=192.168.2.5
        lan1_gateway=192.169.2.5
        dhcp1_start=192.168.2.180
        dhcp1_end=192.168.2.199
        fw_disable=1
        router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47175
        wl0_channel=1
        wl0_radio=0
        antswitch=0
    }
    
# Linksys WRT600N 4321 wireless (4321 card from a BCM Reference Router) router AP6.
UTF::Router AP6 \
    -sta "43211 eth1" \
    -relay "scr1mc1end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.6 \
    -console "scr1mc1end1:40006" \
    -power {npc44 6} \
    -power_button {auto} \
    -brand linux-external-router \
    -nvram {
        # Switch WRT600n LAN to vlan1, since vlan0 doesn't work
        "vlan1ports=1 2 3 4 8*"
        vlan1hwname=et0
        "landevs=vlan1 wl0 wl1"
        "lan_ifnames=vlan1 eth1 eth2"
        # Enable untagging on vlan2 else WAN doesn't work
        "vlan2ports=0 8u"
        et0macaddr=00:90:4c:11:00:8b
        wl0_ssid=test432110
        wl_msglevel=0x101
        lan_ipaddr=192.168.1.6
        lan_gateway=192.168.1.6
        dhcp_start=192.168.1.200
        dhcp_end=192.168.1.219
        lan1_ipaddr=192.168.2.6
        lan1_gateway=192.169.2.6
        dhcp1_start=192.168.2.200
        dhcp1_end=192.168.2.219
        fw_disable=1
        router_disable=1
        wl0_ssid=test432110
        wl0_channel=1
        wl0_radio=0
    }

}

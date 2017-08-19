
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package provide UTF::WebRelay 

package require UTF::Airport

#package require UTF::Cygwin
#package require UTF::Multiperf
#package require UTF::TclReadLines

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc62"
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/fyang/mc71"
#set ::UTF::SummaryDir "/projects/hnd_sig_ext3/fyang/mc62"

################Define Testbed Objects
# Define power controllers on cart
#UTF::Power::Synaccess mc62npc22_1 -lan_ip 172.19.16.36
#UTF::Power::Synaccess mc62npc22_5 -lan_ip 172.19.16.31
#UTF::Power::Synaccess mc62npc22_2 -lan_ip 172.19.16.37
#UTF::Power::Synaccess mc62npc22_3 -lan_ip 172.19.16.34
#UTF::Power::Synaccess mc62npc22_4 -lan_ip 172.19.16.135
UTF::Power::Synaccess mc71npc22_1 -lan_ip 172.19.16.51 
UTF::Power::Synaccess mc71npc22_2 -lan_ip 172.19.16.53  
UTF::Power::Synaccess mc71npc22_3 -lan_ip 172.19.16.52  
UTF::Power::Synaccess mc71npc22_4 -lan_ip 172.19.16.39 


#UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.61 -invert 1 "mc55end1"
#UTF::Power::WebRelay webrelay2 -lan_ip 172.19.16.21 -invert 1
# UTF::Power::WebRelay webrelay3 -lan_ip 172.19.16.21 -invert 2 
#Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.16.11 -relay "mc71end1" -group { G1 {1 2 3 } G2 {4 5 6} ALL {1 2 3 4 5 6} } -relay "mc71end1"

#init
set ::UTF::SetupTestBed {
    af setGrpAttn G1 0
    af setGrpAttn G2 0
#    af setGrpAttn G3 0
    unset ::UTF::SetupTestBed  
    return
}
#########p2p
set ::bt_dut ""      		;# BlueTooth device under test
set ::bt_ref ""      	;# BlueTooth reference board
set ::wlan_dut 43228FC9-p2p		;# HND WLAN device under test
set ::wlan_rtr 47171   		;# HND WLAN router
set ::wlan_tg lan 
###################

# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc71end1 \
        -sta {lan eth2}
#        -lan_ip 192.168.1.102
#        -wlinitcmds { service iperf restart }\
#         -lan_ip 192.168.1.99

 
# A Laptop DUT Dell E6400 with FC9
# BCMs: "Linux 4312HMG_Dell" and "Linux 43228hm4l P403" 
UTF::Linux mc71sta1 \
        -sta "4331FC11 eth3 4331FC11.1 wl0.1"\
        -tag RUBY_BRANCH_6_20\
	-power {mc71npc22_4 1} \
	-power_button {auto} 
	#-console "mc71end1:40003"
        #-wlinitcmds { service dhcpd status }
 
# Clones for mc71sta1
#43228FC9 clone 43228FC9-KIRIN -tag KIRIN_BRANCH_5_100
#43228FC9 clone 43228FC9-KIRIN-REL -tag KIRIN_REL_5_100_*
#43228FC9 clone 43228FC9-RUBY -tag "RUBY_BRANCH_6_20"
#43228FC9 clone 43228FC9_softap clone -wlinitcmds { wl down ;  wl apsta 1; wl ssid -C 1 "softap_123"; wl up; wl -i eth2 bss -C 1 ;up ifconfig wl0.1 up} 
#43228FC9.1 configure -ipaddr 11.10.10.1 -hasdhcpd 1 -ap 1
4331FC11 configure -ipaddr 192.168.1.91

4331FC11 clone 4331FC11ap  -wlinitcmds {wl msglevel +mchan} -type debug-p2p-mchan -tag NIGHTLY   


#UTF::Linux 4331FC11-ap  \
     -lan_ip mc71sta1 \
     -sta {43228FC9-p2p eth2 43228FC9-p2p.1 p2p0}\
     -power {mc71npc22_0 2} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"
#softap
#UTF::Linux 43228FC9-APSTA  \
     -lan_ip mc71sta1 \
     -sta {43228FC9-apsta eth1 43228FC9-apsta.1 wl0.1 43228FC9-apsta.1_0 wl0.1:0}\
     -power {mc71npc22_0 2} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"

#43228FC9-apsta clone 43228FC9-apsta-NIGHTLY -tag NIGHTLY
#43228FC9-apsta.1 clone 43228FC9-apsta-NIGHTLY.1 -tag NIGHTLY
#43228FC9-apsta.1_0 clone 43228FC9-apsta-NIGHTLY.1_0 -tag NIGHTLY
#43228FC9-apsta clone 43228FC9-apsta-KIRIN-REL -tag KIRIN_REL_5_100_*
#43228FC9-apsta.1 clone 43228FC9-apsta-KIRIN-REL.1 -tag NIGHTLY
#43228FC9-apsta.1_0 clone 43228FC9-apsta-KIRIN-REL.1_0 -tag NIGHTLY

#UTF::Linux 4312P2P-STA  \
     -lan_ip mc71sta1\
     -sta {4312FC9-p2p eth2 4312FC9-p2p.1 wl0.1}\
     -power {mc71npc22_2 1} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"


#FC9
#UTF::Linux mc71tst3 \
        -sta "4313FC9 eth2 4313FC9_0 eth2:0"\
        -tag NIGHTLY \
        -power {mc71npc22_2 1} \
        -power_button {auto} 
        #-console "mc71end1:40003"
        #-wlinitcmds { service dhcpd status }

# Clones for mc71tst3
#4313FC9 clone 4313FC9-KIRIN -tag KIRIN_BRANCH_5_100
#4313FC9 clone 4313FC9-KIRIN-REL -tag KIRIN_REL_5_100_*



#Sniffer
UTF::Sniffer mc71tst3 \
     -sta  {snif eth3} \
     -power {mc71npc22_4 2} \
     -power_button {auto}\
     -tag RUBY_BRANCH_6_20
#KIRIN_REL_5_100_139

#55
#MAC OS client
#MacOS X16(X0)  add a tag , default is NIGHTLY 
UTF::Power::Laptop x19_power -button {webrelay1 1}
UTF::MacOS mc71tst4 \
           -sta {MacOSX19 en1} \
           -power {x19_power} \
           -tag "KIRIN_REL_5_106_98_30" \
           -brand  "macos-internal-wl-lion" \
           -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}}  \
 -pre_perf_hook {{%S wl ampdu_clear_dump}} \
           -wlinitcmds {wl msglevel 0x101} \
           -type Debug_10_7 \
           -coreserver AppleCore \
           -kextload true\
           -noafterburner 1
       #   -console "mc71end1:40003"

MacOSX19 clone MacOSX19-KIRIN  -tag "KIRIN_BRANCH_5_100" 
MacOSX19 clone MacOSX19-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
MacOSX19 clone MacOSX19-RUBY  -tag "RUBY_BRANCH_6_20"

#MacOSX16 clone MacOSX16-3 -date "2011.3.11.4" -kexload true
#MacOSX16 clone MacOSX16.1
#MacOSX16.1 configure -ipaddr 10.10.10.2 
#MacOSX16 clone MacOSX16-p2p  -type Debug_P2P_10_6

#UTF::MacOS MacOSX16-P2PSTA \
           -lan_ip mc71tst3 \
           -sta {MacOSX16-p2p en1 MacOSX16-p2p.1 p2p0 } \
           -power {x16_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_P2P_10_6 \
           -coreserver AppleCore \
           -kextload true \
           -noafterburner 1
    #MacOSX16 clone MacOSX16-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
    # -console "mc57end1:40003"
   # # -power_button { auto }
    #  -type Debug_10_6 \


#57
#UTF::MacOS  mc71tst3 \
           -sta {X16-p2p en1 X16-p2p.1 p2p0 } \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_P2P_10_6 \
           -coreserver AppleCore \
           -kextload true \
           -noafterburner 1
    #MacOSX16 clone MacOSX16-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
    # -console "mc71end1:40003"
   # # -power_button { auto }
    #  -type Debug_10_6 \


#vista  43224  
#UTF::Cygwin mc71tst2 \
        -osver 7 \
        -user user \
        -sta {43224Vista} \
        -power {mc71npc22_1 1} \
	-installer inf\
        -power_button {auto} 
#       -console "mc57end1:40004"
  
#43224Vista clone 43224Vista-KIRIN  -tag "KIRIN_BRANCH_5_100"
#43224Vista clone 43224Vista-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#43224Vista clone 43224Vista-RUBY  -tag "RUBY_BRANCH_6_20" 

#windows 7  4313
#UTF::Cygwin mc71tst3 \
        -osver 7 \
        -user user \
        -sta {4313Vista} \
        -power {mc71npc22_1 2} \
        -installer inf\
        -power_button {auto}
#       -console "mc57end1:40004"

#4313Vista clone 4313Vista-KIRIN  -tag "KIRIN_BRANCH_5_100"
#4313Vista clone 4313Vista-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#4313Vista clone 4313Vista-RUBY  -tag "RUBY_BRANCH_6_20"

#43224Win7 clone 43224Win7_p2p
#43224Win7_p2p configure -ipaddr 192.168.1.21


#windows 7 43142
UTF::Cygwin mc71tst1 \
        -osver 7 \
        -user user \
        -sta {4314Win7} \
        -tag "RUBY_REL_6_20_33"\
        -power { mc71npc22_3 1 } \
        -installer inf\
        -power_button {auto} \
        -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0 }
#       -console "mc57end1:40004"
4314Win7 clone 43142Win7-cal -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl phy_watchdog 0; wl glacial_timer 3 }
4314Win7 clone 43142Win7-wdog0 -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl phy_watchdog 0 } 

4314Win7 clone 43142Win7 -tag "RUBY_REL_6_20_*"  
43142Win7 configure -ipaddr 192.168.1.92 

###

UTF::Cygwin mc71tst2 \
        -osver 7 \
        -user user \
        -sta {4314Win7_2} \
        -tag "RUBY_REL_6_20_33"\
        -power { mc71npc22_3 1 } \
        -installer inf\
        -power_button {auto} \
        -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0 }
#       -console "mc57end1:40004"
#4314Win7_2 clone 43142Win7-cal -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl phy_watchdog 0; wl glacial_timer 3 }
#4314Win7 clone 43142Win7-wdog0 -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl phy_watchdog 0 }
4314Win7_2 clone 43142Win7_2 -tag "RUBY_REL_6_20_*"
43142Win7_2 configure -ipaddr 192.168.1.93

#########################

#windows 7 4312
#UTF::Cygwin mc71tst2 \
        -osver 7 \
        -user user \
        -sta {43142Win7} \
        -tag "NIGHTLY"\
        -power {mc71npc22_2 2} \
        -installer inf\
        -power_button {auto}
#       -console "mc57end1:40004"

#4312Win7 clone 4312Win7-KIRIN  -tag "KIRIN_BRANCH_5_100"
#4312Win7 clone 4312Win7-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#43224Win7 clone 43224Win7_p2p
#43224Win7_p2p configure -ipaddr 192.168.1.21




# Linksys AP2. wrt32
UTF::Router AP2 \
        -sta "4717_2 eth1" \
        -lan_ip 192.168.1.4 \
        -relay "mc71end1" \
        -lanpeer lan \
        -console "mc71end1:40002" \
        -power {mc71npc22_2 1} \
        -brand linux-external-router \
        -tag "AKASHI_BRANCH_5_110" \
        -nvram {
                # et0macaddr=00:23:69:3b:85:34
                #macaddr=00:90:4C:07:00:2a
                Lan_ipaddr=192.168.1.4
                lan_gateway=192.168.1.4
                dhcp_start=192.168.1.70
                dhcp_end=192.168.1.80
                lan1_ipaddr=192.168.2.4
                lan1_gateway=192.169.2.4
                dhcp1_start=192.168.2.70
                dhcp1_end=192.168.2.80
                lan1_ipaddr=192.168.2.4
                lan1_gateway=192.169.2.4
                dhcp1_start=192.168.2.70
                dhcp1_end=192.168.2.80
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=test4717-2
                wl0_channel=1
                wl0_radio=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
               # Used to WAR PR#86385
               wl0_obss_coex=0
        }
#


# Linksys AP2. wrt32
UTF::Router AP3 \
        -sta "4717_3 eth1" \
        -lan_ip 192.168.1.5 \
        -relay "mc71end1" \
        -lanpeer lan \
        -console "mc71end1:40007" \
        -power {mc71npc22_3 1} \
        -brand linux-external-router \
        -tag "AKASHI_BRANCH_5_110" \
        -nvram {
               # et0macaddr=68:7f:74:09:1e:ce
                #macaddr=00:90:4C:07:00:2a
                Lan_ipaddr=192.168.1.5
                lan_gateway=192.168.1.5
                dhcp_start=192.168.1.60
                dhcp_end=192.168.1.69
                lan1_ipaddr=192.168.2.5
                lan1_gateway=192.169.2.5
                dhcp1_start=192.168.2.60
                dhcp1_end=192.168.2.69
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=test4717-3
                wl0_channel=1
                wl0_radio=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
            # Used to WAR PR#86385
                wl0_obss_coex=0
        }

#####

# mc71npc22_1
UTF::Router AP4 \
        -sta "4718/4331 eth2" \
        -lan_ip 192.168.1.2 \
        -relay "mc71end1" \
        -lanpeer lan \
        -console "mc71end1:40001" \
        -power { mc71npc22_1 1 } \
        -brand linux-external-router \
        -tag "AKASHI_BRANCH_5_110" \
        -nvram {
                boot_hw_model=E4200
                #wandevs=et0
                fw_disable=1
                console_loglevel=7
                lan_ipaddr=192.168.1.2
                lan_gateway=192.168.1.2
                dhcp_start=192.168.1.21
                dhcp_end=192.168.1.30
                lan1_ipaddr=192.168.2.2
                lan1_gateway=192.169.2.2
                dhcp1_start=192.168.2.20
                dhcp1_end=192.168.2.30
                wl0_ssid=test_4718_0
                wl1_ssid=test_4718_1
                wl0_radio=0
                wl1_radio=1
                wl1_obss_coex=0
                wl0_obss_coex=0
        }



#
# -tag "AKASHI_BRANCH_5_110" 
#  -tag "AKASHI_REL_5_110"  2011.5.26.0


# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

package provide UTF::WebRelay 


#package require UTF::Cygwin
#package require UTF::Multiperf
#package require UTF::TclReadLines

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc57"
set ::UTF::SummaryDir "/projects/hnd_sig_ext4/fyang/mc57"


################Define Testbed Objects
# Define power controllers on cart
UTF::Power::Synaccess mc57npc22_1 -lan_ip 172.19.16.5
UTF::Power::Synaccess mc57npc22_0 -lan_ip 172.19.16.23

UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.22 -invert 1
UTF::Power::WebRelay webrelay2 -lan_ip 172.19.16.21 -invert 1
UTF::Power::WebRelay webrelay3 -lan_ip 172.19.16.21 -invert 2 
#Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc57end1" -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}


#init
set ::UTF::SetupTestBed {
     af setGrpAttn G1 0
    #af setGrpAttn G2 0
    #af setGrpAttn G3 0
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
UTF::Linux mc57end1 \
        -sta {lan eth1}
#        -wlinitcmds { service iperf restart }\
       #  -lan_ip 192.168.1.99
 
# A Laptop DUT Dell E6400 with FC9
# BCMs: "Linux 4312HMG_Dell" and "Linux 43228hm4l P403" 
UTF::Linux mc57sta1 \
        -sta "4312FC9 eth2 43228FC9 eth1 43228FC9.1 wl0.1"\
        -tag NIGHTLY \
	-power {mc57npc22_0 1} \
	-power_button {auto} \
	-console "mc57end1:40003"
        #-wlinitcmds { service dhcpd status }
 
# Clones for mc57sta1
43228FC9 clone 43228FC9-KIRIN -tag KIRIN_BRANCH_5_100
43228FC9 clone 43228FC9-KIRIN-REL -tag KIRIN_REL_5_100_*
4312FC9 clone 4312FC9-KIRIN -tag KIRIN_BRANCH_5_100
4312FC9 clone 4312FC9-KIRIN-REL -tag KIRIN_REL_5_100_*

#43228FC9 clone 43228FC9-KIRIN-REL -tag KIRIN_REL_5_100_?? 
#43228FC9 clone 43228FC9-KIRIN-REL -tag KIRIN_REL_5_100_???  
#	 -tag KIRIN_REL_5_100_98_23  
#43228FC9 clone 43228FC9_softap clone -wlinitcmds { wl down ;  wl apsta 1; wl ssid -C 1 "softap_123"; wl up; wl -i eth2 bss -C 1 ;up ifconfig wl0.1 up} 
43228FC9.1 configure -ipaddr 11.10.10.1  
#43228FC9.1 configure -ipaddr 11.10.10.1 -hasdhcpd 1 -ap 1


UTF::Linux 43228P2P-AP  \
     -lan_ip mc57sta1 \
     -sta {43228FC9-p2p eth1 43228FC9-p2p.1 p2p0}\
     -power {mc57npc22_0 1} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"

#p2p
UTF::Linux 4312FC9-P2PSTA  \
     -lan_ip mc57sta1\
     -sta {4312FC9-p2p eth2 4312FC9-p2p.1 wl0.1 4312FC-p2p.1_0 wl0.1:0 }\
     -power {mc57npc22_0 1} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"

#softap
UTF::Linux 4312FC9-APSTA \
     -lan_ip mc57sta1\
     -sta {4312FC9-apsta eth2 4312FC9-apsta.1 wl0.1 4312FC9-apsta.1_0 wl0.1:0 }\
     -power {mc57npc22_0 1} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"


UTF::Linux 43228FC9-APSTA \
     -lan_ip mc57sta1\
     -sta {43228FC9-apsta eth1 43228FC9-apsta.1 wl0.1 43228FC9-apsta.1_0 wl0.1:0 }\
     -power {mc57npc22_0 1} \
     -tcpwindow auto \
     -wlinitcmds {wl msglevel +mchan} \
     -type debug-p2p-mchan \
     -brand linux-internal-wl \
     -tag "KIRIN_BRANCH_5_100"

43228FC9-apsta clone 43228FC9-apsta-NIGHTLY -tag  NIGHTLY
43228FC9-apsta.1 clone 43228FC9-apsta-NIGHTLY.1 -tag  NIGHTLY
43228FC9-apsta.1_0 clone 43228FC9-apsta-NIGHTLY.1_0 -tag  NIGHTLY
43228FC9-apsta clone 43228FC9-apsta-KIRIN-REL -tag KIRIN_REL_5_100_*
43228FC9-apsta.1 clone 43228FC9-apsta-KIRIN-REL.1 -tag  NIGHTLY
43228FC9-apsta.1_0 clone 43228FC9-apsta-KIRIN-REL.1_0 -tag  NIGHTLY




#55
#MAC OS client
#MacOS X16(X0)  add a tag , default is NIGHTLY 
UTF::Power::Laptop x16_power -button {webrelay2 1}
UTF::MacOS mc55tst3 \
           -sta {MacOSX16 en1} \
           -power {x16_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_10_6 \
           -coreserver AppleCore \
           -kextload true
       #   -console "mc57end1:40003"

MacOSX16 clone MacOSX16-KIRIN  -tag "KIRIN_BRANCH_5_100" 
MacOSX16 clone MacOSX16-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#MacOSX16 clone MacOSX16-3 -date "2011.3.11.4" -kexload true
#MacOSX16 clone MacOSX16.1
#MacOSX16.1 configure -ipaddr 10.10.10.2 
#MacOSX16 clone MacOSX16-p2p  -type Debug_P2P_10_6

UTF::MacOS MacOSX16-P2PSTA \
           -lan_ip mc55tst3 \
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
UTF::Power::Laptop x16_power_2 -button {webrelay2 2}
UTF::MacOS  mc57tst3 \
           -sta {X16-p2p en1 X16-p2p.1 p2p0 } \
           -power {x16_power_2} \
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




#MAC OS client
#57
#MacOS X19(X0)  add a tag , default is NIGHTLY 
#UTF::Power::Laptop x19_power -button {webrelay1 1}
#UTF::MacOS mc57tst4 \
           -sta {MacOSX19 en1} \
           -power {x19_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_10_6 \
           -coreserver AppleCore \
           -kextload true 
  #       -console "mc57end1:40003"
           #-power_button { auto }

#MacOSX19 clone MacOSX19-KIRIN  -tag "KIRIN_BRANCH_5_100" 
#MacOSX19 clone MacOSX19-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#MacOSX19 clone MacOSX19-2 -date "2011.3.11.4" -kexload true
#MacOSX19 clone MacOSX19_3 -type Release_10_6 -date "2011.3.11.4"  

#UTF::MacOS MacOSX19-P2PSTA \
           -lan_ip mc57tst4 \
           -sta {MacOSX19-p2p en1 MacOSX19-p2p.1 p2p0 } \
           -power {x19_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_P2P_10_6 \
           -coreserver AppleCore \
           -kextload true \
           -noafterburner 1

#windows 7  43224  
UTF::Cygwin mc57tst2 \
        -osver 7 \
        -user user \
        -sta {43224Win7} \
        -power {mc57npc22_0 2} \
	-installer inf\
        -power_button {auto} 
#       -console "mc57end1:40004"
  
43224Win7 clone 43224Win7-KIRIN  -tag "KIRIN_BRANCH_5_100"
43224Win7 clone 43224Win7-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#43224Win7 clone 43224Win7_p2p 
#43224Win7_p2p configure -ipaddr 192.168.1.21          

#57
# Linksys 320N 4717/4322 wireless router AP1.
UTF::Router AP1 \
	-sta "4717_1 eth1" \
	-lan_ip 192.168.1.1 \
	-relay "mc57end1" \
	-lanpeer lan \
	-console "mc57end1:40001" \
	-power {mc57npc22_1 1} \
	-brand linux-external-router \
        -tag "AKASHI_BRANCH_5_110" \
	-nvram {
                et0macaddr=00:90:4c:01:00:8b
		macaddr=00:90:4c:01:00:9b
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.101
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
#

# Linksys AP2. wrt32
UTF::Router AP2 \
	-sta "4717_2 eth1" \
	-lan_ip 192.168.1.3 \
	-relay "mc57end1" \
	-lanpeer lan \
	-console "mc57end1:40005" \
	-power {mc57npc22_0 1} \
	-brand linux-external-router \
        -tag "AKASHI_BRANCH_5_110" \
	-nvram {
                et0macaddr=68:7f:74:09:1e:ce
		macaddr=00:90:4C:07:00:2a
		lan_ipaddr=192.168.1.3
		lan_gateway=192.168.1.3
		dhcp_start=192.168.1.50
  		dhcp_end=192.168.1.60
		lan1_ipaddr=192.168.2.3
		lan1_gateway=192.169.2.3
		dhcp1_start=192.168.2.50
	        dhcp1_end=192.168.2.60
		fw_disable=1
		#router_disable=1
		wl_msglevel=0x101
		wl0_ssid=test4717-2
		wl0_channel=1
		wl0_radio=0
		# Used for RSSI -35 to -45 TP Variance
		antswitch=0
       	    # Used to WAR PR#86385
		wl0_obss_coex=0
	}
#









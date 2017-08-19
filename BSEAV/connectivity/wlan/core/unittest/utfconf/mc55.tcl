

# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer

#package provide UTF::WebRelay 

package require UTF::Airport


#package require UTF::Cygwin
#package require UTF::Multiperf
#package require UTF::TclReadLines

set UTF::DataRatePlot 1
set ::UTF::Gnuplot "/usr/bin/gnuplot"

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc55"
set ::UTF::SummaryDir "/projects/hnd_sig_ext15/fyang/mc55"



################Define Testbed Objects
# Define power controllers on cart
#UTF::Power::Synaccess mc55npc22_1 -lan_ip 10.19.87.84
UTF::Power::Synaccess mc55npc22_2 -lan_ip 172.19.16.6 -relay mc55end1
UTF::Power::Synaccess mc55npc22_1 -lan_ip 172.19.16.5 -relay mc55end1 
UTF::Power::Synaccess mc55npc22_0 -lan_ip 172.19.16.4 -relay mc55end1 -rev 1

#UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.21
#UTF::Power::WebRelay webrelay2 -lan_ip 172.19.16.22
UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.22 -invert 1 -relay mc55end1 
UTF::Power::WebRelay webrelay2 -lan_ip 172.19.16.21 -invert 1 -relay mc55end1

#Attenuator - Aeroflex
#UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc55end1" -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}
UTF::Aeroflex af -lan_ip 172.19.16.30 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}} -relay mc55end1


#init
set ::UTF::SetupTestBed {
     af setGrpAttn G1 0
     af setGrpAttn G2 0
     af setGrpAttn G3 0
    unset ::UTF::SetupTestBed  
    return
}
###################

# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc55end1 \
        -sta {lan eth1}
#        -wlinitcmds { service iperf restart }\
       #  -lan_ip 192.168.1.99
lan configure -ipaddr 192.168.1.99 
# A Laptop DUT Dell E6400  
# BCMs: "Linux 4312HMG_Dell" and "Linux 43228hm4l P403" 
UTF::Linux mc55sta1 \
	-sta "43228FC15 eth0"\
	-power {mc55npc22_0 1} \
	-power_button {auto} \
        -perfchans "36l 36 6l 6"  
 

43228FC15 clone 43228FC15n -tag "NIGHTLY"
43228FC15 clone 43228FC15bi -tag "BISON_BRANCH_7_10"

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
           -perfchans "6 36 36l" \
           -kextload true
    #MacOSX16 clone MacOSX16-KIRiIN-REL  -tag "KIRIN_REL_5_100_98_*"
    #   -console "mc55end1:40003"
    # -power_button { auto }  
    #  -type Debug_10_6 
MacOSX16 clone MacOSX16-KIRIN  -tag "KIRIN_BRANCH_5_100" 
MacOSX16 clone MacOSX16-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
MacOSX16 clone MacOSX16-RUBY -tag RUBY_BRANCH_6_20
MacOSX16 clone MacOSX16v -tag "AARDVARK_BRANCH_6_30"

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
    # -console "mc55end1:40003"
   # # -power_button { auto }
    #  -type Debug_10_6 \




UTF::MacOS mc57tst3 \
           -sta {X16-p2p en1 X16-p2p.1 p2p0 } \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_P2P_10_6 \
           -coreserver AppleCore \
           -kextload true \
           -noafterburner 1 \
           -perfchans "6 36 36l"
    #MacOSX16 clone MacOSX16-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
    # -console "mc55end1:40003"
   # # -power_button { auto }
    #  -type Debug_10_6 \


UTF::MacOS StaAppleX16-1 -lan_ip mc57tst3 -user root -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_P2P_10_6  -kextload true -tag KIRIN_BRANCH_5_100  -noafterburner 1 

UTF::MacOS StaAppleX16-3 -lan_ip mc55tst3 -user root -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_P2P_10_6  -kextload  true -tag KIRIN_BRANCH_5_100 -noafterburner 1
###

UTF::Power::Laptop X29b_power -button {webrelay1 2}
UTF::MacOS mc57tst4 \
           -sta {X29b en0 X29b.1 p2p0 } \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-lion" \
           -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}}  \
 -pre_perf_hook {{%S wl ampdu_clear_dump}} \
           -wlinitcmds {wl msglevel 0x101} \
           -type Debug_10_7 \
           -power {X29b_power} \
           -coreserver AppleCore \
           -kextload true \
           -perfchans "36l 36 6" \
           -noafterburner 1

X29b clone X29b-KIRIN-REL -tag "KIRIN_REL_5_100_98_*"
X29b clone X29b-KIRIN  -tag "KIRIN_BRANCH_5_100"
X29b clone X29b-RUBY  -tag "RUBY_BRANCH_6_20"
X29b clone X29bv -tag "AARDVARK_BRANCH_6_30"

X29b clone X29bbi -tag "BISON_BRANCH_7_10"



#-tag "KIRIN_REL_5_106_98_30" 


    #MacOSX16 clone MacOSX16-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
    # -console "mc55end1:40003"
   # # -power_button { auto }
    #  -type Debug_10_6 \

#macos-internal-wl-lion -type Debug_10_6 \  -tag "KIRIN_REL_5_106_98_18" 
# --type Debug_10_6 \type Debug_10_6 \
#UTF::MacOS StaAppleX16-1 -lan_ip mc57tst3 -user root -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_P2P_10_6  -kextload true -tag KIRIN_BRANCH_5_100  -noafterburner 1 
X29b clone X29b-RUBY -tag "RUBY_BRANCH_6_20"


#UTF::MacOS StaAppleX16-3 -lan_ip mc55tst3 -user root -brand "macos-internal-wl-snowleopard" -coreserver AppleCore -type Debug_P2P_10_6  -kextload  true -tag KIRIN_BRANCH_5_100 -noafterburner 1

###

#MAC OS client
#MacOS X19(X0)  add a tag , default is NIGHTLY 
UTF::Power::Laptop x19_power -button {webrelay1 1}
UTF::MacOS mc55tst4 \
           -sta {MacOSX19 en1} \
           -power {x19_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-zin" \
           -type Debug_10_8 \
           -coreserver AppleCore \
           -kextload true \
          -perfchans "36l 36 6" 
  #       -console "mc55end1:40003"
           #-power_button { auto }


MacOSX19 clone MacOSX19_s
MacOSX19_s configure -ipaddr 192.168.1.22
MacOSX19 clone MacOSX19-KIRIN  -tag "KIRIN_BRANCH_5_100" 
MacOSX19 clone MacOSX19-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
MacOSX19 clone MacOSX19-RUBY  -tag "RUBY_BRANCH_6_20"
MacOSX19 clone MacOSX19v -tag "AARDVARK_BRANCH_6_30"
MacOSX19 clone MacOSX19v_s -tag "AARDVARK_BRANCH_6_30"
MacOSX19v_s configure -ipaddr 192.168.1.22 




#MacOSX19 clone MacOSX19-2 -date "2011.3.11.4" -kexload true
#MacOSX19 clone MacOSX19_3 -type Release_10_6 -date "2011.3.11.4"  

UTF::MacOS MacOSX19-P2PSTA \
           -lan_ip mc55tst4 \
           -sta {MacOSX19-p2p en1 MacOSX19-p2p.1 p2p0 } \
           -power {x19_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-snowleopard" \
           -type Debug_P2P_10_6 \
           -coreserver AppleCore \
           -kextload true \
           -noafterburner 1\
           -perfchans "6 36 36l"

#windows 7  43224  
UTF::Cygwin mc55tst2 \
        -osver 7 \
        -user user \
        -sta {43224Win7} \
        -power {mc55npc22_0 2} \
	-installer inf\
         -perfchans "6 6l 36 36l" \
        -power_button {auto} 
#       -console "mc55end1:40004"
  
43224Win7 clone 43224Win7-KIRIN  -tag "KIRIN_BRANCH_5_100"
43224Win7 clone 43224Win7-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
43224Win7 clone 43224Win7n -tag "NIGHTLY"
43224Win7 clone 43224Win7v -tag "AARDVARK_BRANCH_6_30"
43224Win7 clone 43224Win7bi -tag "BISON_BRANCH_7_10"

43224Win7 clone 43224Win7_p2p 
43224Win7 clone 43224Win7-ip
43224Win7-ip configure -ipaddr 11.10.10.2
          
# Linksys 320N 4717/4322 wireless router AP1.
UTF::Router AP1 \
	-sta "47171 eth1" \
	-lan_ip 192.168.1.1 \
	-relay "mc55end1" \
	-lanpeer lan \
	-console "mc55end1:40001" \
	-power {mc55npc22_1 1} \
	-brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
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

# Linksys E4200 wireless router AP2.
#AKASHI_BRANCH_5_110
UTF::Router AP2_0 \
	-sta "4718 eth1" \
	-lan_ip 192.168.1.2 \
	-relay "mc55end1" \
	-lanpeer lan \
	-console "mc55end1:40004" \
        -power {mc55npc22_2 1} \
	-brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
	-nvram {
                boot_hw_model=E4200
	        fw_disable=1
                console_loglevel=7
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.50
  		dhcp_end=192.168.1.80
		lan1_ipaddr=192.168.2.50
		lan1_gateway=192.169.2.80
		dhcp1_start=192.168.2.100
	        dhcp1_end=192.168.2.149
		wl0_ssid=test4718_0
                wl1_ssid=test4718_1
                wl0_radio=1
                wl1_radio=0
                wl1_obss_coex=0
                wl0_obss_coex=0
	}




UTF::Router AP2_1 \
	-sta "4718/4331 eth2" \
	-lan_ip 192.168.1.2 \
	-relay "mc55end1" \
	-lanpeer lan \
	-console "mc55end1:40004" \
        -power {mc55npc22_2 1} \
	-brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
	-nvram {
                boot_hw_model=E4200
	        fw_disable=1
                console_loglevel=7
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.50
  		dhcp_end=192.168.1.80
		lan1_ipaddr=192.168.2.50
		lan1_gateway=192.169.2.80
		dhcp1_start=192.168.2.100
	        dhcp1_end=192.168.2.149
		wl0_ssid=test4718_0
                wl1_ssid=test4718_1
                wl0_radio=0
                wl1_radio=1
                wl1_obss_coex=0
                wl0_obss_coex=0
	}

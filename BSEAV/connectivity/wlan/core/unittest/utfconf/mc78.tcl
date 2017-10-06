
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package provide UTF::WebRelay 

package require UTF::Airport

set UTF::DataRatePlot 1
set ::UTF::Gnuplot "/usr/bin/gnuplot"

#package require UTF::Cygwin
#package require UTF::Multiperf
#package require UTF::TclReadLines

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc62"
#set ::UTF::SummaryDir "/projects/hnd_sig_ext7/fyang/mc71"
#set ::UTF::SummaryDir "/projects/hnd_sig_ext3/fyang/mc62"
#set ::UTF::SummaryDir "/projects/hnd_sig_ext9/fyang/mc78"
set ::UTF::SummaryDir "/projects/hnd_sig_ext9/$::env(LOGNAME)/mc78"

################Define Testbed Objects
# Define power controllers on cart
UTF::Power::Synaccess mc78npc22_0 -lan_ip 172.19.16.74 -relay "mc78end1" -rev 1
UTF::Power::Synaccess mc78npc22_1 -lan_ip 172.19.16.72 -relay "mc78end1"


#sub 78_2
UTF::Power::Synaccess mc78_2npc22_0 -lan_ip 172.19.16.76 -relay "mc78end2"
UTF::Power::Synaccess mc78_2npc22_1 -lan_ip 172.19.16.77 -relay "mc78end2" -rev 1
UTF::Power::Synaccess mc78_2npc22_2 -lan_ip 172.19.16.79 -relay "mc78end2" -rev 1
#UTF::Power::Synaccess mc78_2npc22_3 -lan_ip 172.19.16.81 -relay "mc78end2" -rev 1
UTF::Power::Synaccess mc78_2npc22_4 -lan_ip 172.19.16.82 -relay "mc78end2" -rev 1

#dhd console and power cycler for win8_64
UTF::Power::Synaccess mc78npc22_2 -lan_ip 172.19.16.81 -relay "mc78end1" -rev 1


#UTF::Power::Synaccess mc62npc22_5 -lan_ip 172.19.16.31
#UTF::Power::Synaccess mc62npc22_2 -lan_ip 172.19.16.37
#UTF::Power::Synaccess mc62npc22_3 -lan_ip 172.19.16.34
#UTF::Power::Synaccess mc62npc22_4 -lan_ip 172.19.16.135
#UTF::Power::Synaccess mc71npc22_1 -lan_ip 172.19.16.51 
#UTF::Power::Synaccess mc71npc22_2 -lan_ip 172.19.16.53  
#UTF::Power::Synaccess mc71npc22_3 -lan_ip 172.19.16.52  
#UTF::Power::Synaccess mc71npc22_4 -lan_ip 172.19.16.39 
#
#UTF::Power::Synaccess mc71npc22_5 -lan_ip 172.19.16.71 -relay "mc78end1" -rev 1
#UTF::Power::Synaccess mc71npc22_6 -lan_ip 172.19.16.72 -relay "mc78end1" -rev 1


UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.75 -invert 1 -relay "mc78end1"
#UTF::Power::WebRelay webrelay2 -lan_ip 172.19.16.21 -invert 1
# UTF::Power::WebRelay webrelay3 -lan_ip 172.19.16.21 -invert 2 
#Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc78end1" -group { G1 {1 2 3 } G2 {4 5 6} ALL {1 2 3 4 5 6} } 

#init UTF.tcl sta shutdown_reboot
set ::UTF::SetupTestBed {
     package require UTF::Test::RebootTestbed
      UTF::Try "Reboot Testbed" {
      #UTF::Test::RebootTestbed -hostlist "4360FC15v 43224FC15 43224MacX16"
      UTF::Test::RebootTestbed -hostlist "r63/4360_3 4706/4360v 4360FC15v 43224MacX16 43224Win7_1 4706/4360_2v 4360FC15_2v 43142Win7" 
    }
    
    AP1 restart wl0_radio=0
    AP1 restart wl1_radio=0
    AP2 restart wl0_radio=0
    AP2 restart wl1_radio=0
    catch { 4360FC15v wl down }
    catch { 4717_2 wl down }
    catch { 4706/4360v wl down }
    catch { 4706/4360_2v wl down }
    catch { 4706/4360_2 deinit }
    catch { 4706/4360 deinit }
    catch { 43228Win8_2 deinit }
    catch { 4706/4331_2v wl down }
    catch { 43228FC15_2 wl down }
    catch { 43224MacX16 wl down }
    catch { snif wl down }
    catch { r63/4360_3 wl down }
    catch { r63/4331_3 wl down }
    catch { 43526Win7  power_sta cycle }
    #af setGrpAttn G1 10 , for txpower issue
    af setGrpAttn G1 0
    af setGrpAttn G2 0
    unset ::UTF::SetupTestBed  
    return
}
#########p2p
set ::bt_dut ""      		;# BlueTooth device under test
set ::bt_ref ""      	;# BlueTooth reference board
set ::wlan_dut 43228FC9-p2p		;# HND WLAN device under test
set ::wlan_rtr 4717-1   		;# HND WLAN router
set ::wlan_tg lan 

#AP1 and AP2
set  ::src_apmac_index "wl0_hwaddr"
set  ::src_ap_ssid_index "wl0_ssid"
set  ::dst_apmac_index "wl1_hwaddr"
set  ::dst_ap_ssid_index "wl1_ssid"

#
###################

#RvRFastSweep mystepprofile
set UTF::StepProfile(myprofile) ""
set dwell 1
set max 65
set stepsize 1
set upramp ""
set downramp ""
for {set ix 0} {$ix < $max} {incr ix +$stepsize} { 
lappend upramp "$stepsize $dwell" 
}
for {set ix $max} {$ix > 0} {incr ix -$stepsize} {
  lappend downramp "-$stepsize $dwell"
}
set UTF::StepProfile(myprofile) [concat $upramp $downramp]



# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
UTF::Linux mc78end1 \
        -sta {lan eth1}
#        -lan_ip 192.168.1.99
#        -wlinitcmds { service iperf restart }\
#        -lan_ip 192.168.1.99
#
lan configure -ipaddr 192.168.1.99
# its was -sta {lan2 eth6}

UTF::Linux mc78end2 \
         -sta {lan2 eth0 }
#        -lan_ip 192.168.1.99
#        -wlinitcmds { service iperf restart }\
#        -lan_ip 192.168.1.99
#
lan2 configure -ipaddr 192.168.1.100



 
# A Laptop DUT Dell E6400 with FC9
# BCMs: "Linux 4312HMG_Dell" and "Linux 43228hm4l P403" 
UTF::Linux mc78sta1 \
        -sta "4360FC15 eth0"\
	-power {mc78npc22_0 1}\
	-power_button {auto}\
        -wlinitcmds {wl msglevel +mchan}\
        -brand linux-internal-wl\
	-console "mc78end1:40002" \
        -perfchans {6 6l 36 36l 36/80}
# 
4360FC15 clone 4360FC15v -tag AARDVARK_BRANCH_6_30

4360FC15v clone 4360FC15v_txpw11  -tag AARDVARK_BRANCH_6_30 -wlinitcmds {wl msglevel +mchan ; wl up ; wl txpwr1 -o 11 }

4360FC15 clone 4360FC15p2pv -tag AARDVARK_BRANCH_6_30 -type debug-p2p-mchan
4360FC15 clone 4360FC15p2p -type debug-p2p-mchan


4360FC15 clone 4360FC15_s 
4360FC15_s configure -ipaddr 192.168.1.91

4360FC15v clone 4360FC15v_s
4360FC15v_s configure -ipaddr 192.168.1.91


4360FC15p2pv clone  4360FC15p2pv_s
4360FC15p2pv_s configure -ipaddr 192.168.1.9

4360FC15p2p clone  4360FC15p2p_s
4360FC15p2p_s configure -ipaddr 192.168.1.9



 
4360FC15 clone 4360FC15ap -ap 1 
4360FC15ap configure -ipaddr 192.168.1.91

4360FC15v clone 4360FC15vap -ap 1
4360FC15vap configure -ipaddr 192.168.1.91

4360FC15p2pv clone 4360FC15p2pvap -ap 1
4360FC15p2pvap configure -ipaddr 192.168.1.91

4360FC15p2p clone 4360FC15p2pap -ap 1
4360FC15p2pap configure -ipaddr 192.168.1.91

4360FC15 clone 4360FC15v_rel  -tag AARDVARK_REL_6_30_34

####p2p feature test
4360FC15p2pv clone 4360FC15v-P2P \
        -sta { 4360FC15v-p2p.1 wl0.1}
4360FC15p2pv clone 4360FC15v-WLAN \
        -sta {  4360FC15v-p2p eth0}

############

#hostconsole 40002
# mc78end2:40002" -> mc78end1:40008" 
UTF::Linux mc78tst2 \
        -sta "4360FC15_2 eth0 43228FC15 eth1" \
        -power {mc78_2npc22_1 1}\
        -power_button {auto}\
        -wlinitcmds {wl msglevel +mchan}\
        -brand linux-internal-wl\
        -perfchans {6 6l 36 36l 36/80} \
        -console "mc78end2:40002"

4360FC15_2 clone 4360FC15_2_s
4360FC15_2_s configure -ipaddr 192.168.1.95

4360FC15_2 clone 4360FC15_2v -tag AARDVARK_BRANCH_6_30
4360FC15_2v clone 4360FC15_2v_s
4360FC15_2v_s configure -ipaddr 192.168.1.95

4360FC15_2 clone 4360FC15_2p2p -type debug-p2p-mchan
4360FC15_2v clone 4360FC15_2p2pv -type debug-p2p-mchan




#
4360FC15_2v clone 4360FC15_2vap -ap 1
4360FC15_2vap configure -ipaddr 192.168.1.95

4360FC15_2 clone 4360FC15_2ap -ap 1
4360FC15_2ap configure -ipaddr 192.168.1.95

#
43228FC15 clone 43228FC15_2 -perfchans {6 6l 36 36l}

43228FC15_2 clone 43228FC15_2_s
43228FC15_2_s configure -ipaddr 192.168.1.95

43228FC15_2 clone 43228FC15_2ap -ap 1
43228FC15_2ap configure -ipaddr 192.168.1.95

43228FC15_2 clone 43228FC15_2v -tag AARDVARK_BRANCH_6_30
43228FC15_2v clone 43228FC15_2v_s
43228FC15_2v_s configure -ipaddr 192.168.1.95
43228FC15_2v clone 43228FC15_2vap -ap 1
43228FC15_2vap configure -ipaddr 192.168.1.95



####p2p feature test, the p2p interface is wl0.1
4360FC15_2p2pv clone 4360FC15_2v-P2P \
        -sta { 4360FC15_2v-p2p.1 wl0.1}
4360FC15_2p2pv clone 4360FC15_2v-WLAN \
        -sta {  4360FC15_2v-p2p eth0}

#old
#E4:D5:3D:DF:21:F7 
#new A 516
UTF::Cygwin mc78tst3 \
          -osver 7 \
        -user user \
        -sta {43142Win7 C0:18:85:C6:C5:F9  }\
        -node {DEV_4365}\
        -tag "NIGHTLY"\
        -power { mc78_2npc22_1 2 } \
        -installer inf\
        -power_button {auto}
#        -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0 }

#

43142Win7 clone 43142Win7_RUBY55 -tag "RUBY_REL_6_20_55_54"
#date 2012.5.23.0 is good  build
# 
43142Win7 clone 43142Win7_s
43142Win7_s configure -ipaddr 192.168.1.92

43142Win7 clone 43142Win7v -tag "AARDVARK_BRANCH_6_30"



43142Win7v clone 43142Win7v_s
43142Win7v_s configure -ipaddr 192.168.1.92

#DEV_4353
UTF::Cygwin mc78tst3B\
      -osver 7 \
    -user user \
    -lan_ip mc78tst3\
    -node {DEV_4353}\
    -sta { 43224Win7 C4:46:19:A3:3B:D1} \
    -tag "NIGHTLY"\
    -power { mc78npc22_0 2 } \
    -installer inf\
    -power_button {auto}


43224Win7 clone 43224Win7_s
43224Win7_s configure -ipaddr 192.168.1.93

43224Win7 clone 43224Win7v -tag "AARDVARK_BRANCH_6_30"
43224Win7v clone 43224Win7v_s
43224Win7v_s configure -ipaddr 192.168.1.93



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

#sniffer

#Sniffer 43224fc15 and 4360fc15
UTF::Sniffer mc78tst4 \
     -sta  {snif eth1 } \
     -power_button {auto}\
     -power { mc78_2npc22_2 1 } \
     -tag  AARDVARK_BRANCH_6_30 \
     -date 2012.7.30.1

UTF::Sniffer mc78tst4_b \
     -sta  {snif2 eth0 } \
     -lan_ip mc78tst4\
     -power_button {auto}\
     -power { mc78_2npc22_2 1 } \
       -brand linux-internal-wl\
     -tag NIGHTLY
#     -date 2012.3.14.0
#
#     -date 2011.12.14.0





######################
#  -hostconsole "mc78end1:40003" \
#

# C0:CB:38:7C:40:7F
UTF::Cygwin mc78tst5 \
       -osver 7 \
       -user user \
        -sta {43224Win7_1 C0:CB:38:7C:40:7F}\
        -node {DEV_4353}\
        -power {mc78npc22_0 2}\
        -installer inf\
        -power_button {auto}\
        -perfchans {6 6l 36 36l } 
#
43224Win7_1 clone 43224Win7_1v -tag AARDVARK_BRANCH_6_30

43224Win7_1v clone 43224Win7_1vs -tag AARDVARK_BRANCH_6_30
43224Win7_1vs configure -ipaddr 192.168.1.94

43224Win7_1 clone 43224Win7_1s
43224Win7_1s configure -ipaddr 192.168.1.94

#
UTF::Power::WebRelay hub -lan_ip 172.19.16.80 -relay mc78end1 -invert 1 


################
#
#43526
#   -console "mc78end1:40002"\
#     -hostconsole "mc78end1:40003"\
#

UTF::Power::WebRelay hub -lan_ip 172.19.16.80 -relay mc78end1 -invert 1 
# new 00:90:4C:0E:60:11
# 00:90:4C:0D:F4:3E 
UTF::WinDHD  mc78tst5_2 \
    -lan_ip mc78tst5 \
     -sta {43526Win7  00:90:4C:0E:60:11 } \
    -node {PID_BD1D} \
    -embeddedimage 4360a0 \
    -power { mc78npc22_0 2 } \
    -user user -osver 7 \
    -tag  "AARDVARK_BRANCH_6_30"\
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -power_sta {hub 1} \
    -nobighammer 1 \
    -nocal 1 \
    -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3}\
    -msgactions {
        {dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL
    }

43526Win7 clone 43526Win7v  -tag  "AARDVARK_BRANCH_6_30" -type checked/DriverOnly_Sdio_BMac 

###################
#55
#MAC OS client
#MacOS X16(X0)  add a tag , default is NIGHTLY 
#UTF::Power::Laptop x19_power -button {webrelay1 1}
#UTF::MacOS mc71tst4 \
           -sta {MacOSX19 en1} \
           -power {x19_power} \
           -tag "KIRIN_REL_5_106_98_30" \
           -brand  "macos-internal-wl-lion" \
           -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}}  \
 -pre_perf_hook {{%S wl ampdu_clear_dump}} \
           -wlinitcmds {wl msglevel 0x101} \
           -type Debug_10_7 \
s           -coreserver AppleCore \
           -kextload true\
           -noafterburner 1
       #   -console "mc71end1:40003"

#MacOSX19 clone MacOSX19-KIRIN  -tag "KIRIN_BRANCH_5_100" 
#MacOSX19 clone MacOSX19-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#MacOSX19 clone MacOSX19-RUBY  -tag "RUBY_BRANCH_6_20"

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
UTF::Power::Laptop x16_power -button {webrelay1 1}
UTF::MacOS  mc78tst1 \
           -sta {43224MacX16 en1 X16-p2p.1 p2p0 } \
           -power {x16_power} \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-zin" \
           -type Debug_10_8 \
           -coreserver AppleCore \
           -kextload true \
           -perfchans "2 2l 36 36l"\
           -noafterburner 1

43224MacX16 clone 43224MacX16v -tag  "AARDVARK_BRANCH_6_30"
43224MacX16 clone 43224MacX16-KIRIN -tag  "KIRIN_REL_5_106_98_87"

#vista  43224  
#UTF::Cygwin mc71tst2 \
        -user user \
        -sta {4313XP} \
	-installer inf\
        -power_button {auto} 
  
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
#UTF::Cygwin mc71tst1 \
        -osver 7 \
        -user user \
        -sta {4314Win7} \
        -tag "RUBY_REL_6_20_33"\
        -power { mc71npc22_3 1 } \
        -installer inf\
        -power_button {auto}
#        -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0 }
#       -console "mc57end1:40004"
#4314Win7 clone 43142Win7-cal -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl phy_watchdog 0; wl glacial_timer 3 }
#4314Win7 clone 43142Win7-wdog0 -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl phy_watchdog 0 } 

#4314Win7 clone 43142Win7 -tag "RUBY_REL_6_20_55_*"  
#4314Win7 clone 43142Win7 -tag "RUBY_REL_6_20_55_43"

#config the sta with static ip 
#43142Win7 configure -ipaddr 192.168.1.92 

#config the sta with static ip
#4314Win7 clone 43142Win7_s -tag "RUBY_REL_6_20_55_*" 
#43142Win7_s configure -ipaddr 192.168.1.92
 
#4314Win7 clone 43142Win7v -tag "AARDVARK_REL_6_30_9_*"
#4314Win7 clone 43142Win7twig -tag "RUBY_TWIG_6_20_5*"
#4314Win7 clone 43142Win7n -tag "NIGHTLY"

#4314Win7 clone 43142Win7n_s -tag "NIGHTLY"
#43142Win7n_s configure -ipaddr 192.168.1.92




#4314Win7 clone 43142Win7-wd1 -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0;wl wd_disable 1 } -tag "RUBY_REL_6_20_55_*" 
#43142Win7-wd1 configure -ipaddr 192.168.1.9

###43142 on XP
#UTF::Cygwin mc71tst4 \
        -user user \
        -sta {4314XP} \
        -tag "RUBY_REL_6_20_33"\
        -power { mc71npc22_6 1 } \
        -installer inf\
        -power_button {auto} \
        -wlinitcmds { wl PM 0 ; wl aspm 0x102 ; wl mpc 0 }
#
#4314XP clone 43142XP -tag "RUBY_REL_6_20_55_*"  
#43142Win7 configure -ipaddr 192.168.1.92 


###

#########################
#ramsyc 2 
#windows 8
#E4:D5:3D:DF:21:F7 43142
 
# 00:26:82:25:40:4F 43224

UTF::Cygwin mc78tst6 \
        -osver 8 \
        -user user \
        -sta {43142Win8_2 E4:D5:3D:DF:21:F7 } \
        -node {DEV_4365}\
        -power {mc71npc22_2 2} \
        -installer inf\
        -brand win8_internal_wl\
        -tag AARDVARK_REL_6_30_59_44 \
        -power_button {auto}


#43224Win8_2 clone 43224Win8_2v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl
43142Win8_2 clone 43142Win8_2v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl


UTF::Cygwin mc78tst6_2 \
        -lan_ip mc78tst6\
       -osver 8 \
        -user user \
        -sta {43228Win8_2 C0:F8:DA:7C:33:74} \
        -node {DEV_4359}\
        -power {mc71npc22_2 2} \
        -installer inf\
        -brand win8_internal_wl\
        -tag AARDVARK_REL_6_30_59_44 \
         -power_button {auto}



43228Win8_2 clone 43228Win8_2v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl


###win8_64
UTF::Cygwin mc78tst7 \
        -osver 864 \
        -user user \
        -sta {43224Win8_64 00:26:82:25:40:4F } \
        -node {DEV_4353} \
        -power {mc78npc22_2 2} \
        -installer inf\
        -brand win8_internal_wl\
        -tag AARDVARK_REL_6_30_59_44 \
        -power_button {auto}

43224Win8_64 clone 43224Win8_64v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl
#43142Win8_2 clone 43142Win8_2v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl

UTF::Cygwin mc78tst7_1 \
        -lan_ip mc78tst7\
        -osver 864 \
        -user user \
        -sta {43142Win8_64 E4:D5:3D:DF:29:4B}\
        -node {DEV_4365}\
        -power {mc78npc22_2 2} \
        -installer inf\
        -brand win8_internal_wl\
        -tag AARDVARK_REL_6_30_59_44 \
        -power_button {auto}

43142Win8_64 clone 43142Win8_64v -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl
43142Win8_64 clone 43142Win8_64n -tag NIGHTLY -brand win8_internal_wl
43142Win8_64 clone 43142Win8_64r -tag RUBY_BRANCH_6_20 -brand win8_internal_wl




#RUBY_REL_6_20_55_*

###

#4312Win7 clone 4312Win7-KIRIN  -tag "KIRIN_BRANCH_5_100"
#4312Win7 clone 4312Win7-KIRIN-REL  -tag "KIRIN_REL_5_100_98_*"
#43224Win7 clone 43224Win7_p2p
#43224Win7_p2p configure -ipaddr 192.168.1.21

#

UTF::Router AP1 \
    -sta { 4706/4360 eth1 } \
    -relay "mc78end1" \
    -lanpeer lan \
     -lan_ip 192.168.1.2\
    -brand linux26-external-vista-router-full-src \
    -console "mc78end1:40001" \
    -power { mc78npc22_1 1} \
    -tag "AARDVARK_REL_6_30_18_2" \
    -txt_override {
        wl_dmatxctl=0x24c0040
        wl_dmarxctl=0x24c0000
        wl_pcie_mrrs=128
        watchdog=6000
    } \
    -nvram {
        et0macaddr=00:90:4c:01:22:6f
       # macaddr=00:10:18:A9:17:6C
       # wl0_hwaddr=00:10:18:A9:17:6C
        watchdog=6000; # PR#90439
        fw_disable=1
        wl_msglevel=0x101
        wl1_ssid=mc78_4706/4360    # it  was 4706/4360_1
	boardtype=0x05b2; # 4706nr
        wl0_channel=1
        wl0_radio=1
        wl0_ssid=mc78_4706/4360 # 4706/4360_0
        wl1_channel=36
        wl1_radio=0
        wl_dmatxctl=0x24c0040
        wl_dmarxctl=0x24c0000
        wl_pcie_mrrs=128
        wl0_obss_coex=0
        lan_ipaddr=192.168.1.2
    }\
  -perfchans {6 6l 36 26l 36/80} -nowep 1 -nombss 1



4706/4360 clone 4706/4360v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router
4706/4360 clone 4706/4360rel -tag "AARDVARK_REL_6_30_70" -brand linux26-internal-router


4706/4360v configure -attngrp G1
4706/4360 configure -attngrp G1
4706/4360rel configure -attngrp G1

#mc78_2 's ap
#change to lan from lan2 ;from mc78end1 from mc78end2
# mc78end2:40001 -> mc78end1:40006
UTF::Router AP2 \
    -sta { 4706/4331_2 eth1 4706/4360_2 eth2 4706/4360_2.%15 wl1.% } \
    -relay "mc78end2" \
    -lanpeer lan2 \
    -lan_ip 192.168.1.3\
    -brand  linux26-internal-router\
    -console "mc78end2:40001" \
    -power { mc78_2npc22_0 1} \
    -tag "AARDVARK_REL_6_30_18_2" \
    -txt_override {
        wl_dmatxctl=0x24c0040
        wl_dmarxctl=0x24c0000
        wl_pcie_mrrs=128
        watchdog=6000
    } \
    -nvram {
        et0macaddr=00:90:4C:08:A9:27
        pci/2/1/macaddr=00:90:4c:09:39:27
        #1_bssid4360macaddr=00:10:18:A9:35:CD
        #wl1_hwaddr=00:10:18:A9:35:CD
        #0_bssid4331macaddr=00:10:18:A9:05:F5
        #wl0_hwaddr=00:10:18:A9:05:F5
        watchdog=6000; # PR#90439
        fw_disable=1
        wl_msglevel=0x101
        wl0_ssid=4706/4331_2   # for 4331 4706/4331_2
        boardtype=0x05b2; # 4706nr
        wl0_channel=1
        wl0_radio=0
        wl1_ssid=mc78_4706/4360   #  for 4360 4706/4360_2
        wl1_channel=36
        wl1_radio=0
        wl_dmatxctl=0x24c0040
        wl_dmarxctl=0x24c0000
        wl_pcie_mrrs=128
        wl0_obss_coex=0
        wl1_obss_coex=0
        lan_ipaddr=192.168.1.3
    }\
  -perfchans {6 6l 36 26l 36/80} -nowep 1 -nombss 1




4706/4360_2 clone 4706/4360_2v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router

4706/4331_2 clone 4706/4331_2v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router

## AARDVARK_REL_6_30_70  2012.6.4.0

4706/4360_2 clone 4706/4360_2rel -tag "AARDVARK_REL_6_30_70" -brand linux26-internal-router
4706/4331_2 clone 4706/4331_2rel -tag "AARDVARK_REL_6_30_70" -brand linux26-internal-router


4706/4360_2v configure -attngrp G2
4706/4331_2v configure -attngrp G2

4706/4360_2 configure -attngrp G2
4706/4331_2 configure -attngrp G2

4706/4360_2rel configure -attngrp G2
4706/4331_2rel configure -attngrp G2


#

###########3

# Linksys AP2. wrt32
UTF::Router AP5 \
        -sta "4717_2 eth1" \
        -lan_ip 192.168.1.6 \
        -relay "mc78end1" \
        -lanpeer lan \
         -lan_ip 192.168.1.6 \
        -console "mc78end1:40005" \
    -power { mc78npc22_1 2 } \
        -brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
        -nvram {
                # et0macaddr=00:23:69:3b:85:34
                #macaddr=00:90:4C:07:00:2a # bssid or radio mac
                lan_ipaddr=192.168.1.6 
                lan_gateway=192.168.1.6
                dhcp_start=192.168.1.115
                dhcp_end=192.168.1.110
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
                wl0_ssid=test4717_2
                wl0_channel=1
                wl0_radio=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
               # Used to WAR PR#86385
                 wl0_obss_coex=0
                #
                #lan_proto=static
                #lan_dhcp=0
        }


4717_2 configure -attngrp G1

#




####



#netgear R6300  r6300
# to lan1 from lan2 ; to mc78end1 from mc78end2
# mc78end2:40003 -> mc78end1:40007
UTF::Router AP4 \
    -sta { r63/4331_3 eth1 r63/4360_3 eth2 } \
    -relay "mc78end2" \
    -lanpeer lan2 \
    -lan_ip 192.168.1.5\
    -brand  linux26-internal-router\
    -console "mc78end2:40003" \
    -power { mc78_2npc22_4 1} \
    -tag "AARDVARK_REL_6_30_70" \
    -nvram {
        #et0macaddr=00:90:4C:08:A9:27
        #pci/2/1/macaddr=00:90:4c:09:39:27
        #1_bssid4360macaddr=00:10:18:A9:35:CD
        #wl1_hwaddr=00:10:18:A9:35:CD
        #0_bssid4331macaddr=00:10:18:A9:05:F5
        #wl0_hwaddr=00:10:18:A9:05:F5
        #watchdog=6000; # PR#90439
        fw_disable=1   
       # wl_msglevel=0x101
        wl0_ssid=mc78r634331_3   # for 4331 4706/4331_2
        #boardtype=0x05b2; # 4706nr
        wl0_channel=1
        #wl0_radio=0
        wl1_ssid=mc78r634360_3   #  for 4360 4706/4360_2
        wl1_channel=36
        #wl1_radio=0
        #wl_dmatxctl=0x24c0040
        #wl_dmarxctl=0x24c0000
        #wl_pcie_mrrs=128
        wl0_obss_coex=0
        wl1_obss_coex=0
        lan_ipaddr=192.168.1.5
        1:boardflags2=0x4000000
    }\
  -perfchans {6 6l 36 26l 36/80} -nowep 1 -nombss 1

# AARDVARK_REL_6_30_70  2012.6.4.0



r63/4360_3 clone r63/4360_3v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router

r63/4331_3 clone r63/4331_3v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router

#r63/4360_3v configure -attngrp G1
#r63/4331_2v configure -attngrp G1

#r63/4360_2 configure -attngrp G1
#r636/4331_2 configure -attngrp G1

#

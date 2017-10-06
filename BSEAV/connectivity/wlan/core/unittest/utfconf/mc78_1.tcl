
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package provide UTF::WebRelay 
package require UTF::utils
package require utfconf
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
#set ::UTF::SummaryDir "/projects/hnd_sig_ext9/$::env(LOGNAME)/mc78_1"
set ::UTF::SummaryDir "/projects/hnd_sig_ext15/$::env(LOGNAME)/mc78_1"
#set ::UTF::SummaryDir "/projects/hnd_sig_arc2/$::env(LOGNAME)/mc78_1"

################Define Testbed Objects
# Define power controllers on cart
UTF::Power::Synaccess mc78npc22_0 -lan_ip 172.19.16.74 -relay "mc78end1" -rev 1
UTF::Power::Synaccess mc78npc22_1 -lan_ip 172.19.16.72 -relay "mc78end1"

UTF::Power::Synaccess mc78npc23_1 -lan_ip 172.19.16.73 -relay "mc78end1" -rev 1




#mc78sta1 dhd
UTF::Power::WebRelay hub2 -lan_ip 172.19.16.83 -relay "mc78end1" -invert 1

#dhd console and power cycler for win8_64
UTF::Power::Synaccess mc78npc22_2 -lan_ip 172.19.16.81 -relay "mc78end1" -rev 1

UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.75 -invert 1 -relay "mc78end1"
#Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc78end1" -group { G1 {1 2 3 } G2 {4 5 6} ALL {1 2 3 4 5 6} } 

#init UTF.tcl sta shutdown_reboot
set ::UTF::SetupTestBed {
     package require UTF::Test::RebootTestbed
      UTF::Try "Reboot Testbed" {
    #  UTF::Test::RebootTestbed -hostlist "4360FC15apv 4360FC15 X51v 43224Win7_1 43224Win8_64" 
       UTF::Test::RebootTestbed -hostlist "4360FC15apv 4360FC15 X51v 43224Win7_1"  ;# remove 43224Win8_64 cause an issue
    }
    
   catch { snif wl down }
#   catch { 4717_2 wl down }
#    catch { 4706/4360v wl down }
    catch { 4360FC15apv wl down }
    catch { 43224Win7_1 wl down }
#    catch { 43224Win8_64 wl down }
    catch { 43236Win7v power_sta cycle }
    catch { 43526Win7v power_sta cycle }
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
        -sta {lan eth2}
#        -lan_ip 192.168.1.99
#        -wlinitcmds { service iperf restart }\
#        -lan_ip 192.168.1.99

lan configure -ipaddr 192.168.1.99
# its was -sta {lan2 eth6}

# softap
#mc78npc23_1
UTF::Linux mc78tst1 \
        -sta "4360FC15AP eth0"\
	-power {mc78npc23_1 1}\
	-power_button {auto}\
        -wlinitcmds {wl msglevel 0x101; wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0 } \
        -brand linux-internal-wl\
        -tag  AARDVARK_BRANCH_6_30\
        -perfchans {36/80 36l 36 6l 6}

4360FC15AP clone 4360FC15apv -ap 1 
4360FC15apv configure -ipaddr 192.168.1.89

4360FC15apv configure -attngrp G1

4360FC15AP clone 4360FC15apv_bf -ap 1 
4360FC15apv_bf configure -attngrp G1 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }  -ipaddr 192.168.1.89


########

#
# A Laptop DUT Dell E6400 with FC9
# BCMs: "Linux 4312HMG_Dell" and "Linux 43228hm4l P403" 
UTF::Linux mc78sta1 \
        -sta "4360FC15 eth0"\
	-power {mc78npc22_0 1}\
	-power_button {auto}\
        -wlinitcmds {wl msglevel 0x101; wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0 } \
        -brand linux-internal-wl\
	-console "mc78end1:40002" \
        -perfchans {6 6l 36 36l 36/80}
#  
4360FC15 clone 4360FC15v -tag AARDVARK_BRANCH_6_30

4360FC15v clone 4360FC15v_ifs  -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) }  -wlinitcmds { wl down ; wl msglevel +mchan ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ;wl up }

 4360FC15v_ifs  configure -ipaddr 192.168.1.91


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

########

UTF::DHD mc78sta1usb \
     -lan_ip mc78sta1 \
     -sta "43242usb eth0" \
     -power "mc78npc22_0 1" \
     -power_button "auto"\
     -power_sta "hub2 1"\
     -brand linux-external-dongle-usb \
     -tag PHOENIX2_REL_6_10_166_3 \
     -type "43242a0min-roml/usb-ag.bin.trx" \
     -dhd_tag DHD_REL_1_49 \
     -dhd_brand linux-internal-dongle-usb \
     -nvram /home/fyang/images/43242/133_242usbref_p304.nvm \
     -driver dhd-cdc-usb-gpl
#    -nvram /home/fyang/images/43242/bcm943242usbref_p304.txt\
# 

#43242usb clone 43242usb_s configure -ipaddr 192.168.1.21

43242usb clone 43242usb_rel \
      -brand linux-external-dongle-usb \
      -tag PHOENIX2_REL_6_10_166_* \
     -type "43242a0-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tlds-vsdb-proptxstatus-autoabn.bin.trx"\
     -dhd_tag NIGHTLY \
     -dhd_brand linux-internal-dongle-usb\
     -nvram /home/fyang/images/43242/133_242usbref_p304.nvm \
     -driver dhd-cdc-usb-gpl


#   -nvram src/shared/nvram/bcm943242usbref_p304.txt\
#     -nvram /home/fyang/images/43242/bcm943242usbref_p304.txt\
# -nvram /projects/hnd/software/gallery/src/shared/nvram/bcm943242usbref_p304.txt


43242usb clone 43242usb_b6 \
      -brand linux-internal-dongle-usb \
     -tag PHOENIX2_BRANCH_6_10 \
      -type "43242a0-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-vsdb-proptxstatus-err-assert-autoabn.bin.trx"\
     -dhd_tag NIGHTLY \
     -dhd_brand linux-internal-dongle-usb\
     -nvram src/shared/nvram/bcm943242usbref_p304.txt\
     -driver dhd-cdc-usb-gpl


#     -nvram /home/fyang/images/43242/bcm943242usbref_p304.txt\

# -dhd_tag AARDVARK_BRANCH_6_30 or NIGHTLY

43242usb_b6 clone 43242usb_b7 -dhd_tag NIGHTLY -nvram /home/fyang/images/43242/133_242usbref_p304.nvm 

#privious one 
43242usb_b6 clone 43242usb_b8 -type "43242a0min-roml/usb-ag-p2p-mchan-idauth-idsup-keepalive-vsdb.bin.trx" -nvram /home/fyang/images/43242/133_242usbref_p304.nvm 

#43242usb_b7 clone 43242usb_b7s
#43242usb_b7s configure -ipaddr 10.10.10.5

43242usb_b6 clone 43242usb_b9 -type "43242a0min-roml/usb-ag-mfgtest-seqcmds.bin.trx" -dhd_tag DHD_REL_1_49 -tag P2166RC3_REL_6_10_168_4 -brand linux-mfgtest-dongle-usb -nvram /home/fyang/images/43242/133_242usbref_p304.nvm

####
#p461 a1
43242usb clone 43242usbv\
     -brand linux-internal-wl\
     -tag AARDVARK_BRANCH_6_30\
     -type "43242a1-bmac/ag-assert/rtecdc.bin.trx"\
     -dhd_tag AARDVARK_BRANCH_6_30\
     -dhd_brand linux-internal-wl\
     -nvram "/projects/hnd/software/gallery/src/shared/nvram/bcm943242usbref_p461.txt"\
     -driver "debug-apdef-stadef-high"
#-alwayspowercycledongle 0
#-stabin /projects/hnd_software_ext9/work/kadavath/UTF_Testing/rtecdc.bin.trx 
#-stadhd /projects/hnd_software_ext9/work/kadavath/UTF_Testing/wl.ko

43242usbv clone 43242usbv_s
43242usbv_s configure -ipaddr 192.168.1.88


43242usbv clone 43242usbv_p2ps -type "43242a1-bmac/ag-assert-p2p-mchan-media/rtecdc.bin.trx"
43242usbv_p2ps configure -ipaddr 192.168.1.88



####p2p feature test
43242usb_b6 clone 43242usb_b6-P2P \
        -sta { 43242usb_b6-p2p.1 wl0.1}
43242usb_b6 clone 43242usb_b6-WLAN \
        -sta { 43242usb_b6-p2p eth0 }

43242usbv clone 43242usbv-P2P \
        -sta { 43242usbv-p2p.1 wl0.1}
43242usbv clone 43242usv-WLAN \
        -sta { 43242usbv-p2p eth0 }


#HIGHTLY

43242usb clone 43242usbn\
     -brand linux-internal-wl\
     -tag NIGHTLY\
     -type "43242a0-bmac/ag-assert-p2p-mchan/rtecdc.bin.trx"\
     -dhd_tag AARDVARK_BRANCH_6_30\
     -dhd_brand linux-internal-wl\
     -nvram "/projects/hnd/software/gallery/src/shared/nvram/bcm943242usbref_p461.txt"\
     -driver "debug-apdef-stadef-high"


43242usbn clone 43242usbn_s
43242usbn_s configure -ipaddr 192.168.1.88

############
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
# new 00:90:4C:0E:60:93
# 00:90:4C:0D:F4:3E 
#  
# -embeddedimage 4360a0  -> 43526a p452
UTF::WinDHD  mc78tst5_2 \
    -lan_ip mc78tst5 \
     -sta {43526Win7 00:90:4C:0E:60:93 } \
    -node  {VID_0A5C&PID_BD1D} \
    -embeddedimage 43526a-bmac \
    -power { mc78npc22_0 2 } \
    -user user -osver 7 \
    -tag  "AARDVARK_BRANCH_6_30"\
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -power_sta {hub 1} \
    -alwayspowercycledongle 1\
    -nobighammer 1 \
    -nocal 1 \
    -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3}\
    -wlinitcmds { wl btc_mode 0; wl down; wl vht_features 1; wl up }\
    -msgactions {
        {dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL
    }


43526Win7 clone 43526Win7v  -tag  "AARDVARK_BRANCH_6_30" -type checked/DriverOnly_Sdio_BMac 


43526Win7 clone 43526Win7_s 
43526Win7_s configure -ipaddr 192.168.1.93

#NIGHTLY TOT
43526Win7 clone 43526Win7n  -tag  "NIGHTLY" -type checked/DriverOnly_Sdio_BMac 

43526Win7n clone 43526Win7n_s 
43526Win7n_s configure -ipaddr 192.168.1.93


43526Win7_s clone 43526Win7_bfs -wlinitcmds { wl btc_mode 0; wl down; wl vht_features 1; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up; wl assert_type 1 } 



###

# USB\VID_0A5C&PID_BD17\000000000001
#00:90:4C:03:21:23
#
UTF::WinDHD  mc78tst5_3 \
    -lan_ip mc78tst5 \
    -sta {43236Win7 00:90:4C:03:21:23 } \
    -node {VID_0A5C&PID_BD17} \
    -embeddedimage 43236b \
    -power { mc78npc22_0 2 } \
    -user user -osver 7 \
    -tag  "NIGHTLY"\
    -brand win_internal_wl \
    -type checked/DriverOnly_BMac \
    -alwayspowercycledongle 1 \
    -power_sta {hub 4} \
     -hack 0 \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S rte rpcdump} {%S wl dump rpc} {%S wl dump ampdu} {%S rte mu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}\
    -user user 

43236Win7 clone 43236Win7v  -tag  "AARDVARK_BRANCH_6_30" -type checked/DriverOnly_BMac 

43236Win7v clone 43236Win7v_s
43236Win7v_s configure -ipaddr 192.168.1.95

#NIGHTLY
43236Win7 clone 43236Win7_s
43236Win7_s configure -ipaddr 192.168.1.95


###################
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


43224MacX16v clone 43224MacX16v-WLAN \
        -sta {  43224MacX16v-p2p en1}
43224MacX16v clone 43224MacX16v-P2P \
        -sta { 43224MacX16v-p2p.1 p2p0}



##boot air
UTF::MacOS mc78tst8 \
           -sta {X51 en1 X51.1 p2p0 } \
           -tag "NIGHTLY" \
           -brand  "macos-internal-wl-cab" \
           -wlinitcmds {wl msglevel 0x101 wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1 } \
           -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
           -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
           -type Debug_10_9 \
           -power {x16_power} \
           -coreserver AppleCore \
           -kextload true \
           -perfchans "36/80 36l 3"

X51 clone X51v -tag "AARDVARK_BRANCH_6_30"
X51v clone X51v_s -tag "AARDVARK_BRANCH_6_30"
X51v_s configure -ipaddr 192.168.1.90

X51v clone X51v_rel -tag AARDVARK_REL_6_30_223_*
X51v_rel clone X51v_rels -tag AARDVARK_REL_6_30_223_* 
X51v_rels configure -ipaddr 192.168.1.90




X51v clone X51v-WLAN \
        -sta {  X51v-p2p en1}
X51v clone X51v-P2P \
        -sta {  X51v-p2p.1 p2p0}


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
43142Win8_64v clone 43142Win8_64vs 
43142Win8_64vs configure -ipaddr 192.168.1.92

43142Win8_64 clone 43142Win8_64n -tag NIGHTLY -brand win8_internal_wl

43142Win8_64 clone 43142Win8_64r -tag RUBY_BRANCH_6_20 -brand win8_internal_wl


UTF::Power::Synaccess mc78_2npc22_2 -lan_ip 172.19.16.79 -relay "mc78end1" -rev 1

UTF::Sniffer mc78tst4 \
     -sta  {snif2 eth1 } \
     -power_button {auto}\
     -power { mc78_2npc22_2 1 } \
     -tag  AARDVARK_BRANCH_6_30
 #    -date 2012.7.30.1

UTF::Sniffer mc78tst4_b \
     -sta  {snif eth1 } \
     -lan_ip mc78tst4\
     -power_button {auto}\
     -power { mc78_2npc22_2 1 } \
      -brand linux-internal-wl\
     -tag KIRIN_REL_5_100_139
   #  -date 2012.3.14.0

#KIRIN_REL_5_100_139
#RUBY_BRANCH_6_20
#
UTF::Linux mc78tst4_snif \
     -lan_ip mc78tst4\
     -sta { 43224_interf eth1}\
     -wlinitcmds {wl mpc 0} \
     -tag AARDVARK_BRANCH_6_30 \
     -wlinitcmds {wl mpc 0} \
    -brand "linux-internal-wl" \
    -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu}}\
    -pre_perf_hook {{%S wl ampdu_clear_dump}}
 
# -date 2012.12.14.0  
#43224_interf configure -attngrp G3


UTF::Router AP1 \
    -sta { 4706/4360 eth1 } \
    -relay "lan" \
    -lanpeer lan \
    -lan_ip 192.168.1.2\
    -brand linux26-external-vista-router-full-src \
    -console "mc78end1:40001" \
    -power { mc78npc22_1 1} \
    -tag "AARDVARK_REL_6_30_18_2" \
    -nvram {
        watchdog=6000
        #boardtype=0x054d; # 4706nr
        fw_disable=1
        wl_msglevel=0x101
        #et0macaddr=00:90:4c:01:22:6f
        #macaddr=00:90:4C:0F:50:9A
        #wl0_hwaddr=00:90:4C:0F:50:9A
       
        wl1_ssid=mc78_4706/4360    # it  was 4706/4360_1
	#boardtype=0x05b2; # 4706nr
        wl0_radio=1
        wl0_ssid=mc78_4706/4360
        wl0_channel=36
        wl1_radio=0
        #wl_dmatxctl=0x24c0040
        #wl_dmarxctl=0x24c0000
        #wl_pcie_mrrs=128
        wl0_obss_coex=0
        lan_ipaddr=192.168.1.2
        lan_gateway=192.168.1.2
        antswitch=0
        dhcp_start=192.168.1.110
        dhcp_end=192.168.1.119
        lan1_ipaddr=192.168.2.2
        lan1_gateway=192.168.2.2
        dhcp1_start=192.168.2.110
        dhcp1_end=192.168.2.119
        lan_proto=dhcp
        lan_dhcp=1
   }\
  -perfchans {36/80 36l 36 } -nowep 1 -nombss 1



4706/4360 clone 4706/4360v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router
#4706/4360 clone 4706/4360rel -tag "AARDVARK_REL_6_30_70" -brand linux26-internal-router


4706/4360 clone 4706/4360rel -tag "AARDVARK_REL_6_30_163_22" -brand linux26-internal-router


4706/4360v configure -attngrp G1
4706/4360 configure -attngrp G1
4706/4360rel configure -attngrp G1


4706/4360v clone 4706/4360v_bf 
4706/4360v_bf configure -attngrp G1 -wlinitcmds { wl down; wl -u txbf_bfe_cap 1 ; wl -u txbf_bfr_cap 1; wl -u txbf 1 ; wl up }


##########

# Linksys AP2. wrt32 it was .6
UTF::Router AP2 \
        -sta "4717_2 eth1" \
        -lan_ip 192.168.1.3 \
        -relay "mc78end1" \
        -lanpeer lan \
        -console "mc78end1:40005" \
         -power { mc78npc22_1 2 } \
        -brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
        -nvram {
                et0macaddr=00:23:69:3b:85:34
                macaddr=00:90:4C:07:00:3a 
                lan_ipaddr=192.168.1.3 
                lan_gateway=192.168.1.3
                dhcp_start=192.168.1.160
                dhcp_end=192.168.1.170
                lan1_ipaddr=192.168.2.3
                lan1_gateway=192.169.2.3
                dhcp1_start=192.168.2.70
                dhcp1_end=192.168.2.80
                lan1_ipaddr=192.168.2.3
                lan1_gateway=192.169.2.3
                dhcp1_start=192.168.2.160
                dhcp1_end=192.168.2.170
                fw_disable=1
                router_disable=1
                wl_msglevel=0x101
                wl0_ssid=test4717_2
                wl0_channel=1
                wl0_radio=1
                # Used for RSSI -35 to -45 TP Variance
                antswitch=0
               # Used to WAR PR#86385
                 wl0_obss_coex=0
                #
                lan_proto=dhcp
                lan_dhcp=1
        }

4717_2 configure -attngrp G1

#




####




#

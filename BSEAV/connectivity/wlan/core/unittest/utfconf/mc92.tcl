
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::utils

package require utfconf
package provide UTF::WebRelay 

package require UTF::Airport

set UTF::DataRatePlot 1
set ::UTF::Gnuplot "/usr/bin/gnuplot"

#package require UTF::Cygwin
#package require UTF::Multiperf
#package require UTF::TclReadLines

# Optional items for controlchart.test to run each iteration
#set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD Linux]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
#set ::UTF::SummaryDir "/projects/hnd_sig_ext9/$::env(LOGNAME)/mc92"

set ::UTF::SummaryDir "/projects/hnd_sig_ext15/$::env(LOGNAME)/mc92"


################Define Testbed Objects
# Define power controllers on cart


UTF::Power::Synaccess mc92_2npc22_1 -lan_ip 172.19.16.110 -relay "mc92end1" -rev 1
UTF::Power::Synaccess mc92npc22_2 -lan_ip 172.19.16.39 -relay "mc92end1"

UTF::Power::Synaccess mc92npc22_3 -lan_ip 172.19.16.111 -relay "mc92end1" -rev 1


UTF::Power::WebRelay hub -lan_ip 172.19.16.112 -relay "mc92end1"

#softap
UTF::Power::Synaccess mc92npc22_4 -lan_ip 172.19.16.113 -relay "mc92end1" -rev 1

#Attenuator - Aeroflex
#UTF::Aeroflex af -lan_ip 172.19.16.10 -relay "mc92end1" -group { G1 {1 2 3 } G2 {4 5 6} ALL {1 2 3 4 5 6} } 

UTF::Aeroflex af -lan_ip 172.19.16.10:20000/udp \
    -relay "mc92end1" -group { G3 {7 8 9 }  G3_1 { 7 8 } G3_2 { 9 }  ALL {7 8 9} }



#init UTF.tcl sta shutdown_reboot
set ::UTF::SetupTestBed {
     catch { 4360FC15ap wl down }
    #catch { snif load }
    #catch { snif wl down }
    af setGrpAttn G3 0
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
UTF::Linux mc92end1 \
        -sta {lan eth1 }
#        -lan_ip 192.168.1.99
#        -wlinitcmds { service iperf restart }\
#        -lan_ip 192.168.1.99
#
lan configure -ipaddr 192.168.1.100
# its was -sta {lan2 eth6}


##



#UTF::Linux mc92end2 \
#         -sta {lan2 p19p1 }
#        -lan_ip 192.168.1.101
#        -wlinitcmds { service iperf restart }\
#        -lan_ip 192.168.1.99
#
#lan configure -ipaddr 192.168.1.101




# UTF Endpoint1 FC9 - Traffic generators (no wireless cards)
# A Laptop DUT Dell E6400 with FC9
# BCMs: "Linux 4312HMG_Dell" and "Linux 43228hm4l P403" 
# soft ap
UTF::Linux mc92tst1 \
        -sta "4360FC15 eth0"\
	-power { mc92npc22_4 1}\
	-power_button {auto}\
        -tcpwindow 2m \
        -udp 1.2g\
        -wlinitcmds {wl msglevel +mchan ; wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans { 36/80 6l }
# 
4360FC15 configure -attngrp G3

4360FC15 clone 4360FC15v -tag AARDVARK_BRANCH_6_30

4360FC15 clone 4360FC15bi -tag BISON_BRANCH_7_10


#p2p
4360FC15 clone 4360FC15p2p -type debug-p2p-mchan
4360FC15 clone 4360FC15p2pv -tag AARDVARK_BRANCH_6_30 -type debug-p2p-mchan


4360FC15 clone 4360FC15ap -tag AARDVARK_BRANCH_6_30 -type debug-p2p-mchan 
4360FC15ap configure -ipaddr 10.10.10.1 -ap 1



4360FC15 clone 4360FC15apbi -tag  BISON_BRANCH_7_10
4360FC15apbi configure -ipaddr 10.10.10.1 -ap 1


4360FC15ap clone 4360FC15ap_ifs -wlinitcmds { wl down ;wl msglevel +mchan ; wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0; wl ampdu 0;  wl bi 200 ; wl up } -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) }

4360FC15ap_ifs configure -ipaddr 10.10.10.1 -ap 1


4360FC15v clone 4360FC15v_ifs -wlinitcmds { wl down ;wl msglevel +mchan ; wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0; wl ampdu 0; wl bi 200 ; wl up } -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) }


UTF::Linux mc92tst1_x29 \
        -lan_ip mc92tst1\
        -sta "X29cFC15 eth0"\
        -power { mc92npc22_2 1}\
        -power_button {auto}\
        -tcpwindow 3600K \
        -wlinitcmds {wl msglevel +mchan ; wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans { 36/80 6l }






#        -sta "43242FC15_2 eth0"\
#        -power { mc92npc22_2 2}\
#        -power_button {auto}\
#        -wlinitcmds {wl msglevel +mchan}\
#        -brand linux-internal-wl\
#        -perfchans {6 6l 36 36l}
# 
#4360FC15 clone 4360FC15v -tag AARDVARK_BRANCH_6_30
# Zin highest macos-internal-wl-ml
# cab macos-internal-wl-cab
UTF::Power::WebRelay webrelay1 -lan_ip 172.19.16.40 -invert 1 -relay mc92end1 
UTF::Power::Laptop X29c_power -button {webrelay1 1 }
UTF::MacOS mc92tst3 \
           -sta {X29c en0 X29c.1 p2p0 } \
           -tag "NIGHTLY" \
           -tcpwindow 2m \
           -udp 1.2g\
           -brand  "macos-internal-wl-cab" \
           -wlinitcmds { wl msglevel 0x101 wl msglevel +scan ; wl btc_mode 0; wl down; wl vht_features 1; wl up; wl assert_type 1 } \
           -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump phycal}} \
           -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
           -type Debug_10_9 \
           -power {X29c_power} \
           -coreserver AppleCore \
           -kextload true \
           -perfchans "36/80 36l 3" 



X29c clone X29cv -tag "AARDVARK_BRANCH_6_30"
X29c clone X29cr -tag "AARDVARK_REL_6_30_118_28"


X29c clone X51
X51 clone X51_s
X51_s configure -ipaddr 10.10.10.2


X29cv clone X51v

X29c clone X29cv_s -tag "AARDVARK_BRANCH_6_30" 
X29cv_s configure -ipaddr 10.10.10.2


X51v clone X51v_s -tag "AARDVARK_BRANCH_6_30"  -datarate {-b 900m -i 0.5 } -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl up }
X51v_s configure -ipaddr 10.10.10.2


X51v clone X51_rel -tag BU4360B1_REL_6_30_227_* 
X51_rel clone X51_rels -tag BU4360B1_REL_6_30_227_* -tcpwindow 3600K
X51_rels configure -ipaddr 10.10.10.2

X51v clone X51v_rel -tag   AARDVARK_REL_6_223_*
X51v_rel clone X51v_rels -tag AARDVARK_REL_6_223_*
X51v_rels configure -ipaddr 10.10.10.2

X51v clone X51n_s -tag NIGHTLY
X51n_s configure -ipaddr 10.10.10.2


X51v clone X51bis -tag BISON_BRANCH_7_10
X51bis configure -ipaddr 10.10.10.2


X51bis clone X51bis_ifs -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) } -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ; wl wme 1 ; wl up }
X51bis_ifs configure -ipaddr 10.10.10.2


X51_rels clone X51_rels_ifs -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) } -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ; wl wme 1 ; wl up }

X51_rels_ifs configure -ipaddr 10.10.10.2


######p2p feature test######
4360FC15p2pv clone 4360FC15v-P2P \
        -sta { 4360FC15v-p2p.1 wl1.1}
4360FC15p2pv clone 4360FC15v-WLAN \
        -sta { 4360FC15v-p2p eth1}




# -console "mc92end1:40001"\
#    -hostconsole "mc92end1:40003"
# -power_sta "mc92npc5 2"

#4360FC15_2 -> X52c 4360 2x2
UTF::Linux mc92tst2 \
        -sta "4360FC15_2 eth0"\
	-power { mc92npc22_2 2}\
	-power_button {auto}\
        -tcpwindow 2m \
        -udp 1.2g\
        -wlinitcmds {wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans { 36/80 6l }
#wl msglevel +scan;  wl vht_features 1 ; wl btc_mode 0  

4360FC15_2 clone 4360FC15_2s
4360FC15_2s configure -ipaddr 10.10.10.3

4360FC15_2 clone 4360FC15_2v -tag AARDVARK_BRANCH_6_30
4360FC15_2 clone 4360FC15_2p2p -type debug-p2p-mchan
4360FC15_2 clone 4360FC15_2p2pv -tag AARDVARK_BRANCH_6_30 -type debug-p2p-mchan
4360FC15_2 clone 4360FC15_2vs -tag AARDVARK_BRANCH_6_30
4360FC15_2vs configure -ipaddr 10.10.10.3 


# BISON_BRANCH_7_10
4360FC15_2 clone 4360FC15_2bi -tag  BISON_BRANCH_7_10
4360FC15_2 clone 4360FC15_2bis -tag BISON_BRANCH_7_10
4360FC15_2bis configure -ipaddr 10.10.10.3

#AARDVARK01T_TWIG_6_37_14
4360FC15_2 clone 4360FC15_2twig -tag AARDVARK01T_TWIG_6_37_14 -type debug-p2p-mchan
4360FC15_2twig clone 4360FC15_2twigs
4360FC15_2twigs configure -ipaddr 10.10.10.3



4360FC15_2bis clone 4360FC15_2bis_ifs -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) } -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ; wl wme 1; wl up }

4360FC15_2vs clone 4360FC15_2vs_ifs -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) } -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ; wl wme 1; wl up }


4360FC15_2bi  clone 4360FC15_2bi_ifs -datarate {-b 350m -i 0.5 -frameburst 0  -skiptx (1-48) -skiprx (1-48) } -wlinitcmds { wl down ; wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 ; wl PM 0 ; wl ampdu 0 ; wl wme 1; wl up }




#4360FC15_2vs_ifs clone 4360FC15_2vs37_ifs -tag  AARDVARK01T_BRANCH_6_37 
#4360FC15_2vs37_ifs configure -ipaddr 10.10.10.3

 


#4360FC15_2vs clone 4360FC15_2vs_rel -tag AARDVARK_REL_6_30_254_4

4360FC15_2 clone 4360FC15_2ap -tag AARDVARK_BRANCH_6_30 -type debug-p2p-mchan
4360FC15_2ap configure -ipaddr 10.10.10.3 -ap 1


4360FC15_2p2pv clone 4360FC15_2v-P2P \
        -sta { 4360FC15_2v-p2p.1 wl0.1}
4360FC15_2p2pv clone 4360FC15_2v-WLAN \
        -sta { 4360FC15_2v-p2p eth0}


###
UTF::Linux mc92tst7 \
        -sta "4360FC15ap0 eth0"\
        -power_button {auto}\
        -wlinitcmds {wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans {36/80 36l 36 6l 6}\
        -tag AARDVARK_BRANCH_6_30\
        -type debug-p2p-mchan

4360FC15ap0 configure -ipaddr 10.10.10.7 -ap 1

###




# 43143
#-hostconsole "mc89end1:40003" \
# -power {npctst1 1} 
UTF::DHD mc92tst7usb \
        -lan_ip mc92tst7 \
        -sta {43143b0 eth0} \
        -tcpwindow 1152K \
        -power_button "auto" \
        -brand linux-internal-dongle-usb \
        -dhd_brand linux-internal-dongle-usb \
        -driver dhd-cdc-usb-gpl \
        -tag PHOENIX2_BRANCH_6_10 \
        -dhd_tag NIGHTLY \
        -nvram "src/shared/nvram/bcm943143usbirdsw_p450.txt" \
        -type "43143b0-roml/usb-g-p2p-mchan-idauth-idsup-keepalive-wowlpf-pno-aoe-vsdb-proptxstatus-assert.bin.trx" \
        -wlinitcmds {wl msglevel +assoc; wl down; wl mimo_bw_cap 1} \
        -perfchans {3l 3} -nocal 1 -slowassoc 5 -noframeburst 1 \
        -datarate {-skiptx 0x3-9x3 -skiprx 0x3-9x3} \
        -nobighammer 1 \
        -pre_perf_hook {{%S wl ampdu_clear_dump}} \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0}} \
        -msgactions {{dbus_usb_dl_writeimage: Bad Hdr or Bad CRC} FAIL} \
        -nvram_add {
            # Disable noise cal (PR#103469 PR#103468)
            noise_cal_enable_2g=0
            noise_cal_enable_5g=0
        }


#
# add X52c
UTF::Linux mc92tst2X52c \
        -lan_ip mc92tst2 \
        -sta "X52c eth0"\
        -power { mc92npc22_2 2}\
        -power_button {auto}\
         -tcpwindow 3600K \
        -wlinitcmds {wl msglevel +mchan ; wl vht_features 1 ; wl btc_mode 0 }\
        -brand linux-internal-wl\
        -perfchans { 36/80 6 }\
        -tag AARDVARK_BRANCH_6_30 



#

UTF::DHD mc92tst2usb \
     -lan_ip mc92tst2 \
     -sta "43242usb eth0" \
     -power "mc92npc22_2 2" \
     -power_button "auto" \
     -power_sta "hub 1"\
     -brand linux-external-dongle-usb \
     -tag PHOENIX2_REL_6_10_166_3 \
     -type "43242a0min-roml/usb-ag.bin.trx" \
     -dhd_tag DHD_REL_1_49 \
     -dhd_brand linux-internal-dongle-usb\
     -nvram /home/fyang/images/43242/131_242usbref_p304.nvm \
     -driver dhd-cdc-usb-gpl 


#     -nvram /home/fyang/images/43242/bcm943242usbref_p304.txt\
#
43242usb clone 43242usb_s
43242usb_s configure -ipaddr 10.10.10.5

##########

43242usb clone 43242usb_rel \
      -brand linux-external-dongle-usb \
      -tag PHOENIX2_REL_6_10_166_* \
     -type "43242a0-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tlds-vsdb-proptxstatus-autoabn.bin.trx"\
     -dhd_tag NIGHTLY \
     -dhd_brand linux-internal-dongle-usb\
     -nvram /home/fyang/images/43242/131_242usbref_p304.nvm \
     -driver dhd-cdc-usb-gpl

#     -nvram /home/fyang/images/43242/bcm943242usbref_p304.txt\
# -nvram src/shared/nvram/bcm943242usbref_p304.txt\

43242usb_rel clone 43242usb_rels
43242usb_rels configure -ipaddr 10.10.10.5

#

43242usb clone 43242usb_b6 \
      -brand linux-internal-dongle-usb \
      -tag PHOENIX2_BRANCH_6_10 \
     -type "43242a0-roml/usb-ag-p2p-mchan-idauth-idsup-aoe-pno-keepalive-pktfilter-wowlpf-tdls-vsdb-proptxstatus-err-assert-autoabn.bin.trx"\
     -dhd_tag NIGHTLY \
     -dhd_brand linux-internal-dongle-usb\
     -nvram src/shared/nvram/bcm943242usbref_p304.txt\
     -driver dhd-cdc-usb-gpl


#     -nvram /home/fyang/images/43242/bcm943242usbref_p304.txt\

43242usb_b6 clone 43242usb_b6s
43242usb_b6s configure -ipaddr 10.10.10.5

43242usb_b6 clone 43242usb_b7 -dhd_tag NIGHTLY -nvram /home/fyang/images/43242/131_242usbref_p304.nvm
43242usb_b7 clone 43242usb_b7s 
43242usb_b7s configure -ipaddr 10.10.10.5

# previous one 
43242usb_b6 clone 43242usb_b8 -type "43242a0min-roml/usb-ag-p2p-mchan-idauth-idsup-keepalive-vsdb.bin.trx" -nvram /home/fyang/images/43242/133_242usbref_p304.nvm
43242usb_b8 clone 43242usb_b8s
43242usb_b8s configure -ipaddr 10.10.10.5

43242usb_b6 clone 43242usb_b9 -type "43242a0min-roml/usb-ag-mfgtest-seqcmds.bin.trx" -dhd_tag DHD_REL_1_49 -tag P2166RC3_REL_6_10_168_4 -brand linux-mfgtest-dongle-usb -nvram /home/fyang/images/43242/131_242usbref_p304.nvm 
43242usb_b9 clone 43242usb_b9s
43242usb_b9s configure -ipaddr 10.10.10.5



#-dhd_tag NIGHTLY
# or -dhd_tag AARDVARK_BRANCH_6_30
#
#P2P
43242usb_rel clone 43242usb_rel-P2P \
        -sta { 43242usb_rel-p2p.1 wl0.1}
43242usb_rel clone 43242usb_rel-WLAN \
        -sta { 43242usb_rel-p2p eth0 }


43242usb_b6 clone 43242usb_b6-P2P \
        -sta { 43242usb_b6-p2p.1 wl0.1}
43242usb_b6 clone 43242usb_b6-WLAN \
        -sta { 43242usb_b6-p2p eth0 }



##########
UTF::Sniffer mc92tst4 \
     -sta  {snif eth0} \
     -power_button {auto}\
     -tag AARDVARK_BRANCH_6_30\
      -power {  mc92npc22_3 1 }


###########3
#-relay "mc92end2" 
# Linksys AP2. wrt32
UTF::Router AP2 \
        -sta "4717_2 eth1" \
        -lan_ip 192.168.1.3 \
        -lanpeer lan \
         -relay "mc92end1" \
        -console "mc92end1:40001" \
        -power {  mc92_2npc22_1 1 } \
        -brand linux-external-router \
        -tag "AKASHI_REL_5_110_65" \
        -nvram {
                et0macaddr=00:01:36:22:7f:3d
                macaddr=00:90:4C:07:00:4a
                lan_ipaddr=192.168.1.3
                lan_gateway=192.168.1.3
                dhcp_start=192.168.1.131
                dhcp_end=192.168.1.140
                lan1_ipaddr=192.168.2.3
                lan1_gateway=192.169.2.3
                dhcp1_start=192.168.2.131
                dhcp1_end=192.168.2.140
                fw_disable=1
                #router_disable=1
                wl_msglevel=0x101
                wl0_ssid=test4717_2
                wl0_channel=1
                wl0_radio=1
                # Used for RSSI -35 to -45 TP Variance
                 antswitch=0
                 wl0_obss_coex=0
                 lan_proto=dhcp
                 lan_dhcp=1
        }


#4717_2 configure -attngrp G1

#




####



#netgear R6300  r6300
UTF::Router AP3 \
    -sta { r63/4331_1 eth1 r63/4360_1 eth2 } \
    -relay "mc92end1" \
    -lanpeer lan \
    -lan_ip 192.168.1.4\
    -brand  linux26-internal-router\
    -console "mc92end1:40002" \
    -power { mc92_2npc22_1 2 } \
    -tag "AARDVARK_REL_6_30_70" \
    -nvram {
     et0macaddr=00:90:4C:0D:Bb:61
     1:macaddr=00:90:4C:0D:Cb:61
     0:macaddr=00:90:4C:0E:5b:61
       # fw_disable=1   
        wl_msglevel=0x101
        wl0_ssid=mc92r634331_1  
        wl0_channel=1
        wl0_radio=1
        wl1_ssid=mc92r634360_1 
        wl1_channel=36
        wl1_radio=1
        wl0_obss_coex=0
        wl1_obss_coex=0
        lan_ipaddr=192.168.1.4
        lan_proto=dhcp 
        lan_dhcp=1
        dhcp_start=192.168.1.141
        dhcp_end=192.168.1.150
        console_loglevel=7
    }\
  -perfchans {6 6l 36 36l 36/80} -nowep 1 -nombss 1

# AARDVARK_REL_6_30_70  2012.6.4.0


r63/4360_1 clone r63/4360_1v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router
#2012.9.11.0 is good
r63/4331_1 clone r63/4331_1v -tag "AARDVARK_BRANCH_6_30" -brand linux26-internal-router



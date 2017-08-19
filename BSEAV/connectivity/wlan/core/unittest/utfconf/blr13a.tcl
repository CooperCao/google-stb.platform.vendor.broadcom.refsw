#Testbed configuration file for blr09end1 UTF  Teststation
#Created by Sumesh Nair on 19Nov2014 05:00 PM  
#Last checkin 19Nov2014 
####### Controller section:
# blr09end1: FC15
# IP ADDR 10.131.80.101
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
#  
# 
#
####### STA section:
#
# blr08tst1: 
# blr08tst2: 
# blr08tst3:
# blr08tst4: 
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr13end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
				G3 {7 8 9}
		ALL {1 2 3}
                }
#G1 configure default 0
#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 0
    catch {G2 attn 0;}
    catch {G1 attn 0;}
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr09"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \


# Turn off most RvR initialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""


####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr09end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr09end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr09end1 -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr09end1 -rev 1
UTF::Power::WebRelay  web43 -lan_ip 172.1.1.43 -invert 1
########################### Test Manager ################

UTF::Linux blr13end1 \
     -lan_ip 10.131.80.143 \
     -sta {lan eth0}

# Power          - npc21 port 1    172.1.1.21
################################################################################
UTF::Router AP \
        -sta "4708/43602 eth1" \
        -lan_ip 192.168.1.6 \
        -relay "blr13end1" \
        -lanpeer lan \
        -console "blr13end1.ban.broadcom.com:40001" \
        -power {npc21 1} \
        -brand linux-2.6.36-arm-internal-router-dhdap \
	-tag BISON04T_REL_7_14_89_33 \
        -nvram {
                
                fw_disable=1
                console_loglevel=7
                lan_ipaddr=192.168.1.2
                lan_gateway=192.168.1.2
                dhcp_start=192.168.1.150
                dhcp_end=192.168.1.200
                lan1_ipaddr=192.168.2.2
                lan1_gateway=192.169.2.2
                dhcp1_start=192.168.2.20
                dhcp1_end=192.168.2.30
                wl0_ssid=test_4708_0
                wl1_ssid=test_4708_1
                wl0_radio=0
                wl1_radio=1
                wl1_obss_coex=0
                wl0_obss_coex=0
        }

4708/43602 configure  -attngrp G1 \

4708/43602 clone 4708/43602b10 -tag BISON_BRANCH_7_10 


4708/43602 clone 4360t78   -tag BISON04T_REL_7_14_78 
4708/43602 clone 4360t89_2 -tag BISON04T_REL_7_14_89_2 

4708/43602 clone 4360a630  -tag AARDVARK_BRANCH_6_30 
4708/43602 clone 4360b714  -tag BISON04T_BRANCH_7_14

###########################################
# BLR09TST1
# 4360mc_P199 - 11n 3x3
###########################################
UTF::Linux blr13tst1  \
        -lan_ip 10.131.80.144 \
        -sta {43602 enp1s0} \
        -power {npc41 1} \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag BISON_BRANCH_7_10 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -tcpwindow 2m -udp 800m \
        -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
        -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0;}

43602 configure -attngrp G1 -ipaddr 192.168.1.102   \
		
43602 clone 43602a -tag AARDVARK_BRANCH_6_30
43602a clone 43602ax2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

43602 clone 43602b -tag BISON_BRANCH_7_10
43602b clone 43602bx2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

43602 clone 43602t -tag trunk
43602t clone 43602tx2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

############## 43430 ##########

UTF::DHD blr13tst2 \
	-lan_ip 10.131.80.105 \
	-sta {43430 eth0} \
    -power {npc31 2} \
	-perfchans {11} \
	-wlinitcmds {wl chanspec 11} \
    
43430 configure -ipaddr 192.168.1.92 \

############  blr09ref1 (43228/4360/43228) ###########

#UTF::Linux blr09ref0 -sta {4360a eth0} \
 #   -lan_ip 10.131.80.102 \
  #  -power "npc21 2"\
#    -power_button "auto" \
#    -slowassoc 5 -reloadoncrash 1 \
 #   -tag BISON_BRANCH_7_10 \
  #  -brand "linux-internal-wl" \
	#-perfchans {11} \
#    -preinstall_hook {{%S dmesg -n 7}} \
#    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl phy_cal_disable 0} {wl scansuppress 1}} \
#    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl phy_cal_disable 1} {%s wl scansuppress 1}} \
#    -tcpwindow 4M \
#    -wlinitcmds {
 #                wl msglevel 0x101;wl mpc 0; wl btc_mode 0; wl dtim 3; \
 #                wl mimo_bw_cap 1; wl obss_coex 0; wl vht_features 3; wl frameburst 1; wl country US ;wl ampdu_mpdu 24; wl assert_type 1;wl scansuppress 1; wl chanspec 11;
 #               }
#4360 configure -ipaddr 192.168.1.101 -ap 1  -attngrp G2 \




UTF::Q blr09


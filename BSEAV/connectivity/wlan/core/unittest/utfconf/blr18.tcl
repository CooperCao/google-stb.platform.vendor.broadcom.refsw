	#Testbed configuration file for blr18end1 UTF  Teststation
#Created by Anuj Gupta on 30 March 2015 05:00 PM  
#Last checkin 19Nov2014 
####### Controller section:
# blr18end1: FC15
# IP ADDR 10.132.116.101
# NETMASK 255.255.254.0 
# GATEWAY 10.132.116.1
#
####### SOFTAP section:
#
#  
# 
#
####### STA section:
#
# blr18tst1: 
# blr18tst2: 
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix

set UTF::ChannelPerf 1
set UTF::Use11 1
set UTF::Use11h 1
set UTF::UseCSA 1
set UTF::Web 2

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr18end1.ban.broadcom.com" \
        -group {
                 G1 {1 2 3 4}
		ALL {1 2 3 4}
                }
 G1 configure -default 18
#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value

	foreach S {4360a 4360b 47189MC5} {
	catch {$S wl down}
	$S deinit
	G1 attn default
    }
    return
}


# SummaryDir sets the location for test results
set k $::env(LOGNAME)
regexp {/home/(.*)/} $k m k
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr18"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \


# Turn off most RvR initialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up} {%S wl roam_trigger -100 all}} 
set ::rvr_ap_init "" 

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck

####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr18end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr18end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr18end1.ban.broadcom.com -rev 1
UTF::Power::Synaccess npc42 -lan_ip 172.1.1.42 -relay blr18end1 -rev 1
UTF::Power::WebRelay  web43 -lan_ip 172.1.1.43 -invert 1
########################### Test Manager ################

UTF::Linux blr18end1.ban.broadcom.com \
     -lan_ip 10.132.116.126 \
     -sta {lan p16p2}

# Power          - npc21 port 1    172.1.1.21
################################################################################
# 

#UTF::Router _53574 -sta {
    # 53574 eth1 53574.%15 wl0.%
# } \
    # -lanpeer lan \
    # -lan_ip 192.168.1.3 \
    # -power {npc21 1} \
    # -console "blr18end1.ban.broadcom.com:40003" \
    # -relay blr18end1.ban.broadcom.com \
    # -brand "linux-2.6.36-arm-up-internal-router-rsdb" \
    # -tag DINGO_TWIG_9_10_178 \
    # -nvram {
	# watchdog=2000
	# wl0_ssid=53574
	# wl0_chanspec=3
	# wl1_ssid=53574_5G
	# wl1_chanspec=36

	# # Features
	# wl1_vht_features=2
	# wl0_vht_features=3

	# # leave both radios turned on for RSDB
	# wl0_radio=1
	# wl1_radio=1

	# rsdb_mode=0
    # } -docpu 1 \
    # -datarate {-i 0.5 -frameburst 1} -udp 800m \
    # -noradio_pwrsave 1 -perfchans {36/80 36l 36 3l 3} \
    # -yart {-attn5g {20-95 95-20} -attn2g {20-95 95-20} -frameburst 1 -pad 28} \
	# -wlinitcmds {sleep 5; wl ver} \

# 53574 configure -attngrp G3

# 53574 clone 53574x -docpu 1 \
    # -brand linux-2.6.36-arm-up-external-vista-router-full-src

# # Skip CPU test, since it may pick the wrong band.
# 53574 clone 53574_2G -docpu 0 \
    # -sta {53574_2G eth1 53574_2G.%15 wl0.%}  \
    # -nvram [regsub {rsdb_mode=0} [53574 cget -nvram] {rsdb_mode=1}] \
    # -perfchans {3l 3} -channelsweep {-band g} \
	# -wlinitcmds {sleep 5; wl ver}

# 53574 clone 53574_5G -docpu 1 \
    # -sta {53574_5G eth2 53574_5G.%15 wl1.%} \
    # -nvram [regsub {rsdb_mode=0} [53574 cget -nvram] {rsdb_mode=1}] \
    # -perfchans {36/80 36l 36} -channelsweep {-band a} \
	# -wlinitcmds {sleep 5; wl ver;} \
	# -pre_perf_hook {{%S wl rssi } {%S wl dump rssi}} \
	# -post_perf_hook {{%S wl rssi} {5S wl dump rssi}}

# 53574_5G configure -dualband {53574_2G -c1 36/80}

# # Gold twig
# 53574 clone 53574d10 -tag DINGO_BRANCH_9_10
# 53574_5G clone 53574d10_5G -tag DINGO_BRANCH_9_10
# 53574_2G clone 53574d10_2G -tag DINGO_BRANCH_9_10

# 53573 clone 4708/4360   -tag BISON_BRANCH_7_10


# 53573 clone 4360t78   -tag BISON04T_REL_7_14_78
# 53573 clone 4360t89_2 -tag BISON04T_REL_7_14_89_2

# 53573 clone 4360a630  -tag AARDVARK_BRANCH_6_30
# 53573 clone 4360b714  -tag BISON04T_BRANCH_7_14


############# 53574 b0 

UTF::Router _53574b0 \
	-sta "53574b0 eth1 53574b0.%15 wl0.%" \
    -lanpeer lan \
    -lan_ip 192.168.1.1 \
    -power {npc21 2} \
    -console "blr18end1.ban.broadcom.com:40000" \
    -relay blr18end1.ban.broadcom.com \
    -brand "linux-2.6.36-arm-up-internal-router-rsdb" \
    -tag DINGO_TWIG_9_10_178 \
	-wanpeer 192.168.1.1 -web 2 \
    -nvram {
	watchdog=2000
	wl0_ssid=53574b0
	wl0_chanspec=3
	wl1_ssid=53574_5G
	wl1_chanspec=36

	# Features
	wl1_vht_features=2
	wl0_vht_features=3

	# leave both radios turned on for RSDB
	wl0_radio=1
	wl1_radio=1

	rsdb_mode=0
    } -docpu 1 \
    -datarate {-i 0.5} -udp 1.2g \
    -noradio_pwrsave 1 -perfchans {36/80 36l 36 3l 3} \
    -yart {-attn5g {20-95} -attn2g {20-95} -frameburst 1 -pad 28} \
	-wlinitcmds {sleep 5; wl ver; wl txchain 3; wl rxchain 3} \
	-perfchans {36/80 36l 36 3l 3} \
	
	# -pre_perf_hook {{%S wl nrate} {%S wl rsdb_mode} {%S wl dump rsdb} {%S wl dump rssi}} \
	# -pre_perf_hook {{%S wl nrate} {%S wl rsdb_mode} {%S wl dump rsdb} {%S wl dump rssi}} \
	
	# -channelsweep {-band a -chanspec "149 153 157 161 165 36l 40u 44l"} \

53574b0 configure -attngrp G3

53574b0 clone 53574b0x -docpu 1 \
    -brand linux-2.6.36-arm-up-external-vista-router-full-src

# Skip CPU test, since it may pick the wrong band.
53574b0 clone 53574b0_2G -docpu 0 \
    -sta {53574b0_2G eth1 53574b0_2G.%15 wl0.%}  \
    -nvram [regsub {rsdb_mode=0} [53574b0 cget -nvram] {rsdb_mode=1}] \
    -perfchans {3l 3} -channelsweep {-band g} \
	-wlinitcmds {sleep 5; wl ver}


53574b0 clone 53574b0t \
	-tag trunk

53574b0 clone 53574b0_5G -docpu 1 \
    -sta {53574b0_5G eth2 53574b0_5G.%15 wl1.%} \
    -nvram [regsub {rsdb_mode=0} [53574b0 cget -nvram] {rsdb_mode=1}] \
    -perfchans {36/80 36l 36} -channelsweep {-band a} \
	-wlinitcmds {sleep 5; wl ver;} \
	-pre_perf_hook {{%S wl rssi } {%S wl dump rssi}} \
	-post_perf_hook {{%S wl rssi} {5S wl dump rssi}} \


53574b0_5G configure -dualband {53574b0_2G -c1 36/80}

53574b0_5G clone 53574b0_5Gt \
	-tag trunk 

53574b0_2G clone 53574b0_2Gt \
	-tag trunk 

##################################
# 47189
##################################

UTF::Router 47189 \
        -sta "47189MC5 eth2 47189MC5.%15 wl0.% 47189MC2 eth3 47189MC2.%15 wl1.%" \
        -lan_ip 192.168.1.2 \
        -relay "blr18end1.ban.broadcom.com" \
        -lanpeer lan -web 2 \
        -console "blr18end1.ban.broadcom.com:40001" \
        -power {npc11 2} \
        -brand linux-2.6.36-arm-up-internal-router \
        -tag DINGO_TWIG_9_10_178 \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=47189MC5
    wl0_chanspec=36
    wl0_radio=0
	wl1_country_code=US
	wl1_ssid=47189MC2
	wl1_chanspec=3
	wl1_radio=0
    wl1_vht_features=3
    wl0_vht_features=2
		} \
		-docpu 1 \
    -channelsweep {-band a} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -noradio_pwrsave 1 \
	-wlinitcmds {wl ver} \
	-pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-post_perf_hook {{%S wl txchain} {%S wl rxchain}} \
	-yart {-attn5g {20-83} -frameburst 1} \
	-coldboot 1 \


47189MC5 configure  -attngrp G1 \

47189MC5 clone 47189_5G_2x2 \
	-wlinitcmds {wl txbf 1; wl tx_bfr_cap 1;wl txchain 3; wl rxchain 3; wl ver} \

47189MC5 clone 47189_5G_1x1_txbf1 \
	-wlinitcmds {wl txchain 1; wl rxchain 1; wl ver; wl txbf 1; wl tx_bfr_cap 1} \

47189MC5 clone 47189_5G_1x1_txbf0 \
	-wlinitcmds {wl txchain 1; wl rxchain 1; wl ver } \


47189MC2 configure -attngrp G1 \
-channelsweep {-band b} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -noradio_pwrsave 1 \
    -yart {-attn2g {20-83} -frameburst 1} \
    -wlinitcmds {sleep 2; wl ver} \

47189MC5 clone 47189MC5t \
	-tag trunk \

47189MC2 clone 47189MC2t \
	-tag trunk \


# ---------------Txchain/Rxchain Altered -------------- #

47189MC5 clone 47189MC5_txrx1 \
	-wlinitcmds {wl txchain 1; wl rxchain 1; wl ver } \
	
47189MC5 clone 47189MC5_txrx2 \
	-wlinitcmds {wl txchain 2; wl rxchain 2; wl ver } \

47189MC2 clone 47189MC2_txrx1 \
	-wlinitcmds {wl txchain 1; wl rxchain 1; wl ver } \
	
47189MC2 clone 47189MC2_txrx2 \
	-wlinitcmds {wl txchain 2; wl rxchain 2; wl ver } \

#####43602##

UTF::Router 4709/43602 \
        -power_button "auto" \
        -sta "43602MC5 eth1 43602MC5.%15 wl0.% 43602MC2 eth2 43602MC2.%15 wl1.%" \
        -lan_ip 192.168.1.2 \
		-power {npc11 2} \
        -relay blr18end1.ban.broadcom.com \
        -lanpeer lan -web 2 \
        -console "blr18end1.ban.broadcom.com:40001" \
        -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
        -tag "BISON04T_BRANCH_7_14" \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=43602MC5
    wl0_chanspec=36
    wl0_radio=0
	wl1_country_code=US
	wl1_ssid=43602MC2
	wl1_chanspec=3
	wl1_radio=0
	wl0_atf=0
	wl0_bw_cap=-1
	wl0_chanspec=3
	wl0_obss_coex=0
	wl0_radio=1
	wl0_reg_mode=off
	wl1_bw_cap=-1
	wl1_obss_coex=0
	wl1_vht_features=7
	wl0_vht_features=6
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.6g \
    -noradio_pwrsave 1 \
    -yart {-attn5g {20-83} -frameburst 1} \
    -coldboot 1 \
	-pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-wlinitcmds {sleep 5; wl ver; wl country US; wl txchain 7; wl rxchain 7} \
	-post_perf_hook {{%S wl txchain} {%S wl rxchain}}


43602MC5 configure -attngrp G4 \

 
43602MC2 configure -attngrp G4 \

############# 53574 ACNR ##

# UTF::Router _53574NR -sta {
    # 53574NR eth1 53574NR.%15 wl0.%
# } \
    # -lanpeer lan \
    # -lan_ip 192.168.1.4 \
    # -power {npc21 1} \
    # -console "blr18end1.ban.broadcom.com:40000" \
    # -relay blr18end1.ban.broadcom.com \
    # -brand "linux-2.6.36-arm-up-internal-router" \
    # -tag DINGO_TWIG_9_10_178 \
    # -nvram {
	# watchdog=2000
	# wl0_ssid=53574NR
	# wl0_chanspec=36

	# Features
	# wl0_vht_features=3

	# leave both radios turned on for RSDB
	# wl0_radio=1

    # } -docpu 1 \
    # -datarate {-i 0.5 -frameburst 1} -udp 800m \
    # -noradio_pwrsave 1 -perfchans {36/80 36l 36 3l 3} \
    # -yart {-attn5g {20-95} -attn2g {20-95} -frameburst 1 -pad 28} \
	# -wlinitcmds {sleep 5; wl ver} \

# 53574NR configure -attngrp G3

UTF::Router 47189b \
        -power_button "auto" \
        -sta "47189bMC5 eth2 47189bMC5.%15 wl0.% 47189bMC2 eth3 47189bMC2.%15 wl1.%" \
        -lan_ip 192.168.1.1 \
        -relay blr18end1.ban.broadcom.com \
        -lanpeer lan -web 2 \
        -console "blr18end1.ban.broadcom.com:40000" \
        -power {npc21 2} \
        -brand linux-2.6.36-arm-up-internal-router-dhdap-moca \
        -tag "DINGO_TWIG_9_10_178" \
        -nvram {
	watchdog=2000
    wl0_country_code=US
    wl0_ssid=47189MC5
    wl0_chanspec=36
    wl0_radio=0
	wl1_country_code=US
	wl1_ssid=47189MC2
	wl1_chanspec=6
	wl1_radio=0
    wl1_vht_features=3
    wl0_vht_features=2
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.6g \
    -noradio_pwrsave 1 \
    -yart {-attn5g {20-83} -frameburst 1} \
    -coldboot 1 \
	-perfchans {1 1l} \
	-wlinitcmds {wl ver; wl -i eth2 band a} \
	-pre_perf_hook {{%S wl txbf} {%S wl tx_bfr_cap} {%S wl txchain} {%S wl rxchain}} \
	-post_perf_hook {{%S wl txchain} {%S wl rxchain}}


47189bMC5 configure -attngrp G1 \
	-channelsweep {-band a} \
	-yart {-attn5g {20-83} -frameburst 1} \
	-datarate {-i 0.5 -frameburst 1} -udp 1.2g

 
47189bMC2 configure -attngrp G1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -channelsweep {-band b} \
     -noradio_pwrsave 1 \
    -yart {-attn2g {20-83} -frameburst 1} \
    -wlinitcmds {sleep 2; wl ver}

###########################################
# blr18TST1
# 4360mc_P199 - 11n 3x3
###########################################
UTF::Linux blr18ref0  \
        -lan_ip 10.132.116.127 \
        -sta {4360a enp1s0} \
        -power {npc31 2} \
		-slowassoc 5 -reloadoncrash 1 \
        -power_button "auto" \
        -brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -tcpwindow 3m -udp 800m \

4360a configure -attngrp G1 -ipaddr 192.168.1.102   \

4360a clone 4360ar0 \
	-wlinitcmds {wl txchain 1 ; wl rxchain 1; wl txbf 1; wl txbf_bfe_cap 1} \

4360ar0 clone 4360ar0_txbf1 \
	-wlinitcmds { wl }
	
4360ar0 clone 4360ar0_roam_trigger_off \
	-wlinitcmds {wl band a; wl roam_trgger -90; wl txbf 1; wl txbf_bfe_cap 1} \
	
4360a clone 4360ar1 \
	-wlinitcmds {wl txchain 2 ; wl rxchain 2}

4360a clone 4360am \
	-wlinitcmds {wl down; wl vht_features 3; wl txchain 3 ; wl rxchain 3; wl bw_cap 2g -1; wl country US/0;wl obss_coex 0; }

4360am clone 4360am_roam_trigger \
	-wlinitcmds { wl band a; wl roam_trigger -90}
	
4360am clone 4360amb -tag  BISON_BRANCH_7_10 		

# 4360 clone 4360a -tag AARDVARK_BRANCH_6_30
# 4360a clone 4360ax2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

# 4360 clone 4360b -tag  BISON_BRANCH_7_10   
# 4360b clone 4360bx2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

# 4360 clone 4360t -tag trunk
# 4360t clone 4360tx2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

############## blr18ref1 ##########`

UTF::Linux blr18ref1 \
	-lan_ip 10.132.116.128 \
	-sta {4360b enp1s0} \
    -power {npc31 1} \
	-slowassoc 5 -reloadoncrash 1 \
	-brand linux-internal-wl \
        -tag EAGLE_BRANCH_10_10 \
        -tcpwindow 3m -udp 800m \

4360b configure -ipaddr 192.168.1.92 -attngrp G1 \

4360b clone 4360br0 \
	-wlinitcmds {wl txchain 1 ; wl rxchain 1}

4360b clone 4360br1 \
	-wlinitcmds {wl txchain 2 ; wl rxchain 2}

4360b clone 4360bm \
	-wlinitcmds {wl down; wl vht_features 3; wl txchain 3 ; wl rxchain 3; wl bw_cap 2g -1; wl country US/0;wl obss_coex 0;}

4360bm clone 4360bmb -tag  BISON_BRANCH_7_10

set UTF::RouterNightlyCustom {

    catch {
	package require UTF::Test::MiniUplinks
	UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
    }

}



UTF::Q blr18


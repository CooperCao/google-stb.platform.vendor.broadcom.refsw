#Testbed configuration file for blr10end1 UTF  Teststation
#Created by Guru on 18 Jan 2017 11:30 AM  
#Last checkin 19Nov2014 
####### Controller section:
# blr10end1: FC19
# IP ADDR 10.132.116.79
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
# blr10-iptv-lx1: 
# blr10-iptv-lx4: 
######################################################### #
# Load Packages
#package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
#package require UTF::Power
#package require UTF::Airport
#package require UTF::Vaunix

set UTF::ChannelPerf 1
set UTF::Use11 1
set UTF::UseCSA 1 
#CSA for Channel Sweep


# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
	foreach S {4366a 4366b} {
	catch {$S wl1 down}
	$S deinit
	G1 attn default
    }
    return
}

# UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
# set UTF::BuildFileServer repo
# set UTF::UseFCP nocheck

# SummaryDir sets the location for test results
set k $::env(LOGNAME)
regexp {/home/(.*)/} $k m k
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/blr10"

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \


# Turn off most RvR initialization
# set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
# set ::rvr_ap_init ""

UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck


########################### Test Manager ################

UTF::Linux blr10end1.ban.broadcom.com \
     -lan_ip 10.132.116.79 \
     -sta {lan p16p2}

#UTF::Linux blr01end2.ban.broadcom.com -sta "wan enp4s0" -lan_ip 10.132.116.185
#wan configure -ipaddr 192.168.254.250

lan configure -ipaddr 192.168.1.100

# wan configure -ipaddr 192.168.254.253
# Power          - npc21 port 1    172.1.1.21
################################################################################

# UTF::Router _53574NR -sta {
    # 53574NR eth1 53574NR.%15 wl0.%
# } \
    # -lanpeer lan \
    # -lan_ip 192.168.1.1 \
    # -power {npc21 2} \
    # -console "blr01end1.ban.broadcom.com:40000" \
    # -relay blr01end1.ban.broadcom.com \
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
    # -noradio_pwrsave 1 -perfchans {36/80 36l 36} \
	# -wlinitcmds {sleep 5; wl ver; wl txbf 0} \
	# -pre_perf_hook {{%S wl nrate} {%S wl rssi}} \
	# -post_perf_hook {{%S wl nrate} {%S wl rssi}} \
	# -channelsweep {-band a} \

# 53574NR configure -attngrp G3


UTF::Router AP \
        -sta "53574 eth1 53574.%15 wl0.%" \
        -lan_ip 192.168.1.1 \
        -relay "blr10end1.ban.broadcom.com" \
        -lanpeer lan -web 2 \
        -console "blr10end1.ban.broadcom.com:40000" \
        -power {npc21 2} \
        -brand linux-2.6.36-arm-up-internal-router-rsdb \
        -tag "DINGO_TWIG_9_10_178_64" \
        -nvram {
		devicemode=0
	watchdog=2000
    wl0_ssid=53574_2G
    wl0_chanspec=3
    wl0_radio=1
	wl1_ssid=53574_5G
    wl1_chanspec=36
    wl1_radio=1
	wl1_vht_features=2
    wl0_vht_features=3
	rsdb_mode=0
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -yart {-attn5g {20-95} -frameburst 1 -attn2g {20-83} } \
-noradio_pwrsave 1\
-channelsweep {-band a} \
-wlinitcmds {wl ver; wl txbf 0} \
-perfchans {36/80 36l 36 3l 3} \

#53574 configure -attngrp G1
53574 clone 53574t \
	-tag trunk

53574t clone 53574test \
	-perfchans 36/80

53574 clone 53574_2G \
	-docpu 0 \
	-sta "53574_2G eth1 53574_2G.%15 wl0.%" \
	-nvram [regsub {rsdb_mode=0} [53574 cget -nvram] {rsdb_mode=1}] \
        -brand linux-2.6.36-arm-up-internal-router-rsdb \
		-perfchans {3l 3} \
		-channelsweep {-band g} \
		-wlinitcmds {wl ver} \

53574 clone 53574_5G \
	-docpu 1 \
	-sta "53574_5G eth2 53574_5G.%15 wl1.%" \
        -brand linux-2.6.36-arm-up-internal-router-rsdb \
		-nvram [regsub {rsdb_mode=0} [53574 cget -nvram] {rsdb_mode=1}] \
	-perfchans {36/80 36l 36} \
	-channelsweep {-band a} \
	-wlinitcmds {wl ver}

53574_5G configure -dualband {53574_2G -c1 36/80}

53574_5G clone 53574_5Gt \
	-tag trunk 

53574_2G clone 53574_2Gt \
	-tag trunk 

# 53573 clone 4708/4360   -tag BISON_BRANCH_7_10


# 53573 clone 4360t78   -tag BISON04T_REL_7_14_78
# 53573 clone 4360t89_2 -tag BISON04T_REL_7_14_89_2

# 53573 clone 4360a630  -tag AARDVARK_BRANCH_6_30
# 53573 clone 4360b714  -tag BISON04T_BRANCH_7_14


##################################
# 47189
#
#################################

UTF::Router 4709 \
        -sta "4366MCH5 eth1 4366MCH5.%15 wl0.%" \
        -lan_ip 192.168.1.2 \
        -relay "blr10end1.ban.broadcom.com" \
        -lanpeer lan -web 2\
        -console "blr10end1.ban.broadcom.com:40001" \
        -brand linux-2.6.36-arm-internal-router-dhdap-atlas \
        -tag "BISON04T_BRANCH_7_14" \
        -nvram {
	watchdog=2000
    wl0_country_code=ALL
    wl0_ssid=4366MCH5UTF
    wl0_chanspec=36
    wl0_radio=1
    wl0_vht_features=2
		} \
		-docpu 1 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.2g \
    -noradio_pwrsave 1 \
    -yart {-attn5g {20-83} -frameburst 1} \
    -coldboot 1 \
	-channelsweep {-band a} \
	-wlinitcmds {wl ver} \

4366MCH5 clone 4366MCH5_trunk \
	-tag trunk

# --------------------------------------------------- #

# 53573 clone 4708/4360   -tag BISON_BRANCH_7_10


# 53573 clone 4360t78   -tag BISON04T_REL_7_14_78
# 53573 clone 4360t89_2 -tag BISON04T_REL_7_14_89_2

# 53573 clone 4360a630  -tag AARDVARK_BRANCH_6_30
# 53573 clone 4360b714  -tag BISON04T_BRANCH_7_14

###########################################
# blr10-iptv-lx1
# 4360mc_P199 - 11AC 4x4
###########################################
UTF::Linux blr10-iptv-lx1  \
        -lan_ip 10.132.116.80 \
        -sta {4366a eth0} \
        -brand linux-internal-wl \
        -tag trun \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -tcpwindow 3m -udp 1.2g \
		-reloadoncrash 1 \
		
4366a configure -ipaddr 192.168.1.200 \

4366a clone 4366ar0 \
	-wlinitcmds {wl txchain 1 ; wl rxchain 1}
	
4366a clone 4366ar1 \
	-wlinitcmds {wl txchain 2 ; wl rxchain 2}

4366a clone 4366am \
	-wlinitcmds {wl msglevel +assoc; wl down; wl bw_cap 2g -1; wl country US/0;wl obss_coex 0;wl vht_features 3} \
	
4366am clone 4366ame \
	-tag EAGLE_BRANCH_10_10 \
	
		
# 4360 clone 4360a -tag AARDVARK_BRANCH_6_30
# 4360a clone 4360ax2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

# 4360 clone 4360b -tag BISON_BRANCH_7_10
# 4360b clone 4360bx2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

# 4360 clone 4360t -tag trunk
# 4360t clone 4360tx2 -wlinitcmds {wl msglevel +assoc; wl mimo_bw_cap 1; wl PM 0; wl rxchain 2; wl txchain 2;}

############## blr10-iptv-lx4 ##########

UTF::Linux blr10-iptv-lx4 \
	-lan_ip 10.132.116.83 \
	-sta {4366b eth0} \
	-brand linux-internal-wl \
        -tag trunk \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -tcpwindow 3m -udp 1.2g \
    
4366b configure -ipaddr 192.168.1.210 \

4366b clone 4366br0 \
	-wlinitcmds {wl txchain 1 ; wl rxchain 1}


set UTF::RouterNightlyCustom {

   package require UTF::Test::MiniUplinks
   UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 

    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2

}

UTF::Q blr10a


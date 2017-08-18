##################################################################
#
# UTF configuration for kpiblr01 ( KPI Phase 2 testbed ) SEPTEMBER 2015              
# Testbed location : 5th Floor , Bldg 190 , Mathilda Place  
#                                         
################Station Generic Information########################
#
# Controller  kpiblr01end1   10.132.116.187 
# AP1         4708/4360  		 192.168.1.1      NPC PORT 1  ATTN G1
# AP2         4708/4360  		 192.168.1.2      NPC PORT 2  ATTN G2
# AP HOST     kpiblr01aphost   	 10.132.116.197   NPC PORT 3  
# LINUX DHD   kpiblr01dhd   	 10.132.116.196   NPC PORT 4  ATTN G3
# MACBOOK PRO kpiblr01tst1   	 10.132.116.171   DUT
# MACBOOK PRO kpiblr01tst2   	 10.132.116.173   DUT
# MACBOOK AIR kpiblr01tst3   	 10.132.116.174   DUT
# MACBOOK AIR kpiblr01tst4       10.132.116.175   DUT
# MACBOOK PRO kpiblr01stressor   10.132.116.178   STRESSOR
# LINUX BOX   kpiblr01snf1   	 10.132.116.170   SNIFFER1 -> AP
# LINUX BOX2  kpiblr01snf2  	 10.132.116.171   SNIFFER2 -> STA   -> For Temporary purpose using blr16tst1 : 10.132.116.113 as sniffer as Brix was not working.
#
# 
#
# Attenuator       Aefoflex AF1         172.16.1.106
# Attenuator       Aeroflex AF2         172.16.1.107
#
# CISCO SG300-10  Distribution Switch   172.16.1.100
# CISCO SG300-10  Distribution Switch   172.16.1.101
# CISCO SG300-10  SPAN SWITCH AP1       172.16.1.103
# CISCO SG300-19  SPAN SWITCH AP2       172.16.1.104
# 
# NPC 8 PORT      POWER CONTROLLER      172.16.1.108
#
# PORT 1(AP1) PORT 2(AP2) PORT 3(APHOST)  PORT 4(LINUX DHD) PORT 5(SNIFFER 1)  PORT 6 (SNIFFER 2)  PORT 7(AF1)  PORT 8( AF2)
# 
# NPC 8 PORT      POWER CONTROLLER      172.16.1.109
#
# PORT 1(AP1) PORT 2(AP2) PORT 3(APHOST)  PORT 4(LINUX DHD) PORT 5(SNIFFER 1)  PORT 6 (SNIFFER 2)  PORT 7(AF1)  PORT 8( AF2)
#
# NPC 2 PORT      POWER CONTROLLER      172.16.1.110
#
# PORT 1(kpiblr01tst1) PORT 2(kpiblr01tst2)
#
# NPC 2 PORT      POWER CONTROLLER      172.16.1.111
#
# PORT 1(kpiblr01tst3) PORT 2(kpiblr01tst4)
#
#
# NPC 2 PORT      POWER CONTROLLER      172.16.1.123
#
#
#  PORT 2(kpiblr01tst5)
#
#
#SAP (Netgear)L1     af1 1,5,9
#AP1 (Cisco)  L2     af1 2,6,10
#AP2 (Linksys)L3     af1 3,7,11
#BT           L4     af1 4,8,12
#MBP          L5     af2 1,5,9 
#MBA          L6     af2 2,6,10
#sressor      L7     af2 3,7,11
#MacBook      L8     af2 4,8,12
#
# Mirror Port : 
# 172.16.1.103 : port 3
# port 3 : AP1 Box (brcm ref ap)
# 
#
# 172.16.1.104 : port 3
#
# port 3 : SOFTAP Box (Netgear R7000)
#####################################################################

set ::UTF::SummaryDir "/projects/hnd_sig_ext21/anujgupt/kpiblr01"

# package require UTFD
# set ::env(UTFDPORT) 9977
package require UTF::Power
package require UTF::Q
package require UTF::AeroflexDirect
package require UTF::Aeroflex
package require UTF::Airport
package require UTF::WebRelay
package require UTF::Streams
package require UTF::Sniffer
package require UTF::MacBT_dev
package require UTF::Test::ConnectAPSTA
package require UTF::Test::APChanspec
package require UTF::FTTR
package require UTF::Test::ConfigBridge
package require UTF::Test::AutoJoinSetup


# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
}

set ::afmaxattn 95

set ::UTF::MSTimeStamps 1

set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

# UTF::Linux repo -lan_ip xl-sj1-24.sj.broadcom.com -user $::tcl_platform(user)
# set UTF::BuildFileServer repo
# set UTF::UseFCP nocheck

######################## Power Controllers ################

UTF::Power::Synaccess npc108 -lan_ip  172.16.1.108 -relay kpi01end1 -rev 1
UTF::Power::Synaccess npc109 -lan_ip  172.16.1.109 -relay kpi01end1 -rev 1
UTF::Power::Synaccess npc110 -lan_ip 172.16.1.110 -relay kpi01end1 -rev 1
UTF::Power::Synaccess npc111 -lan_ip  172.16.1.111 -relay kpi01end1 -rev 1
UTF::Power::Synaccess npc123 -lan_ip  172.16.1.123 -relay kpi01end1 -rev 1

####################### Attenuator  ######################



UTF::AeroflexDirect af1  -lan_ip 172.16.1.106 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
       -relay kpi01end1 \
		-power {npc108 7} \
        -group {
                L1 {1  5   9}
                L2 {2  6  10}
                L3 {3  7  11}
				L4 {4  8  12}
                ALL1 {1 2 3 4 5 6 7 8 9 10 11 12}
                }

UTF::AeroflexDirect af2  -lan_ip 172.16.1.107 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
		-relay kpi01end1 \
		-power {npc108 8} \
        -group {
                L5 {1  5   9}
                L6 {2  6  10}
                L7 {3  7  11}
				L8 {4  8  12}
                ALL2 {1 2 3 4 5 6 7 8 9 10 11 12}
                }

L1 configure -default 0
L2 configure -default 0
L3 configure -default 0
L4 configure -default 95
L5 configure -default 0
L6 configure -default 0
L7 configure -default 0
L8 configure -default 0


########################################### test bed setup #################

set UTF::SetupTestBed {
    L1 attn 95
    L2 attn 95
    L3 attn 95
    L4 attn 95
    L5 attn 95
    L6 attn 95
    L7 attn 95
    L8 attn 95
	#ALL1 attn default
	#ALL2 attn default
  # foreach STA $physicalmacstas {
	#set tag INFO
	# if {[catch {$STA power on}]} {
	    # set tag ERROR
	# }
	# $STA rexec networksetup -setairportpower [$(sta) cget -device] on
#	UTF::Message $tag "" "AC power on and Radio on for $STA"
    # }
 #   foreach S [concat $physicalmacstas $physicalrouterstas] {
	#catch {$S wl down}
	#$S deinit
	#MacX-A rexec "wl ampdu_clear_dump"
	
	#foreach STA {MacX-A MacX-B MacAirX-A MacAirX-B MacBook-B DOME_STRESS} {}
	foreach STA {MacX-B MacAirX-B DOME_STRESS} {
	set tag INFO
	if {[catch {$STA power on}]} {
	    set tag ERROR
	}
	if {[$STA hostis MacOS]} {
	catch {$STA killall dns-sd}
	catch {$STA wl awdl 0}
	UTF::Sleep 1
	$STA rexec "/usr/local/bin/applebt --setPowerState 1"
	$STA rexec "/usr/local/bin/applebt --getPowerState"
	$STA rexec networksetup -listpreferredwirelessnetworks [$(sta) cget -device]
	$STA rexec networksetup -removeallpreferredwirelessnetworks [$(sta) cget -device]
	$STA rexec networksetup -setairportpower [$(sta) cget -device] off
	UTF::Sleep 1
	} else {
	catch {$STA wl down}
	}
}
#    }

    set sniffers [UTF::Sniffer info instances]
    foreach sniffer $sniffers {
	catch ($sniffer unload)
    }
    return
}

## $physicalmacstas - > $physicalmacstastas

############## Controller and hosts####################

UTF::Linux kpi01end1 -lan_ip 10.132.116.159 \
     -sta {lan p1p1} \
	 
UTF::Linux kpiblr01end1 -lan_ip 10.132.116.169 \
     -sta {lan eth5} \
	 -power {npc109 6}

UTF::Linux kpiblr01aphost -lan_ip 10.132.116.177 \
                    -sta {ApHost em2 mirror1 p7p1 mirror2 p7p2} \

ApHost configure -ipaddr 192.168.1.51 \


################### Sniffer ###########################

UTF::Sniffer SNIF_AP \
	-lan_ip 10.132.116.170 \
	-user root \
	-tag BISON_BRANCH_7_10 \
	-app_tag BISON_BRANCH_7_10 \
	-sta {4360SNIF_AP p3p1} \
	-power {npc108 5} \


UTF::Sniffer SNIF_STA \
	-lan_ip 10.132.116.113 \
   -user root \
   -tag EAGLE_BRANCH_10_10 \
   -app_tag EAGLE_BRANCH_10_10 \
   -sta {4360SNIF_STA  enp1s0} \
   -power {npc108 6}


################# Routers ##############################

set LANPEER ApHost
# set LANPEER ApHostB

UTF::Router KPI_AP1 \
    -sta {4360ap1 eth1 } \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {npc108 1} \
    -bootwait 30 \
    -relay "ApHost" \
    -lanpeer ApHost \
    -console 10.132.116.159:40000 \
    -lan_ip 192.168.1.1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror1 \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        wl0_ssid=4360ap1
        watchdog=6000
        lan_stp=0
    }

4360ap1 configure  -attngrp L2 \


UTF::Router KPI_AP2 \
    -sta {4360ap2 eth1 } \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {npc108 2} \
    -bootwait 30 \
    -relay "ApHost" \
    -lanpeer ApHost \
    -console 10.132.116.159:40002 \
    -lan_ip 192.168.1.2 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror2 \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        lan_ipaddr=192.168.1.2
        wl0_ssid=4360ap2
        watchdog=6000
        lan_stp=0
    }

4360ap2 configure -attngrp L3 \


UTF::Router NETGEAR \
    -sta {NETGEAR_2G eth1 NETGEAR_5G eth2} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "BISON04T_REL_7_14_43_22" \
    -power {npc108 3} \
    -console 10.132.116.159:40001 \
    -relay "ApHost" \
    -lanpeer ApHost \
    -lan_ip 192.168.1.3 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -portmirror mirror2 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        lan_ipaddr=192.168.1.3
        wl0_ssid=NETGEAR_2G
		wl0_country_code=US
        wl1_ssid=NETGEAR_5G
		wl1_country_code=US
    } \
    -wlinitcmds { sleep 5; wl -i eth1 country US; wl -i eth2 country US}
	

NETGEAR_2G configure -attngrp L1
NETGEAR_5G configure -attngrp L1



UTF::Router LINKSYSEA3500 \
    -sta {LINKSYS eth1} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "1.1.40.162464" \
    -power {npc108 3} \
    -relay "ApHost" \
    -lanpeer ApHost \
    -lan_ip 192.168.1.4 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror1 \
    -nvram {
        lan_ipaddr=192.168.1.4
        wl0_ssid=LINKSYSKPI2G
		wl0_country_code=US
        wl1_ssid=LINKSYSKPI5G
		wl1_country_code=US
    } \
    -wlinitcmds { sleep 5; wl -i eth1 country US; wl -i eth2 country US}
	

LINKSYS configure -attngrp L3


###################### Cisco AP ############

UTF::Router Cisco \
    -sta {CISCO eth1} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "15.2(2)JA" \
    -power {npc108 3} \
    -relay "ApHost" \
    -lanpeer ApHost \
    -lan_ip 192.168.1.5 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror1 \
    -nvram {
        lan_ipaddr=192.168.1.5
        wl0_ssid=CISCO_TEST
		wl0_country_code=US
        wl1_ssid=CISCO_TEST
		wl1_country_code=US
    } \
    -wlinitcmds { sleep 5; wl -i eth1 country US; wl -i eth2 country US}
	

CISCO configure -attngrp L2

################# DUT configuration #############################

UTF::MacOS kpiblr01tst1 -sta {MacX-A en0 AMacX-A awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-gala" \
		-tag BIS715GALA_REL_7_21_94_25 \
		-app_tag BIS715GALA_REL_7_21_94_25 \
        -name "MacX-A" \
        -power {npc110 1} \
        -type Release_10_11 \
        -channelsweep {-count 15} \
        -lan_ip 10.132.116.172 \
        -kextload false \
        -slowassoc 5 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacX-A configure -ipaddr 192.168.1.72 -attngrp L5
AMacX-A configure -ipaddr 192.168.5.72 -attngrp L5

MacX-A clone MacX-15A284-A

MacX-15A284-A clone MacX-15A284-ext-A \
	-brand macos-external-wl-gala \
	-type Release_10_11 
	
MacX-15A284-A clone MacX-16A125a-AWDL_Peer-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_137 \
	-app_tag BIS715GALA_REL_7_21_137

MacX-15A284-A clone MacX-16A125a-ext-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1

MacX-15A284-A clone MacX-16A154-ext-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141

MacX-15A284-A clone MacX-16A154-AWDL_Peer-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141

MacX-15A284-A clone MacX-16A165-ext-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_143 \
	-app_tag BIS715GALA_REL_7_21_143
	
UTF::MacOS kpiblr01tst2 -sta {MacX-B en0 AMacX-B awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
		-tag BIS715GALA_REL_7_21_120 \
		-app_tag BIS715GALA_REL_7_21_120 \
        -name "MacX-B" \
        -power {npc110 2} \
        -type Release_10_12 \
        -kextload false \
        -slowassoc 5 \
        -lan_ip 10.132.116.173 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacX-B configure -ipaddr 192.168.1.73 -attngrp L5
AMacX-B configure -ipaddr 192.168.5.73 -attngrp L5

MacX-B clone MacX-16A99-B

MacX-16A99-B clone MacX-16A99-ext-B \
	-brand macos-external-wl-10_12 \
	-type Release_10_12
	
MacX-16A99-B clone MacX-16A125a-B \
	-tag BIS715GALA_REL_7_21_137 \
	-app_tag BIS715GALA_REL_7_21_137

MacX-16A99-ext-B clone MacX-16A125a-ext-B \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1
	
MacX-16A99-ext-B clone MacX-15A284-ext-B \
	-tag BIS715GALA_REL_7_21_94_25 \
	-app_tag BIS715GALA_REL_7_21_94_25 \
	-brand macos-external-wl-gala \
	-type Release_10_11

MacX-16A99-ext-B clone MacX-15A284-ext-10_12-B \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1 \
	-brand macos-external-wl-10_12 \
	-type Release_10_12
	
MacX-16A99-ext-B clone MacX-16A154-ext-B \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141 \
	-brand macos-external-wl-10_12 \
	-type Release_10_12

MacX-16A99-ext-B clone MacX-16A165-ext-B \
	-tag BIS715GALA_REL_7_21_143 \
	-app_tag BIS715GALA_REL_7_21_143 \
	-brand macos-external-wl-10_12 \
	-type Release_10_12

UTF::MacOS kpiblr01tst3 -sta {MacAirX-A en0 AMacAirX-A awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-gala" \
		-tag BIS715GALA_REL_7_21_94_25 \
		-app_tag BIS715GALA_REL_7_21_94_25 \
        -name "MacAirX-A" \
        -power {npc111 1} \
        -type Release_10_11 \
        -kextload false \
        -lan_ip 10.132.116.174 \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1

MacAirX-A configure -ipaddr 192.168.1.74 -attngrp L6
AMacAirX-A configure -ipaddr 192.168.5.74 -attngrp L6

MacAirX-A clone MacAirX-15A284-A \
	-tag BIS715GALA_REL_7_21_94_25

MacAirX-15A284-A clone MacAirX-15A284-ext-A \
	-brand macos-external-wl-gala \
	-type Release_10_11 

MacAirX-15A284-A clone MacAirX-16A125a-AWDL_Peer-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_137 \
	-app_tag BIS715GALA_REL_7_21_137

MacAirX-15A284-A clone MacAirX-16A125a-ext-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1

MacAirX-15A284-A clone MacAirX-16A154-ext-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141

MacAirX-15A284-A clone MacAirX-16A154-AWDL_Peer-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141

MacAirX-15A284-A clone MacAirX-16A165-ext-A \
	-brand macos-external-wl-10_12 \
	-type Release_10_12 \
	-tag BIS715GALA_REL_7_21_143 \
	-app_tag BIS715GALA_REL_7_21_143

UTF::MacOS kpiblr01tst4 -sta {MacAirX-B en0 AMacAirX-B awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl scansuppress 1; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
		-tag BIS715GALA_REL_7_21_94_25 \
		-app_tag BIS715GALA_REL_7_21_94_25 \
        -name "MacAirX-B" \
        -power {npc111 2} \
        -type Release_10_12 \
        -lan_ip 10.132.116.175 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1 \

MacAirX-B configure -ipaddr 192.168.1.75 -attngrp L6
AMacAirX-B configure -ipaddr 192.168.5.75 -attngrp L6

MacAirX-B clone MacAirX-16A99-B \
	-tag BIS715GALA_REL_7_21_120
	
MacAirX-16A99-B clone MacAirX-16A125a-B \
	-tag BIS715GALA_REL_7_21_137 \
	-app_tag BIS715GALA_REL_7_21_137

MacAirX-16A99-B clone MacAirX-16A99-ext-B \
	-brand macos-external-wl-10_12 \
	-type Release_10_12
	
MacAirX-16A99-ext-B clone MacAirX-16A125a-ext-B \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1

MacAirX-16A99-ext-B clone MacAirX-15A284-ext-B \
	-tag BIS715GALA_REL_7_21_94_25 \
	-app_tag BIS715GALA_REL_7_21_94_25 \
	-type Release_10_11 \
        -brand macos-external-wl-gala

MacAirX-16A99-ext-B clone MacAirX-16A154-ext-B \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12

MacAirX-16A99-ext-B clone MacAirX-16A165-ext-B \
	-tag BIS715GALA_REL_7_21_143 \
	-app_tag BIS715GALA_REL_7_21_143 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12

######################## MacBook #########################


UTF::MacOS kpiblr01tst6 -sta {MacBook-A en0 AMacBook-A awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
		-tag BIS715GALA_REL_7_21_137 \
		-app_tag BIS715GALA_REL_7_137 \
        -name "MacBook-B" \
        -power {npc123 2} \
        -type Release_10_12 \
        -lan_ip 10.132.116.165 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1
		
MacBook-A configure -ipaddr 192.168.1.65 -attngrp L8
AMacBook-A configure -ipaddr 192.168.5.65 -attngrp L8

MacBook-A clone MacBook-16A125a

MacBook-A clone MacBook-16A125a-ext-A \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12

MacBook-A clone MacBook-16A154-ext-A \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12

MacBook-A clone MacBook-16A165-ext-A \
	-tag BIS715GALA_REL_7_21_143 \
	-app_tag BIS715GALA_REL_7_21_143 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12


UTF::MacOS kpiblr01tst5 -sta {MacBook-B en0 AMacBook-B awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
		-tag BIS715GALA_REL_7_21_137 \
		-app_tag BIS715GALA_REL_7_137 \
        -name "MacBook-B" \
        -power {npc123 2} \
        -type Release_10_12 \
        -lan_ip 10.132.116.164 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 3640K -custom 1
		
MacBook-B configure -ipaddr 192.168.1.64 -attngrp L8
AMacBook-B configure -ipaddr 192.168.5.64 -attngrp L8

MacBook-B clone MacBook-16A125a

MacBook-B clone MacBook-16A125a-ext-B \
	-tag BIS715GALA_REL_7_21_139_1 \
	-app_tag BIS715GALA_REL_7_21_139_1 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12

MacBook-B clone MacBook-16A154-ext-B \
	-tag BIS715GALA_REL_7_21_141 \
	-app_tag BIS715GALA_REL_7_21_141 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12

MacBook-B clone MacBook-16A165-ext-B \
	-tag BIS715GALA_REL_7_21_143 \
	-app_tag BIS715GALA_REL_7_21_143 \
	-type Release_10_12 \
        -brand macos-external-wl-10_12
		
		


######################## Stressor ###########################

UTF::MacOS kpiblr01stressor -sta {DOME_STRESS en0 PDOME_STRESSOR awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan;} \
        -brand  "macos-internal-wl-syr" \
        -name "DOME_STRESS" \
        -type Debug_10_10 \
        -lan_ip 10.132.116.178 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 0 \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 3640K -custom 1


DOME_STRESS configure -tag "BIS120RC4_REL_7_15_166_24" -custom 1
DOME_STRESS configure -ipaddr 192.168.1.76
DOME_STRESS configure -attngrp L7


UTF::Linux kpiblr01softap -sta {4360softap p3p1} \
    -lan_ip 10.132.116.176 \
    -power "npc108 3" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump rssi}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M \
    -udp 1600m \
    -wlinitcmds {
                 wl down; wl country US/0; wl vht_features 3; wl bw_cap 2g -1; wl txbf 0
                }

4360softap configure -ipaddr 192.168.1.93 -attngrp L1 -ap 1 -hasdhcpd 1

UTF::DHD kpiblr01linuxdhd -sta {4360linux p3p1} \
    -lan_ip 10.132.116.181 \
	-power "npc108 4" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu} {%S wl dump rssi}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M \
    -udp 1600m \
    -wlinitcmds {
                 wl down; wl country US/0; wl vht_features 3; wl bw_cap 2g -1; wl txbf 0
                }

4360linux configure -ipaddr 192.168.1.94 -attngrp L8



# set physicalmacstas {}
# set physicalmacstas {}
# set physicalrouterstas {}
# foreach STA [UTF::STA info instances] {
    # if {[$STA hostis Router]} {
	# lappend physicalrouterstas $STA
    # }
# }
# foreach STA [UTF::MacOS info instances] {
    # if {$STA eq "::AppleCore"} {
	# continue
    # } else {
	# lappend physicalmacstas $STA
    # }
# }
# foreach STA [UTF::STA info instances] {
    # if {![$STA hostis MacOS]} {
	# continue
    # } else {
	# lappend physicalmacstas $STA
    # }
# }


## physicalrouterstas  -> physicalrouterstas

##################### Mac Custom tools configurations #######################################

proc ::update_mac_tools {{devices ""}} {
    # set MACIPERF "/projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf/iperf2-code/src/iperf"
   # set MACIPERF "/projects/hnd_sig_ext16/rmcmahon/Code/iperf/sourceforge/counters/iperf2-code/src/iperf"
    if {$devices eq {}} {
	set devices {MacX-A MacX-B MacAirX-B MacAirX-A MacBook-B DOME_STRESS}
    }
    foreach DUT $devices {
#	catch {eval [concat $DUT copyto $MACIPERF /usr/bin/iperf]}
	catch {eval [concat $DUT copyto /projects/hnd_sig/anujgupt/KPI/iperf/iperf_awdl /usr/bin/iperf_awdl]}
	catch {eval [concat $DUT copyto /projects/hnd_sig/anujgupt/KPI/iperf/iperf /usr/bin/iperf]}
	catch {eval [concat $DUT copyto /projects/hnd_sig/anujgupt/KPI/10.10/wl /usr/bin/wl]}
	catch {eval [concat $DUT copyto /projects/hnd_sig/anujgupt/KPI/ptp/ptpd2 /usr/bin/ptpd2]}
	catch {eval [concat $DUT copyto /projects/hnd_sig/anujgupt/KPI/applebt/applebt /usr/local/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig/anujgupt/KPI/wget/wget /usr/bin/wget]}
	$DUT deinit
    }
}

## BT objects ##

##################### PTP Configurations #######################################

#array set ::ptpinterfaces [list ApHost enp3s0f0 MacX-A en5 MacX-B en3 MacAirX-A en4 MacAirX-B en3 DOME_STRESS en3 ApHostB eth0 MacBook-B en2]
#array set ::ptpinterfaces [list MacX-A en3 MacX-B en3 MacAirX-B en2 MacAirX-A en2 DOME_STRESS en3 ApHost em1 ]
array set ::ptpinterfaces [list MacX-B en3 MacAirX-B en2 DOME_STRESS en3 ApHost em1]
proc ::enable_ptp {args} {
    if {![llength $args]} {
	set devices [array names ::ptpinterfaces]
    }  else {
	set devices $args
    }
    foreach dut $devices {
	if {[catch {$dut uname -r} err]} {
	    UTF::Message WARN $dut $err
	    continue
	}
	set interface $::ptpinterfaces([namespace tail $dut])
	if {[$dut hostis MacOS] && ![catch {$dut uname -r}]} {
	    catch {$dut rexec pkill ptpd2}
	    catch {$dut rexec rm /var/log/ptp*.log}
	    catch {$dut rexec /usr/bin/ptpd2 -L -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	    catch {$dut rexec route add -host 224.0.0.107 -interface $interface}
	    catch {$dut rexec route add -host 224.0.1.129 -interface $interface}
	} else {
	    catch {$dut rexec systemctl stop ntpd.service}
	    catch {$dut rexec pkill ptpd2}
	    catch {$dut rm /var/log/ptp*.log}
	    catch {$dut rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.3.0/[$dut uname -r]/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	#  catch {$dut ip route replace multicast 224.0.0.107/32 dev $interface}
	#  catch {$dut ip route replace multicast 224.0.1.129/32 dev $interface}
	    catch {$dut rexec route del -net 224.0.0.107 netmask 255.255.255.255}
	    if {[catch {$dut rexec route add -net 224.0.0.107 netmask 255.255.255.255 dev $interface} err]} {
		UTF::Message WARN $dut $err
	    }
	    catch {$dut rexec route del -net 224.0.1.129 netmask 255.255.255.255}
	    if {[catch {$dut rexec route add -net 224.0.1.129 netmask 255.255.255.255 dev $interface} err]} {
		UTF::Message WARN $dut $err
	    }


	}
    }
    UTF::Sleep 20
    foreach dut $devices {
	catch {$dut rexec tail -10 /var/log/ptpstats.log}
	$dut deinit
    }
}

# Support procedures required by KPISuite

# proc ::isalive {{devices ""}} {
    # if {$devices eq {}} {
	# set devices $physicalmacstas
    # }
    # foreach STA $devices {
	# if {[catch {$STA os_version} results]} {
	    # UTF::Message ERROR $STA "not responding"
	# } else {
	    # UTF::Message INFO $STA "OS Ver: $results"
	# }
	# $STA deinit
    # }
# }

# proc ::iperfver {{devices ""}} {
    # if {$devices eq {}} {
	# set devices $physicalmacstas
    # }
   # foreach STA $devices {
	# catch {$STA rexec iperf -v} results
	# UTF::Message INFO $STA "Iperf ver: $results"
	# $STA deinit
    # }
# }

 #Source the logical configuration

#source "utfconf/kpiblr01logical.tcl"

UTF::Q kpiblr01


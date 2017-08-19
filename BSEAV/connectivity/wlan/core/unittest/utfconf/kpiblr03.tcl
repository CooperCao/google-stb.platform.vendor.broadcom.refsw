#Testbed configuration file for blr05end1 UTF ACI Teststation
#Created by Sumesh Nair on 23JUNE2014 10:00 PM  
#Last checkin 30June2014 
####### Controller section:
# blr05end1: FC15
# IP ADDR 10.132.116.43
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.131.80.23 
# blr05ref1 : FC 15 4360mc_1(99)  10.131.80.24
#
####### STA section:
#
# blr05tst1: FC 15 43224 eth0 (10.131.80.25)
# blr05tst2: FC 15 43228 eth0 (10.131.80.26)
# blr05tst3: FC 15 43217 eth0 (10.131.80.27)
# blr05tst4: FC 15 4331  eth0 (10.131.80.28)
# blr05tst5: FC 15 43909  eth0 (10.131.80.63) 
######################################################### #
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/anujgupt/kpiblr03"

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

set ::UTF::ChannelPerf 1 
set DRIVER "7_21_160_2"

set OS "16A209a"

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext21/$::env(LOGNAME)/kpiblr03"

######################## Power Controllers ################

UTF::Power::Synaccess npc102 -lan_ip  192.168.1.102 -relay kpiblr03end1 -rev 1
UTF::Power::Synaccess npc104 -lan_ip  172.16.1.104 -relay kpiblr03end1 -rev 1
UTF::Power::Synaccess npc104 -lan_ip  172.16.1.103 -relay kpiblr03end1 -rev 1
#UTF::Power::Synaccess npc110 -lan_ip 172.16.1.110 -relay kpi01end1 -rev 1
#UTF::Power::Synaccess npc111 -lan_ip  172.16.1.111 -relay kpi01end1 -rev 1
#UTF::Power::Synaccess npc123 -lan_ip  172.16.1.123 -relay kpi01end1 -rev 1

####################### Attenuator  ######################



UTF::AeroflexDirect af1  -lan_ip 172.16.1.105 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
       -relay 10.131.81.174 \
       -group {
                L1 {1  2   3}
                L2 {4  5  6}
                L3 {7  8  9}
				L4 {10  11  12}
                ALL1 {1 2 3 4 5 6 7 8 9 10 11 12}
                }

UTF::AeroflexDirect af2  -lan_ip 172.16.1.107 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
		-relay kpiblr03end1 \
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
	foreach STA {MacX-A MacX-B} {
	set tag INFO
	if {[catch {$STA power on}]} {
	    set tag ERROR
	}
	if {[$STA hostis MacOS]} {
	catch {$STA killall dns-sd}
	catch {$STA wl awdl 0}
	UTF::Sleep 1
	$STA rexec "/usr/local/bin/applebt --setPowerState 0"
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

UTF::Linux kpiblr03end1 -lan_ip 10.131.81.157 \
	-sta {lan em1} \
	
	
UTF::Linux kpiblr03aphost -lan_ip 10.131.81.158 \
	-sta {lan1 em2} \

lan1 configure -ipaddr 192.168.1.51 \





set LANPEER lan
# set LANPEER ApHostB

UTF::Router NETGEAR \
    -sta {NETGEAR_2G eth1 NETGEAR_5G eth2} \
    -brand linux-2.6.36-arm-internal-router \
    -tag "BISON04T_BRANCH_7_14" \
    -power {npc102 1} \
    -relay "lan" \
    -lanpeer lan \
    -lan_ip 192.168.1.1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -portmirror mirror1 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        lan_ipaddr=192.168.1.1
        wl0_ssid=NETGEAR_2G
		wl0_country_code=US
        wl1_ssid=NETGEAR_5G
		wl1_country_code=US
		wl0_frameburst=on
		wl1_frameburst=on
    } \
    -wlinitcmds { sleep 5; wl -i eth1 country US; wl -i eth2 country US; wl -i eth1 frameburst 1; wl -i eth2 frameburst 1}
	

NETGEAR_2G configure -attngrp L1
NETGEAR_5G configure -attngrp L1

UTF::MacOS kpiblr03tst1 -sta {MacAirX-A en0 AMacAirX-A awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-gala" \
		-tag BIS715GALA_REL_7_21_94_25 \
		-app_tag BIS715GALA_REL_7_21_94_25 \
        -name "MacAirX-A" \
        -power {npc102 1} \
        -type Release_10_11 \
        -channelsweep {-count 15} \
        -lan_ip 10.131.81.161 \
        -kextload false \
        -slowassoc 5 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 4M -custom 1
		
MacAirX-A clone MacAirX-$OS-$DRIVER-A \
	-tag BIS715GALA_REL_$DRIVER \
	-app_tag BIS715GALA_REL_$DRIVER \
	-type Release_10_12 \
    -brand macos-external-wl-10_12

UTF::MacOS kpiblr02tst3 -sta {MacAirX-B en0 AMacAirX-B awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
		-tag BIS715GALA_REL_7_21_120 \
		-app_tag BIS715GALA_REL_7_21_120 \
        -name "MacAirX-B" \
        -power {npc102 2} \
        -type Release_10_12 \
        -kextload false \
        -slowassoc 5 \
        -lan_ip 10.131.81.162 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 4M -custom 1

MacAirX-B clone MacAirX-$OS-$DRIVER-B \
	-tag BIS715GALA_REL_$DRIVER \
	-app_tag BIS715GALA_REL_$DRIVER \
	-type Release_10_12 \
    -brand macos-external-wl-10_12

UTF::MacOS kpiblr03tst3 -sta {MacX-A en0 AMacX-A awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-gala" \
		-tag BIS715GALA_REL_7_21_94_25 \
		-app_tag BIS715GALA_REL_7_21_94_25 \
        -name "MacX-A" \
        -power {npc111 1} \
        -type Release_10_11 \
        -kextload false \
        -lan_ip 10.131.81.163 \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 4M -custom 1
		
MacX-A clone MacAirX-$OS-$DRIVER-A \
	-tag BIS715GALA_REL_$DRIVER \
	-app_tag BIS715GALA_REL_$DRIVER \
	-type Release_10_12 \
    -brand macos-external-wl-10_12

UTF::MacOS kpiblr03tst4 -sta {MacX-B en0 AMacX-B awdl0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl scansuppress 1; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
		-tag BIS715GALA_REL_7_21_94_25 \
		-app_tag BIS715GALA_REL_7_21_94_25 \
        -name "MacX-B" \
        -power {npc111 2} \
        -type Release_10_12 \
        -lan_ip 10.131.81.164 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -nobighammer 1 -tcpwindow 4M -custom 1 
		
MacX-B clone MacAirX-$OS-$DRIVER-B \
	-tag BIS715GALA_REL_$DRIVER \
	-app_tag BIS715GALA_REL_$DRIVER \
	-type Release_10_12 \
    -brand macos-external-wl-10_12
	




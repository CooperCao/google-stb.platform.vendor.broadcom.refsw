# -*-tcl-*-
#
# Configuration file for Milosz Zielinski's MC53 testbed
#

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
 

# Enable Windows TestSigning
set UTF::TestSigning 1


# UTFD support
set ::env(UTFDPORT) 9978
package require UTFD

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext13/$::env(LOGNAME)/mc53"

# Power controllers
# NPC1 in small Ramsey
UTF::Power::Synaccess mc53npc1 -lan_ip 10.22.23.60
# NPC2 in bottom large Ramsey
UTF::Power::Synaccess mc53npc2 -lan_ip 10.22.23.61
# NPC3 in top large Ramsey
UTF::Power::Synaccess mc53npc3 -lan_ip 10.22.23.62
# address allocated to mc53eth3, changing it to mc53npc4
# NPC4 in top large Ramsey
UTF::Power::Synaccess mc53npc4 -lan_ip 10.22.23.63

# Attenuator - Aeroflex
UTF::Aeroflex af -lan_ip 10.22.23.58 \
	-relay "mc53end1" \
	-group {AP1 {1 2 3 4} TOP_RAMSEY {3 4} BOTTOM_RAMSEY {1 2} ALL {1 2 3 4 5 6}}
	
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    
# Make Sure Attenuators are initialized
        ALL attn 0
        TOP_RAMSEY attn 20
        BOTTOM_RAMSEY attn 20
    
# Make sure radios are off on APs before testing
# to ensure that Embedded Nightly only sees one AP
#   mc53AP1 restart wl0_radio=0
#    mc53AP2 restart wl0_radio=0

    # now make sure that ALL STAs are deinit and down just in case debugging left them loaded
    foreach S { 43228Win81x64  4352Win8 4331fc15  43142Win81} {
	    UTF::Try "$S Down" {
		    catch {$S wl down}
		    catch {$S deinit}
	    }
    }
    # unset S so it doesn't interfere
    unset S
          
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed

    return
}

if {0} {
proc ::hidden_node {state {db 50}} {
    set state [string tolower $state]
    switch -exact $state {
	"on" {
	# Upper Ramsey with MacBook and 3 WiFi NIC e6400    
    STA1 attn 35 
    # Lower Ramsey with fc15 e6400        
    STA2 attn 42
    
	    set ::hidden_node_state "ON"
	}
	"off" {
	STA1 attn 5
	STA2 attn 5
        AP2 attn 5
		
	    set ::hidden_node_state "OFF"
	}
	default {
	    error "unknown state $state - should be on | off"
	}
    }
}
}


# Endpoint  FC11 - Traffic generators (no wireless cards)
UTF::Linux mc53end1 -sta {lan eth1}


# Dell E6400 laptop with 43228 chip
# in the upper large Ramsey
# Windows 8.1x64
# 2015.3.31 update installed on 2015.4.21
# 2015.7.02 update installed on 2015.7.13
UTF::Cygwin mc53tst1 -sta {43228Win81x64} \
        -lan_ip 10.22.23.51 \
        -osver 8164 \
        -sign true \
        -user user \
        -power {mc53npc3 1} \
        -installer InstallDriver \
        -ssh ssh \
        -tag NIGHTLY \
        -brand win8x_internal_wl \
        -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
        -tcpwindow 4M \
        -wlinitcmds {wl down; wl bw_cap 2g -1; wl msglevel +assoc +scan +regulatory} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

43228Win81x64 clone 43228Win81x64BISON_735 -tag BISON05T_BRANCH_7_35
43228Win81x64 clone 43228Win81x64EAGLE     -tag EAGLE_BRANCH_10_10



# Dell E6400 laptop with 4352 chip
# in the lower large Ramsey
# 2015.3.31 update installed on 2015.4.17
# 2015.7.09 - replaced this STA with a different system.  Almost 30 mins for TCP tests did not change.
#           - used same RF and other cables.
# 2015.7.02 update installed on 2015.7.09
UTF::Cygwin mc53tst2 -user user -sta {4352Win8} \
    -lan_ip 10.22.23.52 \
    -osver 8 \
    -sign true \
    -user user \
    -installer InstallDriver \
    -ssh ssh \
    -brand win8x_internal_wl \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
    -tcpwindow 4M \
    -power {mc53npc2 1} \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}


4352Win8 clone 4352Win8BISON     -tag BISON_BRANCH_7_10
4352Win8 clone 4352Win8EAGLE     -tag EAGLE_BRANCH_10_10
4352Win8 clone 4352Win8BISON_735 -tag BISON05T_BRANCH_7_35
4352Win8 clone 4352Win8aa        -tag AARDVARK_BRANCH_6_30



# Frank STA with 43142 chip
# in the lower large Ramsey
UTF::Linux mc53tst3 -sta {4331fc15 eth0} \
    -lan_ip 10.22.23.53 \
    -power {mc53npc2 2} \
    -type debug-p2p-mchan \
    -brand "linux-internal-wl" \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1}} \
    -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu} {%S wl msglevel} {%S wl phy_cal_disable 0}} \
    -nobighammer 1

#4331fc15 configure -ipaddr 192.168.1.5

4331fc15 clone 4331fc15BISON -tag BISON_BRANCH_7_10
4331fc15 clone 4331fc15EAGLE -tag EAGLE_BRANCH_10_10
#4331fc15 clone 4331fc15-aa -tag AARDVARK_BRANCH_6_30
#4331fc15 clone 4331fc15-AA37 -tag AARDVARK01T_TWIG_6_37_14


# Dell E6400 laptop with 43142 chip
# in the upper large Ramsey
# Windows 8.1
# 2015.3.31 update installed on 2015.4.21
# 2015.7.02 update installed on 2015.7.08
UTF::Cygwin mc53tst4 -sta {43142Win81} \
    -lan_ip 10.22.23.54 \
    -osver 81 \
    -sign true \
    -user user \
    -power {mc53npc3 2} \
    -installer InstallDriver \
    -ssh ssh \
    -brand win8x_internal_wl \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
    -tcpwindow 4M \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

43142Win81 clone 43142Win81BISON_735 -tag BISON05T_BRANCH_7_35
43142Win81 clone 43142Win81EAGLE     -tag EAGLE_BRANCH_10_10


if {0} {
# Dell E6400 laptop with 43142 chip
# in the upper large Ramsey
# Windows 10x64
# 2015.4.14 update installed on 2015.4.17
# 2015.5.11 update installed on 2015.5.11
# 2015.5.15 update installed on 2015.5.18
# moved STA to mc31 rig as mc31tst5 on 2015.6.4
UTF::Cygwin mc53tst5 -sta {43142Win10x64} \
    -lan_ip 10.22.23.55 \
    -osver 1064 \
    -node DEV_4365 \
    -sign true \
    -user user \
    -power {mc53npc4 1} \
    -installer InstallDriver \
    -ssh ssh \
    -brand win8x_internal_wl \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -tcpwindow 4M \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

43142Win10x64 clone 43142Win10x64BISON_735 -tag BISON05T_BRANCH_7_35
43142Win10x64 clone 43142Win10x64EAGLE     -tag EAGLE_BRANCH_10_10
}



# Windows 8.1  2014.11.8
# - doesn't exist
UTF::Cygwin mc53tst6 -sta {43228Win10} \
    -lan_ip 10.22.23.56 \
    -osver 10 \
    -sign true \
    -user user \
    -power {mc53npc4 2} \
    -installer InstallDriver \
    -ssh ssh \
    -brand win8x_internal_wl \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
    -tcpwindow 4M \
    -wlinitcmds {wl down; wl bw_cap 2g -1} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}

43228Win10 clone 43228Win10BISON_735 -tag BISON05T_BRANCH_7_35
43228Win10 clone 43228Win10EAGLE     -tag EAGLE_BRANCH_10_10


if {0} {
# 43228, 4313, 43142
#        -pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}}
#        -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
# JGP - turned off system on 2015.1.26
# turning it into 43142 Win 10x64 system

UTF::Cygwin mc53sta5 -sta {43228w82t} -node {DEV_4365} \
    -lan_ip 10.22.23.63 \
    -osver 8 \
    -installer InstallDriver \
    -ssh ssh \
    -brand win8x_internal_wl \
    -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
    -tcpwindow 4M \
    -power_button {auto} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl scansuppress 0}}
    
   43228w82t clone 43228w82 -nopm1 1
   43228w82t clone 43142win8 -sta {43142win8} -node {DEV_4365}
   43228w82t clone 4313win8 -sta {4313win8} -node {DEV_4727}
    
   43228w82t clone 43228w82-aa -tag AARDVARK_BRANCH_6_30 -brand win8_internal_wl -nopm1 1
   43228w82-aa clone 4313win8-aa -sta {4313win8-aa} -node {DEV_4727} -brand win8_internal_wl
   43228w82-aa clone 43142win8-aa -sta {43142win8-aa} -node {DEV_4365} -brand win8_internal_wl
 
   43228w82t clone 43228w82BISON -tag BISON_BRANCH_7_10 -nopm1 1
   43228w82t clone 43228w82BISON735 -tag BISON05T_BRANCH_7_35 -nopm1 1
   43228w82BISON clone 4313win8BISON -sta {4313win8BISON} -node {DEV_4727}
   43228w82BISON735 clone 4313win8BISON735 -sta {4313win8BISON735} -node {DEV_4727}
   43228w82BISON clone 43142win8BISON -sta {43142win8BISON} -node {DEV_4365}
   43228w82BISON735 clone 43142win8BISON735 -sta {43142win8BISON735} -node {DEV_4365}
}
      

UTF::Router mc53AP2 \
    -sta "47172 eth1" \
    -lan_ip 192.168.1.4 \
    -relay "mc53end1" \
    -power {mc53npc1 1} \
    -lanpeer lan \
    -console "mc53end1:40003" \
    -tag "AKASHI_REL_5_110_64" \
    -brand linux-internal-router \
    -nvram {
     et0macaddr=00:90:4c:09:00:8b
        macaddr=00:90:4c:09:01:9b
        lan_ipaddr=192.168.1.4
        lan_gateway=192.168.1.4
        dhcp_start=192.168.1.160
        dhcp_end=192.168.1.179
        lan1_ipaddr=192.168.2.4
        lan1_gateway=192.169.2.4
        dhcp1_start=192.168.2.160
        dhcp1_end=192.168.2.179
        fw_disable=1
        # router_disable=1
        wl_msglevel=0x101
        wl0_ssid=test47172
        wl0_channel=1
        wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
        #wl0_bcn=50

    }

47172 clone 47172-COMANCHE2-22 -tag COMANCHE2_REL_5_22_90

47172 clone 47172-BISON -tag BISON_BRANCH_7_10 -brand linux26-external-vista-router-combo -trx "linux-gzip"
47172 clone 47172-AA37 -tag AARDVARK01T_TWIG_6_37_14 -brand linux26-external-vista-router-combo -trx "linux-gzip"
47172 clone 47172-aa -tag AARDVARK_BRANCH_6_30

     
47172 configure -attngrp AP1


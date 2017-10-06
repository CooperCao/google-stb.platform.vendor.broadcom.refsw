# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

# To setup UTFD port for this rig
set ::env(UTFDPORT) 9988

###############################################
# Load Packages
# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
    package require UTF::Power
}

package require UTF::Linux
package require UTF::Power
package require UTF::AeroflexDirect
package require UTFD

# To enable the log to display milliseconds on timestamp
set ::UTF::MSTimeStamps 1

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/md18"

# Set attenuator ranges (needed for RvRNightly1)
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"
#set ::oemAttnRange "25-75 75-25"

#####################################################
#power cyclers (synaccess) admin/admin
UTF::Power::Synaccess md18nb1 -lan_ip 10.19.85.98 -rev 1
#####################################################

##################################################################
# Attenuator - Aeroflex (run 'af setup' before running the script)
UTF::AeroflexDirect af1 -lan_ip 172.16.10.252 \
    -group {G1 {1 2 3 4} G2 {5 6} G3 {7 8} G4 {9 10} G5 {11 12} ALL1 {1 2 3 4 5 6 7 8 9 10 11 12}}

# md18r1 connected to
G1 configure -default 0
# md18st1 connected to
G2 configure -default 0
# md18st2 connected to
G3 configure -default 0
# md18st3 connected to
G4 configure -default 0
# md18st4 connected to
G5 configure -default 0

#################################################################

set ::UTF::SetupTestBed {
    G1 attn default
    G2 attn default
    G3 attn default
    G4 attn default
    G5 attn default
    #power cycle softap
    #md17nb2 power cycle 4
    foreach S {4366st1 4366st2 4366st3 4366st4} {
        UTF::Try "$S Down" {
            catch {$S wl down}
            catch {$S deinit}
        }
    }
    unset S
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {md18r1} {
	foreach D {eth1 eth2 eth3} {
	    catch {$S apshell wl -i $D down}
	}
        #catch {$S apshell wl -i [$S cget -device] down}
    }
    # unset S so it doesn't interfere
    unset S
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed
    return
}

############################ relay/consolelogger ##########################################
# md18relay:
# IP ADDR 10.19.85.213
# NETMASK 255.255.252.0
# GATEWAY 10.19.84.1
# Fedora19
#
UTF::Linux md18relay \
    -lan_ip 10.19.85.213 \
    -power {md18nb1 1} \
    -brand "linux-internal-wl"

############################cisco switch ########################################
#md17-sw
#IPADDR: 10.19.86.152
#cisco/cisco
#power {md17nb1 7}

############################ md18-lx1 ##########################################
#directly connected to r1, used as controller and etherend Fedora 22
# md18-lx1 :
# eno1:
# IP ADDR 10.19.85.97
# NETMASK 255.255.252.0
# GATEWAY 10.19.84.1
# eno2:
# IP ADDR 172.16.10.254
# NETMASK 255.255.255.0
# 
#
UTF::Linux md18-lx1 \
    -lan_ip 10.19.85.97 \
    -sta {192int1 enp7s0f0 192int2 enp7s0f1 192int3 enp8s0f0 192int4 enp8s0f1} \
    -brand "linux-internal-wl"
192int1 configure -ipaddr 192.168.1.31
192int2 configure -ipaddr 192.168.1.32
192int3 configure -ipaddr 192.168.1.33
192int3 configure -ipaddr 192.168.1.34


############################ md1dr1 ##########################################
# md18r1: 4709/4366C0 MCH 4x4
# 4 links connected to md18-lx1 10.19.86.151
################################################################################
UTF::Router md18r1 -sta {md18ap1 eth1} \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas  \
    -relay "md18-lx1" \
    -lanpeer "192int1 192int2" \
    -lan_ip "192.168.1.11" \
    -console {10.19.85.213:40000} \
    -power {md18nb1 2} \
    -tag BISON04T_BRANCH_7_14 \
    -noradio_pwrsave 1 \
    -datarate {-b 1.8G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.6G \
    -embeddedimage {4366c} \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1} \
    -nvram "
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=md18/2G
	wl0_chanspec=11
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_vht_features=7
	wl1_ssid=md18/5GL
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
	wl1_vht_features=6
	wl2_ssid=md18/5GH
	wl2_chanspec=161
	wl2_obss_coex=0
	wl2_bw_cap=-1
	wl2_radio=0
 	wl2_vht_features=7
        lan_ipaddr=192.168.1.11
 	dhcp_start=192.168.1.121
  	dhcp_end=192.168.1.140
    "
md18ap1 configure -attngrp G1

############################ md18st1 ##########################################################
# md18st1 : FC 19  10.19.85.214 4366C0 MCH (J1 J2)
###############################################################################################

#NIC
UTF::Linux md18st1 \
    -sta {4366st1 enp1s0}  \
    -lan_ip 10.19.85.214 \
    -console 10.19.85.213:40001 \
    -power {md18nb1 3} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl txchain 3; wl rxchain 3; wl vht_features 7; wl country US/0;} \
    -brand "linux-internal-wl" \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -modopts {assert_type=1 nompc=1} \
    -msgactions {
	{ai_core_reset: Failed to take core} {
	    $self worry $msg;
	    $self power cycle;
	    return 1
	}
    }

4366st1 configure -attngrp G2 -tag  EAGLE_BRANCH_10_10


############################ md18st2 ##########################################
# md18st2 : FC 19  10.19.85.215 4366C0 MCH (J1 J2)
################################################################################
UTF::Linux md18st2 \
    -sta {4366st2 enp1s0}  \
    -lan_ip 10.19.85.215 \
    -console 10.19.85.213:40002 \
    -power {md18nb1 4} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl txchain 3; wl rxchain 3; wl vht_features 7; wl country US/0;} \
    -brand "linux-internal-wl" \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -modopts {assert_type=1 nompc=1} \
    -msgactions {
	{ai_core_reset: Failed to take core} {
	    $self worry $msg;
	    $self power cycle;
	    return 1
	}
    }

4366st2 configure -attngrp G3 -tag  EAGLE_BRANCH_10_10

############################ md18st3 ##########################################
# md18st3 : FC 19  10.19.85.216 4366C0 MCH (J1 J2)
################################################################################
UTF::Linux md18st3 \
    -sta {4366st3 enp1s0}  \
    -lan_ip 10.19.85.216 \
    -console 10.19.85.213:40003 \
    -power {md18nb1 5} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl txchain 3; wl rxchain 3; wl vht_features 7; wl country US/0;} \
    -brand "linux-internal-wl" \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -modopts {assert_type=1 nompc=1} \
    -msgactions {
	{ai_core_reset: Failed to take core} {
	    $self worry $msg;
	    $self power cycle;
	    return 1
	}
    }


4366st3 configure -attngrp G4 -tag  EAGLE_BRANCH_10_10

############################ md18st4 ##########################################
# md18st34: FC 19  10.19.85.217 4366C0 MCH (J1 J2)
################################################################################
UTF::Linux md18st4 \
    -sta {4366st4 enp1s0}  \
    -lan_ip 10.19.85.217 \
    -console 10.19.85.213:40004 \
    -power {md18nb2 6} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl txchain 3; wl rxchain 3; wl vht_features 7; wl country US/0;} \
    -brand "linux-internal-wl" \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -modopts {assert_type=1 nompc=1} \
    -msgactions {
	{ai_core_reset: Failed to take core} {
	    $self worry $msg;
	    $self power cycle;
	    return 1
	}
    }

4366st4 configure -attngrp G5 -tag  EAGLE_BRANCH_10_10

##################################################################################
# this is the proc to enable PTP protocol
array set ::ptpinterfaces [list md18st1 em1 md18st2 em1 md18st3 em1 md18st4 em1 md18relay em1]

proc ::enable_ptp {args} {
    if {![llength $args]} {
	set devices [array names ::ptpinterfaces]
    }  else {
	set devices $args
    }
    foreach dut $devices {
	# set device [lindex [split $dut _] 0]
	set device $dut
	set interface $::ptpinterfaces([namespace tail $device])
	catch {$device rexec systemctl stop ntpd.service}
	catch {$device rexec pkill ptpd2}
	#  catch {$device rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.2.0/src/[$device uname -r]/ptpd2 -b em1 -W -S}
	catch {$device rm /var/log/ptp*.log}
	catch {$device rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.3.0/[$device uname -r]/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface}
	catch {$device ip route replace multicast 224.0.0.107/32 dev $interface}
	catch {$device ip route replace multicast 224.0.1.129/32 dev $interface}
    }
    UTF::Sleep 120
    foreach dut $devices {
	catch {$dut rexec tail -10 /var/log/ptpstats.log}
	$dut deinit
    }
}

proc closeall {} {
    foreach s [UTF::STA info instances] {
	$s deinit
    }
}

#UTF::Q md18a md18-lx1

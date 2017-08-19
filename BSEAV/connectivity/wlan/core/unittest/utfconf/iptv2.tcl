# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: c6ff118c16badbda71b8f4ee3b4c6426f363ca43 $
#

####### Controller section:
# iptv2-controller:
# IP ADDR 10.19.87.69
# NETMASK 255.255.252.0
# GATEWAY 10.19.84.1
#
####### Host section:
# iptv2-h2 : FC 19   10.19.87.149
# iptv2-h3 : FC 19   10.19.87.150
# iptv2-h1 : FC 19   10.19.87.151
#
####### STA section
# iptv2-sta1 : FC 19   10.19.87.152
# iptv2-sta2 : FC 19   10.19.87.153
#
####### AP Router section:
# iptv2-r1: 4709/4366MCH5L
# iptv2-r2: 4709/4366MCH5L
# iptv2-r3: 4709/4366MCH5L

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
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/iptv2"

#power cycler (synaccess)
UTF::Power::Synaccess nb1 -lan_ip 10.19.87.71 -rev 1
UTF::Power::Synaccess nb2 -lan_ip 10.19.87.75
# Attenuator - Aeroflex (run 'af setup' before running the script)
UTF::AeroflexDirect af1 -lan_ip 172.16.10.250 \
    -group {G1 {1 2 3 4} G2 {5 6 7 8} G3 {9 10 11 12} ALL1 {1 2 3 4 5 6 7 8 9 10 11 12}}

# iptv2-ap1 connected to
G1 configure -default 0
# iptv2-r2 connected to
G2 configure -default 0
# not connected
G3 configure -default 0
UTF::AeroflexDirect af2 -lan_ip 172.16.10.251 \
    -group {G4 {1 2 3 4} G5 {5 6 7 8} G6 {9 10 11 12} ALL2 {1 2 3 4 5 6 7 8 9 10 11 12}}

# not connected
G4 configure -default 90
# iptv2-r3 connected to
G5 configure -default 15
# iptv2-sta2 connected to
G6 configure -default 0
#set ::UTF::SetupTestBed {
#    G1 attn default
#    G2 attn default
#    G3 attn default
#    G4 attn default
#    G5 attn default
#    G6 attn default
#    # Use apshell to bypass the init process.  This is safer if the
#    # router is in a bad state.
#    foreach S {iptv2-r1 iptv2-r2 iptv2-r3 iptv2-r4} {
#        catch {$S apshell wl -i [$S cget -device] down}
#    }
#    return
#}
set ::UTF::SetupTestBed {
    G1 attn default
    G2 attn default
    G3 attn default
    G4 attn default
    G5 attn default
    G6 attn default
    foreach S {iptv2-sta1 iptv2-sta2} {
        UTF::Try "$S Down" {
            catch {$S wl down}
            catch {$S deinit}
        }
    }
    unset S
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {iptv2-r1 iptv2-r2 iptv2-r3} {
        catch {$S apshell wl -i [$S cget -device] down}
    }
    unset ::UTF::SetupTestBed
    return
}
############################ iptv2-controller ##########################################
UTF::Linux iptv2-controller \
     -lan_ip 10.19.87.69 \
     -brand "linux-internal-wl"


############################ iptv2-h1 ##########################################
#directly connected to r1
#Coresair FC22
UTF::Linux iptv2-h1 \
    -lan_ip 10.19.87.151 \
    -sta {192int1 enp6s0f0 192int5 enp6s0f1 } \
    -power {nb1 7} \
    -brand "linux-internal-wl"
192int1 configure -ipaddr 192.168.1.111
192int5 configure -ipaddr 192.168.1.115
#192int2 configure -ipaddr 192.168.1.112
#192int3 configure -ipaddr 192.168.1.113

############################# iptv2-h2 ##########################################
##directly connected to r2
UTF::Linux iptv2-h2 \
    -lan_ip 10.19.87.149 \
    -sta {192int2 p1p1} \
    -brand "linux-internal-wl"
192int2 configure -ipaddr 192.168.1.112
############################# iptv2-h3 ##########################################
##directly connected to r3
UTF::Linux iptv2-h3 \
    -lan_ip 10.19.87.150 \
    -sta {192int3 p1p1} \
    -power {nb2 3} \
    -brand "linux-internal-wl"
192int3 configure -ipaddr 192.168.1.113


set stacmn {
    -tag EAGLE_BRANCH_10_10
    -brand "linux-internal-wl"
    -tcpwindow 4M
    -slowassoc 5 -reloadoncrash 1
    -udp 1.8g
    -wlinitcmds {
	wl msglevel +assoc;
	wl msglevel +error;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 7
	wl txbf_bfr_cap 0; 
	wl txbf_bfe_cap 1;
    }
}

############################ iptv2-sta1 ##########################################
# iptv2-sta      - 4366C0 MC
# Linux ver      - Fedora Core 19
################################################################################ 

UTF::Linux iptv2-sta1 -sta {4366sta1 enp5s0} -lan_ip 10.19.87.152 \
    -console 10.19.87.69:40004 -power {nb1 5} \
    {*}$stacmn

#enp5s0 HW: 00:10:18:f8:d2:cc

4366sta1 configure -ipaddr 192.168.1.15 -attngrp G3

############################ iptv2-sta2 ##########################################
# iptv2-sta2      - 4366C0 MC
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
################################################################################
UTF::Linux iptv2-sta2 -sta {4366sta2 enp3s0} -lan_ip 10.19.87.153 \
    -console 10.19.87.69:40003 -power {nb1 6} \
    {*}$stacmn

4366sta2 configure -attngrp G2 -ipaddr 192.168.1.16

############################ iptv2-r1 ##########################################
# iptv2-r1      - 4709C0/4366mc_P143 (C0)
################################################################################
UTF::Router iptv2-r1 -sta {4709ap1 eth1 4709ap1.%15 wl0.%} \
    -brand linux-2.6.36-arm-internal-router  \
    -relay "iptv2-h1" \
    -lanpeer "192int1 192int5" \
    -lan_ip "192.168.1.11" \
    -console {10.19.87.69:40000} \
    -power {nb1 1} \
    -tag EAGLE_BRANCH_10_10 \
    -noradio_pwrsave 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -embeddedimage {4366c} \
    -nvram {
	"fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
	gmac3_enable=1
        wl0_vht_features=7
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.11
 	dhcp_start=192.168.1.117
  	dhcp_end=192.168.1.120
	wl0_ssid=4366C0-r1
	watchdog=6000; # PR#90439
    }
4709ap1 configure -attngrp G1
#eth1 hw addr: 00:10:18:F8:CD:9E
############################ iptv2-r2 ##########################################
# iptv2-r2      - 4709C0/4366mc_P105 (B1)
################################################################################
UTF::Router iptv2-r2 -sta {4709proxap eth1} \
    -brand linux-2.6.36-arm-internal-router  \
    -relay "iptv2-h2" \
    -lanpeer "192int2 192int3" \
    -lan_ip "192.168.1.12" \
    -console {10.19.87.69:40001} \
    -power {nb1 2} \
    -tag BISON04T_BRANCH_7_14 \
    -noradio_pwrsave 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -embeddedimage {4366c} \
    -nvram {
	"fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
	gmac3_enable=1
        wl0_vht_features=7
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.12
 	dhcp_start=192.168.1.121
  	dhcp_end=192.168.1.130
	watchdog=6000; # PR#90439
    }
4709proxap configure  -attngrp G1
#eth1 hw addr: 00:10:18:F8:D2:DC

############################ iptv2-r3 ##########################################
# iptv2-r3      - 4709C0/4366mc (C0)
################################################################################
UTF::Router iptv2-r3 \
    -sta {4709psta2 eth1} \
    -brand linux-2.6.36-arm-internal-router  \
    -relay "iptv2-h3" \
    -lanpeer "192int3" \
    -lan_ip "192.168.1.13" \
    -console {10.19.87.69:40002} \
    -power {nb1 3} \
    -tag EAGLE_BRANCH_10_10 \
    -trx linux-apsta \
    -noradio_pwrsave 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -embeddedimage {4366c} \
    -nvram {
	"fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
	gmac3_enable=1
        wl0_vht_features=7
	wl_msglevel=0x101
	fw_disable=1
        lan_ipaddr=192.168.1.13
 	dhcp_start=192.168.1.131
  	dhcp_end=192.168.1.140
	watchdog=6000; # PR#90439
    }

4709psta2 configure  -attngrp G5
#eth1 hw addr: 00:10:18:F8:C7:D7

################################################################################
# private/base
4709ap1 clone ap_p -image /home/jqliu/arch/gclient/trunk.rtr/main/components/router/arm-uclibc/linux.trx -name AP
4366sta2 clone sta_p -name STA
4709proxap clone proxap_p -image /home/jqliu/arch/gclient/trunk.rtr/main/components/router/arm-uclibc/linux.trx -name ProxAP
4709psta2 clone psta_p -image /home/jqliu/arch/gclient/trunk.rtr/main/components/router/arm-uclibc/linux.trx -name PSTA

##################################################################################
# this is the proc to enable PTP protocol
array set ::ptpinterfaces [list iptv2-controller eth0 iptv2-sta1 eth0 iptv2-sta2 eth0 iptv2-h2 em1 iptv2-h3 em1]

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
	catch {$device rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.3.0/[$device uname -r]/ptpd2 -s -f /var/log/ptp.log -S \
		   /var/log/ptpstats.log -i $interface} ret
	UTF::Message INFO "" "starting ptpd2 returned: $ret"
	catch {$device ip route replace multicast 224.0.0.107/32 dev $interface} ret
	UTF::Message INFO "" "adding 224.0.0.107/32 returned: $ret"
	catch {$device ip route replace multicast 224.0.1.129/32 dev $interface} ret
	UTF::Message INFO "" "adding 224.0.1.129/32 returned: $ret"

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
#UTF::Q iptv2 iptv2-controller

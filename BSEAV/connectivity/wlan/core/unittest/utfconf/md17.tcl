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
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/md17"

# Set attenuator ranges (needed for RvRNightly1)
set ::cycle5G80AttnRange "0-90 90-0"
set ::cycle5G40AttnRange "0-90 90-0"
set ::cycle5G20AttnRange "0-90 90-0"
set ::cycle2G40AttnRange "0-95 95-0"
set ::cycle2G20AttnRange "0-95 95-0"
#set ::oemAttnRange "25-75 75-25"

#####################################################
#power cyclers (synaccess) admin/admin
UTF::Power::Synaccess md17nb1 -lan_ip 10.19.86.155 -rev 1
UTF::Power::Synaccess md17nb2 -lan_ip 10.19.86.153 -rev 1
#####################################################

##################################################################
# Attenuator - Aeroflex (run 'af setup' before running the script)
UTF::AeroflexDirect af1 -lan_ip 172.16.10.253 \
    -group {G1 {1 2 3 4} G2 {5 6} G3 {7 8} G4 {9 10} G5 {11 12} ALL1 {1 2 3 4 5 6 7 8 9 10 11 12}}

UTF::AeroflexDirect af2 -lan_ip 172.16.10.246 \
    -group {G6 {1 2} G7 {3 4} G8 {5 6} G9 {7 8} G10 {9}}

# md17r1/md17st1(sap) connected to
G1 configure -default 0

# md17st3 connected to
G2 configure -default 0

# md17st5 connected to
G3 configure -default 0

# md17st2 connected to
G4 configure -default 0

# md17st7 connected to
G5 configure -default 0

# md17st6 connected to
G6 configure -default 0

# md17st4 connected to
G7 configure -default 0

# md17st9 connected to
G8 configure -default 0

#md17st8 connected to
G9  configure -default 0

G10 configure -default 95
#################################################################

set ::UTF::SetupTestBed {
    G1 attn default
    G2 attn default
    G3 attn default
    G4 attn default
    G5 attn default
    G6 attn default
    G7 attn default
    G8 attn default
    #power cycle softap
    #md17nb2 power cycle 4
    foreach S {4357st1 4366st2 4366st3 4366st3 4366st4 4366st5 4366st6 4366st7 4366st8 4366st9} {
        UTF::Try "$S Down" {
            catch {$S wl down}
            catch {$S deinit}
        }
    }
    unset S
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {md17r1} {
        catch {$S apshell wl -i [$S cget -device] down}
    }
    # unset S so it doesn't interfere
    unset S
    # delete myself to make sure it doesn't rerun by some script issue
    unset ::UTF::SetupTestBed
    return
}

############################ relay/consolelogger ##########################################
# md17relay:
# IP ADDR 10.19.87.155
# NETMASK 255.255.252.0
# GATEWAY 10.19.84.1
# Fedora19
#
UTF::Linux md17relay \
    -lan_ip 10.19.87.155 \
    -power {md17nb1 6} \
    -brand "linux-internal-wl"

############################cisco switch ########################################
#md17-sw
#IPADDR: 10.19.86.152
#cisco/cisco
#power {md17nb1 7}

############################ md17-lx1 ##########################################
#directly connected to r1, used as controller and etherend Fedora 22
# md17-lx1 :
# eno1:
# IP ADDR 10.19.86.151
# NETMASK 255.255.252.0
# GATEWAY 10.19.84.1
# eno2:
# IP ADDR 172.16.10.254
# NETMASK 255.255.255.0
# 
#
UTF::Linux md17-lx1 \
    -lan_ip 10.19.86.151 \
    -sta {192int1 enp7s0f0 192int2 enp7s0f1} \
    -power {md17nb1 8} \
    -brand "linux-internal-wl"
192int1 configure -ipaddr 192.168.1.111
192int2 configure -ipaddr 192.168.1.112

############################ md17-lx2 ##########################################
#directly connected to r1, used as etherend Fedora 22

UTF::Linux md17-lx2 \
    -lan_ip 10.19.86.149 \
    -sta {192int3 enp7s0f0 192int4 enp7s0f1} \
    -power {md17nb1 5} \
    -brand "linux-internal-wl"
192int3 configure -ipaddr 192.168.1.113
192int4 configure -ipaddr 192.168.1.114
############################ md17r1 ##########################################
# md17r1: 4709/4366C0 MCH 4x4
# 4 links connected to md17-lx1 10.19.86.151
################################################################################
UTF::Router md17r1 -sta {md17ap1 eth1} \
    -brand linux-2.6.36-arm-internal-router-dhdap-atlas  \
    -relay "md17-lx1" \
    -lanpeer "192int1 192int3" \
    -lan_ip "192.168.1.11" \
    -console {10.19.87.155:40002} \
    -power {md17nb1 1} \
    -tag BISON04T_TWIG_7_14_131 \
    -noradio_pwrsave 1 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.6G \
    -embeddedimage {4366c} \
    -nvram {
        watchdog=3000
        wl0_ssid=4709/4366c0
        wl0_chanspec=36
        wl0_radio=0
        wl0_bw_cap=-1
        wl0_country_code=US/0
        wl0_atf=0
        wl0_vht_features=7
        wl0_mu_features="-1"
	fw_disable=1
        lan_ipaddr=192.168.1.11
 	dhcp_start=192.168.1.121
  	dhcp_end=192.168.1.140
	"fwd_cpumap=d:u:5:163:0 d:x:2:169:1 d:l:5:169:1"
	gmac3_enable=1
    }
md17ap1 configure -attngrp G1

############################ md17st1 ##########################################
# md17st1 : FC 19  10.19.85.150 4366C0 MC (J1 J2)
# dhcpd range is set to 192.168.1.141-160 in /etc/dhcp/dhcpd.conf
# updated 10/14/2016 (swapped ipaddr/hostname with md17st7)
################################################################################
#
# NIC SoftAP

UTF::Linux md17sap -sta {md17ap2 enp1s0} \
    -lan_ip md17st1 \
    -console "10.19.87.155:40004" \
    -power {md17nb2 4} -tag EAGLE_BRANCH_10_10 \
    -brand linux-internal-wl \
    -slowassoc 5 -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +error +assoc; wl down;wl dtim 3;wl vht_features 7;wl txchain 15; wl rxchain 15}

md17ap2 configure -ipaddr 192.168.1.12 -ap 1 -hasdhcpd 1 -attngrp G1

# DHD SoftAP
# added 4/21/2016
UTF::DHD AP \
        -lan_ip md17st1 \
        -sta {4366sapfd eth0} \
        -console "10.19.87.155:40006" \
        -power {md17nb2 4} \
        -dhd_tag DHD_BRANCH_1_363 \
        -dhd_brand linux-internal-dongle-pcie \
        -driver dhd-msgbuf-pciefd-debug \
        -brand linux-internal-dongle-pcie \
        -tag EAGLE_BRANCH_10_10 \
        -slowassoc 10 -reloadoncrash 1 \
        -nopm1 1 -nopm2 1 -nocal 1 \
        -datarate {-i 0.5 -frameburst 1} \
        -wlinitcmds {wl msglevel +assoc +mumimo; wl down; wl bw_cap 2g -1; wl country US/0; wl vht_features 7;} \
	-type 4366c0-roml/pcie-ag-splitrx-fdap-mbss-mfp-wnm-osen-wl11k-wl11u-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-proptxstatus-txpwr-11nprop-obss-dbwsw-err-assert-dbgam-dbgams-ringer-dmaindex16-bgdfs-dbgmu-mfgtest-dump-murx.bin
4366sapfd configure -ipaddr 192.168.1.12 -ap 1 -hasdhcpd 1 -attngrp G1
################################################################################
set stacmn {
    -tag EAGLE_BRANCH_10_10
    -brand "linux-internal-wl" \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M
    -slowassoc 10 -reloadoncrash 1
    -udp 1.8g
    -wlinitcmds {
	wl msglevel +assoc;
	wl msglevel +error;
	wl down;
	wl txchain 3; wl rxchain 3;
	wl country '#a/0';
	wl vht_features 7
    }
}

############################ md17st2 ##########################################
# md17st2 : FC 19  10.19.85.236 4366C0 MC (J1 J2)
################################################################################
UTF::Linux md17st2 \
    -sta {4366st2 enp1s0}  \
    -lan_ip 10.19.85.236 \
    -console 10.19.87.155:40000 \
    -power {md17nb1 3} \
    {*}$stacmn

4366st2 configure -attngrp G4

############################ md17st3 ##########################################
# md17st3 : FC 19  10.19.85.237 4366C0 MC (J1 J2)
################################################################################
UTF::Linux md17st3 \
    -sta {4366st3 enp1s0}  \
    -lan_ip 10.19.85.237 \
    -console 10.19.87.155:40001 \
    -power {md17nb1 4} \
    {*}$stacmn

4366st3 configure -attngrp G2

############################ md17st4 ##########################################
# md17st34: FC 19  10.19.85.238 4366C0 MCH (J1 J2)
################################################################################
UTF::Linux md17st4 \
    -sta {4366st4 enp1s0}  \
    -lan_ip 10.19.85.238 \
    -console 10.19.87.155:40009 \
    -power {md17nb2 1} \
    {*}$stacmn

4366st4 configure -attngrp G7

############################ md17st5 ##########################################
# md17st5 : FC 19  10.19.85.239 4366C0 MC (J1 J2)
################################################################################
UTF::Linux md17st5 \
    -sta {4366st5 enp1s0}  \
    -lan_ip 10.19.85.239 \
    -console 10.19.87.155:40009 \
    -power {md17nb2 2} \
    {*}$stacmn

4366st5 configure -attngrp G3

############################ md17st6 ##########################################
# md17st6 : FC 19  10.19.85.240 4366C0 MC (J1 J2)
# replaced 4357a1 to 4366C1 on 10/14/2016
################################################################################
UTF::Linux md17st6 \
    -sta {4366st6 enp1s0}  \
    -lan_ip 10.19.85.240 \
    -console 10.19.87.155:40007 \
    -power {md17nb2 3} \
    {*}$stacmn

4366st6 configure -attngrp G6

############################ md17st7 ##########################################################
# md17st7 : FC 19  10.19.86.241 
# updated to 4366C0 on 10/14/2016 (replaced mc4357a1 card, swapped ipaddr/hostname with md17st1)
###############################################################################################
UTF::Linux md17st7 \
    -sta {4366st7 enp1s0}  \
    -lan_ip 10.19.85.241 \
    -console 10.19.87.155:40005 \
    -power {md17nb1 2} \
    {*}$stacmn

4366st7 configure -attngrp G5
############################ md17st8 ##########################################################
# md17st8 : FC 19  10.19.85.242 
#Added on 10/14/2016
###############################################################################################
UTF::Linux md17st8 \
    -sta {4366st8 enp1s0}  \
    -lan_ip 10.19.85.242 \
    -console 10.19.87.155:40003 \
    -power {md17nb2 5} \
    {*}$stacmn

4366st8 configure -attngrp G9
############################ md17st9 ##########################################################
# md17st8 : FC 19  10.19.Temp
#Added on Temp
###############################################################################################
UTF::Linux md17st9 \
    -sta {4366st9 enp1s0}  \
    -lan_ip 10.19.87.244 \
    -console 10.19.87.155:40004 \
    -power {md17nb2 6} \
    {*}$stacmn

4366st8 configure -attngrp G8
###############################################################################################

# this is the proc to enable PTP protocol
array set ::ptpinterfaces [list md17st2 em1 md17st3 em1 md17st4 em1 md17st5 em1 md17st6 em1 md17st7 em1 md17st8 em1  md17st9 em1 md17sap em1 md17relay em1]

proc ::enable_ptp {args} {
    set ret 0
    if {![llength $args]} {
	set devices [array names ::ptpinterfaces]
    }  else {
	set devices $args
    }
    foreach device $devices {
	# set device [lindex [split $dut _] 0]
	#set device $dut
	set interface $::ptpinterfaces([namespace tail $device])
	catch {$device rexec systemctl stop ntpd.service}
	catch {$device rexec pkill ptpd2}
	#  catch {$device rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.2.0/src/[$device uname -r]/ptpd2 -b em1 -W -S}
	catch {$device rm /var/log/ptp*.log}
	if {[catch {$device rexec /projects/hnd_sig_ext16/rmcmahon/Code/ptp/ptpd-2.3.0/[$device uname -r]/ptpd2 -s -f /var/log/ptp.log -S /var/log/ptpstats.log -i $interface} err]} {
	    UTF::Message ERROR "" "$err"
	    set ret 1
	}
	catch {$device ip route replace multicast 224.0.0.107/32 dev $interface}
	catch {$device ip route replace multicast 224.0.1.129/32 dev $interface}
    }
    UTF::Sleep 120
    foreach device $devices {
	if {[catch {$device rexec tail -10 /var/log/ptpstats.log} err]} {
	    UTF::Message ERROR "" "$err"
	    set ret 1
	}
	$device deinit
    }
    return $ret
}


proc closeall {} {
    foreach s [UTF::STA info instances] {
	$s deinit
    }
}

#UTF::Q md17a md17-lx1

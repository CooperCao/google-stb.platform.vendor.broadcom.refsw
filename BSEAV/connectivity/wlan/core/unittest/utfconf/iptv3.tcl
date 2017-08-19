# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: d8c6235f92b3e902f3419c156151b91b50011588 $
#

####### Controller section:
# iptv3-controller: FC 19
# IP ADDR 10.19.86.148
# NETMASK 255.255.252.0 
# GATEWAY 10.19.84.1
#
####### Host section:
# iptv3-h1 : FC 22   10.19.86.129
# iptv3-h2 : FC 22
####### STA section:
# iptv3-sta1 : FC 19   10.19.86.124
# iptv3-sta2 : FC 19   10.19.86.125
# iptv3-sta3 : FC 19   10.19.86.126
# iptv3-sta4 : FC 19   10.19.86.127
#
####### AP Router section:
# iptv3-r1: ATLASII/4366b1
# iptv3-r2: ATLASII/4366C0


# To setup UTFD port for this rig
set ::env(UTFDPORT) 9988
set ::env(UTFDCONTROLLER) iptv3-controller
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
package require UTF::Sniffer

# To enable the log to display milliseconds on timestamp
set ::UTF::MSTimeStamps 1

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/iptv3"

#power cycler (synaccess)
UTF::Power::Synaccess nb1 -lan_ip 10.19.86.154 -rev 1
UTF::AeroflexDirect af1 -lan_ip 172.16.10.247 \
    -group {G1 {1 3 5 7} G2 {2 4 6 8} G3 {9 10 11 12} ALL1 {1 2 3 4 5 6 7 8 9 10 11 12}}
UTF::AeroflexDirect af2 -lan_ip 172.16.10.248 \
    -group {G4 {1 3 5 7} G5 {2 4 6 8} G6 {9 10 11 12} ALL2 {1 2 3 4 5 6 7 8 9 10 11 12}}

# iptv3-sta1 connected to
G1 configure -default 0
# iptv3-sta2 connected to
G2 configure -default 0
# iptv3-r1 connected to
G3 configure -default 0
# iptv3-sta3 connected to
G4 configure -default 0
# iptv3-sta4 connected to
G5 configure -default 0
# iptv3-r2 connected to
G6 configure -default 0


set ::UTF::SetupTestBed {
    G1 attn default
    G2 attn default
    G3 attn default
    G4 attn default
    G5 attn default
    G6 attn default
    # Use apshell to bypass the init process.  This is safer if the
    # router is in a bad state.
    foreach S {iptv3-r1 iptv3-r2} {
	foreach D {eth1 eth2 eth3} {
        #catch {$S apshell wl -i [$S cget -device] down}
	    catch {$S apshell wl -i $D down}
	}
    }
    return
}

############################ iptv3-controller ##########################################
UTF::Linux iptv3-controller \
    -lan_ip 10.19.86.148 \
    -power {nb1 7} \
    -brand "linux-internal-wl"

############################ iptv3-h1 ##########################################
#directly connected to r1
#Coresair
UTF::Linux iptv3-h1 \
    -lan_ip 10.19.86.129 \
    -sta {192int11 enp3s0f0 192int12 enp3s0f1 192int13 enp4s0f0 192int14 enp4s0f1} \
    -brand "linux-internal-wl"
192int11 configure -ipaddr 192.168.1.11
192int12 configure -ipaddr 192.168.1.12
192int13 configure -ipaddr 192.168.1.13
192int14 configure -ipaddr 192.168.1.14
#    -power {nb2 8} \

############################# iptv3-h2 ##########################################
##directly connected to r2
UTF::Linux iptv3-h2 \
    -lan_ip 10.19.86.130 \
    -sta {192int21 enp7s0f0 192int22 enp7s0f1 192int23 enp8s0f0 192int24 enp8s0f1} \
    -brand "linux-internal-wl"
192int21 configure -ipaddr 192.168.1.21
192int22 configure -ipaddr 192.168.1.22
192int23 configure -ipaddr 192.168.1.23
192int24 configure -ipaddr 192.168.1.24

set stacmn {
    -tag EAGLE_BRANCH_10_10
    -tcpwindow 4M
    -slowassoc 5 -reloadoncrash 1
    -udp 1.8g
    -wlinitcmds {
	wl msglevel +assoc;
	wl down;
	wl country '#a/0';
	wl bw_cap 2g -1;
	wl vht_features 7
    }
}
############################ iptv3-sta1 ##########################################
# iptv3-sta1      - 4366C0 (updated from 4366B1 on 9/27/2016)
# Linux ver      - Fedora Core 19
################################################################################ 
UTF::Linux iptv3-sta1 -sta {4366sta1 enp5s0} -lan_ip 10.19.86.124 \
    -console 10.19.86.148:40000 -power {nb1 1} \
    {*}$stacmn

4366sta1 configure -attngrp G1 -ipaddr 192.168.1.121
#00:10:18:F8:D9:31
############################ iptv3-sta2 ##########################################
# iptv3-sta2      - 4366C0 (updated from 4366B1 on 9/27/2016)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
################################################################################
UTF::Linux iptv3-sta2 -sta {4366sta2 enp5s0} -lan_ip 10.19.86.125 \
    -console 10.19.86.148:40001 -power {nb1 2} \
    {*}$stacmn

#enp3s0 HW: 00:10:18:f8:d2:cb

4366sta2 configure -attngrp G2 -ipaddr 192.168.1.122

############################ iptv3-sta3 ##########################################
# iptv3-sta      - 4366C0 (updated from 4366B1 on 9/27/2016)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
################################################################################
UTF::Linux iptv3-sta3 -sta {4366sta3 enp5s0} -lan_ip 10.19.86.126 \
    -console 10.19.86.148:40002 -power {nb1 3} \
    {*}$stacmn

#enp5s0 HW: 00:10:18:f8:d8:4f
4366sta3 configure -attngrp G4 -ipaddr 192.168.1.123

############################ iptv3-sta4 ##########################################
# iptv3-sta      - 4366C0 (updated from 4366B1 on 9/27/2016)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
################################################################################
UTF::Linux iptv3-sta4 -sta {4366sta4 enp3s0} -lan_ip 10.19.86.127 \
    -console 10.19.86.148:40003 -power {nb1 4} \
    {*}$stacmn

#enp5s0 HW: 00:10:18:f8:d9:38
4366sta4 configure -attngrp G5 -ipaddr 192.168.1.124

############################ iptv3-r1 ##########################################
# iptv3-r1      - AtlasII (4366b1)
# Note:
# The mapping between ethx/wlx and 5h, 5l, and 2g seems to be fixed in hw
# eth1/wl0 -> 2g
# eth2/wl1 -> 5l
# eth3/wl2 -> 5h
################################################################################
UTF::Router iptv3-r1 -sta {r1 r1}  \
    -relay "iptv3-h1" \
    -lanpeer "192int11 192int12 192int13 192int14" \
    -lan_ip "192.168.1.15" \
    -console {10.19.86.148:40004} \
    -power {nb1 5} \
    -tag BISON04T_BRANCH_7_14 \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8g \
    -brand "linux-2.6.36-arm-internal-router-dhdap-atlas" \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1} \
    -embeddedimage {4366b} \
    -noradio_pwrsave 1    -nvram "
	watchdog=3000
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=Atlas2/MCH2/r1
	wl0_chanspec=11
	wl0_obss_coex=0
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_vht_features=7
	wl1_ssid=Atlas2/MCH5l/r1
	wl1_chanspec=36
	wl1_obss_coex=0
	wl1_bw_cap=-1
	wl1_radio=0
	wl1_vht_features=6
	wl2_ssid=Atlas2/MCH5h/r1
	wl2_chanspec=161
	wl2_obss_coex=0
	wl2_bw_cap=-1
	wl2_radio=0
	wl2_vht_features=6
        lan_ipaddr=192.168.1.15
 	dhcp_start=192.168.1.115
  	dhcp_end=192.168.1.124
    "
r1 configure -attngrp G3

############################ iptv3-r2 ##########################################
# iptv3-r2      - AtlasII (4366C0)
# Note:
# The mapping between ethx/wlx and 5h, 5l, and 2g seems to be fixed in hw
# eth1/wl0 -> 2g
# eth2/wl1 -> 5l
# eth3/wl2 -> 5h
################################################################################
UTF::Router iptv3-r2 -sta {r2 r2}  \
    -tag BISON04T_BRANCH_7_14 \
    -brand "linux-2.6.36-arm-internal-router-dhdap-atlas" \
    -relay "iptv3-h2" \
    -lanpeer "192int21 192int22 192int23 192int24" \
    -lan_ip "192.168.1.16" \
    -console {10.19.86.148:40005} \
    -power {nb1 6} \
    -datarate {-i 0.5 -frameburst 1} -udp 1.8g \
    -postboot {sysctl -w kernel.panic=1 kernel.panic_on_oops=1} \
    -embeddedimage {4366c} \
    -noradio_pwrsave 1 -perfchans {36/80} -nosamba 1 \
    -yart {-attn5g 0-93 -attn2g 18-93 -pad 24} \
    -model bcm94709acdcrh_p415_nvram \
    -nvram {
	samba_mode=2
	#watchdog=3000
	lan_stp=0
	lan1_stp=0
	wl_msglevel=0x101
	console_loglevel=7
	wl0_ssid=Atlas2/MCH2/r2
	wl0_chanspec=11
	wl0_bw_cap=-1
	wl0_radio=0
	wl0_vht_features=5
	wl1_ssid=Atlas2/MCH5l/r2
	wl1_chanspec=36
	wl1_bw_cap=-1
	wl1_radio=0
	wl1_vht_features=6
	wl2_ssid=Atlas2/MCH5h/r2
	wl2_chanspec=161
	wl2_obss_coex=0
	wl2_bw_cap=-1
	wl2_radio=0
	wl2_vht_features=6
        lan_ipaddr=192.168.1.16
 	dhcp_start=192.168.1.125
  	dhcp_end=192.168.1.135
    }
r2 configure -attngrp G6

##################################################################################
# this is the proc to enable PTP protocol
array set ::ptpinterfaces [list iptv3-sta1 em1 iptv3-sta2 em1 iptv3-sta3 em1 iptv3-sta4 em1 iptv3-controller eth0 ]

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

set UTF::RouterNightlyCustom {

    if {[regexp {(.*x)/} $Router - base]} {
	# external
	if {$STA3 ne ""} {

	    package require UTF::Test::TripleBand

	    TripleBand ${base}/0 ${base}/1 ${base}/2 $STA1 $STA2 $STA3 \
		-c1 3l -c2 44/80 -c3 157/80 -lan1 192int21 -lan2 192int22 -lan3 192int23

	}
    } else {
	# Internal

#	UTF::Try "$Router: Vendor IE" {
#	    package require UTF::Test::vndr_ie
#	    UTF::Test::vndr_ie $Router $STA1
#	}
#	catch {
#	    package require UTF::Test::MiniUplinks
#	    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
#	}
#    }
#    package require UTF::Test::MiniUplinks
#    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
    package require UTF::Test::Repeaters
    UTF::Test::Repeaters $Router $STA1 -sta $STA2 -otherlans {192int21}
}

#UTF::Q iptv3 iptv3-controller

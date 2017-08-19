####### Controller section:
# iptv1-controller: 
# IP ADDR 10.19.87.65
# NETMASK 255.255.252.0 
# GATEWAY 10.19.84.1
#
####### SOFTAP section:
#
# iptv1-softap : FC 19   10.19.87.66
#
####### STA section 
# iptv1-sta : FC 19   10.19.87.67
#


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
#package require UTF::WebRelay
#package require UTF::Airport
#package require UTF::utils
package require UTFD

# To enable the log to display milliseconds on timestamp
set ::UTF::MSTimeStamps 1

# To setup UTFD port for this rig
set ::env(UTFDPORT) 9977

# To disable automatic restore of UTFD previous state
#set ::UTFD::norestrore 1
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext20/$::env(LOGNAME)/iptv1"

#replaced on 1/6/2017
#power cycler (synaccess)
UTF::Power::Synaccess nb -lan_ip 10.19.87.75  -rev 1

#power cycler (webswitch MC35WS2 - serial 00-0C-C8-02-29-9E)
#UTF::WebRelay RamseyLx1 -lan_ip 10.19.87.76 -port 1
#UTF::WebRelay RamseyLx2 -lan_ip 10.19.87.76 -port 2

# Attenuator - Aeroflex (run 'af setup' before running the script)
UTF::AeroflexDirect af -lan_ip 172.16.10.249 \
    -group {G1 {1 2 3} G2 {4 5 6} G3 {7 8 9} G4 {10 11 12} ALL {1 2 3 4 5 6 7 8 9 10 11 12}}

# To use wl from trunk (set default); Use -app_tag to modify.
set UTF::TrunkApps 1

# To enable ChannelSweep performance test
set UTF::ChannelPerf 1

# To set attenuator range
set ::cycle5G80AttnRange "0-80 80-0"
set ::cycle5G40AttnRange "0-80 80-0"
set ::cycle5G20AttnRange "0-80 80-0"
set ::cycle2G40AttnRange "0-80 80-0"
set ::cycle2G20AttnRange "0-80 80-0"


############################ iptv1-controller ##########################################
UTF::Linux iptv1-controller \
     -lan_ip 10.19.87.65 \
    -power {nb 1} \
     -brand "linux-internal-wl"

############################ iptv1-h1 ##########################################
#directly connected to r1
# FC22
UTF::Linux iptv1-h1 \
    -lan_ip 10.19.87.78 \
    -sta {192int1 enp7s0f0} \
    -power {nb 2} \
    -brand "linux-internal-wl"
192int1 configure -ipaddr 192.168.1.211
############################ iptv1-h2 ##########################################
#directly connected to r1
# FC19
UTF::Linux iptv1-h2 \
    -lan_ip 10.19.87.79 \
    -sta {192int2 p4p1} \
    -power {nb 5} \
    -brand "linux-internal-wl"
192int2 configure -ipaddr 192.168.1.212

############################ iptv1-r1 ##########################################
# iptv1-r1      - 4709C0/43602
################################################################################ 
#SCB test needs following nvram settings:
#http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/PcieFullDongleScbExpand#4-1_Architecture_overview
#-nvram {
#    wl0_scb_alloc=0|1   <--- global nvram var to disable/enable scb allocation in host memory for dongle routers
#    wlx_bsscfg_class=0|1|2 <--- nvram var per wlan interface. 0x0 data service, 0x1 public service, 0x2 video service.
#        SCB associating to public bsscfg are called public SCBs, associating to data or video are called private SCB
#        e.g.
#          wl0.1_bsscfg_class=1 <--- wl0.1 bsscfg configured as public service
#          wl0_bsscfg_class=0 wl bsscfg configured as data service
#    wl0_scb_alloc_class=0|1|2|3 <--- global nvram var
#        0x0: scb only allocated from dongle memory
#        0x1: public scbs are allocated from host memory, private scbs are allocated from dongle memory
#        0x2: either allocate scb from dongle upto max limit #, 
#             or allocate scb from dongle when available memory greater than minimum threshold
#        0x3: phase-2
#}
UTF::Router iptv1-r1 -sta {4709ap eth1 4709ap.%2 wl0.%} \
    -brand linux-2.6.36-arm-internal-router  \
    -relay "iptv1-h1" \
    -lanpeer "192int1 192int2" \
    -lan_ip "192.168.1.210" \
    -console {10.19.87.65:40002} \
    -tag EAGLE_BRANCH_10_10 \
    -power {nb 8} \
    -datarate {-b 1.6G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.6G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -nvram {
        wl0_frameburst=on
        wl0.1_frameburst=on
        wl_frameburst=on
	wl_msglevel=0x101
	fw_disable=0
        lan_ipaddr=192.168.1.210
	dhcp_start=192.168.1.215
  	dhcp_end=192.168.1.219
        wl0_ssid=43602
	watchdog=6000; # PR#90439
	# Allow vht_features to use defaults - 2g will be
	# using prop11n instead of VHT. for using 2g VHT set:
	# wl0_vht_features=7
    } \
    -yart {-attn5g 31-94 -attn2g 61-94 -pad 26} \
    -noradio_pwrsave 1 -perfchans {36/80} -nosamba 1
#eth1 hw: 00:10:18:F8:C0:BB

4709ap configure -attngrp G3 -ap 1

############################ iptv1-softap ##########################################
# iptv1-softap      - 4360mc_1(199)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3 
# Linux ver      - Fedora Core 19
# RF Enclosure 1 - STE 4360
################################################################################ 
UTF::Linux iptv1-sta1 \
    -sta {4360st1 enp1s0}  \
    -lan_ip 10.19.87.66 \
    -console 10.19.87.65:40000 \
    -tcpwindow 4M -udp 1.6G \
    -datarate {-b 1.6G -i 0.5 -frameburst 1} \
    -slowassoc 10 -reloadoncrash 1 \
    -nobighammer 1 \
    -tag trunk \
    -power {nb 6} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -brand "linux-internal-wl" \
    -wlinitcmds {wl msglevel +assoc;wl msglevel +error;wl down;wl bw_cap 2g -1;wl vht_features 3}


#enp1s0 hw: 00:10:18:EE:D1:E2
4360st1 configure -attngrp G1

#4360st1 clone 4360sap -tag EAGLE_BRANCH_10_10
# wifi ip address
#4360sap-bis configure -ipaddr 192.168.10.11 -attngrp G2 -ap 1 -hasdhcpd 1
#4366sap configure -ipaddr 192.168.10.11 -ap 1 -hasdhcpd 1
#4360sap configure -ipaddr 192.168.1.221 -attngrp G1 -ap 1 -hasdhcpd 1


############################ iptv1-sta2 ##########################################
# iptv1-sta      - 4360mc_1(199)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 19
# RF Enclosure 1 - STE 4360
################################################################################
UTF::Linux iptv1-sta2 \
    -sta {4360st2 enp1s0}  \
    -lan_ip 10.19.87.67 \
    -console 10.19.87.65:40001 \
    -power {nb 7} \
    -datarate {-b 1.6G -i 0.5 -frameburst 1} \
    -tcpwindow 4m -udp 1.6G \
    -slowassoc 10 -reloadoncrash 1 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl bw_cap 2g -1;wl vht_features 3;} \
    -brand "linux-internal-wl"

#enp1s0 hw: 00:10:18:EE:D1:E2
4360st2 configure -attngrp G2

############################ iptv1-sta3 ##########################################
# iptv1-sta      - 43602mc_1
# Linux ver      - Fedora Core 19
# RF Enclosure 1 - 43602
##################################################################################
UTF::Linux iptv1-sta3 \
    -sta {43602 enp5s0}  \
    -lan_ip 10.19.87.76 \
    -console 10.19.87.65:40003 \
    -power {nb 4} \
    -datarate {-b 1.6G -i 0.5 -frameburst 1} \
    -tcpwindow 4m -udp 1.6G \
    -slowassoc 10 -reloadoncrash 1 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -brand "linux-internal-wl" \
    -wlinitcmds {wl msglevel +assoc;wl msglevel +error;wl down;wl bw_cap 2g -1;wl vht_features 3}
#    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl vht_features 3;}
#    -wlinitcmds {wl msglevel +assoc; wl msglevel +error; wl down; wl country US/0;}

#enp1s0 hw: 00:10:18:EE:D1:E2
43602 configure -attngrp G4

#=================================================================================

# this is the proc to enable PTP protocol
array set ::ptpinterfaces [list iptv1-controller eth0 iptv1-sta1 eth0 iptv1-sta2 eth0 iptv1-sta3 em1 iptv1-h2 em1]

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
	#catch {$device rexec pkill ptpd2}
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
set UTF::RouterNightlyCustom {
    UTF::Try "$Router: Vendor IE" {
	package require UTF::Test::vndr_ie
	UTF::Test::vndr_ie $Router $STA1
    }
    catch {
	package require UTF::Test::MiniUplinks
	UTF::Test::MiniUplinks $Router $STA1 -sta $STA2 \
	    -pstareboot -dodwdsr 1 -dowetr 1
    }

}
set ::UTF::SetupTestBed {
    G1 attn 0
    G2 attn 0
    G3 attn 15
    G4 attn 95
    #ALL attn?
}

proc closeall {} {
    foreach s [UTF::STA info instances] {
	$s deinit
    }
}

#UTF::Q iptv1 iptv1-controller

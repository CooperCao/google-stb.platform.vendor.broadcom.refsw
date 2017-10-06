##################################################################
#
# UTF configuration for MD15 ( KPI Phase 2 testbed ) SEPTEMBER 2015              
# Testbed location : 5th Floor , Bldg 190 , Mathilda Place  
#                                         
################Station Generic Information########################
#
# Controller  md15end1   10.19.61.187 
# AP1         4708/4360  192.168.1.2    NPC PORT 1  ATTN G1
# AP2         4708/4360  192.168.1.3    NPC PORT 2  ATTN G2
# AP HOST     md15tst7   10.19.61.197   NPC PORT 3  
# LINUX DHD   md15tst6   10.19.61.196   NPC PORT 4  ATTN G3
# MACBOOK PRO md15tst1   10.19.61.191   DUT
# MACBOOK PRO md15tst2   10.19.61.192   DUT
# MACBOOK AIR md15tst3   10.19.61.193   DUT
# MACBOOK AIR md15tst4   10.19.61.194   DUT
# MACBOOK PRO md15tst5   10.19.61.195   STRESSOR
# LINUX BOX   md15snf1   10.19.61.188   SNIFFER1 -> STA
# LINUX BOX2  md15tst8   10.19.61.198   SNIFFER2 -> AP
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
#####################################################################

set ::UTF::SummaryDir "/projects/hnd_sig_ext21/sumesh/md15"
set ::env(UTFDPORT) 9977
package require UTF::Power
package require UTF::AeroflexDirect
package require UTF::WebRelay
package require UTFD
package require UTF::Streams
package require UTF::Sniffer
package require UTF::MacBT_dev

# Packages commonly used in interactive mode
if {[info exists ::tcl_interactive] && $::tcl_interactive} {
    package require UTF::Test::ConnectAPSTA
    package require UTF::Test::APChanspec
    package require UTF::FTTR
    package require UTF::Test::ConfigBridge
}

set ::afmaxattn 95
set ::UTF::MSTimeStamps 1

######################## Power Controllers ################

UTF::Power::Synaccess npc8 -lan_ip 172.16.1.108 -rev 1
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.109
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.110 -rev 1

####################### Attenuator  ######################

UTF::AeroflexDirect af1  -lan_ip 172.16.1.106 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
        -group {
                L1 {1  4  7 }
                L2 {2  5  8 }
                L3 {3  6  9 }
                ALL1 {1 2 3 4 5 6 7 8 9}
                }

UTF::AeroflexDirect af2  -lan_ip 172.16.1.107 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
        -group {
                L4 {1  5   9}
                L5 {2  6  10}
                L6 {3  7  11}
		L7 {4  8  12}
                ALL2 {1 2 3 4 5 6 7 8 9 10 11 12}
                }

L1 configure -default 0
L2 configure -default 15
L3 configure -default 0
L4 configure -default 0
L5 configure -default 0
L6 configure -default 0
L7 configure -default 0

set ::attnmax 95

################### Sniffer ###########################

UTF::Sniffer SNIF_STA \
	-lan_ip 10.19.61.188 \
	-user root \
	-tag BISON_BRANCH_7_10 \
	-sta {4360SNIF_STA enp1s0} \
	-power {npc8 5} \


# UTF::Sniffer SNIF_AP \
   # -lan_ip 10.19.61.198 \
   # -user root \
   # -tag BISON_BRANCH_7_10 \
   # -sta {4360SNIF_AP  enp3s0} \
   # -power {npc8 6}

############## Controller and hosts####################

UTF::Linux md15end1 -lan_ip 10.19.61.187 \

#UTF::Linux md15tst7 -lan_ip 10.19.61.197 \
#                    -sta {ApHost p4p1 mirror1 eth2 mirror2 p2p1} \

#UTF::Linux mc73 -lan_ip 10.19.85.172 \
#                    -sta {ApHost enp4s0f1 mirror1 enp4s0f0 mirror2 enp3s0f1} \

UTF::Linux md15tst9 -lan_ip 10.19.61.189 \
                    -sta {ApHost enp4s0f1 mirror1 enp4s0f0 mirror2 enp3s0f1} -iperf "/usr/local/bin/iperf"\

UTF::Linux Broadwell2U -lan_ip 10.19.85.174 \
                    -sta {srv-eth0 enp7s0f1 srv-eth1 enp7s0f0 srv-eth2 enp8s0f1 srv-eth3 enp8s0f0} \

ApHost configure -ipaddr 192.168.1.51 \

UTF::Linux md15tst7 -lan_ip  10.19.61.197  \
                    -sta {ApHostB p4p1} \

ApHostB configure -ipaddr 192.168.1.50 \

################# Routers ##############################

set LANPEER ApHost
# set LANPEER ApHostB

UTF::Router KPI_AP1 \
    -sta {4360ap1 eth1 } \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {npc8 1} \
    -bootwait 30 \
    -relay "ApHost" \
    -lanpeer $LANPEER \
    -console 10.19.61.197:40000 \
    -lan_ip 192.168.1.2 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror1 \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        lan_ipaddr=192.168.1.2
        wl0_ssid=4360ap1
        watchdog=6000
        lan_stp=0
    }

4360ap1 configure  -attngrp L1 \


UTF::Router KPI_AP2 \
    -sta {4360ap2 eth1 } \
    -brand linux-2.6.36-arm-internal-router \
    -tag "EAGLE_BRANCH_10_10" \
    -power {npc8 2} \
    -bootwait 30 \
    -relay "ApHost" \
    -lanpeer $LANPEER \
    -console 10.19.61.197:40001 \
    -lan_ip 192.168.1.3 \
    -datarate {-b 1.2G -i 0.5 -frameburst 1} \
    -tcpwindow 4M -udp 1.2G \
    -slowassoc 10 \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -portmirror mirror2 \
    -nvram {
        wl_msglevel=0x101
        fw_disable=0
        lan_ipaddr=192.168.1.3
        wl0_ssid=4360ap2
        watchdog=6000
        lan_stp=0
    }

4360ap2 configure -attngrp L2 \


################# DUT configuration #############################

UTF::MacOS md15tst1 -sta {MacX-A en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-gala" \
	-name "MacX-A" \
        -power {npc2 1} \
        -type Release_10_11 \
        -channelsweep {-count 15} \
        -lan_ip 10.19.61.191 \
        -kextload false \
        -slowassoc 5 \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -iperf "/usr/local/bin/iperf" \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 3640K -custom 1 \
		-pre_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \
		-post_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \

MacX-A configure -attngrp L5

UTF::MacOS md15tst2 -sta {MacX-B en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
        -name "MacX-B" \
        -power {npc2 2} \
        -type Release_10_12 \
        -kextload false \
        -slowassoc 5 \
        -lan_ip 10.19.61.192 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -iperf "/usr/local/bin/iperf" \
        -nobighammer 1 -tcpwindow 3640K -custom 1 \
		-pre_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \
		-post_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \

MacX-A configure -attngrp L5

UTF::MacOS md15tst3 -sta {MacAirX-A en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-gala" \
        -name "MacAirX-A" \
        -power {npc3 1} \
        -type Release_10_11 \
        -kextload false \
        -lan_ip 10.19.61.193 \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -iperf "/usr/local/bin/iperf" \
        -nobighammer 1 -tcpwindow 3640K -custom 1 \
		-pre_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \
		-post_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \

MacAirX-A configure -attngrp L4

UTF::MacOS md15tst4 -sta {MacAirX-B en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan } \
        -brand  "macos-external-wl-10_12" \
        -name "MacAirX-B" \
        -power {npc3 2} \
        -type Release_10_12 \
        -lan_ip 10.19.61.194 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 1 \
        -iperf "/usr/local/bin/iperf" \
        -nobighammer 1 -tcpwindow 3640K -custom 1 \
		-pre_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \
		-post_perf_hook {{%S wl counters} {%S wl nrate} {%S wl rate}} \

MacAirX-B configure -attngrp L4

######################## Linux DHD #########################

UTF::DHD brix-lx1 -lan_ip 10.19.61.196 \
    -sta {iosreflx1 eth0} \
    -power {npc8 4}  \
    -extsup 1 \
    -app_tag BIS120RC4_BRANCH_7_15 \
    -tag BIS120RC4_BRANCH_7_15 \
    -dhd_tag DHD_BRANCH_1_359 \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94350RieslingBMurataMT.txt" \
    -clm_blob 4350_riesling_b.clm_blob \
    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv.bin \
    -customer olympic \
    -wlinitcmds {wl msglevel 0x101; wl ampdu_mpdu 24;wl ampdu_release 12;wl vht_features 3} \
    -slowassoc 5 -docpu 1 -escan 1 \
    -datarate {-i 0.5 -frameburst 1} \
    -tcpwindow 2m -udp 800m -nocal 1 \
	

iosreflx1 configure -attngrp G3

iosreflx1 clone ios_stowe \
    -name "STOWE" \
    -type 4350c5-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap-swdiv.bin \
    -app_tag BIS120RC4_REL_7_15_153_71 \
    -tag  BIS120RC4_REL_7_15_153_71

#################################################
# Olympic Latency

UTF::DHD md15-4357DUTA \
	-lan_ip 10.19.61.196 \
	-sta {4357A eth0} \
    -name "4357A-DUT" \
    -nvram_add {macaddr=00:90:4C:12:D0:04} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl country US/0;
	wl band a;
	wl vht_features 7;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a}

4357A configure -attngrp L2

4357A clone 4357At \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

4357At clone 4357Ait -tag IGUANA_BRANCH_13_10

4357Ait clone 4357Aif \
 -dhd_image "/projects/hnd_sw_mobhost/work/frankyl/trunk/src/dhd/linux/dhd-msgbuf-pciefd-mfp-debug-3.11.1-200.fc19.x86_64/dhd.ko" -iperf  /projects/hnd_sw_mobhost/work/frankyl/iperf2/src/iperf

UTF::DHD md15-4357DUTB \
	-lan_ip 10.19.61.190 \
	-sta {4357B eth0} \
    -name "4357B-DUT" \
    -nvram_add {macaddr=00:90:4C:12:D0:05} \
	-app_tag trunk \
	-datarate {-i 0.5 -auto -sgi -frameburst 1} \
    -dhd_brand linux-internal-dongle-pcie \
    -dhd_tag DHD_BRANCH_1_579 \
    -driver dhd-msgbuf-pciefd-debug \
    -nvram "bcm94357fcpagbe.txt" \
    -clm_blob 4347a0.clm_blob \
    -type 4357a0-ram/config_pcie_debug/rtecdc.bin \
    -wlinitcmds {
	dhd -i eth0 msglevel -msgtrace;
	wl down;
	wl country US/0;
	wl band a;
	wl vht_features 7;
	wl -w 1 interface_create sta;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 band b;
    } \
    -nocal 1 -udp 800m -tcpwindow 3m -slowassoc 5 \
    -yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
    -perfchans {36/80} -channelsweep {-band a}

4357B configure -attngrp L3

4357B clone 4357Bt \
    -type 4357a0-ram/config_pcie_tput/rtecdc.bin \
    -perfonly 0 -noaes 1 -notkip 1 -clm_blob {}

4357Bt clone 4357Bit -tag IGUANA_BRANCH_13_10

4357Bit clone 4357Bif \
 -dhd_image "/projects/hnd_sw_mobhost/work/frankyl/trunk/src/dhd/linux/dhd-msgbuf-pciefd-mfp-debug-3.11.1-200.fc19.x86_64/dhd.ko" -iperf  /projects/hnd_sw_mobhost/work/frankyl/iperf2/src/iperf

#################################################

######################## Stressor ###########################

UTF::MacOS md15tst5 -sta {DOME_STRESS en0} \
    -wlinitcmds { Sleep 5; wl msglevel 0x101; wl down; wl vht_features 3; wl up ; wl assert_type 1; apple80211 -debug=err,rsn,scan;} \
        -brand  "macos-external-wl-syr" \
        -name "DOME_STRESS" \
        -type Release_10_10 \
        -lan_ip 10.19.61.195 \
        -kextload false \
        -slowassoc 5 \
        -channelsweep {-count 15} \
        -datarate {-b 1.2G -i 0.5 -frameburst 1} \
        -nativetools 0 \
        -console "/var/log/system.log" \
        -nobighammer 1 -tcpwindow 3640K -custom 1


DOME_STRESS configure -tag "BIS120RC4_REL_7_15_166_24" -custom 1
DOME_STRESS configure -ipaddr 192.168.1.204
DOME_STRESS configure -attngrp L6


set ::UTFD::physicalmacstas {}
set ::UTFD::physicalmacs {}
set ::UTFD::physicalrouterstas {}
foreach STA [UTF::STA info instances] {
    if {[$STA hostis Router]} {
	lappend ::UTFD::physicalrouterstas $STA
    }
}
foreach DUT [UTF::MacOS info instances] {
    if {$DUT eq "::AppleCore"} {
	continue
    } else {
	lappend ::UTFD::physicalmacs $DUT
    }
}
foreach STA [UTF::STA info instances] {
    if {![$STA hostis MacOS]} {
	continue
    } else {
	lappend ::UTFD::physicalmacstas $STA
    }
}

set UTF::SetupTestBed {
    L1 attn $::attnmax
    L2 attn $::attnmax
    L3 attn $::attnmax
    #L3 attn 0
    L4 attn $::attnmax
    L5 attn $::attnmax
    L6 attn $::attnmax
    L7 attn $::attnmax
    #set ::UTFD::physicalmacstas [list MacAirX-A MacAirX-B DOME_STRESS MacX-A MacX-B]
    set ::UTFD::physicalmacstas [list MacAirX-A DOME_STRESS MacX-A MacX-B]
    foreach STA $::UTFD::physicalmacstas {
	set tag INFO
	if {[catch {$STA power on} err]} {
	   UTF::Message ERROR $STA "AC power ON failed: $err"
	}
	catch {$STA killall dns-sd}
	catch {$STA wl awdl 0}
	UTF::Sleep 1
	$STA rexec networksetup -listpreferredwirelessnetworks [$STA cget -device]
	$STA rexec networksetup -removeallpreferredwirelessnetworks [$STA cget -device]
	$STA rexec networksetup -setairportpower [$STA cget -device] off
#	UTF::Sleep 1
#	$STA rexec applebt --setPowerState 0
#	$STA rexec networksetup -setairportpower [$STA cget -device] on
#	UTF::Message $tag "" "AC power on and Radio on for $STA
}
#    foreach S [concat $::UTFD::physicalmacstas $::UTFD::physicalrouterstas] {
#	if {[$STA hostis MacOS]} {
#	$STA rexec networksetup -setairportpower [$STA cget -device] off
#	} else {
#	catch {$S wl down}
#	$S deinit
#	}
 #   }

    set sniffers [UTF::Sniffer info instances]
    foreach sniffer $sniffers {
	catch ($sniffer unload)
    }
    return
}

##################### Mac Custom tools configurations #######################################

proc ::update_mac_tools {{devices ""}} {
    set MACIPERF "/projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf/iperf2-code/src/iperf"
    # set MACIPERF "/projects/hnd_sig_ext16/rmcmahon/Code/iperf/sourceforge/counters/iperf2-code/src/iperf"
    if {$devices eq {}} {
	set devices $::UTFD::physicalmacstas
    }
    foreach DUT $devices {
	# catch {eval [concat $DUT copyto $MACIPERF /usr/bin/iperf]}
	# catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/iperf/iperf_awdl /usr/bin/iperf_awdl]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/ptpd-2.3.1-rc3/src/ptpd2 /usr/bin/ptpd2]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/bt_tools/applebt /usr/local/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/bt_tools/applebt /usr/bin/applebt]}
	catch {eval [concat $DUT copyto /projects/hnd_sig_ext16/rmcmahon/Code/KPI/mac/wget-1.16/src/wget /usr/bin/wget]}
	$DUT deinit
    }
}

## BT objects ##

##################### PTP Configurations #######################################

#array set ::ptpinterfaces [list ApHost enp3s0f0 MacX-A en5 MacX-B en3 MacAirX-A en4 MacAirX-B en3 DOME_STRESS en4 ApHostB eth0]
array set ::ptpinterfaces [list MacX-A en5 MacX-B en5 MacAirX-A en4 MacAirX-B en2 DOME_STRESS en4 ApHostB eth0]
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
	    # catch {$dut ip route replace multicast 224.0.0.107/32 dev $interface}
	    # catch {$dut ip route replace multicast 224.0.1.129/32 dev $interface}
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
    UTF::Sleep 120
    foreach dut $devices {
	catch {$dut rexec tail -10 /var/log/ptpstats.log}
	$dut deinit
    }
}

# Support procedures required by KPISuite

proc ::isalive {{devices ""}} {
    if {$devices eq {}} {
	set devices $::UTFD::physicalmacs
    }
    foreach DUT $devices {
	if {[catch {$DUT os_version} results]} {
	    UTF::Message ERROR $DUT "[$DUT cget -lan_ip] not responding"
	} else {
	    UTF::Message INFO $DUT "[$DUT cget -lan_ip] seems OK, OS Ver: $results"
	}
	$DUT deinit
    }
}

proc ::iperfver {{devices ""}} {
    if {$devices eq {}} {
	set devices $::UTFD::physicalmacstas
    }
   foreach STA $devices {
	catch {$STA rexec iperf -v} results
	UTF::Message INFO $STA "Iperf ver: $results"
	$STA deinit
    }
}

#  Source the logical configuration

source "utfconf/md15logical.tcl"


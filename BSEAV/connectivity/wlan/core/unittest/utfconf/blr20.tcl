#Testbed configuration file for BLR20
#Edited by Rohit B : Date 23 Feb 2016
#Last checkin 08:08pm


######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix


# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr20"

UTF::Aeroflex af -lan_ip 172.1.1.100 \
	-relay "blr20end1" \
	-group {
		G1 {1 2 3}
		G2 {4 5 6}
		ALL {1 2 3}
		}
G1 configure -default 20
G2 configure -default 20

# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    # Make Sure Attenuators are set to 0 value
	foreach S {4360a 4360b 4355c0} {
	catch {$S wl down}
	$S deinit
    }
    G1 attn default
    G2 attn default
    return
}

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.21 -relay blr20end1 -rev 1

#pointing Apps to trunk
set ::UTF::TrunkApps 1 \

#FB
set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

UTF::Linux repo -lan_ip xl-sj1-24.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck


UTF::Linux blr20end1 \
     -lan_ip 10.132.116.137 \
     -sta {lan eth0} \

UTF::Linux blr20softap -sta {4360a enp1s0} \
    -lan_ip 10.132.116.138 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
    -brand "linux-internal-wl" \
    -wlinitcmds {wl msglevel +assoc; wl down;wl country US/0; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3;}

4360a configure -ipaddr 192.168.1.90 -attngrp G1 -ap 1 -hasdhcpd 1 \

	
UTF::Linux blr20ref1 -sta {4360b enp1s0} \
    -lan_ip 10.132.116.139 \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -wlinitcmds {wl msglevel +assoc; wl down; wl country US/0; wl dtim 3; wl bw_cap 2g -1; wl vht_features 3; }				

4360b configure -ipaddr 192.168.1.91 -attngrp G2 -ap 1 -hasdhcpd 1 \


UTF::DHD blr20tst1 \
     -lan_ip 10.132.116.140 \
     -sta {4355c0 eth0} \
     -power {npc21 2} \
     -power_button "auto" \
     -dhd_brand linux-internal-dongle-pcie \
     -brand linux-external-dongle-pcie \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DIN2930R18_BRANCH_9_44 \
     -dhd_tag DHD_BRANCH_1_579 \
     -type 4355c0-roml/config_pcie_release/rtecdc.bin \
     -nvram "src/shared/nvram/bcm94355StabelloMurataKK.txt" \
     -nocal 1 -slowassoc 5 -reloadoncrash 1 \
     -clm_blob "4355_kristoff.clm_blob" \
     -udp 800m  \
     -tcpwindow 8m \
     -wlinitcmds {wl down;wl country US; wl mpc 0; wl ampdu_mpdu 48; wl amsdu_aggsf; wl nmode; wl vhtmode; wl frameburst; wl vht_features 3; wl ampdu_mpdu;} \
     -pre_perf_hook {{%S wl rssi 00:10:18:E2:F6:E2}  {%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters} {%S wl rssi} {%S wl nrate} {%S wl chanspec} {%S wl dump rsdb}} \
     -post_perf_hook {{%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl dump phycal} {%S wl dump rssi} {%S wl chanspec} {%S wl dump rsdb}} \
     -perfchans {36/80 36l 3} \
     -yart {-attn5g 30-95 -attn2g 30-95} \

4355c0 configure -ipaddr 192.168.1.100 \

4355c0 clone 4355c0t \
      -type 4355c0-roml/config_pcie_perf/rtecdc.bin \

4355c0 clone 4355c0d \
      -type 4355c0-roml/config_pcie_debug/rtecdc.bin \
    
4355c0 clone 4355C0_9_30 \
	-tag DIN2915T250RC1_BRANCH_9_30 \
	-type 4355c0-roml/config_pcie_release/rtecdc.bin \

UTF::Q blr20


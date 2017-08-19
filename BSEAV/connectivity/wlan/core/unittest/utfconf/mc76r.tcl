# -*-tcl-*-

#
#  $Copyright Broadcom Corporation$
#  $Id: 0ae60023daabf34dbf85e5d8553baf2c034f1832 $
#
#  Testbed configuration file for MC76 Testbed
#

source "utfconf/mc76.tcl"

set ::UTF::SummaryDir "/projects/hnd_sig/utf/mc76r"

# Packages
package require UTF::Aeroflex
UTF::Aeroflex Afr -lan_ip 10.19.86.252 -relay mc76end1 \
    -group {G1 {1 2 3 4}}

G1 configure -default 0

set UTF::SetupTestBed {
    G1 attn default
    return
}


# Endpoints
UTF::Linux mc76end2 -sta {lan2 enp1s0}
UTF::Linux mc76end3 -sta {lan3 p1p1}

lan configure -ipaddr 192.168.0.50
lan2 configure -ipaddr 192.168.0.60
lan3 configure -ipaddr 192.168.1.61

# Power switches/web relays
UTF::Power::Synaccess npc45 -lan_ip 172.16.1.45 -relay mc76end1 -rev 1
UTF::Power::Synaccess npc55 -lan_ip 172.16.1.55 -relay mc76end1 -rev 1


set stacommon {
    -tag "EAGLE_BRANCH_10_10"
    -wlinitcmds {
	wl ver;
	wl msglevel;
	wl msglevel +assoc;
	wl msglevel;
	wl down;
	wl bw_cap 2g
	wl bw_cap 2g -1;
	wl bw_cap 2g;
	wl vht_features;
	wl vht_features 7;
	wl vht_features;
    }
    -tcpwindow 4M -slowassoc 5 -reloadoncrash 1
}

# Fc19 STAs
UTF::Linux mc76tst1 -sta {sta1 enp1s0} \
    -console {mc76end1:40004} \
    -power {npc55 1} \
    {*}$stacommon

# Use static IP as CM's lease time is too short
sta1 configure -ipaddr 192.168.0.99

UTF::Linux mc76tst2 -sta {sta2 enp1s0} \
    -console {mc76end1:40005} \
    -power {npc55 2} \
    {*}$stacommon

############################

# No need for 2 lan since the 3384 ethernet bridge can't handle more
# than 1g

package require UTF::CableModem

UTF::CableModem 3384 -sta {dummy dummy} \
    -lan_ip 192.168.0.1 \
    -relay lan -lanpeer lan \
    -power {npc45 1} \
    -console "mc76end2:40003" \
    -console2 "mc76end2:40002" \
    -nvram {
	wl0_mode=ap
	wl0_chanspec=3
	wl0_radio=0
	wl0_ssid=3384/4360
	wl0_vht_features=3
	wl0_dtim=3
	wl1_mode=ap
	wl1_chanspec=36
	wl1_radio=0
	wl1_ssid=3384/4366
	wl1_vht_features=6
	wl1_dtim=3
	#wl0_txbf_bfe_cap=0
	#wl1_txbf_bfe_cap=0
	#wl0_txbf_bfr_cap=0
	#wl1_txbf_bfr_cap=0
    } \
    -nomaxmem 1 -nocustom 1 -nombss 1 \
    -datarate {-i 0.5} -udp 1.8g \
    -yart {-attn5g 16-63 -attn2g 36-63 -pad 26} -use11h 0

dummy configure -attngrp G1

dummy clone 3384/4366 -sta {3384/4366 wl1 3384/4366.%7 wl1.%} \
    -perfchans {36/80} -nobighammer 1
dummy clone 3384/4360 -sta {3384/4360 wl0 3384/4360.%7 wl0.%} \
    -perfchans {3}

set UTF::RouterNightlyCustom {

    package require UTF::Test::MiniUplinks
    UTF::Test::MiniUplinks $Router $STA1 -sta $STA2
}

UTF::Q mc76r mc76end1

#Testbed configuration file for blr14end1 UTF StaNighly Teststation
#Created by Anuj Gupta on 02 June 2015 10:00 PM  
#
####### Controller section:
# blr14end1: FC19
# IP ADDR 10.132.116.134
# NETMASK 255.255.254.0 
# GATEWAY 10.132.116.1
#
####### SOFTAP section:
#
# blr05ref0 : FC 15 4360mc_1(99)  10.132.116.135 
# blr05ref1 : FC 15 4360mc_1(99)  10.132.116.136
#
####### STA section:
#
# blr14tst1: FC 19       eth0 (10.132.116.137)
#
######################################################### #
# Load Packages

set ::env(UTFDPORT) 9977

package require UTF::Linux
package require UTF::Airport
package require UTF::Vaunix
package require UTF::Power
package require UTF::AeroflexDirect
package require UTF::WebRelay
package require UTFD
package require UTF::Streams
package require UTF::Sniffer

################### Aeroflex attenuator ###################
UTF::AeroflexDirect af -lan_ip 172.1.1.100 -cmdresptimeout 500 -retries 0 -concurrent 0 -silent 0 \
        -relay "blr14end1" \
        -group {
                G1 {1 2 3 4}
                G2 {5 6 7 8}
		ALL {1 2 3 4 5 6 7 8}
                }


ALL configure -default 0

#G2 configure default 0
# Default TestBed Configuration Options
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
	foreach S {4360a 4360b 4364b0} {
	catch {$S wl down}
	$S deinit
    }
 ALL attn default
    return
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr14"



#pointing Apps to trunk

set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

set UTF::PerfCacheMigrate {
    4364a0 4364b0
}

####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc11 -lan_ip 172.1.1.11 -relay blr14end1 -rev 1
UTF::Power::Synaccess npc12 -lan_ip 172.1.1.21 -relay blr14end1 -rev 1
UTF::Power::Synaccess npc31 -lan_ip 172.1.1.31 -relay blr14end1 -rev 1
UTF::Power::Synaccess npc41 -lan_ip 172.1.1.41 -relay blr14end1 -rev 1

########################### Test Manager ################

UTF::Linux blr14end1 \
     -lan_ip 10.132.116.102 \
     -sta {lan eth0}

############################ blr14ref0 ##########################################
# blr14ref0      - 4360mc_1(99)
# hostplatform   - Interl DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 1 - STE 4450
# Power          - npc 31 port 1    172.1.1.31
################################################################################ 

UTF::Linux blr14ref0 -sta {4360a enp1s0} \
    -lan_ip 10.132.116.103 \
    -power "npc11 1" \
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 4M \
    -udp 1600m \
    -wlinitcmds {
                 wl down; wl country US/0; wl vht_features 3; wl bw_cap 2g -1;
                }

4360a configure -ipaddr 192.168.2.90 -attngrp G1 -ap 1 -hasdhcpd 1 \



####################### blr14ref1 Acting as ACI#########################
# blr14ref1      - 4360mc_1(99)
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Miniflex
# RF Enclosure 2 - STE 4450
# Power          - npc41 port 1    172.1.1.41
################################################################################


UTF::Linux blr14ref1 -sta {4360b enp1s0} \
    -lan_ip 10.132.116.104 \
    -power "npc21 1"\
    -power_button "auto" \
    -slowassoc 5 -reloadoncrash 1 \
    -tag BISON_BRANCH_7_10 \
    -brand "linux-internal-wl" \
    -preinstall_hook {{%S dmesg -n 7}} \
    -post_perf_hook {{%S wl rate} {%S wl dump ampdu} {%S wl dump amsdu}} \
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -tcpwindow 8M \
	-udp 1600m \
	-wlinitcmds { wl country US/0; wl down; wl vht_features 3;wl bw_cap 2g -1;} \

4360b configure -ipaddr 192.168.1.90 -ap 1  -attngrp G2 -hasdhcpd 1 \

4360b clone 4360b_1x1 \
	-wlinitcmds {wl country US/0; wl down; wl vht_features 3; wl txchain 4; wl rxchain 4}


################################ blr14tst4 ######################################
# blr14tst4      - 4364
# Hostplatform   - Intel DH77EB + Intel i5-3450 + 8GB DDR3
# Linux ver      - Fedora Core 15 Livelock i686
# Adapter        - Adex PCIe Half Miniflex
# RF Enclosure 3 - Ramsey  STE 5000
# Power          - npc11 port 2     172.1.1.11
################################################################################


UTF::DHD blr14tst1 \
     -lan_ip 10.132.116.105 \
     -sta {4364 eth0 } \
	 -power "npc41 2"\
    -power_button "auto" \
     -dhd_brand linux-internal-dongle-pcie \
	 -app_tag DHD_BRANCH_1_359 \
     -driver dhd-msgbuf-pciefd-debug \
     -tag DINGO2_BRANCH_9_15 \
     -nocal 1 -slowassoc 5 \
     -nvram "bcm94364fcpagb_2.txt"\
	 -brand hndrte-dongle-wl \
     -type 4364b0-roml/threadx-pcie-ag-msgbuf-splitrx-splitbuf-pktctx-proptxstatus-p2p-pno-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-logtrace-assert/rtecdc.bin \
     -udp 1600m  \
     -tcpwindow 8m \
     -wlinitcmds {wl down; wl vht_features 3; wl country US;} \
	 -pre_perf_hook {{%S wl nrate} {%S wl dump rsdb} {%S wl mpc} {%S wl curpower}} \
	 -post_perf_hook {{%S wl nrate } {%S wl dump rsdb}} \
	 -channelsweep {-usecsa} \
	 -perfchans {36/80 36l 36 3} \

4364 clone 4364a0 \
	-type 4364a0-roml/config_pcie_debug/rtecdc.bin \
	-wlinitcmds {wl down; wl vht_features 3; wl country US;wl bw_cap 2g -1} \


		
4364a0 clone 4364a0.1 -sta {4364a0.1 eth0 _4364a0.2 wl0.2} \
    -wlinitcmds {wl down; wl vht_features 3;wl bss -C 2 sta} \
    -perfchans {36/80 36l 36 3}  -nocustom 1 \

_4364a0.2 clone 4364a0.2 -sta {_4364a0.1 eth0 4364a0.2 wl0.2} \
    -nocustom 1 \
	-perfchans {36/80 36l 36 3} \

4364 clone 4364b0 \
	-type 4364b0-roml/config_pcie_debug/rtecdc.bin \
	-clm_blob 4364b0.clm_blob  \
	-wlinitcmds {wl down; wl vht_features 3; wl country US;wl bw_cap 2g -1} \


		
4364b0 clone 4364b0.1 -sta {4364b0.1 eth0 _4364b0.2 wl0.2} \
    -wlinitcmds {wl down; wl vht_features 3;wl bss -C 2 sta} \
    -perfchans {36/80 36l 36 3}  -nocustom 1 \

_4364b0.2 clone 4364b0.2 -sta {_4364b0.1 eth0 4364b0.2 wl0.2} \
    -nocustom 1 \
	-perfchans {36/80 36l 36 3} \
	
4364b0.2 configure -dualband {4360a 4364b0.1 -c1 36/80 -c2 3 -b1 800m -b2 800m } \

4364b0.1 configure -dualband {4360b 4364b0.2 -c1 36/80 -c2 3 -b1 800m -b2 800m } 



set UTF::StaNightlyCustom {
    if {$(ap2) ne ""} {
	
	# package require UTF::Test::MultiSTANightly
	# MultiSTANightly -ap1 $Router -ap2 $(ap2) -sta $STA \
	    # -nosetuptestbed -nostaload -nostareload -nosetup \
	    # -noapload -norestore -nounload
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA
    }
}


UTF::Q blr14

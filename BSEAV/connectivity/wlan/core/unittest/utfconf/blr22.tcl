#Testbed configuration file for BLR22 StaNightly station
#Edited Battu Kaushik 20/09/2016
#Last checkin 
####### Controller section:
# blr22end1: FC19
# IP ADDR 10.131.80.128
# NETMASK 255.255.254.0 
# GATEWAY 10.131.80.1
#
####### SOFTAP and STA section:
#
# blr22ref0 : FC 19 4366  10.131.80.131
# blr22tst1 : Android 4361B0  10.131.80.133
#
######################################################### #
# Load Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::Sniffer
package require UTF::Power
package require UTF::Airport
package require UTF::Vaunix
package require UTF::Android
package require UTF::WinBT

set ::bt_ref BTRefXP

################### Aeroflex attenuator ###################
UTF::Aeroflex af -lan_ip 172.1.1.100 \
        -relay "blr22end1" \
        -group {
                G1 {1 2 3}
                G2 {4 5 6}
		G3 {7 8 9}
		ALL {1 2 3 4 5 6 7 8 9}
                }
#G1 configure default 0
#G2 configure default 0
# Default TestBed Configuration Options
UTF::Linux repo -lan_ip xlinux.sj.broadcom.com -user $::tcl_platform(user)
set UTF::BuildFileServer repo
set UTF::UseFCP nocheck
set ::UTF::SetupTestBed {
    #
    # Make Sure Attenuators are set to 0 value
    #
    G1 attn 10 
    catch {G2 attn 15;}
    catch {G1 attn 15;}
	foreach S {4366 4361B0-android 4361B0-android2 } {
	catch {$S wl down}
	$S deinit
    }
    return
}
# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext18/$::env(LOGNAME)/blr22"

#set ::UTF::TrunkApps 1 \

set ::UTF::FBDefault 1 \

set ::UTF::ChannelPerf 1 \

# Turn off most RvR intialization
set ::rvr_sta_init {{%S wl down} {%S wl mimo_bw_cap 1} {%S wl up}}
set ::rvr_ap_init ""

####################### Power Controllers ################

package require UTF::Power
UTF::Power::Synaccess npc21 -lan_ip 172.1.1.11 -relay blr22end1 -rev 1

########################### Test Manager ################

UTF::Linux blr22end1 \
     -lan_ip 10.132.116.144 \
     -sta {lan em1}

UTF::Linux blr22ref0 -sta {4366 enp1s0} \
    -lan_ip 10.132.116.145 \
    -power_button "auto" \
    -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
	-brand "linux-internal-wl" \
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; \
            wl down; wl vht_features 7 
	}

4366 configure -ipaddr 192.168.1.100 -ap 1  -attngrp G1 -hasdhcpd 1 \


UTF::Linux blr22ref1 -sta {4366ref1 enp1s0} \
    -lan_ip 10.132.116.146 \
    -power_button "auto" \
    -reloadoncrash 1 \
    -tag EAGLE_BRANCH_10_10 \
        -brand "linux-internal-wl" \
        -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl amsdu_clear_counters}} \
    -post_perf_hook {{%S wl rate} {%S wl nrate} {%S wl dump rssi}} \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; \

        }

4366ref1 configure -ipaddr 192.168.2.111 -ap 1  -attngrp G1 -hasdhcpd 1 \



UTF::Android blr22tst1 \
	-relay blr22end1 \
	-adbdevice "10.132.116.39" \
        -lan_ip "-s 10.132.116.39:5555 shell" \
        -sta {4361B0-android wlan0} \
        -extsup 1 \
        -tag IGUANA08T_BRANCH_13_35 \
	-brand hndrte-dongle-wl \
        -dhd_brand android-external-dongle \
        -driver dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug-3.10.20-vendorext-gcc264a6-androidx86 \
	-dhd_tag DHD_BRANCH_1_579 \
	-app_tag WLAPP_REL_1_28 \
        -supp_tag HOSTAPRC128_BRANCH_1_10 \
        -halutil_tag trunk \
	-clm_blob ss_mimo.clm_blob \
        -nvram src/shared/nvram/bcm94361fcpagbi_B0_p301.txt \
        -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
        -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	wl apsta 1
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
        } \
	-nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
	-yart {-attn5g 13-95+1 -attn2g 3-95+1 -pad 23} \
	-perfchans {36/80  3} -channelsweep {-band a } \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}} \

4361B0-android clone 4361B0a.2 \
	-type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
	-wlinitcmds {
	wl down;
	wl vht_features 7;
	wl interface_create sta -c 1;
	sleep 1;
	wl -i wl1.2 vht_features 3;
	wl -i wl1.2 up
	} \
	-clm_blob ss_mimo.clm_blob \
	-noaes 1 -notkip 1 -nocustom 1 \
	-perfchans {36/80 3} \

4361B0-android configure -dualband {4366ref1 4361B0-android -c1 36/80 -c2 3l -b1 1600m -b2 1600m -rate1 11x2s -rate2 9x2s } \

4361B0-android clone 4361B0J \
    -type 4361a0-ram/config_pcie_release/rtecdc.bin \
    -tag JAGUAR_BRANCH_14_10 \
     -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
        } \
    -clm_blob ss_mimo.clm_blob \
    -noaes 1 -notkip 1 -nocustom 1 \
    -perfchans {36/80 3} \

4361B0-android clone 4361B0d \
     -type 4361b0-roml/config_pcie_utf_ipa/rtecdc.bin \
     -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
        } \
    -clm_blob ss_mimo.clm_blob \
    -noaes 1 -notkip 1 -nocustom 1 \
    -perfchans {36/80 3} \

UTF::Android blr22tst2 \
	-relay blr22end1 \
	-adbdevice 10.132.116.148 \
	-lan_ip "-s 10.132.116.148:5555 shell" \
        -sta {4361B0-android2 wlan0} \
        -extsup 1 \
	-tag IGUANA08T_BRANCH_13_35 \
        -supp_tag HOSTAPRC128_BRANCH_1_10 \
	-brand hndrte-dongle-wl \
        -dhd_brand android-external-dongle \
	-driver dhd-msgbuf-pciefd-android-cfg80211-wapi-ccx-fbt-customer4-debug-3.10.20-vendorext-gcc264a6-androidx86 \
        -dhd_tag DHD_REL_1_579_203 \
	-app_tag WLAPP_REL_1_25 \
	-clm_blob ss_mimo.clm_blob \
        -nvram src/shared/nvram/bcm94361fcpagbi_B0_p301.txt \
        -type 4361b0-roml/config_pcie_release_ipa/rtecdc.bin \
        -wlinitcmds {
	dhd -i wlan0 msglevel -msgtrace;
	wl down;
	dhd -i wlan0 wdtick 10 ;
	dhd -i wlan0 dconpoll 10 ;
	wl vht_features 7
        } \
	-nocal 1 -datarate {-i 0.5 -auto -sgi} -udp 1.5G -tcpwindow 4m -slowassoc 5 \
	-yart {-attn5g 13-103+3 -attn2g 3-103+3 -pad 23} \
	-perfchans {36/80} -channelsweep {-band a } \
	-pre_perf_hook {{%S wl reset_cnts} {%S wl dump_clear ampdu}} \
	-post_perf_hook {{%S wl dump ampdu} {%S wl nrate} {%S wl counters}} \

UTF::WinBT BTWIN7 \
    	-lan_ip 10.132.29.227 \
        -sta "BT" \
        -type "BCM2046" \
        -bt_comm "usb0" \
        -user user

set UTF::StaNightlyCustom {
package require UTF::Test::MiniP2P 
package require UTF::Test::BeaconRatio2
package require UTF::Test::TDLS
    if {$(ap2) ne ""} {
	4361B0-android2 reload
	4361B0-android2 supplicant start
### MiniP2P tests between two android devices - *Security by default will be in AESPSK2* ####
	$(ap2) wl band auto 
	   MiniP2P 4361B0-android2 $STA -ap $Router  -apchanspec 3 -security open
#	   MiniP2P 4361B0-android2 $STA -ap $Router  -chanspec 149/80 -apchanspec 36/80 -security open
	   

### TDLS between two Android devices ########	
	if {1 || [regexp {_tput|_release} [$STA cget -type]]} {
	    # Move ap2 to the same subnet and turn off dhcp
	    set ap2 4361B0-android2
	    set ip [$ap2 cget -ipaddr]
	    $ap2 configure -ipaddr 192.168.1.81 -hasdhcpd 0
	    try {
		$(ap2) wl band auto
		TDLS $Router $STA $ap2 -chanspec 36/80 -security open
	    } finally {
		$ap2 configure -ipaddr $ip -hasdhcpd 1
		4361B0-android2 supplicant stop
	    }
	}

#	MiniP2P 4361B0-android2 $STA -ap $Router  -apchanspec 3 -security open

### STA+AP tests######
	package require UTF::Test::APSTA
	APSTA $Router $(ap2) $STA  -chan1 6 -chan2 36/80 -nosta
    }
}

UTF::Q blr22

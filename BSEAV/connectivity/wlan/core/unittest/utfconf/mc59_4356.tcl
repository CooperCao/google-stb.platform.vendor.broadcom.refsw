# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC59
# CoEx testbed configuration
#
# DUT: 4356 ;# replaces 4335c0/4339a0 and 4350c0

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut mc59_BTCohost2		      	;# BlueTooth device under test
set ::bt_ref mc59_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4356			 		    ;# HND WLAN device under test
set ::wlan_rtr mc59_AP1   				;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator

# loaned ip addresses to MC54:
# 10.19.87.102 -- bcm94334 host
# 10.19.87.101 -- BTCohost
# 10.19.87.124 -- WebRelay DUT2

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc59"

# Define power controllers on cart.
# package require UTF::Power
UTF::Power::Synaccess npc_DUT -lan_ip 172.16.59.20
UTF::Power::Synaccess npc_DUT2 -lan_ip 172.16.59.25
UTF::Power::Synaccess npc_Ref -lan_ip 172.16.59.30
UTF::Power::Synaccess npc_AP -lan_ip 172.16.59.40
UTF::Power::Synaccess npc_AP2 -lan_ip 172.16.59.60 -rev 1
# UTF::Power::Synaccess npc_AP2 -lan_ip 10.19.87.120 -rev 1; moved to CoExTB9
UTF::Power::WebRelay 10.19.87.104 ;# WebRelay DUT
UTF::Power::WebRelay 10.19.87.103
UTF::Power::WebRelay 10.19.87.124 ;# WebRelay DUT2

# Attenuator
# G1: 11ac WLAN
# G2: BT
# G3: unused
UTF::Aeroflex af -lan_ip 172.16.59.10 -group {G1 {1 2 3} G2 {6} G3 {4 5} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	G1 attn 10 
    G2 attn 0 ;# changed from 0 to 20 7/18/12 ;# changed to G2 = 10 as of 10/12/2011 ;# value used again on 7/13/12 for RF experiment
	G3 attn 103 ;# changed to 103 from 10 12/6/13 ;# changed from 103 to 20 as of 5/17/13

	# shut down AP radios
   foreach A {mc59_AP1} {
	  catch {$A restart wl0_radio=0}
	  $A deinit
    }
    
    # silence DUT radios
    foreach S {43342h 4356} {
		if {[catch {$S wl down} ret]} {
	        error "SetupTestBed: $S wl down failed with $ret"
	    }
	    $S deinit
	}
    
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc59end1  10.19.87.95   Linux UTF Host

UTF::Linux mc59end1 -sta "lan0 eth1" -lan_ip 10.19.87.95

### lan0 configure -ipaddr 192.168.1.144
# -lan_ip 10.19.87.95
#

# mc59DUT2: 4356 PCIe, mc60end5
#### hostname mc60end5, lan_ip 10.19.87.109; changed to mc59end5 ip 10.19.85.180 8/7/14

UTF::DHD mc59DUT2\
	-lan_ip mc59end5\
 	-sta "4356 eth0"\
    -tag BISON05T_BRANCH_7_35 \
    -customer "bcm"\
    -dhd_tag trunk -app_tag trunk\
	-dhd_brand linux-internal-dongle-pcie \
	-driver dhd-msgbuf-pciefd-debug -slowassoc 5 \
    -brand linux-external-dongle-pcie \
    -nvram "src/shared/nvram/bcm94356wlpagbl.txt" \
    -type 4356a*-roml/pcie-ag-p2p-pno-aoe-pktfilter-keepalive-sr-mchan-pktctx-proptxstatus-ampduhostreorder-lpc-pwropt-txbf-wl11u-mfp-tdls-amsdutx-ltecx-wfds-okc-ccx-ve-idsup-idauth.bin\
    -nocal 1 -noaes 1 -notkip 1 -nomaxmem 1\
    -tcpwindow 2m -udp 1.2g\
	-console 10.19.85.180:40001\
	-power "npc_DUT2 2"\
    -wlinitcmds {wl ampdu_mpdu 24; wl ampdu_release 12; wl vht_features 3; wl msglevel 0x101; wl msglevel +assoc}\
    -pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl rate} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}

# # #  -attngrp B -hasdhcpd 1 -ap 1
    
4356 clone 4356_coex\
	-wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl btc_mode 1; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc}
# # # 	\
# # # 	-device_reset ""
    	
4356_coex clone 4356_coex5 -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 3; wl btc_mode 5; wl ampdu_release 12; wl msglevel 0x101; wl msglevel +assoc}
	
# hostname: mc59end3

UTF::WinBT MC59_BTCOHOSTXP2 \
	-lan_ip 10.19.87.97 \
	-sta "mc59_BTCohost2"\
	-power "npc_DUT2 1"\
	-device_reset "10.19.87.124 2"\
	-bt_comm "com8@3000000"\
	-bt_comm_startup_speed 115200\
    -user user
    
mc59_BTCohost2 clone BTCohost4356\
    -bt_ss_location 0x0021F000\
    -bt_xtal_freq 40\
    -bt_power_adjust 40\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
	-bt_comm "com17@3000000"\
	-type BCM4354/A2\
	-brand Generic/UART/37_4MHz/wlbga_BU_eLNA\
    -version *\
    -file *.cgs
	
BTCohost4356 clone BTCohost4356_local -project_dir "/home/pkwan/src/tools/unittest"

package require UTF::HSIC

# 43342A1: code name Chardonnay
# UART: on mc59end2 USB0
	
UTF::HSIC mc59HSIC\
	-sta {43342h eth1}\
	-lan_ip 192.168.1.1\
    -relay 43241a0sdio\
    -lanpeer 43241a0sdio\
    -power {npc_DUT 2}\
    -console "mc59end2:40010"\
    -tag "PHO2203RC1_REL_6_25_35"\
    -type "*/43342a*/chardonnay.trx"\
	-nvram "*/43342a*/chardonnay-t-kk.txt"\
    -brand "linux-external-dongle-usb"\
    -customer "olympic"\
    -host_tag "RTRFALCON_REL_5_130_56"\
    -host_brand linux26-internal-hsic\
    -host_nvram {
		lan_ipaddr=192.168.1.1
		lan_mask=255.255.255.0
		wandevs=dummy
		clkfreq=530,176,88
		watchdog=6000
		console_loglevel=7
		lan_stp=0
		lan1_stp=0
    }\
    -wlinitcmds {wl event_msgs 0x10000000000000}\
    -nocal 1 -datarate {-skiptx 32}

# removed all Innsbruck references 1/29/14        
# # # 	-nvram "Innsbruck/43342a0/chardonnay-u-kk.txt"\

43342h clone 43342A1
43342h clone 43342A1_ToB -tag "PHO2203RC1_BRANCH_6_25" 
43342A1_ToB clone 43342A1_ToB_coex -wlinitcmds {wl btc_mode 1}

43342h clone 43342A1_105 -tag "PHO2203RC1_REL_6_25_105_*" -type "*/43342a*/chardonnay.trx" -nvram "*/43342a*/chardonnay-t-kk.txt"
43342A1_105 clone 43342A1_105_coex -wlinitcmds {wl btc_mode 1}

# # # \ -channelsweep {-bw 20} -perfchans {64 3}
43342A1_ToB clone 43342A0 -nvram "Innsbruck/43342a0/chardonnay-m-mt.txt"

# # # 43342A1_ToB configure -ipaddr 192.168.1.145
43342A1_ToB configure -ipaddr 192.168.1.154

# BTCohost for 4324a0; hostname mc61end15
# temporarily using local cgs copy from IRV BroadcomInternal BCM4334 folder 1/13/12

UTF::WinBT MC59_BTCOHOSTXP \
    -lan_ip 10.19.85.153\
	-sta "mc59_BTCohost"\
	-device_reset "10.19.87.104 2"\
	-bt_comm "com6@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -type BCM4324\
    -brand ""\
    -bt_ss_location 0x00210000\
    -file *.cgs \
    -version *

# # #  as of 9/20/12 mc59_BTCohost is a WinXP desktop
# # #     -lan_ip 10.19.85.153 ;# replaced with 10.19.87.97 on 9/17/12 for debugging
    
mc59_BTCohost clone BTCohost43241\
	-type BCM43241/B0\
	-version *\
	-brand "Generic/37_4MHzClass1"\
	-file *.cgr
	
mc59_BTCohost clone BTCohost43241_local\
	-project_dir "/home/pkwan"\
	-type BCM43241\
	-brand ""\
	-version *\
	-file *.cg*
	
BTCohost43241 clone BTCohost43241_cfg53 -version BCM43241B0_002.001.013.0053.0000
BTCohost43241_local clone BTCohost43241_cfg55 -version BCM43241B0_002.001.013.0055.0000
BTCohost43241_local clone BTCohost43241_cfg32 -version BCM43241B0_002.001.013.0032.0000
BTCohost43241_local clone BTCohost43241_cfg31 -version BCM43241B0_002.001.013.0031.0000
BTCohost43241_local clone BTCohost43241_cfg30 -version BCM43241B0_002.001.013.0030.0000
BTCohost43241_local clone BTCohost43241_cfg29 -version BCM43241B0_002.001.013.0029.0000
BTCohost43241_local clone BTCohost43241_cfg24BigBuff -version BCM43241B0_002.001.013.0024.0000
BTCohost43241 clone BTCohost43241_cfg24 -version BCM43241B0_002.001.013.0024.0000
mc59_BTCohost clone BTCohost_pwr -power_sta "10.19.87.104 4"


BTCohost43241 clone BTCohost43342A1\
	-bt_comm "com8@3000000"\
    -bt_ss_location 0x00210000\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -bt_rw_mode "Cortex M3 HCI"\
    -bt_w_size 200\
	-type BCM43342/A1\
	-version BCM43342A1*Chardonnay_OS_USI*\
	-brand ""\
	-file *.cg*
	
BTCohost43342A1 clone BTCohost43342A1_local\
	-image "/home/pkwan/BCM43342/BCM43342A1_11.1.702.720_Chardonnay_OS_USI_20130830.cgs"

# BT Ref - WinXP laptop BlueTooth 2046 Reference
# mc59end3  10.19.87.98   mc59_BTRefXP
#	-power_sta "10.19.85.199 1"\

UTF::WinBT MC59_BTREFXP\
    -lan_ip 10.19.87.98\
    -sta "mc59_BTRefXP"\
    -power "172.16.59.30 2"\
    -power_sta "172.16.59.30 1"\
    -device_reset "10.19.87.103 1"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user

#     -power "npc_Ref 1"\
#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router MC59AP2 \
    -lan_ip 192.168.1.3 \
    -sta "mc59_AP2 eth1" \
    -power "npc_AP 1"\
    -relay "mc59end1" \
    -lanpeer lan0 \
    -console "mc59end1:40000" \
    -tag "AKASHI_BRANCH_5_110"\
	-brand "linux-internal-router"\
   -nvram {
	   	lan_ipaddr=192.168.1.3
       	fw_disable=1
       	wl_msglevel=0x101
		dhcp_start=192.168.1.144
	    dhcp_end=192.168.1.149
       	wl0_ssid=TestMC59
      	wl0_radio=0
		wl0_channel=1
		w10_nmode=1
		wl0_antdiv=0
		antswitch=0
		wl0_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }

# # # # clone AP1s
# # # mc59_AP1 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
# # # mc59_AP1 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# # # mc59_AP1 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
# # # mc59_AP1 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
# # # mc59_AP1 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
# # # mc59_AP1 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router-combo
# # # mc59_AP1 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
# # # mc59_AP1 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

# # #  

# 4706 AP with 4360 and 4331 cards

UTF::Router MC59AP1 -sta {
    4706/4360 eth1 4706/4360.%15 wl0.% 
    4706/4331 eth2
    } \
    -lan_ip 192.168.1.6 \
    -power "npc_AP2 1"\
    -relay "mc59end1" \
    -lanpeer lan0 \
    -console "mc59end1:40036" \
    -nvram {
	   	lan_ipaddr=192.168.1.6
		lan_gateway=192.168.1.6
		dhcp_start=192.168.1.150
	    dhcp_end=192.168.1.199
		lan1_ipaddr=192.168.2.6
		lan1_gateway=192.168.2.6
		dhcp1_start=192.168.2.150
	    dhcp1_end=192.168.2.199
       	fw_disable=1
       	wl_msglevel=0x101
        boardtype=0x05b2; # 4706nr
        console_loglevel=7
       	wl0_ssid=TestMC59_4360
		wl0_channel=1
		w10_bw_cap=-1
		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestMC59_4331
		wl1_channel=36
		wl1_bw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1

# # #     -datarate {-b 1.2G -i 0.5 -frameburst 1}\ ;# experiment with lower UDP offered rate 10/23/13
    
4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
# 4360_AARDVARK clone mc59_AP1
4360_AARDVARK clone mc59_AP1_BISON -tag "BISON_BRANCH_7_10" 
4360_AARDVARK clone mc59_AP1_twig -tag "AARDVARK_REL_6_30_???_*"
4360_AARDVARK clone mc59_AP1_trunk -tag "TRUNK"
mc59_AP1_BISON clone mc59_AP1


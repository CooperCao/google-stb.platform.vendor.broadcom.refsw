# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC59
# CoEx testbed configuration
#
# DUT: 4355

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
#    foreach A {mc59_AP1} {
# 	  catch {$A restart wl0_radio=0}
# 	  $A deinit
#     }
    
    # silence DUT radios
    foreach S {43342h 4355} {
		if {[catch {$S wl down} ret]} {
	        error "SetupTestBed: $S wl down failed with $ret"
	    }
	    $S deinit
	}
    
	#catch {mc59DUT rexec initctl stop consoleloggerUSB}
	
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
 	-sta "DUT eth0"\
	-console 10.19.85.180:40001\
	-device_reset "10.19.87.124 1"\
	-power "npc_DUT2 2"
 	
DUT clone 4355b3 \
	-tag DINGO2_REL_9_15_*\
	-customer olympic\
	-brand linux-external-dongle-pcie \
	-type ../C-4355__s-B3/simbaa.trx \
	-dhd_brand linux-external-dongle-pcie \
	-nvram src/shared/nvram/bcm94355fcpagb.txt\
	-clm_blob ../../../build/dongle/4355b3-roml/config_pcie_release_sdb/4355_stabello.clm_blob \
	-dhd_tag DHD_REL_1_359_* \
	-app_tag DHD_REL_1_359_* \
    -driver dhd-msgbuf-pciefd-debug \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}\
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
    -datarate {-i 0.5 -frameburst 1}\
    -nocal 1 -noaes 1 -notkip 1 -nokpps 1 -nomaxmem 1 \
    -tcpwindow 1152k -udp 800m \
    -device_reset {}
    
# 	-type ../C-4355__s-B3/sdb/simbaa.trx \
# 	-type ../C-4355__s-B3/sdb/simbaa.trx -clm_blob ../C-4355__s-B3/simbaa.clmb \
# 	-perfchans {36/80 36l 36 3} 
#     -wlinitcmds {wl mpc 0;wl ampdu_mpdu 48;wl vht_features 3} 

# btc_mode 1 already set by driver
4355b3 clone 4355b3_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}
4355b3 clone 4355b3sdb -type ../C-4355__s-B3/sdb/simbaa.trx -udp 400m
4355b3sdb clone 4355b3sdb_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}
4355b3 clone 4355b3_chan3 -perfchans 3
4355b3 clone 4355b3_941 -tag DIN2915T165R6_BRANCH_9_41 ;# -perfchans {36/80 36l 1}
4355b3_941 clone 4355b3_941_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}
4355b3_941 clone 4355b3_941t -dhd_tag trunk -app_tag trunk ;# -perfchans {36/80 36l 1}
4355b3_941t clone 4355b3_941t_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}
# -clm_blob ../C-4355__s-B3/simbaa.clmb 
# 4355b3_941 clone 4355b3_941_eagle -tag DIN2915T165R6_REL_9_41_21
4355b3_941 clone 4355b3_941_eagle -tag DIN2915T165R6_REL_9_41_*
4355b3_941_eagle clone 4355b3_941_eagle_coex -wlinitcmds {wl mpc 0; wl btc_mode 1}

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

mc59_BTCohost2 clone BTCohost4355b3\
	-bt_comm "com18@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_ss_location 0x00220000\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
	-type BCM4355/B3\
	-version BCM4355B3*SimbaA_*_MUR_MCC*\
	-brand ""\
	-file *.hcd
	
# 	-project_dir "/home/pkwan"\

BTCohost4355b3 clone BTCohost4355b3_eagle -version BCM4355B3*13.*SimbaA_*_MUR_MCC*

# BTCohost4355b3 clone BTCohost4355b3_local -project_dir "/home/pkwan" -type BCM4355 -file *.hcd
# BTCohost4355b3_local clone BTCohost4355b3_dbg -version BCM4355B3*SimbaA_*_USI_MCC*
# BTCohost4355b3_local clone BTCohost4355b3k -version BCM4355*Kristoff*MUR*MCC*
# BTCohost4355b3k clone BTCohost4355b3ku -version BCM4355*Kristoff*USI*MCC*

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
    
#removed for debug 12/11/14     -console "mc59end2:40010"\

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

# 4706 AP with 4360 and 4331 cards ;# debug 12/16/14: backdate build to 2014.10.2.0

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
		wl0_channel=36
		w10_bw_cap=-1
		wl0_radio=1
		wl0_obss_coex=0
		wl1_ssid=TestMC59_4331
		wl1_channel=1
		wl1_bw_cap=-1
		wl1_radio=1
		wl1_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1\
    -date 2014.10.2.

# # # UTF::Router MC59AP1 -sta {
# # #     4706/4331 eth1
# # #     4706/4360 eth2 4706/4360.%15 wl0.% 
# # #     } \
# # #     -lan_ip 192.168.1.6 \
# # #     -power "npc_AP2 1"\
# # #     -relay "mc59end1" \
# # #     -lanpeer lan0 \
# # #     -console "mc59end1:40036" \
# # #     -nvram {
# # # 	   	lan_ipaddr=192.168.1.6
# # # 		lan_gateway=192.168.1.6
# # # 		dhcp_start=192.168.1.150
# # # 	    dhcp_end=192.168.1.199
# # # 		lan1_ipaddr=192.168.2.6
# # # 		lan1_gateway=192.168.2.6
# # # 		dhcp1_start=192.168.2.150
# # # 	    dhcp1_end=192.168.2.199
# # #        	fw_disable=1
# # #        	wl_msglevel=0x101
# # #         boardtype=0x05b2; # 4706nr
# # #         console_loglevel=7
# # #        	wl0_ssid=TestMC59_4331
# # # 		wl0_channel=1
# # # 		w10_bw_cap=-1
# # # 		wl0_radio=0
# # # 		wl0_obss_coex=0
# # # 		wl1_ssid=TestMC59_4360
# # # 		wl1_channel=36
# # # 		wl1_bw_cap=-1
# # # 		wl1_radio=0
# # # 		wl1_obss_coex=0
# # # 		lan_stp=0
# # # 		lan1_stp=0
# # #     }\
# # #     -datarate {-b 1.2G -i 0.5 -frameburst 1}\
# # #     -noradio_pwrsave 1
# # #     \
# # #     -date 2014.10.2.

# # #     -datarate {-b 1.2G -i 0.5 -frameburst 1}\ ;# experiment with lower UDP offered rate 10/23/13
    
4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
4360_AARDVARK clone mc59_AP1_BISON -tag "BISON_BRANCH_7_10" 
4331_AARDVARK clone mc59_AP1_bg_BISON -tag "BISON_BRANCH_7_10"
4360_AARDVARK clone mc59_AP1_twig -tag "AARDVARK_REL_6_30_???_*"
4331_AARDVARK clone mc59_AP1_bg_twig -tag "AARDVARK_REL_6_30_???_*"
4360_AARDVARK clone mc59_AP1_trunk -tag TRUNK
4331_AARDVARK clone mc59_AP1_bg_trunk -tag TRUNK
mc59_AP1_BISON clone mc59_AP1
# mc59_AP1_bg_BISON clone mc59_AP1_bg
mc59_AP1_BISON clone mc59_AP1_EAGLE -tag "EAGLE_BRANCH_10_10" -brand "linux-2.6.36-arm-internal-router"
# mc59_AP1_EAGLE clone mc59_AP1

unset UTF::TcpReadStats

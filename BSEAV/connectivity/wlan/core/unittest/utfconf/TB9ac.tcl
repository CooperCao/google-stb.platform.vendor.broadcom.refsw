# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC23
# CoEx_TB9 testbed configuration
#
# TB6	4325-N88 (bring up device)	
# mc23end1  10.19.85.189   Linux UTF Host   - TB9UTF
# mc23end2  10.19.85.190   DUT Linux        - TB9DUT
# mc23tst1  10.19.85.191   Vista_BtRef      - TB9BTREF
# mc23tst2  10.19.85.192   Vista_BtCoHost   - TB9BTCOHOST
# mc23tst3  10.19.85.193   web_relay_BtRef
# mc23tst4  10.19.85.194   web_relay_DUT
#           10.19.85.212   NPC_AP
# mc59end11 10.19.85.139   DUT2 Linux       - TB9DUT2
# mc59end13 10.19.85.141   BTCohost2        - BTCohost2

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
# # set ::bt_dut BTCohost2				;# BlueTooth device under test
# # set ::bt_ref BTRefXP      			;# BlueTooth reference board
# # set ::wlan_dut 4330B2	  			;# HND WLAN device under test
# # set ::wlan_rtr AP1					;# HND WLAN router
# # # set ::wlan_tg lan              		;# HND WLAN traffic generator
# # set ::wlan_tg lan1              		;# HND WLAN traffic generator for 4335a0 tests

set ::bt_dut Cohost4335				;# BlueTooth device under test
set ::bt_ref BTRefXP      			;# BlueTooth reference board
set ::wlan_dut fc11sdio	  			;# HND WLAN device under test
# # set ::wlan_dut 4335b0	  			;# HND WLAN device under test
set ::wlan_rtr 4360_AARDVARK	;# HND WLAN router
# # set ::wlan_rtr AP1				;# HND WLAN router 4717
# # set ::wlan_rtr 4360_twig			;# 11ac router
# # set ::wlan_tg lan              		;# HND WLAN traffic generator 4717
set ::wlan_tg lan1      			;# 4336 <-> 4335

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# moved to dedicated volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/CoExTB9"

# Define power controllers on cart.
# package require UTF::Power
# UTF::Power::Synaccess 5.1.1.105
# UTF::Power::Agilent ag1 -lan_ip 5.1.1.58 -voltage 3.3 -ssh ssh
# - btref webrelay 10.19.85.199 pwr for 2046B1 ref
UTF::Power::WebRelay 10.19.85.193
# - dut webrelay 10.19.85.200 bt & wl reset lines ;# invert: reverse the position of the relay
UTF::Power::WebRelay 10.19.85.194
# - dut2 webrelay bt & wl reset lines
UTF::Power::WebRelay 10.19.85.140
# AP pwr control & console
UTF::Power::Synaccess 10.19.85.212
# RPC-22 for DUT and Cohost
UTF::Power::Synaccess 172.16.23.35
UTF::Power::Synaccess 172.16.23.36 ;# 5Volt power to embedded adapter
UTF::Power::Synaccess 172.16.23.37 ;# DUT2 and BTCohost2 power
UTF::Power::Synaccess npc_AP2 -lan_ip 10.19.87.120 -rev 1

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.23.40 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	
  	G1 attn 103 ;# completely shutting off G1 2/27 ;# changed to 10 from 0 for 43241b4 SDIO 2/5/13 ;# changed from 7 to 0 for 4330B2WLSD board 3/2/12
	G2 attn 30 ;# set to 30 from 10 8/27/14 ;# set to 10 from 26 8/7/14 ;# debug: changed from 30 to 10 on 2/22/12 ;# changed from 30 to 24 on 11/16/12 for 4335b0
	G3 attn 20 ;# from 26 to 20 8/13/2014 ;# from 31 to 26 on 11/19/13 for 4350c1 ;# changed from 25 to 31 on 1/29/13 ;# this is the group for 4335-4360 11AC tests
#     ALL attn 0

# turn off 4330B1 radio --> testing switched over to 4330B2 as of 10/7/11
	if {[catch {43241b4 wl down} ret]} {
        error "SetupTestBed: 43241b4 wl down failed with $ret"
    }
    		
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# mc24end1  10.19.85.195   Linux UTF Host   - TB6UTF
# -sta "lan eth0"
UTF::Linux TB9UTF  -lan_ip 10.19.85.189  -sta "lan eth1"

# mc60end14, ip addr 10.19.85.147 --> controller to AP2 

# UTF::Linux TB9UTF2 -lan_ip 10.19.85.148 -sta {lan1 eth1} ;# host mc60end15
UTF::Linux TB9UTF2 -lan_ip mc23tst10 -sta {lan1 eth1} ;# ip_addr 10.19.84.152 since 8/6/14

# lan configure -ipaddr 192.168.1.202

# #  mc60end13, ip addr 10.19.85.146, FC11
# #  changed to mc61end7 on 11/9/12 for debug
# #  changed to mc59end14 Bring Up PC 11/20 for debug
# # moved to FC15 host mc23tst9 8/7/14

UTF::DHD mc23tst9 \
	-sta "fc11sdio eth0" \
	-power "172.16.23.35 1"\
	-power_sta "10.19.85.194 4"\
	-device_reset "10.19.85.194 2"\
	-hostconsole "mc23tst10:40001"\
    -tag AARDVARK_{TWIG,REL}_6_30_69{,_*} \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94335wlbgaFEM_AA.txt" \
    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 4335a0min-roml/sdio-ag-idsup-assert-err \
    -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} \
    -nocal 1 -nopm1 1 -nopm2 1 \
    -postinstall {dhd -i eth0 msglevel 0x20001; dhd -i eth0 txglomsize 10} \
    -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1; wl ampdu_mpdu 24; wl vht_features 1} \
    -tcpwindow 1152k
# 	-hostconsole "mc60end15:40001"\  ;# taken out 11/21/12 after /etc/grub.conf file change in host; ip_add 10.19.85.148
#   (change="console=tty0 console=ttyS0,115200n8")   
#   -rwlrelay lan1 
# # #     -wlinitcmds {wl ampdu_mpdu 24; wl vht_features 1} ;# original statement prior to 2/8/13
    
# # clones    
### 4349A2 PCIe, BT dLNA, 3-Antenna, FC19 host mc23tst9, use eth0 for STA
### turn RSDB mode off for MIMO and Coex testing

fc11sdio clone 4359a2\
	-tag DINGO_BRANCH_9_10\
	-type 4359a2-roml/threadx-pcie-ag-p2p-mchan-wl11u-pktctx-tdls-mfp-splitrx-idsup-idauth-sr-ltecx-proptxstatus-ampduhostreorder-sstput-txbf-btcdyn-die2/rtecdc.bin\
	-nvram bcm94359fcpagbss.txt\
	-driver dhd-msgbuf-pciefd-debug\
	-dhd_brand linux-internal-dongle-pcie\
	-app_tag trunk\
	-wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0}\
	-udp 800m -tcpwindow 2m\
	-noaes 1 -perfonly 1\
	-postinstall {} -modopts {}\
	-pre_perf_hook {{%S wl ampdu_clear_dump} {%S wl phy_cal_disable 1} {%S wl scansuppress 1}}\
	-post_perf_hook {{%S wl dump ampdu} {%S wl dump amsdu} {%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl dump phycal} {%S wl phy_cal_disable 0} {%S wl scansuppress 0}}
# # # 	-brand hndrte-dongle-wl\
# # # threadx-pcie-ag-p2p-pno-pktfilter-keepalive-mchan-wl11u-pktctx-tdls-mfp-splitrx-idsup-idauth-sr-ltecx-proptxstatus-ampduhostreorder-aoe-die2*/rtecdc.bin\ ;# target used prior to 8/12/14


4359a2 clone 4359a2_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 1} -device_reset {}
4359a2 clone 4359a2_coex3 -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 3} -device_reset {}
4359a2 clone 4359a2_coex5 -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 5} -device_reset {}
4359a2_coex clone 4359a2_coex_TDM -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 1; wl btc_dynctl 0x00} -device_reset {}
4359a2_coex clone 4359a2_coex3_TDM -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 3; wl btc_dynctl 0x00} -device_reset {}
4359a2_coex clone 4359a2_coex5_TDM -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 5; wl btc_dynctl 0x00} -device_reset {}
4359a2 clone 4359a2_twig60 -tag DINGO_TWIG_9_10_60
4359a2_twig60 clone 4359a2_twig60_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl ampdu_mpdu 24; wl vht_features 3; wl rsdb_mode 0; wl btc_mode 1} -device_reset {}


# hostname mc23tst2, ipadd 10.19.85.192	
	    
UTF::WinBT TB9BTCOHOST\
	-lan_ip 10.19.85.192\
	-sta "BTCohost"\
    -device_reset "10.19.85.194 1"\
   	-bt_comm "com6@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM4330/B1\
    -bt_ss_location 0x00210000\
	-brand ""\
	-version BCM4330B1_002.001.003.0006.0000_McLaren
    
BTCohost clone TmpCohost_Alt\
	-version BCM4330B1_*_McLaren\
	-file *.cgs
	
#     -brand "Generic_NoExtLna/37_4Mhz"\

BTCohost clone TmpCohost\
	-version BCM4330B1_*_McLaren
	
BTCohost clone CohostRoW_Old\
	-version BCM4330B1_*\
	-brand "Generic_37_4MHz/fcbga"
# 	-version BCM4330B1_*87.0000\
# 	-file *.cgs
#; replaced by CohostRoW with new tree structure, below on 1/4/12

BTCohost clone CohostRoW\
	-version BCM4330B1_*\
	-brand "Generic/37_4MHz/fcbga"

BTCohost clone dbgCohost\
	-version BCM4330B1_*379*_McLaren
	
# 4359 is 4349 based: use 4349 BT config
	
BTCohost clone BTCohost4349\
	-bt_comm "com13@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 24\
	-bt_power_adjust 40\
	-type BCM4349/A0\
	-bt_ss_location 0x00220000\
	-brand Generic/UART/37_4MHz/fcbga_ref_sLNA\
	-version *\
	-file *.hcd
# 	\
# 	-device_reset ""
BTCohost4349 clone BTCohost4349A2 -type BCM4349/A2
BTCohost4349A2 clone BTCohost4349A2x -brand Generic/UART/37_4MHz/fcbga_BU_sLNA
BTCohost4349A2 clone BTCohost4349A2d -brand Generic/UART/37_4MHz/fcbga_ref_dLNA
BTCohost4349A2x clone BTCohost4349A2xd -brand Generic/UART/37_4MHz/fcbga_BU_dLNA

# BTRef hostname: mc23tst1
		
UTF::WinBT BTREFXP \
	-lan_ip 10.19.85.191 \
    -sta "BTRefXP" \
    -type "BCM2046" \
	-power "10.19.85.193 1"\
    -bt_comm "usb0" \
    -user user

# TB9DUT2 hostname: mc59end11

# # # UTF::DHD TB9DUT2 \
# # # 	-lan_ip 10.19.85.139\
# # # 	-sta "4330B2 eth1"\
# # # 	-power "172.16.23.37 2"\
# # # 	-power_sta "172.16.23.36 1"\
# # # 	-device_reset "10.19.85.140 1"\
# # # 	-tag FALCON_BRANCH_5_90\
# # # 	-brand linux-internal-dongle\
# # # 	-customer "bcm"\
# # # 	-type "4330b2-roml/sdio-ag-ccx-btamp-p2p-idsup-idauth-pno.bin"\
# # # 	-nvram "4330b2-roml/bcm94330fcbga_McLaren.txt"\
# # #     -noafterburner 1\
# # #     -wlinitcmds {wl mpc 0; wl ampdumac 0}\
# # #     -postinstall {dhd -i eth1 serial 1}\
# # #     -console "10.19.85.139:40000"


# hostname mc59end13 ipaddr 10.19.85.141
	    
# # # UTF::WinBT TB9BTCOHOST2\
# # # 	-lan_ip 10.19.85.141\
# # # 	-sta "BTCohost2"\
# # #     -device_reset "10.19.85.140 2"\
# # #    	-bt_comm "com10@3000000"\
# # # 	-bt_comm_startup_speed 115200\
# # #     -user user\
# # #     -bt_xtal_freq 26\
# # #     -bt_power_adjust 40\
# # #     -type BCM4330/B1\
# # #     -bt_ss_location 0x00210000\
# # # 	-brand ""\
# # # 	-version BCM4330B1_002.001.003.0006.0000_McLaren
# # # 	
# # # BTCohost2 clone Cohost4324B4\
# # #    	-bt_comm "com4@3000000"\
# # # 	-bt_rw_mode "Cortex M3 HCI"\
# # # 	-bt_w_size 200\
# # # 	-bt_xtal_freq 37.4\
# # # 	-bt_power_adjust 40\
# # #     -bt_ss_location 0x00210000 \
# # # 	-type BCM4324/*\
# # # 	-version BCM4324B*\
# # # 	-brand GenericB4/IPAAGB_Class1\
# # # 	-file *.cg*  

# # # Cohost4324B4 clone Cohost4324B4_local -project_dir "/home/pkwan"
#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router TB9AP1 \
    -lan_ip 192.168.1.1 \
    -sta "AP1 eth1" \
    -relay "TB9UTF" \
    -lanpeer lan \
    -console "10.19.85.189:40000" \
    -power "10.19.85.212 1"\
    -brand "linux-external-router-combo" \
    -tag "MILLAU_REL_5_70*" \
    -date * \
   -nvram {
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestCoex9
	wl0_channel=1
	w10_nmode=1
	wl0_antdiv=0
	antswitch=0
	wl0_obss_coex=0
    }
# # #     -tag "COMANCHE2_REL_5_22_\?\?" \
    
# Specify router antenna selection to use
### set ::wlan_rtr_antsel 0x01

# clone AP1s
AP1 clone MC23_AP1
AP1 clone mc23_AP1
AP1 clone TB9_AP1
AP1 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
AP1 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
AP1 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
AP1 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
AP1 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
AP1 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
AP1 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
AP1 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

# 4706 AP with 4360 and 4331 cards

UTF::Router TB9AP2 -sta {
    4706/4360 eth1 4706/4360.%15 wl0.% 
    4706/4331 eth2
    } \
    -lan_ip 192.168.1.2 \
    -power "npc_AP2 1"\
    -relay "TB9UTF2" \
    -lanpeer lan1 \
    -console "mc23tst10:40036" \
    -nvram {
	   	lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.200
	    dhcp_end=192.168.1.249
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.168.2.2
		dhcp1_start=192.168.2.200
	    dhcp1_end=192.168.2.249
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestTB9_4360
		wl0_channel=1
		w10_bw_cap=-1
# 		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestTB9_4331
		wl1_channel=36
		wl1_bw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1
#     -console "mc60end15:40036" \

4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router" 
# # # -date 2012.9.8
# 4360_AARDVARK clone 4360_ToB 
4360_AARDVARK clone 4360_BISON -tag BISON_BRANCH_7_10
4360_AARDVARK clone 4360_CARIBOU -tag CARIBOU_BRANCH_8_10
4360_AARDVARK clone 4360_trunk -tag TRUNK
4360_BISON clone 4360_ToB
4360_ToB clone 4360_ToB_noampdu -wlinitcmds "wl down; wl ampdu 0; wl up"
4360_AARDVARK clone 4360_current -date *
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
# # # 4360_current clone 4360_twig -tag "AARDVARK_{REL,TWIG}_6_30_???{,_*}"
4360_current clone 4360_twig -tag "AARDVARK_REL_6_30_???_*"
4360_current clone 4360_current_twig -tag "AARDVARK_TWIG_6_30_???{_*}"

# win7 host with 4360 NIC card
# STA Laptop DUT Dell E6420; host name mc60end11, ip_addr 10.19.85.144
# assumes nightly top-of-tree build
# factory device 4313

UTF::Cygwin mc60end11 -user user -sta {
	4313Win7 C0:18:85:76:5F:84
    4360Win7 00:10:18:A9:36:06
    } \
	-osver 7 \
	-installer inf \
	-tcpwindow 512k \
	-allowdevconreboot 1 \
	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \
	-pre_perf_hook {{%S wl rssi} {%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl rate} {%S wl scansuppress 0} {%S wl dump ampdu} {4706/4360 wl dump ampdu} {4706/4360 wl nrate} {4706/4360 wl rate} {4706/4360 wl dump rssi}}
    
# # 	-power {npc1 2} \
# # 	-power_button {auto} \
# # 	-kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe} \

4313Win7 clone 4313Win7-AARDVARK -tag AARDVARK_BRANCH_6_30

# # UTF::Cygwin mc61end8 -user user -sta {4360Win7} \
# # 	-osver 7 \
# # 	-installer inf \
# # 	-tcpwindow 512k \
# # 	-allowdevconreboot 1 \
# # 	-pre_perf_hook {{%S wl scansuppress 1} {%S wl ampdu_clear_dump}} \
# #     -post_perf_hook {{%S wl scansuppress 0} {%S wl dump ampdu}}
    
4360Win7 clone 4360Win7-AARDVARK -tag AARDVARK_BRANCH_6_30 
# #    -tcpwindow 4m\
# #    -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} 
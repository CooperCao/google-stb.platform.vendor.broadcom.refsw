# -*-tcl-*-
# ====================================================================================================
# 
# Test rig ID: SIG MC61
# MC61 testbed configuration
#
# mc61end1  10.19.87.115   Linux UTF Host   - mc60UTF
# mc61end2  10.19.87.116   DUT Linux        - mc60DUT
# mc61end3  10.19.87.117   BtRefXP      	- mc60BTREF ; temp replacement mc61end13 10.19.87.151
# mc61end4  10.19.87.118   BtCoHostXP   	- mc60BTCOHOST
# mc61end8	10.19.87.122   HSIC standalone	- HSIC
# mc61end9  10.19.87.123   web_relay_BtRef
# mc61end10 10.19.87.124   web_relay_DUT
#			172.16.61.10   Aeroflex attenuator
#           172.16.61.40   NPC_AP
#           172.16.61.30   NPC_DUT2
#           172.16.61.20   NPC_DUT

# For Coex scripts, need this package as UTF.tcl doesnt load it by default.
package require UTF::WinBT
package require UTF::Aeroflex

# Beta testing new Web report format 10/4/12
set UTF::WebTree 1

# Configuration definitions used by the Coex.test script
# NB: Use the STA names below, not the higher level host object names!
set ::bt_dut BTCohost_4324B5      			;# BlueTooth reference board
set ::bt_ref mc61_BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4324B5
set ::wlan_rtr mc61_AP1					;# HND WLAN router
set ::wlan_tg lan0              		;# HND WLAN traffic generator


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# moved to dedicated filer volume 10/30/13
set UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc61"

# Define power controllers on cart.
# package require UTF::Power
# - btref webrelay 10.19.85.199 pwr for 2046B1 ref
UTF::Power::WebRelay 10.19.87.123
# - dut webrelay 10.19.85.200 bt & wl reset lines
UTF::Power::WebRelay 10.19.87.124
# AP pwr control & console
UTF::Power::Synaccess 172.16.61.40
# RPC-22 for DUT and Cohost
UTF::Power::Synaccess 172.16.61.30
UTF::Power::Synaccess 172.16.61.20
# power control for HSIC
# UTF::Power::Synaccess 172.16.61.21 ;# superceded by hCtl, below, on 12/20/2011
UTF::Power::Synaccess hCtl -lan_ip 10.19.87.119

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.61.10 -group {G1 {1 2} G2 {3 4} G3 {5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	
  	G1 attn 30 ;# change from 35 to 18 on 5/13/13 for 4324B5 ;# changed to 25 from 35 on 2/28/12 ;# changed back ton 2/29/12
	G2 attn 25
	G3 attn 103
#     ALL attn 0
	
    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# hostname mc61end1, ip_addr 10.19.87.115

UTF::Linux mc61UTF  -lan_ip 10.19.87.115  -sta "lan0 eth1"

# hostname mc61end2, ip_addr 10.19.87.116

UTF::DHD mc61DUT \
	-lan_ip 10.19.87.116\
	-sta "mc61dut eth0"\
	-power "172.16.61.20 2"

### HSIC device decomissioned 090816	
package require UTF::HSIC
	
UTF::HSIC mc61HSIC\
	-sta {4330h eth1}\
	-lan_ip 192.168.1.1\
    -relay 4330B1 \
    -lanpeer 4330B1 \
    -power {hCtl 1}\
    -console "mc61end2:40001"\
    -tag FALCON_BRANCH_5_90\
    -type "4330b2-roml/usb-g-oob.bin.trx"\
	-nvram "4330b2-roml/bcm94330OlympicCelia.txt"\
    -brand "linux-external-dongle-usb"\
    -customer "olympic"\
    -host_tag "RTRFALCON_REL_5_130_26"\
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
    -nocal 1 
    

4330h clone 4330B2hSundance\
	-tag FALCON_REL_5_90_207_??\
    -type "Sundance/celia.trx"\
    -nvram "Sundance/celia-u.txt"
# 	-nvram "4330b2-roml/bcm94330OlympicCelia.txt"
#     -type "Sundance/celia.bin.trx"

### replaced 4330B2 Celia HSIC with 4324B5 dopplebock 4/5/13
### added postinstall "bt_enable.sh" 6/17/13: HSIC board uses software control for BT support (J40:1-2 setting not recommended, per LoiTran)

4330B2hSundance clone 4324B5\
    -host_tag "RTRFALCON_REL_5_130_56"\
	-tag PHO2203RC1_REL_6_25_??_*\
	-type "Innsbruck/4324b5/doppelbock.trx"\
	-nvram "Innsbruck/4324b5/syrah-m-mt.txt"\
	-postinstall "bt_enable.sh"


# # # changed host tag from RTRFALCON_REL_5_130_26 to RTRFALCON_REL_5_130_56, per Arun
# # # P408 board received 6/11/with 5.130.54 installed
	
4324B5 clone 4324B5b -wlinitcmds "wl btc_mode 1"
# # # 4324B5 clone 4324B5_bt -postinstall "bt_enable.sh"	
4324B5 clone 4324B5_ToB -tag PHO2203RC1_BRANCH_6_25 -nvram "*/4324b5/syrah-m-mt.txt"
4324B5_ToB clone 4324B5_ToB_coex -wlinitcmds "wl btc_mode 1"
4324B5 clone 4324B5x -tag PHO2203RC1_REL_6_25_*
4324B5x clone 4324B5x_coex -wlinitcmds "wl btc_mode 1"
4324B5 clone 4324B5x105 -tag PHO2203RC1_REL_6_25_105_* 	-type "*/4324b5/doppelbock.trx" -nvram "*/4324b5/syrah-m-mt.txt"
4324B5x105 clone 4324B5x105_coex -wlinitcmds "wl btc_mode 1"
4324B5 clone 4324B5x646 -tag PHO2625105RC255_REL_6_46_* 	-type "*/4324b5/doppelbock.trx" -nvram "*/4324b5/syrah-m-mt.txt"
4324B5x646 clone 4324B5x646_coex -wlinitcmds "wl btc_mode 1"

4324B5 clone 4324B5_dbg\
 -nvram /projects/hnd/swbuild/build_linux/PHO2203RC1_REL_6_25_46_11/linux-external-dongle-usb/*/release/olympic/firmware/Innsbruck/4324b5/syrah-u-tt.txt 

4324B5 clone 4324B5_dbg1 -wlinitcmds "wl stbc_tx 0"

UTF::WinBT mc61end12\
    -lan_ip 10.19.85.150\
    -sta "mc61_BTCohost2"\
   	-bt_comm "com10@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM4330/B1\
    -bt_ss_location 0x00210000\
	-brand Olympic_Uno3/Murata\
	-version *


# hostname mc61end13 ip_address 10.19.85.151

UTF::WinBT mc61BTCOHOST\
	-lan_ip mc61end13\
	-sta "BTCohost_4357b0"\
    -power "172.16.61.20 1"\
   	-bt_comm "com14@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
	-bt_power_adjust 40\
    -type BCM4347\
    -brand Generic/UART/37_4MHz/fcbga_ref4_dLNA\
    -bt_ss_location 0x00212678\
    -version BCM4347B0*\
    -project_dir "/home/pkwan"\
    -file *.cgr
	
# # # 	-version "BCM4324B5*S*O*MUR*"\

#hostname mc61end4, ip_add 10.19.87.118 
       
UTF::WinBT mc61BTREFXP \
	-lan_ip 10.19.87.118 \
    -sta "mc61_BTRefXP" \
	-device_reset "10.19.87.123 1"\
    -type "BCM2046" \
    -bt_comm "usb0" \
    -user user
    
#     -power "172.16.60.30 1"\    

#
# Linksys 320N 4717/4322 wireless router.
# Note: -lanpeer is to help locate LAN endpoint for tests
#

UTF::Router MC61AP1 \
    -lan_ip 192.168.1.3 \
    -sta "mc61_AP1 eth1" \
    -relay "mc61UTF" \
    -lanpeer lan0 \
    -console "10.19.87.115:40000" \
    -power "172.16.61.40 1"\
    -brand "linux-external-router-combo" \
    -tag "MILLAU_REL_5_70*" \
    -date * \
   -nvram {
	    et0macaddr=00:90:4c:61:00:00
	    macaddr=00:90:4c:61:01:0a
		lan_ipaddr=192.168.1.3
		lan_gateway=192.168.1.3
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestMC61
		wl0_channel=1
		w10_nmode=1
		wl0_antdiv=0
		antswitch=0
		wl0_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }

# Specify router antenna selection to use
set ::wlan_rtr_antsel 0x01

# clone AP1s
mc61_AP1 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
# 4717 clone 4717MILLAU -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
# 4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
mc61_AP1 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
# # # mc61_AP1 clone 4717MILLAU -tag "MILLAU_REL_5_70_38" -brand linux-external-router
mc61_AP1 clone 4717MILLAU -tag "MILLAU_REL_5_70*" -brand linux-external-router
mc61_AP1 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_38" -brand linux-internal-router
mc61_AP1 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
mc61_AP1 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
mc61_AP1 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
mc61_AP1 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

unset UTF::TcpReadStats

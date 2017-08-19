# -*-tcl-*-
#
# testbed configuration for Peter Kwan MC32 testbed
# based on mc28
# created 4/21/10
# changes 11/2/10:
#  added device info on top of file 
# ====================================================================================================
# Test rig ID: SIG MC32
# MC32 testbed configuration
#
# mc32end2  10.19.85.244   Endpoint2       	- lan1
# mc32tst1  10.19.85.245   BCM43227			- 43227fc9
# mc32tst2  10.19.85.246   BCM43228			- 43228XP
# mc32tst3  10.19.85.247   BCM4313			- 4313fc9
# mc32tst4  10.19.85.248   BCM4330B1		- 4330b1Dual
# 			172.16.1.31	   npc1
# 			172.16.1.20	   npc2
# 			172.16.1.33	   npc3
# 			172.16.1.34	   npc4
# 			172.16.1.21    Aeroflex			- af

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD
package require UTF::WinBT

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc32_dev" ;# new filer space for PeterK

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.31
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.20
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.33 -rev 1 ;# replaced dead unit with newer (T) unit
UTF::Power::Synaccess npc4 -lan_ip 172.16.1.34
UTF::Power::Synaccess npc5 -lan_ip 172.16.1.35
UTF::Power::Synaccess npc6 -lan_ip 172.16.1.36 -rev 1 ;# new AP2 enclosure
UTF::Power::WebRelay 172.16.1.37 ;# web relay for BTRef boards
UTF::Power::WebRelay 172.16.1.60 ;# web relay for DUT

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.1.132 -group {G1 {1 2 3} G2 {4} G3 {5 6 7 8 9} ALL {1 2 3 4 5 6 7 8 9}} -relay mc32end2

# Test bed initialization
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {

  	G1 attn 17
  	G2 attn 0
# 	G2 attn 20
 	G3 attn 95.5
# 	G4 attn 25

 	# turn off both radios in AP1 and AP2
	#AP2 restart wl1_radio=0

    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# second endpoint; upgraded to FC19 4/1/15
UTF::Linux mc32end2 \
	-sta {lan1 eth0}
# 	-iperf iperf170

# laptop in use: -lan_ip 10.19.87.12;
# changed to desktop lan_ip 10.19.84.202 mc50tst5
#BT Cohosts
# package require UTF::WinBT

UTF::WinBT BTCOHOST\
	-lan_ip mc50tst5\
	-sta "BTCohost"\
	-bt_comm "com5@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
	-bt_ss_location 0x0021C000\
	-bt_xtal_freq 40\
    -type BCM4350B0\
	-version BCM4350B0_*\
	-brand "Generic/UART/37_4MHz/wlbga_BU"\
	-file *.hcd\
	-device_reset "172.16.1.60 1"

BTCohost clone BLECohost
		
BTCohost clone BTCohost_4350c0\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_ss_location 0x0021E000\
	-type BCM4350/C0\
	-version BCM4350C0*\
	-brand Generic/UART/37_4MHz/wlbga_BU_eLNA\
    -bt_power_adjust 40
#     \
#     -bt_comm "com4@3000000"

# BTCohost_4350c0 clone BTCohost_LE -bt_comm "com5@115200"
BTCohost_4350c0 clone BTCohost_4350c0_local -project_dir "/home/pkwan"
BTCohost_4350c0 clone BTCohost_stella\
	-type BCM4350/C* -brand "" -version BCM4350C*_Riesling_OS_MUR_ST_*
	
# BTCohost_stella clone BLECohostBT
BTCohost_stella clone BTCohost_stella_cfg320 -brand "" -version BCM4350C*.320.*Riesling_OS_MUR_ST_*
BTCohost_stella clone BTCohost_320 -brand "" -version BCM4350C*.320.*Riesling_OS_MUR_ST_*
BTCohost_stella clone BTCohost_LE -bt_comm "com5@115200"
BTCohost_LE clone BTClone

UTF::WinBT BTREFXP\
	-lan_ip mc50tst5\
    -sta "BTRef"\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user\
   -device_reset "172.16.1.37 2"

# BLERef BCM2070 installed on usb1 of mc50tst5
# 	-lan_ip mc50tst5

UTF::WinBT BLEREFXP\
	-lan_ip mc50tst5\
	-sta "BLERef"\
	-type "BCM2070"\
	-bt_comm "usb0"\
	-user user
	
# 	-device_reset "172.16.1.37 1"

BLERef clone BLERefBT
BLERef clone BTRefClone

# BLERef clone BLERef_power -power "172.16.1.35 1"

UTF::DHD mc50tst1 \
	-lan_ip 10.19.86.200 \
	-sta "DUT eth0"\
	-power "172.16.1.35 2"\
	-device_reset "172.16.1.60 2"\
	-console mc50tst1:40001
	
DUT clone stella\
	-app_tag BIS120RC4PHY_BRANCH_7_16 \
    -tag BIS120RC4PHY_BRANCH_7_16 \
    -dhd_tag trunk \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
	-nvram bcm94350GamayMurataMT.txt\
    -customer olympic \
    -type ../C-4350__s-C2/rieslingb.trx -clm_blob rieslingb.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
    -datarate {-i 0.5 -frameburst 1}\
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 1152k -udp 400m \
    -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}\
    -device_reset ""

# # # 	-device_reset "172.16.24.20 1"\

stella clone stella_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}

stella clone stella_mon -tag BIS715T185RC1_REL_7_35_62\
	-nvram ../C-4350__s-C2/P-rieslingb_M-STEL_V-m__m-3.9.txt -postinstall {}
stella_mon clone stella_mon_coex -wlinitcmds "wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1"
# 	-dhd_tag DHD_REL_1_359_13

stella_mon clone stella_t735 -tag BIS715T185RC1_REL_7_35_* -type ../C-4350__s-C4/rieslingb.trx -clm_blob rieslingb.clmb
stella_t735 clone stella_t735_coex -wlinitcmds "wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1"

UTF::Router AP2 -sta {
    4706/4360 eth1 4706/4360.%15 wl0.% 
    4706/4331 eth2
    } \
    -lan_ip 192.168.1.2 \
    -power "npc6 2"\
    -relay "mc32end2" \
    -lanpeer lan1 \
    -console "mc32end2:40000" \
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
       	wl0_ssid=mc32_4360
		wl0_channel=36
		w10_bw_cap=-1
# 		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestTB9_4331
		wl1_channel=1
		wl1_bw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
		lan_stp=0
		lan1_stp=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1\
    -date 2014.10.2.

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

### BLE development setup 4/8/14

#BT environment
set ::bt_dut BTCohost_stella		;# BlueTooth device under test
set ::bt_ref BLERef					;# BlueTooth reference board
# set ::bt_ref BTRef					;# BlueTooth reference board
set ::wlan_dut stella				;# HND WLAN device under test
set ::wlan_rtr 4360_ToB				;# HND WLAN router
set ::wlan_tg lan1					;# 4336 <-> 4335

# experimental
unset UTF::TcpReadStats
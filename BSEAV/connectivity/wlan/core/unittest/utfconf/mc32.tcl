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
# mc32end1  10.19.85.243   Endpoint1   		- lan0
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

# -kdpath {C:\Program Files\Debugging Tools for Windows (x86)\kd.exe}

### BT configurations
set ::bt_dut BTCohost				;# BlueTooth device under test
set ::bt_ref BTRef      			;# BlueTooth reference board
set ::wlan_dut 4350C5_rel		  	;# HND WLAN device under test
set ::wlan_rtr 4717Akashi			;# HND WLAN router
set ::wlan_tg lan0              	;# HND WLAN traffic generator

# Packages
package require UTF::Aeroflex
package require UTF::Linux
package require UTF::DHD
# package require UTF::Sniffer

# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# # # set ::UTF::SummaryDir "/projects/hnd_sig_ext4/$::env(LOGNAME)/mc32"
set ::UTF::SummaryDir "/projects/hnd_sig_ext17/$::env(LOGNAME)/mc32" ;# new filer space for PeterK

# NPC definitions
UTF::Power::Synaccess npc1 -lan_ip 172.16.1.31
UTF::Power::Synaccess npc2 -lan_ip 172.16.1.20
UTF::Power::Synaccess npc3 -lan_ip 172.16.1.33 -rev 1 ;# replaced dead unit with newer (T) unit
UTF::Power::Synaccess npc4 -lan_ip 172.16.1.34
UTF::Power::Synaccess npc5 -lan_ip 172.16.1.35
UTF::Power::Synaccess npc6 -lan_ip 172.16.1.36 -rev 1 ;# new AP2 enclosure
UTF::Power::WebRelay 172.16.1.80

# Attenuator
UTF::Aeroflex af -lan_ip 172.16.1.21 -group {G1 {1 2} G2 {3 4} G3 {5 6 7 8} G4 {9} ALL {1 2 3 4 5 6 7 8 9}} -relay mc32end1

# Test bed initialization
# set Aeroflex attenuation, and turn off radio on APs
set ::UTF::SetupTestBed {

  	G1 attn 10 ;# back to 10 from 25 1/15/15;# changed from 10 to 25 for 4350c0 10/5/13
	G2 attn 20
	G3 attn 95.5
	G4 attn 25

 	# turn off both radios in AP1 and AP2
 	AP1 restart wl0_radio=0
	#AP2 restart wl1_radio=0

    # delete myself
    unset ::UTF::SetupTestBed

	return
}

# UTF Endpoint - Traffic generators (no wireless cards)
# use GigE interface eth2 to connect to LAN side of AP
# both end points are FC-9, tcpslowstart 4 does not apply
UTF::Linux mc32end1 \
    -sta {lan0 eth1}

# second endpoint; yet to replace dead power supply as of 1/19/15
# # # UTF::Linux mc32end2 \
# # # 	-sta {lan1 eth1}

# # STA2: Laptop DUT Dell E6400; takes a release build; converted to BTRef cohost 10/30/13
package require UTF::WinBT

UTF::WinBT mc32tst1 -lan_ip 10.19.85.245 -sta BTCohost\
   	-bt_comm "com4@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 40\
    -bt_power_adjust 40\
    -bt_ss_location 0x0021E000\
    -type BCM4350/C5\
	-brand ""\
	-version BCM4350C5*_Albarossa_OS_MUR_BM_20*\
	-file *.hcd \
    -power_sta {npc3 2}
    
BTCohost clone BTCohost_cfg12 -version BCM4350C5_12*_Albarossa_OS_MUR_BM_20*
BTCohost clone BTCohost_cfg46 -version BCM4350C5_*.46.*Albarossa_OS_MUR_BM_20*

BTCohost clone BTCohost_stc\
   	-bt_comm "com5@3000000"\
	-version BCM4350C5*_Brunello_OS_MUR_STC_*\
	-power_sta {}

UTF::WinBT mc32tst2 -lan_ip 10.19.85.246 -sta BTRef\
    -type "BCM2046"\
    -bt_comm "usb0"\
    -user user\
    -device_reset "172.16.1.80 1"

UTF::DHD MC32DUT1 \
	-lan_ip mc32tst3\
	-sta "mc32DUT1 eth0"\
	-power {npc4 2}

mc32DUT1 clone 4350C5\
	-app_tag BIS120RC4PHY_BRANCH_7_16 \
    -tag BIS120RC4PHY_BRANCH_7_16 \
    -dhd_tag trunk \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
	-nvram ../C-4350__s-C5/P-albarossa_M-BLMA_V-m__m-2.1.txt\
    -customer olympic \
    -type ../C-4350__s-C5/albarossa.trx -clm_blob albarossa.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
	-power "npc3 2"\
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 1152k -udp 400m \
    -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}

# # # 	-device_reset "172.16.24.20 1"
4350C5 clone 4350C5_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
4350C5 clone 4350C5_coex5 -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 5}

4350C5 clone 4350C5_rel\
	-app_tag ""\
	-tag BIS120RC4_REL_7_15_*\
	-nvram ../C-4350__s-C5/P-albarossa_M-BLMA_V-m__m-2.1.txt\
	-type ../C-4350__s-C5/albarossa.trx -clm_blob albarossa.clmb


4350C5_rel clone 4350C5_rel_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
4350C5_rel clone 4350C5_rel_coex5 -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 5}

4350C5_rel clone 4350C5_mon\
	-tag BIS715T185RC1_REL_7_35_*
4350C5_mon clone 4350C5_mon_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
4350C5_mon clone 4350C5_mon_t161 -tag BIS715T185RC1_REL_7_35_161_* -dhd_tag DHD_REL_1_359_31
4350C5_mon_t161 clone 4350C5_mon_t161_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
4350C5_mon_t161 clone 4350C5_mon_t161d -dhd_tag DHD_REL_1_359_*
4350C5_mon_t161d clone 4350C5_mon_t161d_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}

# # # mc32tst4 refined as mc32DUT2

UTF::DHD MC32DUT2 \
	-lan_ip mc32tst4\
	-sta "mc32DUT2 eth0"
	
mc32DUT2 clone 4350C5_stc\
    -tag BIS120RC4_BRANCH_7_15 \
    -dhd_tag trunk \
    -dhd_brand linux-external-dongle-pcie \
    -brand linux-external-dongle-pcie \
    -driver dhd-msgbuf-pciefd-debug \
	-nvram ../C-4350__s-C5/P-brunello_M-CIDR_V-m__m-2.15.txt\
    -customer olympic \
    -type ../C-4350__s-C5/brunello.trx -clm_blob brunello.clmb \
    -postinstall {dhd -i eth0 dma_ring_indices 3}\
    -datarate {-i 0.5 -frameburst 1} \
    -nocal 1 -noaes 1 -notkip 1 \
    -tcpwindow 1152k -udp 400m \
    -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc} \
    -pre_perf_hook {{%S wl scansuppress 1}} \
    -post_perf_hook {{%S wl rssi} {%S wl nrate} {%S wl counters} {%S wl scansuppress 0}}\
	-power "npc3 1" 
    
4350C5_stc clone 4350C5_phy\
    -tag BIS120RC4PHY_BRANCH_7_16 \

4350C5_stc clone 4350C5_dhd17\
    -dhd_tag DHD_REL_1_209_*
    
4350C5_stc clone 4350C5_stc_mon -tag BIS715T185RC1_REL_7_35_*
	

4350C5_stc clone 4350C5_stc_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
4350C5_stc clone 4350C5_stc_coex5 -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 5}

4350C5_dhd17 clone 4350C5_dhd17_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
4350C5_stc_mon clone 4350C5_stc_mon_coex -wlinitcmds {wl msglevel 0x101; wl msglevel +assoc; wl btc_mode 1}
	
# Linksys 320N 4717/4322 wireless router.
UTF::Router AP1 \
    -sta "4717 eth1" \
	-lan_ip 192.168.1.1 \
    -relay "mc32end1" \
    -lanpeer lan0 \
    -console "mc32end1:40000" \
    -power {npc1 1} \
    -tag MILLAU_REL_5_70* \
	-brand linux-internal-router \
    -nvram {
        et0macaddr=00:90:4c:20:00:00
        macaddr=00:90:4c:20:01:0a
		lan_ipaddr=192.168.1.1
		lan_gateway=192.168.1.1
		dhcp_start=192.168.1.100
  		dhcp_end=192.168.1.149
		lan1_ipaddr=192.168.2.1
		lan1_gateway=192.169.2.1
		dhcp1_start=192.168.2.100
	    dhcp1_end=192.168.2.149
       	fw_disable=1
       	wl_msglevel=0x101
        wl0_ssid=mc32test
        wl0_channel=1
#         wl0_radio=0
        antswitch=0
        wl0_obss_coex=0
}

# clone AP1s
4717 clone 4717COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
4717 clone 4717Ext -tag "COMANCHE2_REL_5_22_90" -brand linux-external-router
4717 clone 4717MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
4717 clone 4717MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
4717 clone 4717MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
4717 clone 4717MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
4717 clone 4717Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
4717 clone 4717AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

UTF::Router AP2 \
    -sta "47172 eth2" \
    -lan_ip 192.168.1.2 \
    -relay "mc32end2" \
    -lanpeer lan1 \
    -console "mc32end2:40000" \
    -power {npc2 1} \
    -brand linux-internal-router \
    -tag "AKASHI_BRANCH_5_110" \
    -nvram {
        et0macaddr=00:90:4c:20:02:00
        macaddr=00:90:4c:20:03:0a
		lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.199
	    dhcp_end=192.168.1.249
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.199
	    dhcp1_end=192.168.2.249
       	fw_disable=1
       	wl_msglevel=0x101
       	wl1_ssid=mc32_4705
#        	wl0_channel=1
       	wl0_radio=0
       	wl1_channel=1
# 		wl1_radio=0
       	wl1_obss_coex=0
       	{landevs=vlan1 wl0 wl1}
       	wandev=et0
       	{vlan1ports=1 2 3 4 8*}
       	{vlan2ports=0 8u}
}

# clone AP2s
# # # 4705 clone 4705COMANCHE2 -tag "COMANCHE2_REL_5_22_90" -brand linux-internal-router
# # # 4705 clone 4705Ext -tag "COMANCHE2_REL_5_220_90" -brand linux-external-router
# # # 4705 clone 4705MILLAU -tag "MILLAU_REL_5_70_??" -brand linux-external-router
# # # 4705 clone 4705MILLAUInt -tag "MILLAU_REL_5_70_??" -brand linux-internal-router
# # # 4705 clone 4705MExt -tag "MILLAU_BRANCH_5_70" -brand linux-external-router
# # # 4705 clone 4705MInt -tag "MILLAU_BRANCH_5_70" -brand linux-internal-router
# # # 4705 clone 4705Akashi -tag "AKASHI_BRANCH_5_110" -brand linux-internal-router
# # # 4705 clone 4705AkashiExt -tag "AKASHI_BRANCH_5_110" -brand linux-external-router

unset UTF::TcpReadStats

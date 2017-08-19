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
set ::bt_dut BTCohost2				;# BlueTooth device under test
set ::bt_ref BTRefXP      			;# BlueTooth reference board
set ::wlan_dut 4330B2	  			;# HND WLAN device under test
set ::wlan_rtr AP1					;# HND WLAN router
set ::wlan_tg lan              		;# HND WLAN traffic generator


# Optional items for controlchart.test to run each iteration
set UTF::controlchart_cmds {{if {[%S hostis Cygwin DHD DHDUSB MacOS WinDHD]} {%S wl rssi ; %S wl antdiv; %S wl nrate}}}

# SummaryDir sets the location for test results
# UTF::Logfile "~/utf_test.log"
# # # set UTF::SummaryDir "/projects/hnd_sig_ext3/$::env(LOGNAME)/CoExTB9"
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
# # # G1: DUT 1 wlan; G2: BT path; G3: 4360/11ac AP signal path
UTF::Aeroflex af -lan_ip 172.16.23.40 -group {G1 {1 2} G2 {3} G3 {4 5 6} ALL {1 2 3 4 5 6}}

set ::UTF::SetupTestBed {
	
  	G1 attn 15 ;# changed to 0 from 10 after adding fixed pads 2/27 
  	;# changed from 0 to 10 2/11/13 ;# changed to 20 from 10 2/6/13 ;#changed from 7 to 0 for 4330B2WLSD board 3/2/12
#   G1 attn 16 ;# added 6dB to improve signal rejection 10/21/11
#   G2 attn changed from 15 to 10 on 8/16/11
	G2 attn 18 ;# changed to 15 from 25 6/7/13 ;# changed to 25 from 0 2/15/13 ;#changed to 0 from 10 2/6/13 ;# debug: changed from 30 to 10 on 2/22/12
	G3 attn 103 ;# changed to 103 from 10 2/27/13
#     ALL attn 0

# turn off 4335B0 radio --> testing switched over to 4330B2 as of 10/7/11
	if {[catch {fc11sdio wl down} ret]} {
        error "SetupTestBed: 4335B0 wl down failed with $ret"
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

UTF::Linux TB9UTF2 -lan_ip mc23tst10 -sta {lan1 eth1}
# UTF::Linux TB9UTF2 -lan_ip 10.19.85.148 -sta {lan1 eth1} ;# changed to mc23tst10 10.19.84.152 as of 8/6/14
# lan configure -ipaddr 192.168.1.202

# #  mc60end13, ip addr 10.19.85.146, FC11
# # changed to mc59end14

UTF::DHD mc59end14 \
	-sta "fc11sdio eth1" \
	-console "mc60end13:40001"\
	-power "172.16.23.35 1"\
	-power_sta "10.19.85.194 4"\
	-device_reset "10.19.85.194 2"\
    -tag AARDVARK_{TWIG,REL}_6_30_69{,_*} \
    -dhd_tag NIGHTLY \
    -brand linux-external-dongle-sdio \
    -dhd_brand linux-internal-dongle \
    -nvram "bcm94335wlbgaFEM_AA.txt" \
    -modopts {sd_uhsimode=3 sd_txglom=1 sd_tuning_period=10} \
    -type 4335a0min-roml/sdio-ag-idsup-assert-err \
    -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} \
    -nocal 1 -nopm1 1 -nopm2 1 \
    -postinstall {dhd -i eth1 msglevel 0x20001; dhd -i eth1 txglomsize 10} \
    -wlinitcmds {wl ampdu_mpdu 24} \
    -tcpwindow 1152k -rwlrelay lan1 
    
# # clones    
fc11sdio clone 4335a069 -tag AARDVARK_{TWIG,REL}_6_30_69{,_*}
fc11sdio clone 4335a098 -tag AARDVARK_{TWIG,REL}_6_30_98{,_*}


# # setting for SDIO 2.0 card in -modopts
  
fc11sdio clone 4335a0k\
  -modopts {}\
  -postinstall {dhd -i eth1 msglevel 0x20001}\
  -type "4335a0min-roml/sdio-ag-pool-p2p-idsup-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus"  
  
# # intended for use with -bin {src path} option

fc11sdio clone 4335a0km\
  -modopts {}\
  -postinstall {dhd -i eth1 msglevel 0x20001}\
  -type "4335a0min-roml/sdio-ag-pool-p2p-idsup-pno-pktfilter-keepalive-aoe-ccx-sr-mchan-proptxstatus/rtecdc.bin"    
  
#   -type "4335a0min-roml/sdio-ag-idsup-assert-err/rtecdc"
  
#   -type "/home/pkwan/src/tools/unittest/bcm4335/rtecdc"
    
#   "/projects/hnd_software_ext8/work/karthik/firmware/AARDVARK_TWIG_6_30_69/src/dongle/rte/wl/builds/4335a0min-roml/sdio-ag-idsup-assert-err/rtecdc.bin"

fc11sdio clone 4335a0x \
    -type 4335a0min-roml/sdio-ag-idsup \
    -noaes 1 -perfonly 1 -perfchans {36/80}

4335a0x clone 4335a0x69 -tag AARDVARK_{TWIG,REL}_6_30_69{,_*}
4335a0x clone 4335a0x98 -tag AARDVARK_{TWIG,REL}_6_30_98{,_*}

# #  replaced by mc60end13 fc11 host on 8/23/12
# #     

UTF::DHD TB9DUT \
	-lan_ip 10.19.85.190\
	-sta "4330B1 eth1"\
	-power "172.16.23.35 1"\
	-power_sta "10.19.85.194 4"\
	-device_reset "10.19.85.194 2"\
	-tag FALCON_REL_5_90_85_1\
	-brand linux-internal-dongle\
	-customer "bcm"\
	-type "4330b1-roml/sdio-g-pool.bin"\
	-nvram "4330b1-roml/bcm94330fcbga_McLaren.txt"\
    -noafterburner 1\
    -wlinitcmds {wl mpc 0; wl ampdumac 0}\
    -postinstall {dhd -i eth1 serial 1}\
    -console "mc23end2:40000"
    

4330B1 clone 4330B1-Falcon\
	-tag FALCON_BRANCH_5_90\
	-type "4330b1-roml/sdio-g.bin"\
	-nvram "src/shared/nvram/bcm94330fcbga_McLaren.txt"
	
4330B1 clone 4330B1-Falcon-pool\
	-tag FALCON_BRANCH_5_90\
	-type "4330b1-roml/sdio-g-pool.bin"\
	-nvram "src/shared/nvram/bcm94330fcbga_McLaren.txt"
	
4330B1 clone 4330B1-Falcon-pno\
	-tag FALCON_BRANCH_5_90\
	-type "4330b1-roml/sdio-ag*pno.bin"\
	-nvram "src/shared/nvram/bcm94330fcbga_McLaren.txt"	

# 	final supported builds for 4330B1
		
4330B1 clone 4330B1-Final\
	-tag FALCON_TWIG_5_90_125\
	-type "4330b1-roml/sdio-ag-pool-ccx-btamp-p2p-idsup-idauth-pno-aoe-toe-pktfilter-keepalive-wapi.bin"\
	-nvram "src/shared/nvram/bcm94330fcbga_McLaren.txt"	
	
# 	-type "4330b1-roml/sdio-g-pool.bin"\
# 	image name suffix -pool was dropped starting 5/23/11

4330B1 clone 4330B1-twig\
	-tag FALCON_REL_5_90_100_8
	
4330B1 clone 4330B1-twig-late\
	-tag FALCON_REL_5_90_100_24	
	
4330B1 clone 4330B1-cyclops\
	-tag FALCON_REL_5_90_*\
	-type "4330b1-roml/sdio-ag*pno.bin"\
	-nvram "src/shared/nvram/bcm94330fcbga_McLaren.txt"
# 		-tag FALCON_TWIG_5_90_153\
	
4330B1 clone 4330B1-rel125\
	-tag FALCON_REL_5_90_125_*\
	-type "4330b1-roml/sdio-g-pool.bin"	
	
4330B1 clone 4330B1-tmp\
	-tag FALCON_REL_5_90_125_56\
	-brand linux-external-dongle-sdio\
	-type "4330b1-roml/sdio-ag-pool.bin"	
		
    
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
	
BTCohost clone Cohost4335\
	-bt_comm "com10@3000000"\
	-bt_xtal_freq 37.4\
    -bt_power_adjust 40\
    -bt_ss_location 0x00218000\
    -type BCM4335\
	-version BCM4335A0_*\
	-brand "Generic/UART/37_4MHz/wlbga_ePA"\
	-file *.cg*
	
# #  user -btc_com "com9@3000000" for SDIO 2.0; -bt_comm "com10@3000000" for SDIO 3.0
	
# BTRef hostname: mc23tst1
		
UTF::WinBT BTREFXP \
	-lan_ip 10.19.85.191 \
    -sta "BTRefXP" \
    -type "BCM2046" \
	-power "10.19.85.193 1"\
    -bt_comm "usb0" \
    -user user

# TB9DUT2 hostname: mc59end11, lan_ip 10.19.85.139
# upgraded to FC15 2/19/14; replaced 43241b4 with 43451b1/corona_b1 6/4/14

UTF::DHD TB9DUT2\
	-lan_ip mc59end11\
	-sta {corona eth0}\
	-tag BIS120RC4_BRANCH_7_15\
    -brand linux-external-dongle-pcie\
    -type "43451b1-roml/pcie-ag-msgbuf-splitrx-splitbuf-p2p-pno-nocis-keepalive-aoe-idsup-wapi-sr-cca-pwrstats-noclminc-logtrace-err-assert-txpwrcap.bin"\
    -customer "olympic"\
	-nvram src/shared/nvram/bcm94345TempranilloTDKKK.txt\
	-dhd_brand linux-internal-dongle-pcie\
	-driver dhd-msgbuf-pciefd-debug\
	-power "172.16.23.37 2"\
	-noafterburner 1 -noaes 1 -notkip 1 -nomaxmem 1\
	-dhd_tag NIGHTLY\
	-tcpwindow 1152k -udp 200m\
	-datarate {-b 200m} -perfchans {3 36} -channelsweep {-bw 20}
# # # 	\
# # # 	-device_reset "10.19.85.140 1"

corona clone corona_coex
# # # \
# # #  -wlinitcmds "wl btc_mode 1"
corona clone corona_dbg -dhd_date 2014.7.20
corona clone corona_phy -tag BIS120RC4PHY_BRANCH_7_16
corona_phy clone corona_phy_coex -wlinitcmds {wl btc_mode 1}

# hostname mc59end13 ipaddr 10.19.85.141
	    
UTF::WinBT TB9BTCOHOST2\
	-lan_ip 10.19.85.141\
	-sta "BTCohost2"\
    -device_reset "10.19.85.140 2"\
   	-bt_comm "com10@3000000"\
	-bt_comm_startup_speed 115200\
    -user user\
    -bt_xtal_freq 26\
    -bt_power_adjust 40\
    -type BCM4330/B1\
    -bt_ss_location 0x00210000\
	-brand ""\
	-version BCM4330B1_002.001.003.0006.0000_McLaren
    

BTCohost2 clone Cohost43451\
   	-bt_comm "com7@3000000"\
	-bt_rw_mode "Cortex M3 HCI"\
	-bt_w_size 200\
	-bt_xtal_freq 37.4\
	-bt_power_adjust 40\
    -bt_ss_location 0x0021C000 \
	-type BCM4345/B* -brand "" -version BCM4345B*Tempranillo*OS*USI*\
	-file *.cg*

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
    -brand "linux-internal-router" \
    -tag "AKASHI_BRANCH_5_110" \
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

# Specify router antenna selection to use
# set ::wlan_rtr_antsel 0x01 ;# maybe irrelevant with antswitch=0

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
    -console "mc60end15:40036" \
    -nvram {
	   	lan_ipaddr=192.168.1.2
		lan_gateway=192.168.1.2
		dhcp_start=192.168.1.200
	    dhcp_end=192.168.1.249
		lan1_ipaddr=192.168.2.2
		lan1_gateway=192.169.2.2
		dhcp1_start=192.168.2.200
	    dhcp1_end=192.168.2.249
       	fw_disable=1
       	wl_msglevel=0x101
       	wl0_ssid=TestMC59_4360
		wl0_channel=1
		w10_bw_cap=-1
# 		wl0_radio=0
		wl0_obss_coex=0
		wl1_ssid=TestMC59_4331
		wl1_channel=36
		wl1_vw_cap=-1
		wl1_radio=0
		wl1_obss_coex=0
    }\
    -datarate {-b 1.2G -i 0.5 -frameburst 1}\
    -noradio_pwrsave 1

4706/4360 clone 4360_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"
4706/4331 clone 4331_AARDVARK -tag "AARDVARK_BRANCH_6_30" -brand "linux26-internal-router"

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

4360Win7 clone 4360Win7-AARDVARK -tag AARDVARK_BRANCH_6_30 
# #    -tcpwindow 4m\
# #    -datarate {-b 350m -i 0.5 -frameburst 1 -skiptx 0x2-9x3 -skiprx 0x2-9x3} 